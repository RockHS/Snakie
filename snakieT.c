#include "raylib.h"
#include <stdint.h>
#include <stdbool.h>
#include <time.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define GRID_SIZE 20

#define GRID_COLS (SCREEN_WIDTH / GRID_SIZE)
#define GRID_ROWS (SCREEN_HEIGHT / GRID_SIZE)

// Allow the snake to grow to fill the entire board (minus the head)
#define MAX_TAIL_LENGTH (GRID_COLS * GRID_ROWS - 1)
#define INPUT_QUEUE_MAX 3
#define BASE_TICK_INTERVAL 0.1f
#define BOOSTED_TICK_INTERVAL 0.08f
#define HORSE_BOOST_SECONDS 22.0f
#define HORSE_GROWTH_AMOUNT 3
#define HORSE_COLLECTIBLE_WIDTH 2
#define HORSE_COLLECTIBLE_HEIGHT 2
#define HORSE_MILESTONE_COUNT 6
#define ROCK_MEDAL_SMASH_REQUIREMENT 7
#define ROCK_MEDAL_CHARGES 3
#define ROCK_MEDAL_COLLECTIBLE_WIDTH 1
#define ROCK_MEDAL_COLLECTIBLE_HEIGHT 1
#define MAX_ROCKS 16
#define ROCK_MAX_CELLS 8
#define ROCK_WARNING_SECONDS 2.0f
#define ROCK_MIN_FALL_DELAY_SECONDS 5.0f
#define ROCK_MAX_FALL_DELAY_SECONDS 12.0f

static const uint32_t HORSE_MILESTONES[HORSE_MILESTONE_COUNT] = {10, 25, 60, 120, 220, 360};

typedef enum {
    DIR_STOP = 0,
    DIR_LEFT,
    DIR_RIGHT,
    DIR_UP,
    DIR_DOWN
} Direction;

typedef struct {
    int32_t x;
    int32_t y;
} Position;

typedef enum {
    FRUIT_APPLE = 0,
    FRUIT_ORANGE,
    FRUIT_BANANA,
    FRUIT_CHERRY,
    FRUIT_COUNT
} FruitType;

typedef struct {
    Position pos;
    FruitType type;
    uint32_t score_value;
} Fruit;

typedef enum {
    COLLECTIBLE_NONE = 0,
    COLLECTIBLE_WHITE_HORSE,
    COLLECTIBLE_ROCK_MEDAL
} CollectibleType;

typedef struct {
    bool active;
    Position pos;
    CollectibleType type;
    uint32_t width;
    uint32_t height;
} Collectible;

typedef enum {
    ROCK_EMPTY = 0,
    ROCK_WARNING,
    ROCK_PLACED
} RockState;

typedef struct {
    RockState state;
    Position cells[ROCK_MAX_CELLS];
    uint32_t cell_count;
    float timer;
} Rock;

typedef enum {
    SCREEN_TITLE = 0,
    SCREEN_GAMEPLAY,
    SCREEN_GAMEOVER,
    SCREEN_VICTORY
} GameScreen;

typedef struct {
    Position head;
    Fruit fruit;
    Collectible collectible;
    Rock rocks[MAX_ROCKS];
    Position tail[MAX_TAIL_LENGTH];
    uint32_t tail_length;
    
    Direction current_dir;
    Direction input_queue[INPUT_QUEUE_MAX];
    uint32_t input_queue_count;
    
    GameScreen current_screen;
    uint32_t score;
    uint32_t fruits_eaten;
    uint32_t horses_collected;
    uint32_t rocks_smashed_since_medal;
    uint32_t rock_medal_charges;
    uint32_t rock_medals_collected;
    uint32_t next_horse_milestone_index;
    
    uint32_t run_seed;
    uint32_t rng_state;
    
    float update_timer;
    float time_between_ticks;
    float boost_timer;
    float rock_fall_timer;
} GameState;

typedef struct {
    Direction moves[INPUT_QUEUE_MAX];
    uint32_t count;
    bool r_pressed;
    bool enter_pressed;
} GameInput;

// Static scratch array to avoid allocating large grids on the stack
static Position g_free_cells[GRID_COLS * GRID_ROWS];

// Deterministic 32-bit Xorshift PRNG
uint32_t random_next(uint32_t *state) {
    uint32_t x = *state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    *state = x;
    return x;
}

bool is_opposite_direction(Direction d1, Direction d2) {
    if (d1 == DIR_LEFT  && d2 == DIR_RIGHT) return true;
    if (d1 == DIR_RIGHT && d2 == DIR_LEFT)  return true;
    if (d1 == DIR_UP    && d2 == DIR_DOWN)  return true;
    if (d1 == DIR_DOWN  && d2 == DIR_UP)    return true;
    return false;
}

Direction direction_between_positions(Position from, Position to) {
    if (to.x == from.x - 1 && to.y == from.y) return DIR_LEFT;
    if (to.x == from.x + 1 && to.y == from.y) return DIR_RIGHT;
    if (to.x == from.x && to.y == from.y - 1) return DIR_UP;
    if (to.x == from.x && to.y == from.y + 1) return DIR_DOWN;
    return DIR_STOP;
}

uint32_t fruit_score_value(FruitType type) {
    switch (type) {
        case FRUIT_APPLE:  return 10;
        case FRUIT_ORANGE: return 10;
        case FRUIT_BANANA: return 20;
        case FRUIT_CHERRY: return 15;
        default:           return 10;
    }
}

FruitType random_fruit_type(uint32_t *rng_state) {
    return (FruitType)(random_next(rng_state) % FRUIT_COUNT);
}

uint32_t horse_milestone_for_index(uint32_t index) {
    if (index < HORSE_MILESTONE_COUNT) {
        return HORSE_MILESTONES[index];
    }

    return HORSE_MILESTONES[HORSE_MILESTONE_COUNT - 1] + (index - HORSE_MILESTONE_COUNT + 1) * 200;
}

float game_tick_interval(const GameState *game) {
    return (game->boost_timer > 0.0f) ? BOOSTED_TICK_INTERVAL : game->time_between_ticks;
}

void game_update_boost_timer(GameState *game, float dt) {
    if (game->boost_timer > 0.0f) {
        game->boost_timer -= dt;
        if (game->boost_timer < 0.0f) {
            game->boost_timer = 0.0f;
        }
    }
}

bool positions_equal(Position a, Position b) {
    return a.x == b.x && a.y == b.y;
}

bool position_occupied_by_snake(const GameState *game, Position p) {
    if (positions_equal(game->head, p)) {
        return true;
    }

    for (uint32_t i = 0; i < game->tail_length; i++) {
        if (positions_equal(game->tail[i], p)) {
            return true;
        }
    }

    return false;
}

bool collectible_contains_position(const Collectible *collectible, Position p) {
    if (!collectible->active) {
        return false;
    }

    return p.x >= collectible->pos.x &&
           p.y >= collectible->pos.y &&
           p.x < collectible->pos.x + (int32_t)collectible->width &&
           p.y < collectible->pos.y + (int32_t)collectible->height;
}

void game_clear_collectible(GameState *game) {
    game->collectible.active = false;
    game->collectible.type = COLLECTIBLE_NONE;
    game->collectible.width = 0;
    game->collectible.height = 0;
}

void game_grow_snake(GameState *game, uint32_t amount) {
    for (uint32_t i = 0; i < amount && game->tail_length < MAX_TAIL_LENGTH; i++) {
        Position last_pos = (game->tail_length > 0) 
                            ? game->tail[game->tail_length - 1] 
                            : game->head;
        game->tail[game->tail_length] = last_pos;
        game->tail_length++;
    }
}

uint32_t rock_cell_count_from_roll(uint32_t roll) {
    if (roll < 70) return 2;
    if (roll < 95) return 4;
    return 6;
}

float random_rock_fall_delay(GameState *game) {
    uint32_t roll = random_next(&game->rng_state) % 701U;
    return ROCK_MIN_FALL_DELAY_SECONDS + (float)roll * 0.01f;
}

int game_find_active_rock(const GameState *game) {
    for (uint32_t i = 0; i < MAX_ROCKS; i++) {
        if (game->rocks[i].state != ROCK_EMPTY) {
            return (int)i;
        }
    }

    return -1;
}

int game_find_empty_rock(const GameState *game) {
    for (uint32_t i = 0; i < MAX_ROCKS; i++) {
        if (game->rocks[i].state == ROCK_EMPTY) {
            return (int)i;
        }
    }

    return -1;
}

void game_clear_rock(Rock *rock) {
    rock->state = ROCK_EMPTY;
    rock->cell_count = 0;
    rock->timer = 0.0f;
}

bool position_in_rock(const Rock *rock, Position p) {
    if (rock->state == ROCK_EMPTY) {
        return false;
    }

    for (uint32_t i = 0; i < rock->cell_count; i++) {
        if (positions_equal(rock->cells[i], p)) {
            return true;
        }
    }

    return false;
}

bool position_occupied_by_rock(const GameState *game, Position p) {
    for (uint32_t i = 0; i < MAX_ROCKS; i++) {
        if (game->rocks[i].state == ROCK_PLACED && position_in_rock(&game->rocks[i], p)) {
            return true;
        }
    }

    return false;
}

bool position_reserved_by_rock(const GameState *game, Position p) {
    for (uint32_t i = 0; i < MAX_ROCKS; i++) {
        if (position_in_rock(&game->rocks[i], p)) {
            return true;
        }
    }

    return false;
}

bool collectible_area_is_clear(const GameState *game, Position top_left, uint32_t width, uint32_t height) {
    if (top_left.x < 0 || top_left.y < 0 ||
        top_left.x + (int32_t)width > GRID_COLS ||
        top_left.y + (int32_t)height > GRID_ROWS) {
        return false;
    }

    for (uint32_t y = 0; y < height; y++) {
        for (uint32_t x = 0; x < width; x++) {
            Position p = {top_left.x + (int32_t)x, top_left.y + (int32_t)y};
            if (position_occupied_by_snake(game, p) ||
                positions_equal(game->fruit.pos, p) ||
                position_reserved_by_rock(game, p)) {
                return false;
            }
        }
    }

    return true;
}

bool game_spawn_rock_medal_collectible(GameState *game) {
    if (game->collectible.active) {
        return false;
    }

    uint32_t free_count = 0;
    for (int32_t y = 0; y <= GRID_ROWS - ROCK_MEDAL_COLLECTIBLE_HEIGHT; y++) {
        for (int32_t x = 0; x <= GRID_COLS - ROCK_MEDAL_COLLECTIBLE_WIDTH; x++) {
            Position p = {x, y};
            if (collectible_area_is_clear(game, p, ROCK_MEDAL_COLLECTIBLE_WIDTH, ROCK_MEDAL_COLLECTIBLE_HEIGHT)) {
                g_free_cells[free_count++] = p;
            }
        }
    }

    if (free_count == 0) {
        return false;
    }

    uint32_t index = random_next(&game->rng_state) % free_count;
    game->collectible.active = true;
    game->collectible.pos = g_free_cells[index];
    game->collectible.type = COLLECTIBLE_ROCK_MEDAL;
    game->collectible.width = ROCK_MEDAL_COLLECTIBLE_WIDTH;
    game->collectible.height = ROCK_MEDAL_COLLECTIBLE_HEIGHT;
    return true;
}

void game_try_spawn_rock_medal(GameState *game) {
    if (game->rock_medal_charges > 0 ||
        game->rocks_smashed_since_medal < ROCK_MEDAL_SMASH_REQUIREMENT) {
        return;
    }

    (void)game_spawn_rock_medal_collectible(game);
}

void game_record_rock_smashed(GameState *game) {
    if (game->rock_medal_charges > 0 ||
        (game->collectible.active && game->collectible.type == COLLECTIBLE_ROCK_MEDAL)) {
        return;
    }

    if (game->rocks_smashed_since_medal < ROCK_MEDAL_SMASH_REQUIREMENT) {
        game->rocks_smashed_since_medal++;
    }

    game_try_spawn_rock_medal(game);
}

bool game_spend_rock_medal_charge(GameState *game) {
    if (game->rock_medal_charges == 0) {
        return false;
    }

    game->rock_medal_charges--;
    if (game->rock_medal_charges == 0) {
        game->rocks_smashed_since_medal = 0;
    }

    return true;
}

bool position_occupied_by_any_spawn_blocker(const GameState *game, Position p) {
    if (position_occupied_by_snake(game, p)) {
        return true;
    }

    if (positions_equal(game->fruit.pos, p)) {
        return true;
    }

    if (collectible_contains_position(&game->collectible, p)) {
        return true;
    }

    return position_reserved_by_rock(game, p);
}

bool rock_cells_are_clear(const GameState *game, const Position *cells, uint32_t cell_count) {
    for (uint32_t i = 0; i < cell_count; i++) {
        if (position_occupied_by_any_spawn_blocker(game, cells[i])) {
            return false;
        }
    }

    return true;
}

bool game_spawn_rock_warning(GameState *game, uint32_t cell_count) {
    int rock_index = game_find_empty_rock(game);
    if (rock_index < 0 || cell_count == 0 || cell_count > ROCK_MAX_CELLS) {
        return false;
    }

    Position cells[ROCK_MAX_CELLS] = {0};

    for (uint32_t attempt = 0; attempt < 256; attempt++) {
        bool horizontal = (random_next(&game->rng_state) & 1U) != 0;
        int32_t shape_width = horizontal ? (int32_t)cell_count : 1;
        int32_t shape_height = horizontal ? 1 : (int32_t)cell_count;

        if (cell_count == 4) {
            shape_width = 2;
            shape_height = 2;
        } else if (cell_count == 6) {
            shape_width = horizontal ? 3 : 2;
            shape_height = horizontal ? 2 : 3;
        }

        int32_t max_x = GRID_COLS - shape_width;
        int32_t max_y = GRID_ROWS - shape_height;

        if (max_x < 0 || max_y < 0) {
            return false;
        }

        int32_t start_x = (int32_t)(random_next(&game->rng_state) % (uint32_t)(max_x + 1));
        int32_t start_y = (int32_t)(random_next(&game->rng_state) % (uint32_t)(max_y + 1));

        if (cell_count == 4 || cell_count == 6) {
            uint32_t index = 0;
            for (int32_t y = 0; y < shape_height; y++) {
                for (int32_t x = 0; x < shape_width; x++) {
                    cells[index++] = (Position){start_x + x, start_y + y};
                }
            }
        } else {
            for (uint32_t i = 0; i < cell_count; i++) {
                cells[i].x = start_x + (horizontal ? (int32_t)i : 0);
                cells[i].y = start_y + (horizontal ? 0 : (int32_t)i);
            }
        }

        if (rock_cells_are_clear(game, cells, cell_count)) {
            Rock *rock = &game->rocks[rock_index];
            rock->state = ROCK_WARNING;
            rock->cell_count = cell_count;
            rock->timer = ROCK_WARNING_SECONDS;
            for (uint32_t i = 0; i < cell_count; i++) {
                rock->cells[i] = cells[i];
            }
            return true;
        }
    }

    return false;
}

void game_update_rock_fall_spawner(GameState *game, float dt) {
    game->rock_fall_timer -= dt;
    if (game->rock_fall_timer > 0.0f) {
        return;
    }

    uint32_t roll = random_next(&game->rng_state) % 100;
    (void)game_spawn_rock_warning(game, rock_cell_count_from_roll(roll));
    game->rock_fall_timer = random_rock_fall_delay(game);
}

void game_resolve_rock_collision(GameState *game) {
    for (uint32_t rock_index = 0; rock_index < MAX_ROCKS; rock_index++) {
        Rock *rock = &game->rocks[rock_index];
        if (rock->state != ROCK_PLACED) {
            continue;
        }

        if (position_in_rock(rock, game->head)) {
            if (game->boost_timer > 0.0f) {
                game_clear_rock(rock);
                game_record_rock_smashed(game);
            } else if (game_spend_rock_medal_charge(game)) {
                game_clear_rock(rock);
            } else {
                game->current_screen = SCREEN_GAMEOVER;
            }
            return;
        }
    }
}

void game_update_rocks(GameState *game, float dt) {
    for (uint32_t rock_index = 0; rock_index < MAX_ROCKS; rock_index++) {
        Rock *rock = &game->rocks[rock_index];
        if (rock->state != ROCK_WARNING) {
            continue;
        }

        rock->timer -= dt;
        if (rock->timer <= 0.0f) {
            rock->state = ROCK_PLACED;
            rock->timer = 0.0f;
            bool landed_on_snake = false;
            for (uint32_t i = 0; i < rock->cell_count; i++) {
                if (position_occupied_by_snake(game, rock->cells[i])) {
                    landed_on_snake = true;
                    break;
                }
            }

            if (landed_on_snake) {
                if (game->boost_timer > 0.0f) {
                    game_clear_rock(rock);
                    game_record_rock_smashed(game);
                } else if (game_spend_rock_medal_charge(game)) {
                    game_clear_rock(rock);
                } else {
                    game->current_screen = SCREEN_GAMEOVER;
                    return;
                }
            }
        }
    }

    game_resolve_rock_collision(game);
    if (game->current_screen == SCREEN_GAMEOVER || game->current_screen == SCREEN_VICTORY) {
        return;
    }

    game_update_rock_fall_spawner(game, dt);
}

void queue_push(GameState *game, Direction dir) {
    Direction last_dir = (game->input_queue_count > 0) 
                         ? game->input_queue[game->input_queue_count - 1] 
                         : game->current_dir;
    
    if (dir == last_dir || is_opposite_direction(dir, last_dir)) {
        return;
    }
    
    if (game->input_queue_count < INPUT_QUEUE_MAX) {
        game->input_queue[game->input_queue_count] = dir;
        game->input_queue_count++;
    }
}

// Scans grid to list all unoccupied coordinates
void game_spawn_fruit(GameState *game) {
    uint32_t free_count = 0;

    for (int32_t y = 0; y < GRID_ROWS; y++) {
        for (int32_t x = 0; x < GRID_COLS; x++) {
            bool occupied = false;
            if (x == game->head.x && y == game->head.y) {
                occupied = true;
            } else if (collectible_contains_position(&game->collectible, (Position){x, y})) {
                occupied = true;
            } else if (position_reserved_by_rock(game, (Position){x, y})) {
                occupied = true;
            } else {
                for (uint32_t i = 0; i < game->tail_length; i++) {
                    if (game->tail[i].x == x && game->tail[i].y == y) {
                        occupied = true;
                        break;
                    }
                }
            }
            if (!occupied) {
                g_free_cells[free_count++] = (Position){x, y};
            }
        }
    }

    if (free_count > 0) {
        uint32_t index = random_next(&game->rng_state) % free_count;
        FruitType type = random_fruit_type(&game->rng_state);
        game->fruit.pos = g_free_cells[index];
        game->fruit.type = type;
        game->fruit.score_value = fruit_score_value(type);
    } else {
        game->current_screen = SCREEN_VICTORY;
    }
}

void game_spawn_collectible(GameState *game) {
    uint32_t free_count = 0;

    for (int32_t y = 0; y <= GRID_ROWS - HORSE_COLLECTIBLE_HEIGHT; y++) {
        for (int32_t x = 0; x <= GRID_COLS - HORSE_COLLECTIBLE_WIDTH; x++) {
            Position p = {x, y};
            if (collectible_area_is_clear(game, p, HORSE_COLLECTIBLE_WIDTH, HORSE_COLLECTIBLE_HEIGHT)) {
                g_free_cells[free_count++] = p;
            }
        }
    }

    if (free_count > 0) {
        uint32_t index = random_next(&game->rng_state) % free_count;
        game->collectible.active = true;
        game->collectible.pos = g_free_cells[index];
        game->collectible.type = COLLECTIBLE_WHITE_HORSE;
        game->collectible.width = HORSE_COLLECTIBLE_WIDTH;
        game->collectible.height = HORSE_COLLECTIBLE_HEIGHT;
    } else {
        game_clear_collectible(game);
    }
}

void game_note_fruit_eaten(GameState *game) {
    game->fruits_eaten++;

    if (!game->collectible.active &&
        game->fruits_eaten >= horse_milestone_for_index(game->next_horse_milestone_index)) {
        game_spawn_collectible(game);
        game->next_horse_milestone_index++;
    }
}

void game_collect_horse(GameState *game) {
    if (!game->collectible.active || game->collectible.type != COLLECTIBLE_WHITE_HORSE) {
        return;
    }

    game_clear_collectible(game);
    game->horses_collected++;
    game_grow_snake(game, HORSE_GROWTH_AMOUNT);
    game->boost_timer = HORSE_BOOST_SECONDS;
    game_try_spawn_rock_medal(game);
}

void game_collect_rock_medal(GameState *game) {
    if (!game->collectible.active || game->collectible.type != COLLECTIBLE_ROCK_MEDAL) {
        return;
    }

    game_clear_collectible(game);
    game->rock_medals_collected++;
    game->rock_medal_charges = ROCK_MEDAL_CHARGES;
    game->rocks_smashed_since_medal = 0;
}

void game_init(GameState *game, uint32_t seed) {
    game->run_seed = seed;
    game->rng_state = seed;
    if (game->rng_state == 0) {
        game->rng_state = 5489U; // Xorshift seed must not be zero
    }
    
    game->current_screen = SCREEN_TITLE;
    game->current_dir = DIR_STOP;
    game->input_queue_count = 0;
    
    game->head.x = GRID_COLS / 2;
    game->head.y = GRID_ROWS / 2;
    
    game->tail_length = 0;
    game->score = 0;
    game->fruits_eaten = 0;
    game->horses_collected = 0;
    game->rocks_smashed_since_medal = 0;
    game->rock_medal_charges = 0;
    game->rock_medals_collected = 0;
    game->next_horse_milestone_index = 0;
    game_clear_collectible(game);
    for (uint32_t i = 0; i < MAX_ROCKS; i++) {
        game_clear_rock(&game->rocks[i]);
    }
    game->update_timer = 0.0f;
    game->time_between_ticks = BASE_TICK_INTERVAL;
    game->boost_timer = 0.0f;
    game->rock_fall_timer = random_rock_fall_delay(game);
    
    game_spawn_fruit(game);
}

void game_process_input(GameState *game, const GameInput *input) {
    switch (game->current_screen) {
        case SCREEN_TITLE:
            if (input->enter_pressed) {
                game->current_screen = SCREEN_GAMEPLAY;
            }
            break;
            
        case SCREEN_GAMEPLAY:
            for (uint32_t i = 0; i < input->count; i++) {
                queue_push(game, input->moves[i]);
            }
            break;
            
        case SCREEN_GAMEOVER:
        case SCREEN_VICTORY:
            if (input->r_pressed) {
                // Advance the seed line deterministically using an LCG constant
                uint32_t next_seed = game->run_seed + 1013904223U;
                game_init(game, next_seed);
                game->current_screen = SCREEN_GAMEPLAY;
            }
            break;
    }
}

void game_tick(GameState *game) {
    if (game->input_queue_count > 0) {
        Direction next = game->input_queue[0];
        for (uint32_t i = 1; i < game->input_queue_count; i++) {
            game->input_queue[i - 1] = game->input_queue[i];
        }
        game->input_queue_count--;
        game->current_dir = next;
    }

    if (game->current_dir == DIR_STOP) return;

    if (game->tail_length > 0) {
        uint32_t limit = game->tail_length;
        if (limit >= MAX_TAIL_LENGTH) {
            limit = MAX_TAIL_LENGTH - 1;
        }
        for (uint32_t i = limit; i > 0; i--) {
            game->tail[i] = game->tail[i - 1];
        }
        game->tail[0] = game->head;
    }

    switch (game->current_dir) {
        case DIR_LEFT:  game->head.x--; break;
        case DIR_RIGHT: game->head.x++; break;
        case DIR_UP:    game->head.y--; break;
        case DIR_DOWN:  game->head.y++; break;
        default: break;
    }

    if (game->head.x < 0 || game->head.x >= GRID_COLS || 
        game->head.y < 0 || game->head.y >= GRID_ROWS) {
        game->current_screen = SCREEN_GAMEOVER;
        return;
    }

    for (uint32_t i = 0; i < game->tail_length; i++) {
        if (game->tail[i].x == game->head.x && game->tail[i].y == game->head.y) {
            game->current_screen = SCREEN_GAMEOVER;
            return;
        }
    }

    game_resolve_rock_collision(game);
    if (game->current_screen != SCREEN_GAMEPLAY) {
        return;
    }

    if (collectible_contains_position(&game->collectible, game->head)) {
        switch (game->collectible.type) {
            case COLLECTIBLE_WHITE_HORSE:
                game_collect_horse(game);
                break;
            case COLLECTIBLE_ROCK_MEDAL:
                game_collect_rock_medal(game);
                break;
            default:
                break;
        }
    }

    if (game->head.x == game->fruit.pos.x && game->head.y == game->fruit.pos.y) {
        game->score += game->fruit.score_value;
        game_grow_snake(game, 1);
        game_note_fruit_eaten(game);
        
        game_spawn_fruit(game);
    }
}

void draw_apple(Position p) {
    int x = p.x * GRID_SIZE;
    int y = p.y * GRID_SIZE;

    DrawCircle(x + 10, y + 11, 7, RED);
    DrawCircle(x + 7, y + 10, 5, MAROON);
    DrawRectangle(x + 10, y + 3, 2, 5, BROWN);
    DrawEllipse(x + 14, y + 5, 4, 2, GREEN);
}

void draw_orange(Position p) {
    int x = p.x * GRID_SIZE;
    int y = p.y * GRID_SIZE;

    DrawCircle(x + 10, y + 11, 7, ORANGE);
    DrawCircleLines(x + 10, y + 11, 7, GOLD);
    DrawCircle(x + 8, y + 8, 2, YELLOW);
    DrawRectangle(x + 10, y + 4, 2, 3, BROWN);
    DrawEllipse(x + 14, y + 5, 4, 2, LIME);
}

void draw_banana(Position p) {
    int x = p.x * GRID_SIZE;
    int y = p.y * GRID_SIZE;

    DrawEllipse(x + 10, y + 11, 8, 5, GOLD);
    DrawEllipse(x + 10, y + 8, 7, 4, DARKGRAY);
    DrawCircle(x + 5, y + 11, 2, BROWN);
    DrawCircle(x + 16, y + 13, 2, BROWN);
    DrawLine(x + 7, y + 14, x + 15, y + 15, YELLOW);
}

void draw_cherry(Position p) {
    int x = p.x * GRID_SIZE;
    int y = p.y * GRID_SIZE;

    DrawLine(x + 10, y + 5, x + 7, y + 10, GREEN);
    DrawLine(x + 10, y + 5, x + 13, y + 10, GREEN);
    DrawCircle(x + 7, y + 13, 5, RED);
    DrawCircle(x + 13, y + 13, 5, MAROON);
    DrawCircle(x + 6, y + 11, 1, PINK);
    DrawCircle(x + 12, y + 11, 1, PINK);
}

void draw_fruit(const Fruit *fruit) {
    switch (fruit->type) {
        case FRUIT_APPLE:  draw_apple(fruit->pos); break;
        case FRUIT_ORANGE: draw_orange(fruit->pos); break;
        case FRUIT_BANANA: draw_banana(fruit->pos); break;
        case FRUIT_CHERRY: draw_cherry(fruit->pos); break;
        default:           draw_apple(fruit->pos); break;
    }
}

void draw_white_horse(Position p) {
    int x = p.x * GRID_SIZE;
    int y = p.y * GRID_SIZE;

    Color glow = Fade(SKYBLUE, 0.75f);
    Color shade = (Color){210, 215, 220, 255};
    Color hoof = (Color){95, 90, 85, 255};
    Color mane = (Color){92, 58, 35, 255};

    DrawEllipse(x + 20, y + 36, 16, 2, Fade(BLACK, 0.24f));

    // Soft outline first, then the actual readable silhouette.
    DrawLineEx((Vector2){x + 7, y + 20}, (Vector2){x + 3, y + 14}, 5.0f, glow);
    DrawLineEx((Vector2){x + 7, y + 21}, (Vector2){x + 3, y + 24}, 5.0f, glow);
    DrawLineEx((Vector2){x + 11, y + 26}, (Vector2){x + 9, y + 37}, 6.0f, glow);
    DrawLineEx((Vector2){x + 24, y + 26}, (Vector2){x + 27, y + 37}, 6.0f, glow);
    DrawEllipse(x + 17, y + 22, 15, 9, glow);
    DrawTriangle((Vector2){x + 24, y + 15}, (Vector2){x + 31, y + 15}, (Vector2){x + 25, y + 28}, glow);
    DrawEllipse(x + 31, y + 13, 8, 6, glow);
    DrawEllipse(x + 35, y + 16, 4, 4, glow);

    DrawLineEx((Vector2){x + 7, y + 20}, (Vector2){x + 3, y + 14}, 3.0f, RAYWHITE);
    DrawLineEx((Vector2){x + 7, y + 21}, (Vector2){x + 3, y + 24}, 3.0f, RAYWHITE);

    DrawLineEx((Vector2){x + 11, y + 26}, (Vector2){x + 9, y + 37}, 4.0f, shade);
    DrawLineEx((Vector2){x + 21, y + 26}, (Vector2){x + 19, y + 37}, 4.0f, shade);
    DrawLineEx((Vector2){x + 15, y + 26}, (Vector2){x + 16, y + 37}, 4.0f, RAYWHITE);
    DrawLineEx((Vector2){x + 25, y + 26}, (Vector2){x + 28, y + 37}, 4.0f, RAYWHITE);
    DrawRectangle(x + 7, y + 35, 6, 3, hoof);
    DrawRectangle(x + 14, y + 35, 6, 3, hoof);
    DrawRectangle(x + 25, y + 35, 7, 3, hoof);

    DrawEllipse(x + 17, y + 22, 14, 8, RAYWHITE);
    DrawRectangle(x + 8, y + 18, 18, 8, RAYWHITE);
    DrawCircle(x + 8, y + 22, 5, RAYWHITE);
    DrawLine(x + 8, y + 28, x + 27, y + 28, shade);

    DrawTriangle((Vector2){x + 24, y + 14}, (Vector2){x + 31, y + 15}, (Vector2){x + 25, y + 28}, RAYWHITE);
    DrawLineEx((Vector2){x + 27, y + 13}, (Vector2){x + 25, y + 27}, 3.0f, mane);

    DrawEllipse(x + 31, y + 13, 7, 6, RAYWHITE);
    DrawEllipse(x + 35, y + 16, 4, 4, RAYWHITE);
    DrawTriangle((Vector2){x + 29, y + 8}, (Vector2){x + 32, y + 8}, (Vector2){x + 30, y + 2}, RAYWHITE);
    DrawTriangle((Vector2){x + 34, y + 8}, (Vector2){x + 37, y + 9}, (Vector2){x + 36, y + 3}, RAYWHITE);

    DrawCircle(x + 33, y + 12, 1, BLACK);
    DrawLine(x + 36, y + 17, x + 38, y + 17, shade);
    DrawCircle(x + 15, y + 18, 2, WHITE);
}

void draw_rock_medal(Position p) {
    int x = p.x * GRID_SIZE;
    int y = p.y * GRID_SIZE;
    int cx = x + GRID_SIZE / 2;
    int cy = y + GRID_SIZE / 2;

    DrawCircle(cx + 1, cy + 2, 9, Fade(BLACK, 0.35f));
    DrawCircle(cx, cy, 9, GOLD);
    DrawCircle(cx, cy, 7, ORANGE);
    DrawCircle(cx - 2, cy - 2, 3, YELLOW);

    DrawTriangle((Vector2){cx - 5, cy - 2},
                 (Vector2){cx + 5, cy - 2},
                 (Vector2){cx, cy + 7},
                 DARKGRAY);
    DrawCircle(cx - 3, cy - 1, 4, GRAY);
    DrawCircle(cx + 3, cy - 1, 4, GRAY);
    DrawLine(cx - 2, cy - 4, cx + 1, cy - 1, LIGHTGRAY);
    DrawLine(cx + 1, cy - 1, cx - 1, cy + 4, SKYBLUE);
    DrawCircleLines(cx, cy, 9, RAYWHITE);
}

void draw_collectible(const Collectible *collectible) {
    if (!collectible->active) {
        return;
    }

    switch (collectible->type) {
        case COLLECTIBLE_WHITE_HORSE: draw_white_horse(collectible->pos); break;
        case COLLECTIBLE_ROCK_MEDAL:  draw_rock_medal(collectible->pos); break;
        default: break;
    }
}

void draw_rock_warning_cell(Position p, float timer) {
    int x = p.x * GRID_SIZE;
    int y = p.y * GRID_SIZE;
    float progress = 1.0f - (timer / ROCK_WARNING_SECONDS);
    if (progress < 0.0f) progress = 0.0f;
    if (progress > 1.0f) progress = 1.0f;

    int cx = x + GRID_SIZE / 2;
    int cy = y + GRID_SIZE / 2;
    int fall_y = y - 18 + (int)(progress * 18.0f);

    DrawEllipse(cx, cy + 1, 8, 5, Fade(BLACK, 0.38f + 0.22f * progress));
    DrawLine(x + 5, y + 9, x + 10, y + 6, Fade(LIGHTGRAY, 0.80f));
    DrawLine(x + 10, y + 6, x + 15, y + 11, Fade(LIGHTGRAY, 0.80f));
    DrawCircle(cx, fall_y, 4, GRAY);
    DrawCircle(cx - 2, fall_y - 1, 2, DARKGRAY);
}

void draw_placed_rock_cell(Position p) {
    int x = p.x * GRID_SIZE;
    int y = p.y * GRID_SIZE;

    DrawCircle(x + 10, y + 11, 8, DARKGRAY);
    DrawCircle(x + 6, y + 14, 5, GRAY);
    DrawCircle(x + 14, y + 14, 5, GRAY);
    DrawTriangle((Vector2){x + 4, y + 8}, (Vector2){x + 10, y + 2}, (Vector2){x + 16, y + 9}, DARKGRAY);
    DrawLine(x + 6, y + 10, x + 11, y + 7, LIGHTGRAY);
    DrawLine(x + 11, y + 7, x + 15, y + 12, LIGHTGRAY);
}

void draw_big_rock_rect(Position top_left, int32_t cell_width, int32_t cell_height) {
    int x = top_left.x * GRID_SIZE;
    int y = top_left.y * GRID_SIZE;
    int w = cell_width * GRID_SIZE;
    int h = cell_height * GRID_SIZE;
    int cx = x + w / 2;
    int cy = y + h / 2;

    DrawEllipse(cx + 1, y + h - 8, w / 2 - 3, h / 5, Fade(BLACK, 0.32f));
    DrawEllipse(cx, cy + h / 10, w / 2 - 2, h / 2 - 4, DARKGRAY);
    DrawCircle(x + w / 3, y + h / 2, h / 3, GRAY);
    DrawCircle(x + (w * 2) / 3, y + h / 2 - 3, h / 3 + 1, GRAY);
    DrawCircle(cx, y + (h * 2) / 3, h / 4, DARKGRAY);
    DrawTriangle((Vector2){x + 6, y + h / 2},
                 (Vector2){cx, y + 4},
                 (Vector2){x + w - 6, y + h / 2 + 1},
                 GRAY);
    DrawLine(x + w / 4, y + h / 2, cx, y + h / 3, LIGHTGRAY);
    DrawLine(cx, y + h / 3, x + (w * 3) / 4, y + (h * 2) / 3, LIGHTGRAY);
    DrawLine(x + w / 3, y + h - 12, x + (w * 2) / 3, y + h - 7, Fade(BLACK, 0.35f));
}

bool rock_rectangle_dimensions(const Rock *rock, int32_t *width, int32_t *height) {
    if (rock->cell_count != 4 && rock->cell_count != 6) {
        return false;
    }

    Position top_left = rock->cells[0];
    int32_t candidate_width = 2;
    int32_t candidate_height = (int32_t)rock->cell_count / candidate_width;

    if (rock->cell_count == 6 &&
        positions_equal(rock->cells[2], (Position){top_left.x + 2, top_left.y})) {
        candidate_width = 3;
        candidate_height = 2;
    }

    uint32_t index = 0;
    for (int32_t y = 0; y < candidate_height; y++) {
        for (int32_t x = 0; x < candidate_width; x++) {
            if (!positions_equal(rock->cells[index++], (Position){top_left.x + x, top_left.y + y})) {
                return false;
            }
        }
    }

    *width = candidate_width;
    *height = candidate_height;
    return true;
}

void draw_rocks(const GameState *game) {
    for (uint32_t rock_index = 0; rock_index < MAX_ROCKS; rock_index++) {
        const Rock *rock = &game->rocks[rock_index];
        if (rock->state == ROCK_EMPTY) {
            continue;
        }

        int32_t rock_width = 0;
        int32_t rock_height = 0;
        if (rock->state == ROCK_PLACED &&
            rock_rectangle_dimensions(rock, &rock_width, &rock_height)) {
            draw_big_rock_rect(rock->cells[0], rock_width, rock_height);
            continue;
        }

        for (uint32_t i = 0; i < rock->cell_count; i++) {
            if (rock->state == ROCK_WARNING) {
                draw_rock_warning_cell(rock->cells[i], rock->timer);
            } else {
                draw_placed_rock_cell(rock->cells[i]);
            }
        }
    }
}

Color boosted_snake_color(uint32_t offset) {
    uint32_t hue = ((uint32_t)(GetTime() * 240.0) + offset) % 360;
    return ColorFromHSV((float)hue, 0.78f, 1.0f);
}

Color snake_body_color(const GameState *game) {
    return (game->boost_timer > 0.0f) ? boosted_snake_color(0) : LIME;
}

Color snake_head_color(const GameState *game) {
    return (game->boost_timer > 0.0f) ? boosted_snake_color(45) : GREEN;
}

void draw_snake_electric_mark(Position p, uint32_t index) {
    int x = p.x * GRID_SIZE;
    int y = p.y * GRID_SIZE;
    Color spark = boosted_snake_color(index * 37 + 120);

    if ((index & 1) == 0) {
        DrawLine(x + 4, y + 6, x + 10, y + 3, spark);
        DrawLine(x + 10, y + 3, x + 16, y + 8, RAYWHITE);
    } else {
        DrawLine(x + 4, y + 14, x + 10, y + 17, spark);
        DrawLine(x + 10, y + 17, x + 16, y + 12, RAYWHITE);
    }
}

void draw_snake_connection(Position a, Position b, Color color) {
    int ax = a.x * GRID_SIZE + GRID_SIZE / 2;
    int ay = a.y * GRID_SIZE + GRID_SIZE / 2;
    int bx = b.x * GRID_SIZE + GRID_SIZE / 2;
    int by = b.y * GRID_SIZE + GRID_SIZE / 2;
    int radius = GRID_SIZE / 2 - 3;
    Direction connection = direction_between_positions(a, b);

    if (connection == DIR_LEFT || connection == DIR_RIGHT) {
        int left = (ax < bx) ? ax : bx;
        DrawRectangle(left, ay - radius, GRID_SIZE, radius * 2, color);
    } else if (connection == DIR_UP || connection == DIR_DOWN) {
        int top = (ay < by) ? ay : by;
        DrawRectangle(ax - radius, top, radius * 2, GRID_SIZE, color);
    }
}

void draw_snake_body_cell(Position p, Color color) {
    int x = p.x * GRID_SIZE + GRID_SIZE / 2;
    int y = p.y * GRID_SIZE + GRID_SIZE / 2;
    DrawCircle(x, y, GRID_SIZE / 2 - 3, color);
    DrawCircle(x - 3, y - 3, 2, Fade(WHITE, 0.20f));
}

void draw_snake_tail_tip(Position p, Direction toward_body, Color color) {
    int x = p.x * GRID_SIZE;
    int y = p.y * GRID_SIZE;
    Vector2 a;
    Vector2 b;
    Vector2 c;

    switch (toward_body) {
        case DIR_LEFT:
            a = (Vector2){x + 16, y + 4};
            b = (Vector2){x + 16, y + 16};
            c = (Vector2){x + 4, y + 10};
            break;
        case DIR_RIGHT:
            a = (Vector2){x + 4, y + 4};
            b = (Vector2){x + 4, y + 16};
            c = (Vector2){x + 16, y + 10};
            break;
        case DIR_UP:
            a = (Vector2){x + 4, y + 16};
            b = (Vector2){x + 16, y + 16};
            c = (Vector2){x + 10, y + 4};
            break;
        case DIR_DOWN:
            a = (Vector2){x + 4, y + 4};
            b = (Vector2){x + 16, y + 4};
            c = (Vector2){x + 10, y + 16};
            break;
        default:
            DrawCircle(x + 10, y + 10, GRID_SIZE / 2 - 5, color);
            return;
    }

    DrawTriangle(a, b, c, color);
}

void draw_snake_head(Position p, Direction dir, Color color, bool boosted) {
    int x = p.x * GRID_SIZE;
    int y = p.y * GRID_SIZE;
    int cx = x + GRID_SIZE / 2;
    int cy = y + GRID_SIZE / 2;

    if (dir == DIR_LEFT || dir == DIR_RIGHT) {
        DrawEllipse(cx, cy, 10, 8, color);
    } else {
        DrawEllipse(cx, cy, 8, 10, color);
    }

    if (boosted) {
        DrawCircleLines(cx, cy, 10, boosted_snake_color(180));
        DrawCircleLines(cx, cy, 8, RAYWHITE);
    }

    int eye1_x = cx;
    int eye1_y = cy;
    int eye2_x = cx;
    int eye2_y = cy;
    int tongue_x = cx;
    int tongue_y = cy;

    switch (dir) {
        case DIR_LEFT:
            eye1_x = cx - 4; eye1_y = cy - 4;
            eye2_x = cx - 4; eye2_y = cy + 4;
            tongue_x = x + 1; tongue_y = cy;
            DrawLine(x + 3, cy, x - 3, cy - 3, RED);
            DrawLine(x + 3, cy, x - 3, cy + 3, RED);
            break;
        case DIR_RIGHT:
            eye1_x = cx + 4; eye1_y = cy - 4;
            eye2_x = cx + 4; eye2_y = cy + 4;
            tongue_x = x + GRID_SIZE - 1; tongue_y = cy;
            DrawLine(x + GRID_SIZE - 3, cy, x + GRID_SIZE + 3, cy - 3, RED);
            DrawLine(x + GRID_SIZE - 3, cy, x + GRID_SIZE + 3, cy + 3, RED);
            break;
        case DIR_UP:
            eye1_x = cx - 4; eye1_y = cy - 4;
            eye2_x = cx + 4; eye2_y = cy - 4;
            tongue_x = cx; tongue_y = y + 1;
            DrawLine(cx, y + 3, cx - 3, y - 3, RED);
            DrawLine(cx, y + 3, cx + 3, y - 3, RED);
            break;
        case DIR_DOWN:
            eye1_x = cx - 4; eye1_y = cy + 4;
            eye2_x = cx + 4; eye2_y = cy + 4;
            tongue_x = cx; tongue_y = y + GRID_SIZE - 1;
            DrawLine(cx, y + GRID_SIZE - 3, cx - 3, y + GRID_SIZE + 3, RED);
            DrawLine(cx, y + GRID_SIZE - 3, cx + 3, y + GRID_SIZE + 3, RED);
            break;
        default:
            eye1_x = cx - 4; eye1_y = cy - 4;
            eye2_x = cx + 4; eye2_y = cy - 4;
            break;
    }

    (void)tongue_x;
    (void)tongue_y;
    DrawCircle(eye1_x, eye1_y, 2, BLACK);
    DrawCircle(eye2_x, eye2_y, 2, BLACK);
    DrawCircle(eye1_x + 1, eye1_y - 1, 1, WHITE);
    DrawCircle(eye2_x + 1, eye2_y - 1, 1, WHITE);
}

void draw_snake_rock_shield(Position head, Direction dir, uint32_t charges) {
    if (charges == 0) {
        return;
    }

    int dx = 0;
    int dy = -1;
    switch (dir) {
        case DIR_LEFT:  dx = -1; dy = 0; break;
        case DIR_RIGHT: dx = 1;  dy = 0; break;
        case DIR_UP:    dx = 0;  dy = -1; break;
        case DIR_DOWN:  dx = 0;  dy = 1; break;
        default: break;
    }

    int cx = head.x * GRID_SIZE + GRID_SIZE / 2 + dx * 8;
    int cy = head.y * GRID_SIZE + GRID_SIZE / 2 + dy * 8;

    DrawCircle(cx + 1, cy + 1, 7, Fade(BLACK, 0.35f));
    DrawCircle(cx, cy, 7, DARKGRAY);
    DrawCircle(cx - 3, cy - 2, 3, GRAY);
    DrawCircle(cx + 3, cy + 1, 3, GRAY);
    DrawCircleLines(cx, cy, 7, GOLD);
    DrawLine(cx - 3, cy - 4, cx + 1, cy - 1, LIGHTGRAY);
    DrawLine(cx + 1, cy - 1, cx - 2, cy + 4, SKYBLUE);

    uint32_t visible_charges = charges > ROCK_MEDAL_CHARGES ? ROCK_MEDAL_CHARGES : charges;
    for (uint32_t i = 0; i < visible_charges; i++) {
        DrawCircle(cx - 4 + (int)i * 4, cy + 8, 1, GOLD);
    }
}

void draw_snake(const GameState *game) {
    bool boosted = game->boost_timer > 0.0f;
    Color body_color = snake_body_color(game);
    Color head_color = snake_head_color(game);
    Color shadow_color = Fade(BLACK, 0.20f);

    if (game->tail_length > 0) {
        draw_snake_connection(game->head, game->tail[0], body_color);
        for (uint32_t i = 0; i + 1 < game->tail_length; i++) {
            draw_snake_connection(game->tail[i], game->tail[i + 1], body_color);
        }

        for (uint32_t i = game->tail_length; i > 0; i--) {
            Position segment = game->tail[i - 1];
            DrawCircle(segment.x * GRID_SIZE + GRID_SIZE / 2 + 1,
                       segment.y * GRID_SIZE + GRID_SIZE / 2 + 1,
                       GRID_SIZE / 2 - 3, shadow_color);
            draw_snake_body_cell(segment, body_color);
            if (boosted) {
                draw_snake_electric_mark(segment, i - 1);
            }
        }

        Position tail_end = game->tail[game->tail_length - 1];
        Position toward_body = (game->tail_length > 1) ? game->tail[game->tail_length - 2] : game->head;
        draw_snake_tail_tip(tail_end, direction_between_positions(tail_end, toward_body), body_color);
    }

    draw_snake_head(game->head, game->current_dir, head_color, boosted);
    draw_snake_rock_shield(game->head, game->current_dir, game->rock_medal_charges);
}

void game_update(GameState *game, float dt) {
    if (game->current_screen != SCREEN_GAMEPLAY) return;

    if (dt > 0.25f) {
        dt = 0.25f;
    }

    game_update_boost_timer(game, dt);
    game_update_rocks(game, dt);
    if (game->current_screen != SCREEN_GAMEPLAY) {
        return;
    }

    game->update_timer += dt;
    while (game->update_timer >= game_tick_interval(game)) {
        game->update_timer -= game_tick_interval(game);
        game_tick(game);
        
        if (game->current_screen != SCREEN_GAMEPLAY) {
            break;
        }
    }
}

void game_draw(const GameState *game) {
    ClearBackground(DARKGRAY);

    switch (game->current_screen) {
        case SCREEN_TITLE: {
            const char *title = "Snake get horse";
            const char *sub1 = "Press ENTER to Start";
            const char *sub2 = "Use Arrow Keys to Move";
            const char *tip1 = "avoid falling rocks";
            const char *tip2 = "collect fruit";
            const char *tip3 = "horse collectable activates snake boost";
            const char *tip4 = "(Boost can break rocks while active)";
            
            int t_w = MeasureText(title, 40);
            int s1_w = MeasureText(sub1, 20);
            int s2_w = MeasureText(sub2, 20);
            int tip1_w = MeasureText(tip1, 18);
            int tip2_w = MeasureText(tip2, 18);
            int tip3_w = MeasureText(tip3, 18);
            int tip4_w = MeasureText(tip4, 18);
            int box_w = 500;
            int box_h = 126;
            int box_x = SCREEN_WIDTH / 2 - box_w / 2;
            int box_y = SCREEN_HEIGHT / 2 - 5;
            
            DrawText(title, SCREEN_WIDTH/2 - t_w/2, SCREEN_HEIGHT/2 - 115, 40, GREEN);

            DrawRectangle(box_x + 3, box_y + 3, box_w, box_h, Fade(BLACK, 0.28f));
            DrawRectangle(box_x, box_y, box_w, box_h, Fade(BLACK, 0.30f));
            DrawRectangleLines(box_x, box_y, box_w, box_h, Fade(LIME, 0.70f));
            DrawText(tip1, SCREEN_WIDTH/2 - tip1_w/2, box_y + 18, 18, RAYWHITE);
            DrawText(tip2, SCREEN_WIDTH/2 - tip2_w/2, box_y + 43, 18, RAYWHITE);
            DrawText(tip3, SCREEN_WIDTH/2 - tip3_w/2, box_y + 68, 18, RAYWHITE);
            DrawText(tip4, SCREEN_WIDTH/2 - tip4_w/2, box_y + 93, 18, LIGHTGRAY);

            DrawText(sub1, SCREEN_WIDTH/2 - s1_w/2, SCREEN_HEIGHT/2 + 155, 20, WHITE);
            DrawText(sub2, SCREEN_WIDTH/2 - s2_w/2, SCREEN_HEIGHT/2 + 185, 20, LIGHTGRAY);
            break;
        }
            
        case SCREEN_GAMEPLAY:
            for (int i = 0; i < GRID_COLS; i++) {
                DrawLine(i * GRID_SIZE, 0, i * GRID_SIZE, SCREEN_HEIGHT, GRAY);
            }
            for (int i = 0; i < GRID_ROWS; i++) {
                DrawLine(0, i * GRID_SIZE, SCREEN_WIDTH, i * GRID_SIZE, GRAY);
            }

            draw_rocks(game);

            draw_fruit(&game->fruit);
            draw_collectible(&game->collectible);

            draw_snake(game);

            DrawText(TextFormat("Score: %u  Horses: %u  Shield: %u",
                                game->score,
                                game->horses_collected,
                                game->rock_medal_charges),
                     15, 15, 20, WHITE);
            break;
            
        case SCREEN_GAMEOVER: {
            const char *title = "GAME OVER";
            const char *sub1 = TextFormat("Final Score: %u", game->score);
            const char *sub2 = "Press R to Restart";
            
            int t_w = MeasureText(title, 40);
            int s1_w = MeasureText(sub1, 20);
            int s2_w = MeasureText(sub2, 20);
            
            DrawText(title, SCREEN_WIDTH/2 - t_w/2, SCREEN_HEIGHT/2 - 60, 40, RED);
            DrawText(sub1, SCREEN_WIDTH/2 - s1_w/2, SCREEN_HEIGHT/2 + 10, 20, WHITE);
            DrawText(sub2, SCREEN_WIDTH/2 - s2_w/2, SCREEN_HEIGHT/2 + 40, 20, LIGHTGRAY);
            break;
        }

        case SCREEN_VICTORY: {
            const char *title = "VICTORY!";
            const char *sub1 = TextFormat("Final Score: %u", game->score);
            const char *sub2 = "Press R to Play Again";
            
            int t_w = MeasureText(title, 40);
            int s1_w = MeasureText(sub1, 20);
            int s2_w = MeasureText(sub2, 20);
            
            DrawText(title, SCREEN_WIDTH/2 - t_w/2, SCREEN_HEIGHT/2 - 60, 40, GOLD);
            DrawText(sub1, SCREEN_WIDTH/2 - s1_w/2, SCREEN_HEIGHT/2 + 10, 20, WHITE);
            DrawText(sub2, SCREEN_WIDTH/2 - s2_w/2, SCREEN_HEIGHT/2 + 40, 20, LIGHTGRAY);
            break;
        }
    }
}

int main(void) {
    uint32_t initial_seed = (uint32_t)time(NULL);

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "RH Snake (Handmade Architecture)");
    SetTargetFPS(60);

    GameState game = {0};
    game_init(&game, initial_seed);
    TraceLog(LOG_INFO, "Initialized game with seed: %u", game.run_seed);

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();

        // 1. Gather input from Raylib's key queue
        GameInput input = {0};
        int key;
        while ((key = GetKeyPressed()) != 0) {
            switch (key) {
                case KEY_LEFT:  if (input.count < INPUT_QUEUE_MAX) input.moves[input.count++] = DIR_LEFT; break;
                case KEY_RIGHT: if (input.count < INPUT_QUEUE_MAX) input.moves[input.count++] = DIR_RIGHT; break;
                case KEY_UP:    if (input.count < INPUT_QUEUE_MAX) input.moves[input.count++] = DIR_UP; break;
                case KEY_DOWN:  if (input.count < INPUT_QUEUE_MAX) input.moves[input.count++] = DIR_DOWN; break;
                case KEY_R:     input.r_pressed = true; break;
                case KEY_ENTER: input.enter_pressed = true; break;
            }
        }

        // 2. Track screen transitions for platform logging
        GameScreen prev_screen = game.current_screen;

        // 3. Process simulation
        game_process_input(&game, &input);
        game_update(&game, dt);

        // 4. Log transitions at the platform level
        if (prev_screen == SCREEN_TITLE && game.current_screen == SCREEN_GAMEPLAY) {
            TraceLog(LOG_INFO, "Starting gameplay run with seed: %u", game.run_seed);
        } else if ((prev_screen == SCREEN_GAMEOVER || prev_screen == SCREEN_VICTORY) && game.current_screen == SCREEN_GAMEPLAY) {
            TraceLog(LOG_INFO, "Restarted gameplay run with seed: %u", game.run_seed);
        }

        // 5. Draw
        BeginDrawing();
        game_draw(&game);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}

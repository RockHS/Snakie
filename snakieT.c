#include "raylib.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define SCREEN_WIDTH 800
#define GRID_SIZE 20
#define HUD_HEIGHT 40
#define GRID_ROWS 30
#define SCREEN_HEIGHT (HUD_HEIGHT + GRID_ROWS * GRID_SIZE)

#define GRID_COLS (SCREEN_WIDTH / GRID_SIZE)

// Allow the snake to grow to fill the entire board (minus the head)
#define MAX_TAIL_LENGTH (GRID_COLS * GRID_ROWS - 1)
#define INPUT_QUEUE_MAX 3
#define BASE_TICK_INTERVAL 0.1f
#define BOOSTED_TICK_INTERVAL 0.08f
#define HORSE_BOOST_SECONDS 22.0f
#define HORSE_GROWTH_AMOUNT 3
#define HORSE_COLLECTIBLE_WIDTH 2
#define HORSE_COLLECTIBLE_HEIGHT 2
#define HORSE_MILESTONE_COUNT 8
#define HORSE_POST_MILESTONE_SPACING 90
#define ROCK_MEDAL_SMASH_REQUIREMENT 7
#define ROCK_MEDAL_CHARGES 3
#define ROCK_MEDAL_COLLECTIBLE_WIDTH 1
#define ROCK_MEDAL_COLLECTIBLE_HEIGHT 1
#define MAX_ROCKS 16
#define ROCK_MAX_CELLS 8
#define ROCK_WARNING_SECONDS 2.0f
#define ROCK_MIN_FALL_DELAY_SECONDS 5.0f
#define ROCK_MAX_FALL_DELAY_SECONDS 12.0f
#define UPGRADE_CHOICES_PER_ROLL 3
#define UPGRADE_MAX_CHOICES_PER_ROLL 4
#define FIRST_UPGRADE_FRUIT_COUNT 15
#define UPGRADE_FRUIT_INTERVAL 30
#define UPGRADE_SPEED_AMOUNT 0.05f
#define UPGRADE_SPEED_MAX 5
#define LUCKY_HORSE_CHANCE_PER_UPGRADE 3
#define LUCKY_HORSE_MAX 4
#define ROCK_DELAY_UPGRADE_SECONDS 2.0f
#define ROCK_DELAY_MAX 3
#define ROCK_WARNING_UPGRADE_SECONDS 0.5f
#define ROCK_WARNING_MAX 4
#define BOOST_DURATION_UPGRADE_SECONDS 3.0f
#define BOOST_DURATION_MAX 4
#define SHIELD_CHARGE_MAX 3
#define SWEET_FRUIT_SCORE_BONUS 5
#define SWEET_FRUIT_MAX 3
#define STABLE_HOOVES_SECONDS 1.0f
#define STABLE_HOOVES_MAX 3
#define STABLE_HOOVES_MAX_EXTRA_SECONDS 5.0f
#define KINDER_ROCKS_MAX 3
#define BOSS_FIRST_FRUIT_COUNT 30
#define BOSS_FIRST_INTERVAL 20
#define BOSS_INTERVAL_STEP 10
#define JOCKEY_MAX_HP 3
#define JOCKEY_WIDTH 2
#define JOCKEY_HEIGHT 2
#define JOCKEY_MAX_TELEGRAPH_CELLS (GRID_COLS * JOCKEY_HEIGHT)
#define JOCKEY_TELEGRAPH_SECONDS 1.20f
#define JOCKEY_DASH_STEP_SECONDS 0.08f
#define JOCKEY_RECOVER_SECONDS 0.80f
#define BOSS_REWARD_CHOICES 3
#define BOSS_GOLDEN_SADDLE_SECONDS 6.0f
#define BOSS_HORSE_MAGNET_CHANCE_PERCENT 6
#define BOSS_CHAMPION_CHARGE_BONUS 2
#define HP_SHARDS_PER_HEART 5
#define MAX_HP_SHARDS_CAP 25
#define DAMAGE_GRACE_BASE_SECONDS 0.35f
#define DAMAGE_GRACE_STAT_SECONDS 0.15f
#define CHEST_COLLECTIBLE_WIDTH 1
#define CHEST_COLLECTIBLE_HEIGHT 1
#define CHEST_REWARD_CHOICES 2
#define CHEST_DROP_CHANCE_4_CELL 20
#define CHEST_DROP_CHANCE_6_CELL 45
#define CHEST_LUCK_CHANCE_BONUS 5
#define FRUIT_SENSE_CHANCE_PER_STAT 5
#define LEAN_BODY_MAX 4
#define GAME_AUDIO_SAMPLE_RATE 48000
#define GAME_AUDIO_BUFFER_FRAMES 1024
#define BGM_EXPORT_PATH "bg_audio_settings.txt"
#define BGM_BPM_MIN 60.0f
#define BGM_BPM_MAX 190.0f
#define BGM_VOLUME_MIN 0.0f
#define BGM_VOLUME_MAX 0.35f
#define BGM_ROOT_HZ_MIN 80.0f
#define BGM_ROOT_HZ_MAX 260.0f
#define BGM_NOISE_MIN 0.0f
#define BGM_NOISE_MAX 1.0f
#define DEV_CONSOLE_COMMAND_MAX 64
#define DEV_CONSOLE_MESSAGE_MAX 128

typedef enum {
    BGM_WAVE_SQUARE = 0,
    BGM_WAVE_TRIANGLE,
    BGM_WAVE_PULSE,
    BGM_WAVEFORM_COUNT
} BgMusicWaveform;

typedef enum {
    BGM_BASS_STEADY = 0,
    BGM_BASS_BOUNCE,
    BGM_BASS_CLIMB,
    BGM_BASS_PATTERN_COUNT
} BgMusicBassPattern;

typedef enum {
    BGM_LEAD_OFF = 0,
    BGM_LEAD_SPARSE,
    BGM_LEAD_MEDIUM,
    BGM_LEAD_BUSY,
    BGM_LEAD_DENSITY_COUNT
} BgMusicLeadDensity;

typedef enum {
    BGM_SCALE_MAJOR = 0,
    BGM_SCALE_PENTATONIC,
    BGM_SCALE_LYDIAN,
    BGM_SCALE_COUNT
} BgMusicScale;

typedef enum {
    BGM_MELODY_ARP = 0,
    BGM_MELODY_BOUNCE,
    BGM_MELODY_WALK,
    BGM_MELODY_SPARKLE,
    BGM_MELODY_PATTERN_COUNT
} BgMusicMelodyPattern;

typedef enum {
    BGM_DRUM_OFF = 0,
    BGM_DRUM_TICK,
    BGM_DRUM_BEAT,
    BGM_DRUM_BUSY,
    BGM_DRUM_AMOUNT_COUNT
} BgMusicDrumAmount;

typedef enum {
    BGM_PARAM_BPM = 0,
    BGM_PARAM_VOLUME,
    BGM_PARAM_ROOT_HZ,
    BGM_PARAM_WAVEFORM,
    BGM_PARAM_BASS_PATTERN,
    BGM_PARAM_LEAD_DENSITY,
    BGM_PARAM_SCALE,
    BGM_PARAM_MELODY_PATTERN,
    BGM_PARAM_DRUM_AMOUNT,
    BGM_PARAM_NOISE,
    BGM_PARAM_COUNT
} BgMusicParam;

typedef struct {
    float bpm;
    float volume;
    float root_hz;
    uint32_t waveform;
    uint32_t bass_pattern;
    uint32_t lead_density;
    uint32_t scale;
    uint32_t melody_pattern;
    uint32_t drum_amount;
    float noise_amount;
} BgMusicSettings;

typedef struct {
    AudioStream stream;
    bool ready;
    bool muted;
    BgMusicSettings settings;
    float bass_phase;
    float lead_phase;
    uint64_t sample_cursor;
    uint32_t samples_per_step;
    uint32_t sample_in_step;
    uint32_t music_step;
    uint32_t noise_state;
    int16_t sample_buffer[GAME_AUDIO_BUFFER_FRAMES];
} GameAudioState;

typedef enum {
    DEV_CONSOLE_COMMAND = 0,
    DEV_CONSOLE_BG_AUDIO
} DevConsoleMode;

typedef struct {
    bool open;
    DevConsoleMode mode;
    char command[DEV_CONSOLE_COMMAND_MAX];
    uint32_t command_length;
    BgMusicSettings draft_music;
    int32_t active_param;
    char message[DEV_CONSOLE_MESSAGE_MAX];
    float message_timer;
    uint32_t tool_rng_state;
} DevConsoleState;

static const uint32_t HORSE_MILESTONES[HORSE_MILESTONE_COUNT] = {10, 25, 45, 70, 100, 140, 190, 260};

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
    COLLECTIBLE_ROCK_MEDAL,
    COLLECTIBLE_CHEST
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
    float warning_duration;
} Rock;

typedef enum {
    UPGRADE_SPEED_5 = 0,
    UPGRADE_LUCKY_HORSE,
    UPGRADE_ROCK_DELAY,
    UPGRADE_LONG_WARNING,
    UPGRADE_HORSEPOWER,
    UPGRADE_HARD_HEAD,
    UPGRADE_SWEET_FRUIT,
    UPGRADE_SMALL_APPETITE,
    UPGRADE_STABLE_HOOVES,
    UPGRADE_KINDER_ROCKS,
    UPGRADE_COUNT,
    UPGRADE_NONE = 0xffffffffU
} UpgradeType;

typedef struct {
    UpgradeType type;
    const char *name;
    const char *description;
    uint32_t max_level;
} UpgradeDef;

static const UpgradeDef UPGRADE_DEFS[UPGRADE_COUNT] = {
    {UPGRADE_SPEED_5,        "Quick Scales",   "snake speed +5%",                 UPGRADE_SPEED_MAX},
    {UPGRADE_LUCKY_HORSE,    "Lucky Horse",    "fruit can reveal bonus horses",   LUCKY_HORSE_MAX},
    {UPGRADE_ROCK_DELAY,     "Stone Patience", "rockfalls arrive later",          ROCK_DELAY_MAX},
    {UPGRADE_LONG_WARNING,   "Long Warning",   "rock shadows linger longer",      ROCK_WARNING_MAX},
    {UPGRADE_HORSEPOWER,     "Horsepower",     "horse boost lasts +3s",           BOOST_DURATION_MAX},
    {UPGRADE_HARD_HEAD,      "Hard Head",      "rock medal grants +1 charge",     SHIELD_CHARGE_MAX},
    {UPGRADE_SWEET_FRUIT,    "Sweet Fruit",    "fruit score +5",                  SWEET_FRUIT_MAX},
    {UPGRADE_SMALL_APPETITE, "Small Appetite", "every 4th fruit skips growth",    1},
    {UPGRADE_STABLE_HOOVES,  "Stable Hooves",  "boost rock-smashes add time",     STABLE_HOOVES_MAX},
    {UPGRADE_KINDER_ROCKS,   "Kinder Rocks",   "giant rocks become rarer",        KINDER_ROCKS_MAX},
};

typedef enum {
    STAT_MAX_HP = 0,
    STAT_FRUIT_VALUE,
    STAT_CHEST_LUCK,
    STAT_DAMAGE_GRACE,
    STAT_FRUIT_SENSE,
    STAT_LEAN_BODY,
    STAT_COUNT,
    STAT_NONE = 0xffffffffU
} SnakeStatType;

typedef struct {
    SnakeStatType type;
    const char *name;
    const char *description;
} SnakeStatDef;

static const SnakeStatDef SNAKE_STAT_DEFS[STAT_COUNT] = {
    {STAT_MAX_HP,       "Shed Skin",    "max HP +0.2 and heal +0.2"},
    {STAT_FRUIT_VALUE,  "Sweet Tooth",  "fruit score +1"},
    {STAT_CHEST_LUCK,   "Treasure Nose", "chest chance +5%"},
    {STAT_DAMAGE_GRACE, "Slip Away",    "longer grace after damage"},
    {STAT_FRUIT_SENSE,  "Fruit Sense",  "better fruit appears more often"},
    {STAT_LEAN_BODY,    "Lean Body",    "some fruit skips tail growth"},
};

typedef struct {
    uint32_t max_hp_shards;
    uint32_t hp_shards;
    uint32_t fruit_value_bonus;
    uint32_t chest_luck;
    uint32_t damage_grace;
    uint32_t fruit_sense;
    uint32_t lean_body;
} SnakeStats;

typedef enum {
    JOCKEY_BOSS_INACTIVE = 0,
    JOCKEY_BOSS_TELEGRAPH,
    JOCKEY_BOSS_DASH,
    JOCKEY_BOSS_RECOVER
} JockeyBossPhase;

typedef struct {
    bool active;
    JockeyBossPhase phase;
    Position pos;
    Direction dash_dir;
    float timer;
    float dash_step_timer;
    uint32_t hp;
    Position telegraph_cells[JOCKEY_MAX_TELEGRAPH_CELLS];
    uint32_t telegraph_count;
} JockeyBoss;

typedef enum {
    BOSS_REWARD_GOLDEN_SADDLE = 0,
    BOSS_REWARD_HORSE_MAGNET,
    BOSS_REWARD_CHAMPION_CHARGE,
    BOSS_REWARD_FOUR_CHOICES,
    BOSS_REWARD_COUNT,
    BOSS_REWARD_NONE = 0xffffffffU
} BossRewardType;

typedef struct {
    BossRewardType type;
    const char *name;
    const char *description;
} BossRewardDef;

static const BossRewardDef BOSS_REWARD_DEFS[BOSS_REWARD_COUNT] = {
    {BOSS_REWARD_GOLDEN_SADDLE,   "Golden Saddle",  "horse boost lasts +6s"},
    {BOSS_REWARD_HORSE_MAGNET,    "Horse Magnet",   "lucky horse chance +6%"},
    {BOSS_REWARD_CHAMPION_CHARGE, "Champion Charge", "rock medals hold +2 charges"},
    {BOSS_REWARD_FOUR_CHOICES,    "Four Choices",   "upgrade screens show one extra choice"},
};

typedef enum {
    SCREEN_TITLE = 0,
    SCREEN_GAMEPLAY,
    SCREEN_BOSS,
    SCREEN_GAMEOVER,
    SCREEN_UPGRADE,
    SCREEN_BOSS_REWARD,
    SCREEN_STAT_REWARD,
    SCREEN_VICTORY
} GameScreen;

typedef struct {
    Position head;
    Fruit fruit;
    Collectible collectible;
    JockeyBoss boss;
    SnakeStats stats;
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
    uint32_t next_boss_milestone_index;
    uint32_t next_upgrade_fruit_count;
    uint32_t next_boss_fruit_count;
    uint32_t upgrade_levels[UPGRADE_COUNT];
    UpgradeType upgrade_choices[UPGRADE_MAX_CHOICES_PER_ROLL];
    BossRewardType boss_reward_choices[BOSS_REWARD_CHOICES];
    SnakeStatType stat_reward_choices[CHEST_REWARD_CHOICES];
    uint32_t reward_selection_index;
    uint32_t boss_golden_saddle_count;
    uint32_t boss_horse_magnet_count;
    uint32_t boss_champion_charge_count;
    bool boss_four_choices_unlocked;
    
    uint32_t run_seed;
    uint32_t rng_state;
    
    float update_timer;
    float time_between_ticks;
    float boost_timer;
    float damage_grace_timer;
    float rock_fall_timer;
    bool easy_mode;
    bool rocks_enabled;
    bool paused;
    bool console_pause_active;
} GameState;

typedef struct {
    Direction moves[INPUT_QUEUE_MAX];
    uint32_t count;
    bool r_pressed;
    bool enter_pressed;
    bool easy_mode_toggle_pressed;
    bool rocks_toggle_pressed;
    bool pause_pressed;
    bool choice_up_pressed;
    bool choice_down_pressed;
    bool upgrade_choice_pressed;
    uint32_t upgrade_choice_index;
    bool boss_reward_choice_pressed;
    uint32_t boss_reward_choice_index;
    bool stat_reward_choice_pressed;
    uint32_t stat_reward_choice_index;
} GameInput;

// Static scratch array to avoid allocating large grids on the stack
static Position g_free_cells[GRID_COLS * GRID_ROWS];

uint32_t random_next(uint32_t *state);

float clamp_float(float value, float min_value, float max_value) {
    if (value < min_value) return min_value;
    if (value > max_value) return max_value;
    return value;
}

uint32_t clamp_u32(uint32_t value, uint32_t min_value, uint32_t max_value) {
    if (value < min_value) return min_value;
    if (value > max_value) return max_value;
    return value;
}

int cell_screen_x(Position p) {
    return p.x * GRID_SIZE;
}

int cell_screen_y(Position p) {
    return HUD_HEIGHT + p.y * GRID_SIZE;
}

bool dev_console_command_matches(const char *command, const char *target) {
    while (*command == ' ' || *command == '\t') {
        command++;
    }

    for (uint32_t i = 0; target[i] != 0; i++) {
        if (command[i] != target[i]) {
            return false;
        }
    }

    command += strlen(target);
    while (*command == ' ' || *command == '\t') {
        command++;
    }

    return *command == 0;
}

bool dev_console_command_is_bg_audio(const char *command) {
    return dev_console_command_matches(command, "bg.audio");
}

bool dev_console_command_is_spawn_boss(const char *command) {
    return dev_console_command_matches(command, "spawn.boss");
}

BgMusicSettings bg_music_default_settings(void) {
    BgMusicSettings settings = {0};
    settings.bpm = 120.41f;
    settings.volume = 0.058f;
    settings.root_hz = 152.53f;
    settings.waveform = BGM_WAVE_TRIANGLE;
    settings.bass_pattern = BGM_BASS_BOUNCE;
    settings.lead_density = BGM_LEAD_BUSY;
    settings.scale = BGM_SCALE_MAJOR;
    settings.melody_pattern = BGM_MELODY_WALK;
    settings.drum_amount = BGM_DRUM_BUSY;
    settings.noise_amount = 0.130f;
    return settings;
}

void bg_music_clamp_settings(BgMusicSettings *settings) {
    settings->bpm = clamp_float(settings->bpm, BGM_BPM_MIN, BGM_BPM_MAX);
    settings->volume = clamp_float(settings->volume, BGM_VOLUME_MIN, BGM_VOLUME_MAX);
    settings->root_hz = clamp_float(settings->root_hz, BGM_ROOT_HZ_MIN, BGM_ROOT_HZ_MAX);
    settings->waveform = clamp_u32(settings->waveform, 0, BGM_WAVEFORM_COUNT - 1);
    settings->bass_pattern = clamp_u32(settings->bass_pattern, 0, BGM_BASS_PATTERN_COUNT - 1);
    settings->lead_density = clamp_u32(settings->lead_density, 0, BGM_LEAD_DENSITY_COUNT - 1);
    settings->scale = clamp_u32(settings->scale, 0, BGM_SCALE_COUNT - 1);
    settings->melody_pattern = clamp_u32(settings->melody_pattern, 0, BGM_MELODY_PATTERN_COUNT - 1);
    settings->drum_amount = clamp_u32(settings->drum_amount, 0, BGM_DRUM_AMOUNT_COUNT - 1);
    settings->noise_amount = clamp_float(settings->noise_amount, BGM_NOISE_MIN, BGM_NOISE_MAX);
}

int bg_music_format_export(const BgMusicSettings *settings, char *buffer, uint32_t buffer_size) {
    if (buffer_size == 0) {
        return 0;
    }

    BgMusicSettings safe_settings = *settings;
    bg_music_clamp_settings(&safe_settings);

    int written = snprintf(buffer, buffer_size,
                           "static const BgMusicSettings BAKED_BG_MUSIC_SETTINGS = {\n"
                           "    .bpm = %.2ff,\n"
                           "    .volume = %.3ff,\n"
                           "    .root_hz = %.2ff,\n"
                           "    .waveform = %u,\n"
                           "    .bass_pattern = %u,\n"
                           "    .lead_density = %u,\n"
                           "    .scale = %u,\n"
                           "    .melody_pattern = %u,\n"
                           "    .drum_amount = %u,\n"
                           "    .noise_amount = %.3ff,\n"
                           "};\n",
                           safe_settings.bpm,
                           safe_settings.volume,
                           safe_settings.root_hz,
                           safe_settings.waveform,
                           safe_settings.bass_pattern,
                           safe_settings.lead_density,
                           safe_settings.scale,
                           safe_settings.melody_pattern,
                           safe_settings.drum_amount,
                           safe_settings.noise_amount);

    if (written < 0) {
        buffer[0] = 0;
        return 0;
    }

    if ((uint32_t)written >= buffer_size) {
        buffer[buffer_size - 1] = 0;
        return (int)(buffer_size - 1);
    }

    return written;
}

bool bg_music_save_settings(const BgMusicSettings *settings) {
    char export_text[1024] = {0};
    int export_count = bg_music_format_export(settings, export_text, sizeof(export_text));
    if (export_count <= 0) {
        return false;
    }

    FILE *file = fopen(BGM_EXPORT_PATH, "wb");
    if (!file) {
        return false;
    }

    size_t written = fwrite(export_text, 1, (size_t)export_count, file);
    fclose(file);
    return written == (size_t)export_count;
}

static inline float bg_music_wave_sample(float *phase, float phase_step, uint32_t waveform) {
    *phase += phase_step;
    while (*phase >= 1.0f) {
        *phase -= 1.0f;
    }

    switch (waveform) {
        case BGM_WAVE_TRIANGLE:
            return (*phase < 0.5f) ? (-1.0f + *phase * 4.0f) : (3.0f - *phase * 4.0f);
        case BGM_WAVE_PULSE:
            return (*phase < 0.25f) ? 1.0f : -1.0f;
        case BGM_WAVE_SQUARE:
        default:
            return (*phase < 0.5f) ? 1.0f : -1.0f;
    }
}

float bg_music_bass_ratio(uint32_t pattern, uint32_t step) {
    static const float ratios[BGM_BASS_PATTERN_COUNT][4] = {
        {1.0f, 1.0f, 1.0f, 1.0f},
        {1.0f, 1.5f, 1.0f, 1.3333f},
        {1.0f, 1.125f, 1.25f, 1.5f},
    };

    pattern = clamp_u32(pattern, 0, BGM_BASS_PATTERN_COUNT - 1);
    return ratios[pattern][step & 3U];
}

float bg_music_scale_ratio(uint32_t scale, uint32_t degree) {
    static const float ratios[BGM_SCALE_COUNT][8] = {
        {1.0000f, 1.1225f, 1.2599f, 1.3348f, 1.4983f, 1.6818f, 1.8877f, 2.0000f},
        {1.0000f, 1.1225f, 1.2599f, 1.4983f, 1.6818f, 2.0000f, 2.2449f, 2.5198f},
        {1.0000f, 1.1225f, 1.2599f, 1.4142f, 1.4983f, 1.6818f, 1.8877f, 2.0000f},
    };

    scale = clamp_u32(scale, 0, BGM_SCALE_COUNT - 1);
    return ratios[scale][degree & 7U];
}

float bg_music_lead_ratio(uint32_t scale, uint32_t melody_pattern, uint32_t step) {
    static const uint32_t degrees[BGM_MELODY_PATTERN_COUNT][16] = {
        {0, 2, 4, 7, 4, 2, 0, 4, 0, 3, 5, 7, 5, 3, 2, 0},
        {0, 4, 2, 5, 3, 7, 4, 2, 0, 5, 3, 7, 4, 2, 1, 0},
        {0, 1, 2, 3, 4, 5, 4, 3, 2, 3, 4, 5, 6, 5, 4, 2},
        {7, 4, 5, 2, 6, 3, 5, 1, 7, 5, 4, 2, 6, 4, 3, 0},
    };

    melody_pattern = clamp_u32(melody_pattern, 0, BGM_MELODY_PATTERN_COUNT - 1);
    return 2.0f * bg_music_scale_ratio(scale, degrees[melody_pattern][step & 15U]);
}

static inline float bg_music_drum_strength(uint32_t drum_amount, uint32_t step) {
    if (drum_amount == BGM_DRUM_OFF) {
        return 0.0f;
    }

    if (drum_amount == BGM_DRUM_TICK) {
        return ((step & 3U) == 0) ? 0.55f : 0.0f;
    }

    if (drum_amount == BGM_DRUM_BEAT) {
        if ((step & 3U) == 0) return 0.80f;
        if ((step & 3U) == 2) return 0.38f;
        return 0.0f;
    }

    if ((step & 3U) == 0) return 0.85f;
    if ((step & 1U) == 0) return 0.42f;
    return 0.18f;
}

float bg_music_drum_envelope(uint32_t drum_amount, uint32_t step, uint32_t sample_in_step, uint32_t samples_per_step) {
    float strength = bg_music_drum_strength(drum_amount, step);
    if (strength <= 0.0f) {
        return 0.0f;
    }

    uint32_t envelope_samples = samples_per_step / 2U;
    if (envelope_samples == 0) {
        envelope_samples = 1;
    }

    if (sample_in_step >= envelope_samples) {
        return 0.0f;
    }

    return strength * (1.0f - ((float)sample_in_step / (float)envelope_samples));
}

void bg_music_randomize_settings(BgMusicSettings *settings, uint32_t *rng_state) {
    static const float roots[] = {110.00f, 123.47f, 130.81f, 146.83f, 164.81f, 174.61f, 196.00f};
    uint32_t root_count = sizeof(roots) / sizeof(roots[0]);

    settings->bpm = 132.0f + (float)(random_next(rng_state) % 47U);
    settings->volume = 0.10f + (float)(random_next(rng_state) % 16U) * 0.01f;
    settings->root_hz = roots[random_next(rng_state) % root_count];
    settings->waveform = (random_next(rng_state) & 1U) ? BGM_WAVE_PULSE : BGM_WAVE_SQUARE;
    settings->bass_pattern = random_next(rng_state) % BGM_BASS_PATTERN_COUNT;
    settings->lead_density = BGM_LEAD_MEDIUM + (random_next(rng_state) % 2U);
    settings->scale = random_next(rng_state) % BGM_SCALE_COUNT;
    settings->melody_pattern = random_next(rng_state) % BGM_MELODY_PATTERN_COUNT;
    settings->drum_amount = BGM_DRUM_TICK + (random_next(rng_state) % (BGM_DRUM_AMOUNT_COUNT - 1));
    settings->noise_amount = 0.10f + (float)(random_next(rng_state) % 21U) * 0.01f;
    bg_music_clamp_settings(settings);
}

void bg_music_mutate_settings(BgMusicSettings *settings, uint32_t *rng_state) {
    uint32_t choice = random_next(rng_state) % 7U;

    switch (choice) {
        case 0:
            settings->bpm += (float)((int32_t)(random_next(rng_state) % 17U) - 8);
            break;
        case 1:
            settings->root_hz += (float)((int32_t)(random_next(rng_state) % 11U) - 5);
            break;
        case 2:
            settings->scale = random_next(rng_state) % BGM_SCALE_COUNT;
            break;
        case 3:
            settings->melody_pattern = random_next(rng_state) % BGM_MELODY_PATTERN_COUNT;
            break;
        case 4:
            settings->drum_amount = random_next(rng_state) % BGM_DRUM_AMOUNT_COUNT;
            break;
        case 5:
            settings->lead_density = random_next(rng_state) % BGM_LEAD_DENSITY_COUNT;
            break;
        default:
            settings->noise_amount += ((float)((int32_t)(random_next(rng_state) % 11U) - 5)) * 0.01f;
            break;
    }

    bg_music_clamp_settings(settings);
}

void bg_music_reset_runtime(GameAudioState *audio) {
    audio->bass_phase = 0.0f;
    audio->lead_phase = 0.0f;
    audio->sample_cursor = 0;
    audio->samples_per_step = 1;
    audio->sample_in_step = 0;
    audio->music_step = 0;
    audio->noise_state = 0x12345678U;
}

void bg_music_generate_samples(GameAudioState *audio, int16_t *samples, uint32_t frame_count) {
    BgMusicSettings settings = audio->settings;
    bg_music_clamp_settings(&settings);

    uint32_t samples_per_step = (uint32_t)(((60.0f / settings.bpm) * (float)GAME_AUDIO_SAMPLE_RATE) / 4.0f);
    if (samples_per_step == 0) {
        samples_per_step = 1;
    }
    if (audio->samples_per_step != samples_per_step) {
        audio->samples_per_step = samples_per_step;
        audio->sample_in_step = (uint32_t)(audio->sample_cursor % samples_per_step);
        audio->music_step = (uint32_t)((audio->sample_cursor / samples_per_step) & 15U);
    }

    static const uint32_t lead_gaps[BGM_LEAD_DENSITY_COUNT] = {0, 8, 4, 2};
    uint32_t step = audio->music_step;
    uint32_t sample_in_step = audio->sample_in_step;
    uint32_t envelope_samples = samples_per_step / 2U;
    if (envelope_samples == 0) {
        envelope_samples = 1;
    }
    float inverse_envelope_samples = 1.0f / (float)envelope_samples;
    float root_phase_step = settings.root_hz / (float)GAME_AUDIO_SAMPLE_RATE;

    for (uint32_t i = 0; i < frame_count; i++) {
        float bass_phase_step = root_phase_step * bg_music_bass_ratio(settings.bass_pattern, step / 4U);
        float bass = bg_music_wave_sample(&audio->bass_phase, bass_phase_step, settings.waveform);
        float mixed = bass * 0.48f;

        if (settings.lead_density > BGM_LEAD_OFF) {
            uint32_t gap = lead_gaps[settings.lead_density];
            if (gap > 0 && (step & (gap - 1U)) == 0) {
                float lead_phase_step = root_phase_step * bg_music_lead_ratio(settings.scale, settings.melody_pattern, step);
                float lead = bg_music_wave_sample(&audio->lead_phase, lead_phase_step, settings.waveform);
                mixed += lead * 0.22f;
            }
        }

        float drum_strength = bg_music_drum_strength(settings.drum_amount, step);
        if (drum_strength > 0.0f && sample_in_step < envelope_samples && settings.noise_amount > 0.0f) {
            uint32_t noise = random_next(&audio->noise_state);
            float noise_sample = (noise & 1U) ? 1.0f : -1.0f;
            float drum_envelope = drum_strength * (1.0f - ((float)sample_in_step * inverse_envelope_samples));
            mixed += noise_sample * settings.noise_amount * drum_envelope * 0.28f;
        }

        mixed *= settings.volume;
        mixed = clamp_float(mixed, -1.0f, 1.0f);
        samples[i] = (int16_t)(mixed * 32767.0f);
        audio->sample_cursor++;
        sample_in_step++;
        if (sample_in_step >= samples_per_step) {
            sample_in_step = 0;
            step = (step + 1U) & 15U;
        }
    }

    audio->sample_in_step = sample_in_step;
    audio->music_step = step;
}

float game_audio_effective_stream_volume(const GameAudioState *audio) {
    return audio->muted ? 0.0f : 1.0f;
}

void game_audio_apply_mute_state(GameAudioState *audio) {
    if (audio->ready) {
        SetAudioStreamVolume(audio->stream, game_audio_effective_stream_volume(audio));
    }
}

void game_audio_set_muted(GameAudioState *audio, bool muted) {
    audio->muted = muted;
    game_audio_apply_mute_state(audio);
}

void game_audio_toggle_muted(GameAudioState *audio) {
    game_audio_set_muted(audio, !audio->muted);
}

void game_audio_init(GameAudioState *audio) {
    *audio = (GameAudioState){0};
    audio->settings = bg_music_default_settings();
    bg_music_reset_runtime(audio);

    InitAudioDevice();
    if (!IsAudioDeviceReady()) {
        TraceLog(LOG_WARNING, "Audio device could not be initialized.");
        return;
    }

    SetAudioStreamBufferSizeDefault(GAME_AUDIO_BUFFER_FRAMES);
    audio->stream = LoadAudioStream(GAME_AUDIO_SAMPLE_RATE, 16, 1);
    if (!IsAudioStreamReady(audio->stream)) {
        TraceLog(LOG_WARNING, "Audio stream could not be initialized.");
        CloseAudioDevice();
        return;
    }

    audio->ready = true;
    bg_music_generate_samples(audio, audio->sample_buffer, GAME_AUDIO_BUFFER_FRAMES);
    UpdateAudioStream(audio->stream, audio->sample_buffer, GAME_AUDIO_BUFFER_FRAMES);
    bg_music_generate_samples(audio, audio->sample_buffer, GAME_AUDIO_BUFFER_FRAMES);
    UpdateAudioStream(audio->stream, audio->sample_buffer, GAME_AUDIO_BUFFER_FRAMES);
    game_audio_apply_mute_state(audio);
    PlayAudioStream(audio->stream);
}

void game_audio_update(GameAudioState *audio) {
    if (!audio->ready) {
        return;
    }

    while (IsAudioStreamProcessed(audio->stream)) {
        bg_music_generate_samples(audio, audio->sample_buffer, GAME_AUDIO_BUFFER_FRAMES);
        UpdateAudioStream(audio->stream, audio->sample_buffer, GAME_AUDIO_BUFFER_FRAMES);
    }
}

void game_audio_shutdown(GameAudioState *audio) {
    if (audio->ready) {
        UnloadAudioStream(audio->stream);
        audio->ready = false;
    }

    if (IsAudioDeviceReady()) {
        CloseAudioDevice();
    }
}

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

    return HORSE_MILESTONES[HORSE_MILESTONE_COUNT - 1] +
           (index - HORSE_MILESTONE_COUNT + 1) * HORSE_POST_MILESTONE_SPACING;
}

uint32_t boss_milestone_for_index(uint32_t index) {
    uint32_t milestone = BOSS_FIRST_FRUIT_COUNT;
    uint32_t interval = BOSS_FIRST_INTERVAL;
    for (uint32_t i = 1; i <= index; i++) {
        milestone += interval;
        if (i >= 2) {
            interval += BOSS_INTERVAL_STEP;
        }
    }

    return milestone;
}

uint32_t game_upgrade_level(const GameState *game, UpgradeType type) {
    if (type >= UPGRADE_COUNT) {
        return 0;
    }

    return game->upgrade_levels[type];
}

bool game_upgrade_can_apply(const GameState *game, UpgradeType type) {
    if (type >= UPGRADE_COUNT) {
        return false;
    }

    return game->upgrade_levels[type] < UPGRADE_DEFS[type].max_level;
}

bool game_screen_runs_simulation(GameScreen screen) {
    return screen == SCREEN_GAMEPLAY || screen == SCREEN_BOSS;
}

uint32_t game_upgrade_choice_count(const GameState *game) {
    return game->boss_four_choices_unlocked ? UPGRADE_MAX_CHOICES_PER_ROLL : UPGRADE_CHOICES_PER_ROLL;
}

uint32_t game_reward_choice_count(const GameState *game) {
    switch (game->current_screen) {
        case SCREEN_UPGRADE:
            return game_upgrade_choice_count(game);
        case SCREEN_BOSS_REWARD:
            return BOSS_REWARD_CHOICES;
        case SCREEN_STAT_REWARD:
            return CHEST_REWARD_CHOICES;
        default:
            return 0;
    }
}

void game_clamp_reward_selection(GameState *game) {
    uint32_t choice_count = game_reward_choice_count(game);
    if (choice_count == 0) {
        game->reward_selection_index = 0;
    } else if (game->reward_selection_index >= choice_count) {
        game->reward_selection_index = choice_count - 1;
    }
}

void game_move_reward_selection(GameState *game, int32_t direction) {
    uint32_t choice_count = game_reward_choice_count(game);
    if (choice_count == 0 || direction == 0) {
        return;
    }

    if (direction < 0) {
        game->reward_selection_index = (game->reward_selection_index == 0)
                                     ? choice_count - 1
                                     : game->reward_selection_index - 1;
    } else {
        game->reward_selection_index = (game->reward_selection_index + 1) % choice_count;
    }
}

float game_speed_multiplier(const GameState *game) {
    return 1.0f + (float)game_upgrade_level(game, UPGRADE_SPEED_5) * UPGRADE_SPEED_AMOUNT;
}

float game_horse_boost_seconds(const GameState *game) {
    return HORSE_BOOST_SECONDS +
           (float)game_upgrade_level(game, UPGRADE_HORSEPOWER) * BOOST_DURATION_UPGRADE_SECONDS +
           (float)game->boss_golden_saddle_count * BOSS_GOLDEN_SADDLE_SECONDS;
}

uint32_t game_boost_seconds_remaining(const GameState *game) {
    if (game->boost_timer <= 0.0f) {
        return 0;
    }

    uint32_t whole_seconds = (uint32_t)game->boost_timer;
    return ((float)whole_seconds < game->boost_timer) ? whole_seconds + 1 : whole_seconds;
}

uint32_t game_rock_medal_charge_count(const GameState *game) {
    return ROCK_MEDAL_CHARGES +
           game_upgrade_level(game, UPGRADE_HARD_HEAD) +
           game->boss_champion_charge_count * BOSS_CHAMPION_CHARGE_BONUS;
}

uint32_t game_lucky_horse_chance_percent(const GameState *game) {
    return game_upgrade_level(game, UPGRADE_LUCKY_HORSE) * LUCKY_HORSE_CHANCE_PER_UPGRADE +
           game->boss_horse_magnet_count * BOSS_HORSE_MAGNET_CHANCE_PERCENT;
}

float game_damage_grace_seconds(const GameState *game) {
    return DAMAGE_GRACE_BASE_SECONDS +
           (float)game->stats.damage_grace * DAMAGE_GRACE_STAT_SECONDS;
}

uint32_t game_chest_drop_chance_percent(const GameState *game, uint32_t rock_cell_count) {
    uint32_t chance = 0;
    if (rock_cell_count == 4) {
        chance = CHEST_DROP_CHANCE_4_CELL;
    } else if (rock_cell_count >= 6) {
        chance = CHEST_DROP_CHANCE_6_CELL;
    }

    chance += game->stats.chest_luck * CHEST_LUCK_CHANCE_BONUS;
    if (chance > 95) {
        chance = 95;
    }

    return chance;
}

float game_rock_delay_bonus_seconds(const GameState *game) {
    return (float)game_upgrade_level(game, UPGRADE_ROCK_DELAY) * ROCK_DELAY_UPGRADE_SECONDS;
}

float game_rock_warning_seconds(const GameState *game) {
    return ROCK_WARNING_SECONDS +
           (float)game_upgrade_level(game, UPGRADE_LONG_WARNING) * ROCK_WARNING_UPGRADE_SECONDS;
}

uint32_t game_fruit_score_value(const GameState *game) {
    return game->fruit.score_value +
           game_upgrade_level(game, UPGRADE_SWEET_FRUIT) * SWEET_FRUIT_SCORE_BONUS +
           game->stats.fruit_value_bonus;
}

bool game_fruit_should_grow(const GameState *game) {
    if (game_upgrade_level(game, UPGRADE_SMALL_APPETITE) > 0 &&
        ((game->fruits_eaten + 1) % 4U) == 0) {
        return false;
    }

    if (game->stats.lean_body > 0) {
        uint32_t lean_level = game->stats.lean_body;
        if (lean_level > LEAN_BODY_MAX) {
            lean_level = LEAN_BODY_MAX;
        }

        uint32_t interval = 7U - lean_level;
        if (interval < 2U) {
            interval = 2U;
        }

        if (((game->fruits_eaten + 1) % interval) == 0) {
            return false;
        }
    }

    return true;
}

float game_tick_interval(const GameState *game) {
    float base_interval = (game->boost_timer > 0.0f) ? BOOSTED_TICK_INTERVAL : game->time_between_ticks;
    return base_interval / game_speed_multiplier(game);
}

void game_update_boost_timer(GameState *game, float dt) {
    if (game->boost_timer > 0.0f) {
        game->boost_timer -= dt;
        if (game->boost_timer < 0.0f) {
            game->boost_timer = 0.0f;
        }
    }
}

void game_update_damage_grace_timer(GameState *game, float dt) {
    if (game->damage_grace_timer > 0.0f) {
        game->damage_grace_timer -= dt;
        if (game->damage_grace_timer < 0.0f) {
            game->damage_grace_timer = 0.0f;
        }
    }
}

bool game_take_damage(GameState *game) {
    if (game->damage_grace_timer > 0.0f) {
        return true;
    }

    if (game->stats.hp_shards > HP_SHARDS_PER_HEART) {
        game->stats.hp_shards -= HP_SHARDS_PER_HEART;
        game->damage_grace_timer = game_damage_grace_seconds(game);
        return true;
    }

    game->stats.hp_shards = 0;
    game->current_screen = SCREEN_GAMEOVER;
    return false;
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

bool position_in_cell_rect(Position p, Position top_left, uint32_t width, uint32_t height) {
    return p.x >= top_left.x &&
           p.y >= top_left.y &&
           p.x < top_left.x + (int32_t)width &&
           p.y < top_left.y + (int32_t)height;
}

bool jockey_area_is_inside_board(Position top_left) {
    return top_left.x >= 0 &&
           top_left.y >= 0 &&
           top_left.x + JOCKEY_WIDTH <= GRID_COLS &&
           top_left.y + JOCKEY_HEIGHT <= GRID_ROWS;
}

bool position_in_jockey_boss(const JockeyBoss *boss, Position p) {
    return boss->active &&
           position_in_cell_rect(p, boss->pos, JOCKEY_WIDTH, JOCKEY_HEIGHT);
}

bool position_occupied_by_jockey_boss(const GameState *game, Position p) {
    return position_in_jockey_boss(&game->boss, p);
}

bool cell_rect_overlaps_snake(const GameState *game, Position top_left, uint32_t width, uint32_t height) {
    if (position_in_cell_rect(game->head, top_left, width, height)) {
        return true;
    }

    for (uint32_t i = 0; i < game->tail_length; i++) {
        if (position_in_cell_rect(game->tail[i], top_left, width, height)) {
            return true;
        }
    }

    return false;
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

uint32_t game_rock_cell_count_from_roll(const GameState *game, uint32_t roll) {
    uint32_t giant_threshold = 95 + game_upgrade_level(game, UPGRADE_KINDER_ROCKS) * 2U;
    if (giant_threshold > 99) {
        giant_threshold = 99;
    }

    if (roll < 70) return 2;
    if (roll < giant_threshold) return 4;
    return 6;
}

float random_rock_fall_delay(GameState *game) {
    uint32_t roll = random_next(&game->rng_state) % 701U;
    return ROCK_MIN_FALL_DELAY_SECONDS + game_rock_delay_bonus_seconds(game) + (float)roll * 0.01f;
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
    rock->warning_duration = 0.0f;
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
                position_occupied_by_jockey_boss(game, p) ||
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

bool game_spawn_chest_collectible(GameState *game) {
    if (game->collectible.active) {
        return false;
    }

    uint32_t free_count = 0;
    for (int32_t y = 0; y <= GRID_ROWS - CHEST_COLLECTIBLE_HEIGHT; y++) {
        for (int32_t x = 0; x <= GRID_COLS - CHEST_COLLECTIBLE_WIDTH; x++) {
            Position p = {x, y};
            if (collectible_area_is_clear(game, p, CHEST_COLLECTIBLE_WIDTH, CHEST_COLLECTIBLE_HEIGHT)) {
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
    game->collectible.type = COLLECTIBLE_CHEST;
    game->collectible.width = CHEST_COLLECTIBLE_WIDTH;
    game->collectible.height = CHEST_COLLECTIBLE_HEIGHT;
    return true;
}

void game_maybe_spawn_chest_from_rock(GameState *game, uint32_t rock_cell_count) {
    uint32_t chance = game_chest_drop_chance_percent(game, rock_cell_count);
    if (chance == 0 || game->collectible.active) {
        return;
    }

    if ((random_next(&game->rng_state) % 100U) < chance) {
        (void)game_spawn_chest_collectible(game);
    }
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

void game_note_boosted_rock_smash(GameState *game, uint32_t rock_cell_count) {
    uint32_t stable_level = game_upgrade_level(game, UPGRADE_STABLE_HOOVES);
    if (stable_level > 0 && game->boost_timer > 0.0f) {
        float max_boost = game_horse_boost_seconds(game) + STABLE_HOOVES_MAX_EXTRA_SECONDS;
        game->boost_timer += (float)stable_level * STABLE_HOOVES_SECONDS;
        if (game->boost_timer > max_boost) {
            game->boost_timer = max_boost;
        }
    }

    game_record_rock_smashed(game);
    game_maybe_spawn_chest_from_rock(game, rock_cell_count);
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

    if (position_occupied_by_jockey_boss(game, p)) {
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
            rock->timer = game_rock_warning_seconds(game);
            rock->warning_duration = rock->timer;
            for (uint32_t i = 0; i < cell_count; i++) {
                rock->cells[i] = cells[i];
            }
            return true;
        }
    }

    return false;
}

void game_update_rock_fall_spawner(GameState *game, float dt) {
    if (!game->rocks_enabled) {
        return;
    }

    game->rock_fall_timer -= dt;
    if (game->rock_fall_timer > 0.0f) {
        return;
    }

    uint32_t roll = random_next(&game->rng_state) % 100;
    (void)game_spawn_rock_warning(game, game_rock_cell_count_from_roll(game, roll));
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
                uint32_t rock_cell_count = rock->cell_count;
                game_clear_rock(rock);
                game_note_boosted_rock_smash(game, rock_cell_count);
            } else if (game_spend_rock_medal_charge(game)) {
                game_clear_rock(rock);
            } else if (game_take_damage(game)) {
                game_clear_rock(rock);
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
                    uint32_t rock_cell_count = rock->cell_count;
                    game_clear_rock(rock);
                    game_note_boosted_rock_smash(game, rock_cell_count);
                } else if (game_spend_rock_medal_charge(game)) {
                    game_clear_rock(rock);
                } else if (game_take_damage(game)) {
                    game_clear_rock(rock);
                } else {
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

void game_clear_jockey_boss(GameState *game) {
    game->boss.active = false;
    game->boss.phase = JOCKEY_BOSS_INACTIVE;
    game->boss.pos = (Position){0, 0};
    game->boss.dash_dir = DIR_STOP;
    game->boss.timer = 0.0f;
    game->boss.dash_step_timer = 0.0f;
    game->boss.hp = 0;
    game->boss.telegraph_count = 0;
}

Position position_moved_in_direction(Position p, Direction dir) {
    switch (dir) {
        case DIR_LEFT:  p.x--; break;
        case DIR_RIGHT: p.x++; break;
        case DIR_UP:    p.y--; break;
        case DIR_DOWN:  p.y++; break;
        default: break;
    }

    return p;
}

void game_fill_jockey_telegraph(JockeyBoss *boss) {
    boss->telegraph_count = 0;

    if (boss->dash_dir == DIR_LEFT || boss->dash_dir == DIR_RIGHT) {
        for (int32_t y = boss->pos.y; y < boss->pos.y + JOCKEY_HEIGHT; y++) {
            for (int32_t x = 0; x < GRID_COLS; x++) {
                if (boss->telegraph_count < JOCKEY_MAX_TELEGRAPH_CELLS) {
                    boss->telegraph_cells[boss->telegraph_count++] = (Position){x, y};
                }
            }
        }
    } else {
        for (int32_t y = 0; y < GRID_ROWS; y++) {
            for (int32_t x = boss->pos.x; x < boss->pos.x + JOCKEY_WIDTH; x++) {
                if (boss->telegraph_count < JOCKEY_MAX_TELEGRAPH_CELLS) {
                    boss->telegraph_cells[boss->telegraph_count++] = (Position){x, y};
                }
            }
        }
    }
}

void game_choose_jockey_attack(GameState *game) {
    Position chosen_pos = {0, 0};
    Direction chosen_dir = DIR_RIGHT;

    for (uint32_t attempt = 0; attempt < 64; attempt++) {
        bool horizontal = (random_next(&game->rng_state) & 1U) != 0;
        bool forward = (random_next(&game->rng_state) & 1U) != 0;

        if (horizontal) {
            int32_t y = (int32_t)(random_next(&game->rng_state) % (uint32_t)(GRID_ROWS - JOCKEY_HEIGHT + 1));
            chosen_pos = (Position){forward ? 0 : GRID_COLS - JOCKEY_WIDTH, y};
            chosen_dir = forward ? DIR_RIGHT : DIR_LEFT;
        } else {
            int32_t x = (int32_t)(random_next(&game->rng_state) % (uint32_t)(GRID_COLS - JOCKEY_WIDTH + 1));
            chosen_pos = (Position){x, forward ? 0 : GRID_ROWS - JOCKEY_HEIGHT};
            chosen_dir = forward ? DIR_DOWN : DIR_UP;
        }

        if (!cell_rect_overlaps_snake(game, chosen_pos, JOCKEY_WIDTH, JOCKEY_HEIGHT)) {
            break;
        }
    }

    game->boss.pos = chosen_pos;
    game->boss.dash_dir = chosen_dir;
    game->boss.phase = JOCKEY_BOSS_TELEGRAPH;
    game->boss.timer = JOCKEY_TELEGRAPH_SECONDS;
    game->boss.dash_step_timer = 0.0f;
    game_fill_jockey_telegraph(&game->boss);
}

bool game_boss_reward_can_apply(const GameState *game, BossRewardType type) {
    if (type >= BOSS_REWARD_COUNT) {
        return false;
    }

    if (type == BOSS_REWARD_FOUR_CHOICES) {
        return !game->boss_four_choices_unlocked;
    }

    return true;
}

void game_roll_boss_reward_choices(GameState *game) {
    BossRewardType candidates[BOSS_REWARD_COUNT];
    uint32_t candidate_count = 0;

    for (uint32_t i = 0; i < BOSS_REWARD_COUNT; i++) {
        if (game_boss_reward_can_apply(game, (BossRewardType)i)) {
            candidates[candidate_count++] = (BossRewardType)i;
        }
    }

    for (uint32_t i = 0; i < BOSS_REWARD_CHOICES; i++) {
        game->boss_reward_choices[i] = BOSS_REWARD_NONE;
    }

    for (uint32_t choice = 0; choice < BOSS_REWARD_CHOICES && candidate_count > 0; choice++) {
        uint32_t index = random_next(&game->rng_state) % candidate_count;
        game->boss_reward_choices[choice] = candidates[index];
        candidates[index] = candidates[candidate_count - 1];
        candidate_count--;
    }
}

void game_apply_boss_reward(GameState *game, BossRewardType type) {
    if (!game_boss_reward_can_apply(game, type)) {
        return;
    }

    switch (type) {
        case BOSS_REWARD_GOLDEN_SADDLE:
            game->boss_golden_saddle_count++;
            break;
        case BOSS_REWARD_HORSE_MAGNET:
            game->boss_horse_magnet_count++;
            break;
        case BOSS_REWARD_CHAMPION_CHARGE:
            game->boss_champion_charge_count++;
            break;
        case BOSS_REWARD_FOUR_CHOICES:
            game->boss_four_choices_unlocked = true;
            break;
        default:
            break;
    }
}

void game_roll_stat_reward_choices(GameState *game) {
    SnakeStatType candidates[STAT_COUNT];
    uint32_t candidate_count = 0;

    for (uint32_t i = 0; i < STAT_COUNT; i++) {
        candidates[candidate_count++] = (SnakeStatType)i;
    }

    for (uint32_t i = 0; i < CHEST_REWARD_CHOICES; i++) {
        game->stat_reward_choices[i] = STAT_NONE;
    }

    for (uint32_t choice = 0; choice < CHEST_REWARD_CHOICES && candidate_count > 0; choice++) {
        uint32_t index = random_next(&game->rng_state) % candidate_count;
        game->stat_reward_choices[choice] = candidates[index];
        candidates[index] = candidates[candidate_count - 1];
        candidate_count--;
    }
}

void game_apply_snake_stat_reward(GameState *game, SnakeStatType type) {
    switch (type) {
        case STAT_MAX_HP:
            if (game->stats.max_hp_shards < MAX_HP_SHARDS_CAP) {
                game->stats.max_hp_shards++;
                game->stats.hp_shards++;
                if (game->stats.hp_shards > game->stats.max_hp_shards) {
                    game->stats.hp_shards = game->stats.max_hp_shards;
                }
            }
            break;
        case STAT_FRUIT_VALUE:
            game->stats.fruit_value_bonus++;
            break;
        case STAT_CHEST_LUCK:
            game->stats.chest_luck++;
            break;
        case STAT_DAMAGE_GRACE:
            game->stats.damage_grace++;
            break;
        case STAT_FRUIT_SENSE:
            game->stats.fruit_sense++;
            break;
        case STAT_LEAN_BODY:
            if (game->stats.lean_body < LEAN_BODY_MAX) {
                game->stats.lean_body++;
            }
            break;
        default:
            break;
    }
}

void game_apply_stat_reward_choice(GameState *game, uint32_t choice_index) {
    if (choice_index >= CHEST_REWARD_CHOICES) {
        return;
    }

    SnakeStatType choice = game->stat_reward_choices[choice_index];
    if (choice == STAT_NONE || choice >= STAT_COUNT) {
        return;
    }

    game_apply_snake_stat_reward(game, choice);
    for (uint32_t i = 0; i < CHEST_REWARD_CHOICES; i++) {
        game->stat_reward_choices[i] = STAT_NONE;
    }
    game->reward_selection_index = 0;
    game->current_screen = SCREEN_GAMEPLAY;
}

void game_apply_boss_reward_choice(GameState *game, uint32_t choice_index) {
    if (choice_index >= BOSS_REWARD_CHOICES) {
        return;
    }

    BossRewardType choice = game->boss_reward_choices[choice_index];
    if (choice == BOSS_REWARD_NONE) {
        return;
    }

    game_apply_boss_reward(game, choice);
    for (uint32_t i = 0; i < BOSS_REWARD_CHOICES; i++) {
        game->boss_reward_choices[i] = BOSS_REWARD_NONE;
    }
    game->reward_selection_index = 0;
    game->current_screen = SCREEN_GAMEPLAY;
}

void game_start_boss_reward(GameState *game) {
    game_clear_jockey_boss(game);
    game_roll_boss_reward_choices(game);
    game->reward_selection_index = 0;
    game->current_screen = (game->boss_reward_choices[0] != BOSS_REWARD_NONE)
                           ? SCREEN_BOSS_REWARD
                           : SCREEN_GAMEPLAY;
}

void game_begin_jockey_recover(GameState *game) {
    game->boss.phase = JOCKEY_BOSS_RECOVER;
    game->boss.timer = JOCKEY_RECOVER_SECONDS;
    game->boss.dash_step_timer = 0.0f;
}

void game_resolve_jockey_collision(GameState *game) {
    if (!game->boss.active || !position_in_jockey_boss(&game->boss, game->head)) {
        return;
    }

    if (game->boost_timer > 0.0f) {
        if (game->boss.hp > 0) {
            game->boss.hp--;
        }

        if (game->boss.hp == 0) {
            game_start_boss_reward(game);
        } else {
            game_begin_jockey_recover(game);
        }
    } else if (game_spend_rock_medal_charge(game)) {
        game_begin_jockey_recover(game);
    } else if (game_take_damage(game)) {
        game_begin_jockey_recover(game);
    }
}

void game_start_jockey_boss(GameState *game) {
    game->current_screen = SCREEN_BOSS;
    game->boss.active = true;
    game->boss.hp = JOCKEY_MAX_HP;
    game_choose_jockey_attack(game);
}

bool game_maybe_start_jockey_boss(GameState *game) {
    if (game->current_screen != SCREEN_GAMEPLAY ||
        game->fruits_eaten < game->next_boss_fruit_count) {
        return false;
    }

    while (game->next_boss_fruit_count <= game->fruits_eaten) {
        game->next_boss_milestone_index++;
        game->next_boss_fruit_count = boss_milestone_for_index(game->next_boss_milestone_index);
    }

    while (game->next_upgrade_fruit_count <= game->fruits_eaten) {
        game->next_upgrade_fruit_count += UPGRADE_FRUIT_INTERVAL;
    }

    game_start_jockey_boss(game);
    return true;
}

void game_update_jockey_boss(GameState *game, float dt) {
    if (game->current_screen != SCREEN_BOSS || !game->boss.active) {
        return;
    }

    switch (game->boss.phase) {
        case JOCKEY_BOSS_TELEGRAPH:
            game->boss.timer -= dt;
            if (game->boss.timer <= 0.0f) {
                game->boss.phase = JOCKEY_BOSS_DASH;
                game->boss.timer = 0.0f;
                game->boss.dash_step_timer = 0.0f;
            }
            break;

        case JOCKEY_BOSS_DASH:
            game->boss.dash_step_timer += dt;
            while (game->boss.dash_step_timer >= JOCKEY_DASH_STEP_SECONDS) {
                game->boss.dash_step_timer -= JOCKEY_DASH_STEP_SECONDS;
                Position next_pos = position_moved_in_direction(game->boss.pos, game->boss.dash_dir);
                if (!jockey_area_is_inside_board(next_pos)) {
                    game_begin_jockey_recover(game);
                    break;
                }

                game->boss.pos = next_pos;
                game_resolve_jockey_collision(game);
                if (!game_screen_runs_simulation(game->current_screen) || !game->boss.active) {
                    return;
                }
            }
            break;

        case JOCKEY_BOSS_RECOVER:
            game->boss.timer -= dt;
            if (game->boss.timer <= 0.0f) {
                game_choose_jockey_attack(game);
            }
            break;

        default:
            break;
    }
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
            } else if (position_occupied_by_jockey_boss(game, (Position){x, y})) {
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
        uint32_t fruit_sense_chance = game->stats.fruit_sense * FRUIT_SENSE_CHANCE_PER_STAT;
        if (fruit_sense_chance > 75) {
            fruit_sense_chance = 75;
        }
        if (fruit_sense_chance > 0 && (random_next(&game->rng_state) % 100U) < fruit_sense_chance) {
            type = FRUIT_BANANA;
        }
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

void game_try_spawn_lucky_horse(GameState *game) {
    uint32_t chance = game_lucky_horse_chance_percent(game);
    if (chance == 0 || game->collectible.active) {
        return;
    }

    if ((random_next(&game->rng_state) % 100U) < chance) {
        game_spawn_collectible(game);
    }
}

void game_apply_upgrade(GameState *game, UpgradeType type) {
    if (game_upgrade_can_apply(game, type)) {
        game->upgrade_levels[type]++;
    }
}

void game_roll_upgrade_choices(GameState *game) {
    UpgradeType candidates[UPGRADE_COUNT];
    uint32_t candidate_count = 0;
    uint32_t choice_count = game_upgrade_choice_count(game);

    for (uint32_t i = 0; i < UPGRADE_COUNT; i++) {
        if (game_upgrade_can_apply(game, (UpgradeType)i)) {
            candidates[candidate_count++] = (UpgradeType)i;
        }
    }

    for (uint32_t i = 0; i < UPGRADE_MAX_CHOICES_PER_ROLL; i++) {
        game->upgrade_choices[i] = UPGRADE_NONE;
    }

    for (uint32_t choice = 0; choice < choice_count && candidate_count > 0; choice++) {
        uint32_t index = random_next(&game->rng_state) % candidate_count;
        game->upgrade_choices[choice] = candidates[index];
        candidates[index] = candidates[candidate_count - 1];
        candidate_count--;
    }
}

void game_maybe_start_upgrade_choice(GameState *game) {
    if (game->current_screen != SCREEN_GAMEPLAY ||
        game->fruits_eaten < game->next_upgrade_fruit_count) {
        return;
    }

    game_roll_upgrade_choices(game);
    game->next_upgrade_fruit_count += UPGRADE_FRUIT_INTERVAL;
    if (game->upgrade_choices[0] != UPGRADE_NONE) {
        game->reward_selection_index = 0;
        game->current_screen = SCREEN_UPGRADE;
    }
}

void game_apply_upgrade_choice(GameState *game, uint32_t choice_index) {
    if (choice_index >= game_upgrade_choice_count(game)) {
        return;
    }

    UpgradeType choice = game->upgrade_choices[choice_index];
    if (choice == UPGRADE_NONE) {
        return;
    }

    game_apply_upgrade(game, choice);
    for (uint32_t i = 0; i < UPGRADE_MAX_CHOICES_PER_ROLL; i++) {
        game->upgrade_choices[i] = UPGRADE_NONE;
    }
    game->reward_selection_index = 0;
    game->current_screen = SCREEN_GAMEPLAY;
}

void game_note_fruit_eaten(GameState *game) {
    game->fruits_eaten++;

    if (!game->collectible.active &&
        game->fruits_eaten >= horse_milestone_for_index(game->next_horse_milestone_index)) {
        game_spawn_collectible(game);
        game->next_horse_milestone_index++;
    }

    game_try_spawn_lucky_horse(game);
    if (!game_maybe_start_jockey_boss(game)) {
        game_maybe_start_upgrade_choice(game);
    }
}

void game_collect_horse(GameState *game) {
    if (!game->collectible.active || game->collectible.type != COLLECTIBLE_WHITE_HORSE) {
        return;
    }

    game_clear_collectible(game);
    game->horses_collected++;
    game_grow_snake(game, HORSE_GROWTH_AMOUNT);
    game->boost_timer = game_horse_boost_seconds(game);
    game_try_spawn_rock_medal(game);
}

void game_collect_rock_medal(GameState *game) {
    if (!game->collectible.active || game->collectible.type != COLLECTIBLE_ROCK_MEDAL) {
        return;
    }

    game_clear_collectible(game);
    game->rock_medals_collected++;
    game->rock_medal_charges = game_rock_medal_charge_count(game);
    game->rocks_smashed_since_medal = 0;
}

void game_collect_chest(GameState *game) {
    if (!game->collectible.active || game->collectible.type != COLLECTIBLE_CHEST) {
        return;
    }

    game_clear_collectible(game);
    game_roll_stat_reward_choices(game);
    if (game->stat_reward_choices[0] != STAT_NONE) {
        game->reward_selection_index = 0;
        game->current_screen = SCREEN_STAT_REWARD;
    }
}

void game_toggle_pause(GameState *game) {
    if (!game_screen_runs_simulation(game->current_screen)) {
        return;
    }

    game->paused = !game->paused;
    if (!game->paused) {
        game->console_pause_active = false;
    }
}

void game_open_dev_console_pause(GameState *game) {
    if (!game_screen_runs_simulation(game->current_screen)) {
        game->console_pause_active = false;
        return;
    }

    if (!game->paused) {
        game->paused = true;
        game->console_pause_active = true;
    } else {
        game->console_pause_active = false;
    }
}

void game_close_dev_console_pause(GameState *game) {
    if (game->console_pause_active) {
        game->paused = false;
        game->console_pause_active = false;
    }
}

void game_init(GameState *game, uint32_t seed) {
    bool easy_mode = game->easy_mode;
    bool rocks_enabled = game->rocks_enabled;
    if (game->run_seed == 0 && game->rng_state == 0) {
        rocks_enabled = true;
    }

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
    game->next_boss_milestone_index = 0;
    game->next_upgrade_fruit_count = FIRST_UPGRADE_FRUIT_COUNT;
    game->next_boss_fruit_count = boss_milestone_for_index(game->next_boss_milestone_index);
    game->stats.max_hp_shards = HP_SHARDS_PER_HEART;
    game->stats.hp_shards = HP_SHARDS_PER_HEART;
    game->stats.fruit_value_bonus = 0;
    game->stats.chest_luck = 0;
    game->stats.damage_grace = 0;
    game->stats.fruit_sense = 0;
    game->stats.lean_body = 0;
    for (uint32_t i = 0; i < UPGRADE_COUNT; i++) {
        game->upgrade_levels[i] = 0;
    }
    for (uint32_t i = 0; i < UPGRADE_MAX_CHOICES_PER_ROLL; i++) {
        game->upgrade_choices[i] = UPGRADE_NONE;
    }
    for (uint32_t i = 0; i < BOSS_REWARD_CHOICES; i++) {
        game->boss_reward_choices[i] = BOSS_REWARD_NONE;
    }
    for (uint32_t i = 0; i < CHEST_REWARD_CHOICES; i++) {
        game->stat_reward_choices[i] = STAT_NONE;
    }
    game->reward_selection_index = 0;
    game->boss_golden_saddle_count = 0;
    game->boss_horse_magnet_count = 0;
    game->boss_champion_charge_count = 0;
    game->boss_four_choices_unlocked = false;
    game_clear_collectible(game);
    game_clear_jockey_boss(game);
    for (uint32_t i = 0; i < MAX_ROCKS; i++) {
        game_clear_rock(&game->rocks[i]);
    }
    game->update_timer = 0.0f;
    game->time_between_ticks = BASE_TICK_INTERVAL;
    game->boost_timer = 0.0f;
    game->damage_grace_timer = 0.0f;
    game->rock_fall_timer = random_rock_fall_delay(game);
    game->easy_mode = easy_mode;
    game->rocks_enabled = rocks_enabled;
    game->paused = false;
    game->console_pause_active = false;
    
    game_spawn_fruit(game);
}

void game_process_input(GameState *game, const GameInput *input) {
    switch (game->current_screen) {
        case SCREEN_TITLE:
            if (input->easy_mode_toggle_pressed) {
                game->easy_mode = !game->easy_mode;
            }
            if (input->rocks_toggle_pressed) {
                game->rocks_enabled = !game->rocks_enabled;
            }
            if (input->enter_pressed) {
                game->current_screen = SCREEN_GAMEPLAY;
            }
            break;
            
        case SCREEN_GAMEPLAY:
        case SCREEN_BOSS:
            if (input->pause_pressed) {
                game_toggle_pause(game);
            }
            if (!game->paused) {
                for (uint32_t i = 0; i < input->count; i++) {
                    queue_push(game, input->moves[i]);
                }
            }
            break;

        case SCREEN_UPGRADE:
            game_clamp_reward_selection(game);
            if (input->choice_up_pressed) {
                game_move_reward_selection(game, -1);
            }
            if (input->choice_down_pressed) {
                game_move_reward_selection(game, 1);
            }
            if (input->enter_pressed) {
                game_apply_upgrade_choice(game, game->reward_selection_index);
            } else if (input->upgrade_choice_pressed) {
                game_apply_upgrade_choice(game, input->upgrade_choice_index);
            }
            break;

        case SCREEN_BOSS_REWARD:
            game_clamp_reward_selection(game);
            if (input->choice_up_pressed) {
                game_move_reward_selection(game, -1);
            }
            if (input->choice_down_pressed) {
                game_move_reward_selection(game, 1);
            }
            if (input->enter_pressed) {
                game_apply_boss_reward_choice(game, game->reward_selection_index);
            } else if (input->boss_reward_choice_pressed) {
                game_apply_boss_reward_choice(game, input->boss_reward_choice_index);
            }
            break;

        case SCREEN_STAT_REWARD:
            game_clamp_reward_selection(game);
            if (input->choice_up_pressed) {
                game_move_reward_selection(game, -1);
            }
            if (input->choice_down_pressed) {
                game_move_reward_selection(game, 1);
            }
            if (input->enter_pressed) {
                game_apply_stat_reward_choice(game, game->reward_selection_index);
            } else if (input->stat_reward_choice_pressed) {
                game_apply_stat_reward_choice(game, input->stat_reward_choice_index);
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
        if (game->easy_mode) {
            if (game->head.x < 0) game->head.x = GRID_COLS - 1;
            if (game->head.x >= GRID_COLS) game->head.x = 0;
            if (game->head.y < 0) game->head.y = GRID_ROWS - 1;
            if (game->head.y >= GRID_ROWS) game->head.y = 0;
        } else {
            game->current_screen = SCREEN_GAMEOVER;
            return;
        }
    }

    for (uint32_t i = 0; i < game->tail_length; i++) {
        if (game->tail[i].x == game->head.x && game->tail[i].y == game->head.y) {
            game->current_screen = SCREEN_GAMEOVER;
            return;
        }
    }

    game_resolve_rock_collision(game);
    if (!game_screen_runs_simulation(game->current_screen)) {
        return;
    }

    game_resolve_jockey_collision(game);
    if (!game_screen_runs_simulation(game->current_screen)) {
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
            case COLLECTIBLE_CHEST:
                game_collect_chest(game);
                break;
            default:
                break;
        }
    }

    if (!game_screen_runs_simulation(game->current_screen)) {
        return;
    }

    if (game->head.x == game->fruit.pos.x && game->head.y == game->fruit.pos.y) {
        game->score += game_fruit_score_value(game);
        if (game_fruit_should_grow(game)) {
            game_grow_snake(game, 1);
        }
        game_note_fruit_eaten(game);
        
        game_spawn_fruit(game);
    }
}

void draw_apple(Position p) {
    int x = cell_screen_x(p);
    int y = cell_screen_y(p);

    DrawCircle(x + 10, y + 11, 7, RED);
    DrawCircle(x + 7, y + 10, 5, MAROON);
    DrawRectangle(x + 10, y + 3, 2, 5, BROWN);
    DrawEllipse(x + 14, y + 5, 4, 2, GREEN);
}

void draw_orange(Position p) {
    int x = cell_screen_x(p);
    int y = cell_screen_y(p);

    DrawCircle(x + 10, y + 11, 7, ORANGE);
    DrawCircleLines(x + 10, y + 11, 7, GOLD);
    DrawCircle(x + 8, y + 8, 2, YELLOW);
    DrawRectangle(x + 10, y + 4, 2, 3, BROWN);
    DrawEllipse(x + 14, y + 5, 4, 2, LIME);
}

void draw_banana(Position p) {
    int x = cell_screen_x(p);
    int y = cell_screen_y(p);

    DrawEllipse(x + 10, y + 11, 8, 5, GOLD);
    DrawEllipse(x + 10, y + 8, 7, 4, DARKGRAY);
    DrawCircle(x + 5, y + 11, 2, BROWN);
    DrawCircle(x + 16, y + 13, 2, BROWN);
    DrawLine(x + 7, y + 14, x + 15, y + 15, YELLOW);
}

void draw_cherry(Position p) {
    int x = cell_screen_x(p);
    int y = cell_screen_y(p);

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
    int x = cell_screen_x(p);
    int y = cell_screen_y(p);

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
    int x = cell_screen_x(p);
    int y = cell_screen_y(p);
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

void draw_chest(Position p) {
    int x = cell_screen_x(p);
    int y = cell_screen_y(p);

    DrawRectangle(x + 3, y + 7, 15, 11, (Color){105, 61, 32, 255});
    DrawRectangle(x + 4, y + 5, 13, 5, (Color){150, 90, 42, 255});
    DrawRectangle(x + 9, y + 5, 3, 13, GOLD);
    DrawRectangle(x + 3, y + 10, 15, 2, GOLD);
    DrawRectangle(x + 8, y + 11, 5, 5, DARKBROWN);
    DrawCircle(x + 10, y + 13, 1, YELLOW);
    DrawRectangleLines(x + 3, y + 5, 15, 13, Fade(BLACK, 0.65f));
}

void draw_collectible(const Collectible *collectible) {
    if (!collectible->active) {
        return;
    }

    switch (collectible->type) {
        case COLLECTIBLE_WHITE_HORSE: draw_white_horse(collectible->pos); break;
        case COLLECTIBLE_ROCK_MEDAL:  draw_rock_medal(collectible->pos); break;
        case COLLECTIBLE_CHEST:       draw_chest(collectible->pos); break;
        default: break;
    }
}

void draw_rock_warning_cell(Position p, float timer, float warning_duration) {
    int x = cell_screen_x(p);
    int y = cell_screen_y(p);
    if (warning_duration <= 0.0f) {
        warning_duration = ROCK_WARNING_SECONDS;
    }

    float progress = 1.0f - (timer / warning_duration);
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
    int x = cell_screen_x(p);
    int y = cell_screen_y(p);

    DrawCircle(x + 10, y + 11, 8, DARKGRAY);
    DrawCircle(x + 6, y + 14, 5, GRAY);
    DrawCircle(x + 14, y + 14, 5, GRAY);
    DrawTriangle((Vector2){x + 4, y + 8}, (Vector2){x + 10, y + 2}, (Vector2){x + 16, y + 9}, DARKGRAY);
    DrawLine(x + 6, y + 10, x + 11, y + 7, LIGHTGRAY);
    DrawLine(x + 11, y + 7, x + 15, y + 12, LIGHTGRAY);
}

void draw_big_rock_rect(Position top_left, int32_t cell_width, int32_t cell_height) {
    int x = cell_screen_x(top_left);
    int y = cell_screen_y(top_left);
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
                draw_rock_warning_cell(rock->cells[i], rock->timer, rock->warning_duration);
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
    int x = cell_screen_x(p);
    int y = cell_screen_y(p);
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
    int ax = cell_screen_x(a) + GRID_SIZE / 2;
    int ay = cell_screen_y(a) + GRID_SIZE / 2;
    int bx = cell_screen_x(b) + GRID_SIZE / 2;
    int by = cell_screen_y(b) + GRID_SIZE / 2;
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
    int x = cell_screen_x(p) + GRID_SIZE / 2;
    int y = cell_screen_y(p) + GRID_SIZE / 2;
    DrawCircle(x, y, GRID_SIZE / 2 - 3, color);
    DrawCircle(x - 3, y - 3, 2, Fade(WHITE, 0.20f));
}

void draw_snake_tail_tip(Position p, Direction toward_body, Color color) {
    int x = cell_screen_x(p);
    int y = cell_screen_y(p);
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
    int x = cell_screen_x(p);
    int y = cell_screen_y(p);
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

    int cx = cell_screen_x(head) + GRID_SIZE / 2 + dx * 8;
    int cy = cell_screen_y(head) + GRID_SIZE / 2 + dy * 8;

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
            DrawCircle(cell_screen_x(segment) + GRID_SIZE / 2 + 1,
                       cell_screen_y(segment) + GRID_SIZE / 2 + 1,
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

void draw_jockey_telegraph_cell(Position p, float timer) {
    int x = cell_screen_x(p);
    int y = cell_screen_y(p);
    float progress = timer / JOCKEY_TELEGRAPH_SECONDS;
    progress = clamp_float(progress, 0.0f, 1.0f);
    float alpha = 0.12f + (1.0f - progress) * 0.28f;

    DrawRectangle(x + 1, y + 1, GRID_SIZE - 2, GRID_SIZE - 2, Fade(ORANGE, alpha));
    DrawRectangleLines(x + 2, y + 2, GRID_SIZE - 4, GRID_SIZE - 4, Fade(GOLD, alpha + 0.12f));

    if (((p.x + p.y) & 1) == 0) {
        DrawCircle(x + 7, y + 11, 2, Fade(BROWN, 0.70f));
        DrawCircle(x + 13, y + 9, 2, Fade(BROWN, 0.70f));
    } else {
        DrawCircle(x + 8, y + 8, 2, Fade(BROWN, 0.60f));
        DrawCircle(x + 14, y + 12, 2, Fade(BROWN, 0.60f));
    }
}

void draw_jockey_boss_telegraph(const GameState *game) {
    if (!game->boss.active ||
        (game->boss.phase != JOCKEY_BOSS_TELEGRAPH && game->boss.phase != JOCKEY_BOSS_DASH)) {
        return;
    }

    float timer = (game->boss.phase == JOCKEY_BOSS_TELEGRAPH) ? game->boss.timer : 0.0f;
    for (uint32_t i = 0; i < game->boss.telegraph_count; i++) {
        draw_jockey_telegraph_cell(game->boss.telegraph_cells[i], timer);
    }
}

void draw_jockey_boss(const GameState *game) {
    if (!game->boss.active) {
        return;
    }

    int x = cell_screen_x(game->boss.pos);
    int y = cell_screen_y(game->boss.pos);
    int face = (game->boss.dash_dir == DIR_LEFT) ? -1 : 1;
    Color horse_body = (Color){126, 72, 38, 255};
    Color horse_dark = (Color){72, 45, 31, 255};
    Color rider_blue = (Color){35, 90, 180, 255};
    Color skin = (Color){238, 178, 125, 255};

    DrawEllipse(x + 20, y + 37, 17, 3, Fade(BLACK, 0.35f));

    DrawEllipse(x + 20, y + 25, 14, 7, horse_body);
    DrawRectangle(x + 9, y + 19, 22, 9, horse_body);
    DrawLineEx((Vector2){x + 10, y + 28}, (Vector2){x + 6, y + 37}, 4.0f, horse_dark);
    DrawLineEx((Vector2){x + 18, y + 28}, (Vector2){x + 17, y + 37}, 4.0f, horse_body);
    DrawLineEx((Vector2){x + 25, y + 28}, (Vector2){x + 25, y + 37}, 4.0f, horse_dark);
    DrawLineEx((Vector2){x + 31, y + 27}, (Vector2){x + 34, y + 37}, 4.0f, horse_body);

    DrawCircle(x + 20 + face * 13, y + 18, 6, horse_body);
    DrawEllipse(x + 20 + face * 17, y + 20, 5, 3, horse_body);
    DrawCircle(x + 20 + face * 15, y + 16, 1, BLACK);
    DrawLineEx((Vector2){x + 20 - face * 14, y + 22}, (Vector2){x + 20 - face * 19, y + 16}, 3.0f, horse_dark);

    DrawRectangle(x + 16, y + 9, 8, 13, rider_blue);
    DrawCircle(x + 20, y + 7, 5, skin);
    DrawTriangle((Vector2){x + 14, y + 5},
                 (Vector2){x + 25, y + 3},
                 (Vector2){x + 26, y + 8},
                 RED);
    DrawLineEx((Vector2){x + 18, y + 22}, (Vector2){x + 13, y + 29}, 2.0f, DARKBLUE);
    DrawLineEx((Vector2){x + 22, y + 22}, (Vector2){x + 27, y + 29}, 2.0f, DARKBLUE);
}

const char *bg_music_waveform_name(uint32_t waveform) {
    switch (waveform) {
        case BGM_WAVE_SQUARE:   return "square";
        case BGM_WAVE_TRIANGLE: return "triangle";
        case BGM_WAVE_PULSE:    return "pulse";
        default:                return "square";
    }
}

const char *bg_music_bass_pattern_name(uint32_t pattern) {
    switch (pattern) {
        case BGM_BASS_STEADY: return "steady";
        case BGM_BASS_BOUNCE: return "bounce";
        case BGM_BASS_CLIMB:  return "climb";
        default:              return "steady";
    }
}

const char *bg_music_lead_density_name(uint32_t density) {
    switch (density) {
        case BGM_LEAD_OFF:    return "off";
        case BGM_LEAD_SPARSE: return "sparse";
        case BGM_LEAD_MEDIUM: return "medium";
        case BGM_LEAD_BUSY:   return "busy";
        default:              return "off";
    }
}

const char *bg_music_scale_name(uint32_t scale) {
    switch (scale) {
        case BGM_SCALE_MAJOR:      return "major";
        case BGM_SCALE_PENTATONIC: return "pentatonic";
        case BGM_SCALE_LYDIAN:     return "lydian";
        default:                   return "major";
    }
}

const char *bg_music_melody_pattern_name(uint32_t pattern) {
    switch (pattern) {
        case BGM_MELODY_ARP:     return "arp";
        case BGM_MELODY_BOUNCE:  return "bounce";
        case BGM_MELODY_WALK:    return "walk";
        case BGM_MELODY_SPARKLE: return "sparkle";
        default:                 return "arp";
    }
}

const char *bg_music_drum_amount_name(uint32_t amount) {
    switch (amount) {
        case BGM_DRUM_OFF:  return "off";
        case BGM_DRUM_TICK: return "tick";
        case BGM_DRUM_BEAT: return "beat";
        case BGM_DRUM_BUSY: return "busy";
        default:            return "off";
    }
}

float bg_music_param_min(uint32_t param) {
    switch (param) {
        case BGM_PARAM_BPM:          return BGM_BPM_MIN;
        case BGM_PARAM_VOLUME:       return BGM_VOLUME_MIN;
        case BGM_PARAM_ROOT_HZ:      return BGM_ROOT_HZ_MIN;
        case BGM_PARAM_WAVEFORM:     return 0.0f;
        case BGM_PARAM_BASS_PATTERN: return 0.0f;
        case BGM_PARAM_LEAD_DENSITY: return 0.0f;
        case BGM_PARAM_SCALE:        return 0.0f;
        case BGM_PARAM_MELODY_PATTERN: return 0.0f;
        case BGM_PARAM_DRUM_AMOUNT:  return 0.0f;
        case BGM_PARAM_NOISE:        return BGM_NOISE_MIN;
        default:                     return 0.0f;
    }
}

float bg_music_param_max(uint32_t param) {
    switch (param) {
        case BGM_PARAM_BPM:          return BGM_BPM_MAX;
        case BGM_PARAM_VOLUME:       return BGM_VOLUME_MAX;
        case BGM_PARAM_ROOT_HZ:      return BGM_ROOT_HZ_MAX;
        case BGM_PARAM_WAVEFORM:     return (float)(BGM_WAVEFORM_COUNT - 1);
        case BGM_PARAM_BASS_PATTERN: return (float)(BGM_BASS_PATTERN_COUNT - 1);
        case BGM_PARAM_LEAD_DENSITY: return (float)(BGM_LEAD_DENSITY_COUNT - 1);
        case BGM_PARAM_SCALE:        return (float)(BGM_SCALE_COUNT - 1);
        case BGM_PARAM_MELODY_PATTERN: return (float)(BGM_MELODY_PATTERN_COUNT - 1);
        case BGM_PARAM_DRUM_AMOUNT:  return (float)(BGM_DRUM_AMOUNT_COUNT - 1);
        case BGM_PARAM_NOISE:        return BGM_NOISE_MAX;
        default:                     return 1.0f;
    }
}

bool bg_music_param_is_integer(uint32_t param) {
    return param == BGM_PARAM_WAVEFORM ||
           param == BGM_PARAM_BASS_PATTERN ||
           param == BGM_PARAM_LEAD_DENSITY ||
           param == BGM_PARAM_SCALE ||
           param == BGM_PARAM_MELODY_PATTERN ||
           param == BGM_PARAM_DRUM_AMOUNT;
}

const char *bg_music_param_label(uint32_t param) {
    switch (param) {
        case BGM_PARAM_BPM:          return "BPM";
        case BGM_PARAM_VOLUME:       return "Volume";
        case BGM_PARAM_ROOT_HZ:      return "Root Hz";
        case BGM_PARAM_WAVEFORM:     return "Wave";
        case BGM_PARAM_BASS_PATTERN: return "Bass";
        case BGM_PARAM_LEAD_DENSITY: return "Lead";
        case BGM_PARAM_SCALE:        return "Scale";
        case BGM_PARAM_MELODY_PATTERN: return "Melody";
        case BGM_PARAM_DRUM_AMOUNT:  return "Drums";
        case BGM_PARAM_NOISE:        return "Noise";
        default:                     return "Value";
    }
}

float bg_music_get_param_value(const BgMusicSettings *settings, uint32_t param) {
    switch (param) {
        case BGM_PARAM_BPM:          return settings->bpm;
        case BGM_PARAM_VOLUME:       return settings->volume;
        case BGM_PARAM_ROOT_HZ:      return settings->root_hz;
        case BGM_PARAM_WAVEFORM:     return (float)settings->waveform;
        case BGM_PARAM_BASS_PATTERN: return (float)settings->bass_pattern;
        case BGM_PARAM_LEAD_DENSITY: return (float)settings->lead_density;
        case BGM_PARAM_SCALE:        return (float)settings->scale;
        case BGM_PARAM_MELODY_PATTERN: return (float)settings->melody_pattern;
        case BGM_PARAM_DRUM_AMOUNT:  return (float)settings->drum_amount;
        case BGM_PARAM_NOISE:        return settings->noise_amount;
        default:                     return 0.0f;
    }
}

void bg_music_set_param_value(BgMusicSettings *settings, uint32_t param, float value) {
    float min_value = bg_music_param_min(param);
    float max_value = bg_music_param_max(param);
    value = clamp_float(value, min_value, max_value);

    if (bg_music_param_is_integer(param)) {
        value = (float)((uint32_t)(value + 0.5f));
    }

    switch (param) {
        case BGM_PARAM_BPM:          settings->bpm = value; break;
        case BGM_PARAM_VOLUME:       settings->volume = value; break;
        case BGM_PARAM_ROOT_HZ:      settings->root_hz = value; break;
        case BGM_PARAM_WAVEFORM:     settings->waveform = (uint32_t)value; break;
        case BGM_PARAM_BASS_PATTERN: settings->bass_pattern = (uint32_t)value; break;
        case BGM_PARAM_LEAD_DENSITY: settings->lead_density = (uint32_t)value; break;
        case BGM_PARAM_SCALE:        settings->scale = (uint32_t)value; break;
        case BGM_PARAM_MELODY_PATTERN: settings->melody_pattern = (uint32_t)value; break;
        case BGM_PARAM_DRUM_AMOUNT:  settings->drum_amount = (uint32_t)value; break;
        case BGM_PARAM_NOISE:        settings->noise_amount = value; break;
        default: break;
    }

    bg_music_clamp_settings(settings);
}

void bg_music_param_text(const BgMusicSettings *settings, uint32_t param, char *buffer, uint32_t buffer_size) {
    switch (param) {
        case BGM_PARAM_BPM:
            snprintf(buffer, buffer_size, "%.0f", settings->bpm);
            break;
        case BGM_PARAM_VOLUME:
            snprintf(buffer, buffer_size, "%.2f", settings->volume);
            break;
        case BGM_PARAM_ROOT_HZ:
            snprintf(buffer, buffer_size, "%.1f", settings->root_hz);
            break;
        case BGM_PARAM_WAVEFORM:
            snprintf(buffer, buffer_size, "%s", bg_music_waveform_name(settings->waveform));
            break;
        case BGM_PARAM_BASS_PATTERN:
            snprintf(buffer, buffer_size, "%s", bg_music_bass_pattern_name(settings->bass_pattern));
            break;
        case BGM_PARAM_LEAD_DENSITY:
            snprintf(buffer, buffer_size, "%s", bg_music_lead_density_name(settings->lead_density));
            break;
        case BGM_PARAM_SCALE:
            snprintf(buffer, buffer_size, "%s", bg_music_scale_name(settings->scale));
            break;
        case BGM_PARAM_MELODY_PATTERN:
            snprintf(buffer, buffer_size, "%s", bg_music_melody_pattern_name(settings->melody_pattern));
            break;
        case BGM_PARAM_DRUM_AMOUNT:
            snprintf(buffer, buffer_size, "%s", bg_music_drum_amount_name(settings->drum_amount));
            break;
        case BGM_PARAM_NOISE:
            snprintf(buffer, buffer_size, "%.2f", settings->noise_amount);
            break;
        default:
            if (buffer_size > 0) {
                buffer[0] = 0;
            }
            break;
    }
}

Rectangle dev_console_panel_rect(void) {
    return (Rectangle){50.0f, 35.0f, 700.0f, 535.0f};
}

Rectangle dev_console_slider_rect(uint32_t param) {
    Rectangle panel = dev_console_panel_rect();
    return (Rectangle){panel.x + 190.0f, panel.y + 94.0f + (float)param * 32.0f, 340.0f, 8.0f};
}

Rectangle dev_console_apply_button_rect(void) {
    Rectangle panel = dev_console_panel_rect();
    return (Rectangle){panel.x + 40.0f, panel.y + 455.0f, 140.0f, 34.0f};
}

Rectangle dev_console_export_button_rect(void) {
    Rectangle panel = dev_console_panel_rect();
    return (Rectangle){panel.x + 190.0f, panel.y + 455.0f, 140.0f, 34.0f};
}

Rectangle dev_console_random_button_rect(void) {
    Rectangle panel = dev_console_panel_rect();
    return (Rectangle){panel.x + 340.0f, panel.y + 455.0f, 140.0f, 34.0f};
}

Rectangle dev_console_mutate_button_rect(void) {
    Rectangle panel = dev_console_panel_rect();
    return (Rectangle){panel.x + 490.0f, panel.y + 455.0f, 140.0f, 34.0f};
}

void dev_console_set_message(DevConsoleState *console, const char *message) {
    snprintf(console->message, sizeof(console->message), "%s", message);
    console->message_timer = 3.0f;
}

void dev_console_init(DevConsoleState *console, const BgMusicSettings *music_settings) {
    *console = (DevConsoleState){0};
    console->mode = DEV_CONSOLE_COMMAND;
    console->draft_music = *music_settings;
    console->active_param = -1;
    console->tool_rng_state = 0xC0FFEE11U;
}

void dev_console_toggle(DevConsoleState *console, const BgMusicSettings *music_settings) {
    console->open = !console->open;
    if (console->open) {
        console->mode = DEV_CONSOLE_COMMAND;
        console->command_length = 0;
        console->command[0] = 0;
        console->draft_music = *music_settings;
        console->active_param = -1;
        dev_console_set_message(console, "commands: bg.audio, spawn.boss");
    }
}

void dev_console_apply_music(DevConsoleState *console, GameAudioState *audio) {
    bg_music_clamp_settings(&console->draft_music);
    audio->settings = console->draft_music;
    dev_console_set_message(console, "applied to live bgm");
}

void dev_console_export_music(DevConsoleState *console) {
    bg_music_clamp_settings(&console->draft_music);
    if (bg_music_save_settings(&console->draft_music)) {
        dev_console_set_message(console, "exported bg_audio_settings.txt");
    } else {
        dev_console_set_message(console, "export failed");
    }
}

void dev_console_randomize_music(DevConsoleState *console) {
    bg_music_randomize_settings(&console->draft_music, &console->tool_rng_state);
    dev_console_set_message(console, "randomized cute chiptune seed");
}

void dev_console_mutate_music(DevConsoleState *console) {
    bg_music_mutate_settings(&console->draft_music, &console->tool_rng_state);
    dev_console_set_message(console, "mutated current settings");
}

void dev_console_execute_command(DevConsoleState *console, GameAudioState *audio, GameState *game) {
    if (dev_console_command_is_bg_audio(console->command)) {
        console->mode = DEV_CONSOLE_BG_AUDIO;
        console->draft_music = audio->settings;
        dev_console_set_message(console, "bg.audio editor");
    } else if (dev_console_command_is_spawn_boss(console->command)) {
        if (game_screen_runs_simulation(game->current_screen)) {
            game_start_jockey_boss(game);
            dev_console_set_message(console, "spawned jockey boss");
        } else {
            dev_console_set_message(console, "start gameplay before spawn.boss");
        }
    } else {
        console->command_length = 0;
        console->command[0] = 0;
        dev_console_set_message(console, "unknown command");
    }
}

void dev_console_process_key(DevConsoleState *console, GameAudioState *audio, GameState *game, int key) {
    if (!console->open) {
        return;
    }

    if (key == KEY_ESCAPE) {
        console->open = false;
        return;
    }

    if (console->mode == DEV_CONSOLE_COMMAND) {
        if (key == KEY_BACKSPACE && console->command_length > 0) {
            console->command_length--;
            console->command[console->command_length] = 0;
        } else if (key == KEY_ENTER) {
            dev_console_execute_command(console, audio, game);
        }
    } else if (console->mode == DEV_CONSOLE_BG_AUDIO) {
        if (key == KEY_UP) {
            console->active_param--;
            if (console->active_param < 0) {
                console->active_param = BGM_PARAM_COUNT - 1;
            }
        } else if (key == KEY_DOWN || key == KEY_TAB) {
            console->active_param++;
            if (console->active_param >= BGM_PARAM_COUNT) {
                console->active_param = 0;
            }
        } else if (key == KEY_LEFT || key == KEY_RIGHT) {
            if (console->active_param < 0) {
                console->active_param = 0;
            }
            float step = bg_music_param_is_integer((uint32_t)console->active_param) ? 1.0f : 0.02f;
            if (console->active_param == BGM_PARAM_BPM || console->active_param == BGM_PARAM_ROOT_HZ) {
                step = 1.0f;
            }
            float value = bg_music_get_param_value(&console->draft_music, (uint32_t)console->active_param);
            value += (key == KEY_RIGHT) ? step : -step;
            bg_music_set_param_value(&console->draft_music, (uint32_t)console->active_param, value);
        } else if (key == KEY_A || key == KEY_ENTER) {
            dev_console_apply_music(console, audio);
        } else if (key == KEY_E) {
            dev_console_export_music(console);
        } else if (key == KEY_R) {
            dev_console_randomize_music(console);
        } else if (key == KEY_M) {
            dev_console_mutate_music(console);
        }
    }
}

void dev_console_process_text(DevConsoleState *console) {
    if (!console->open || console->mode != DEV_CONSOLE_COMMAND) {
        return;
    }

    int character = 0;
    while ((character = GetCharPressed()) > 0) {
        if (character == '`') {
            continue;
        }

        if (character >= 32 && character <= 126 && console->command_length + 1 < DEV_CONSOLE_COMMAND_MAX) {
            console->command[console->command_length++] = (char)character;
            console->command[console->command_length] = 0;
        }
    }
}

void dev_console_update_slider_from_mouse(DevConsoleState *console, uint32_t param, Vector2 mouse) {
    Rectangle slider = dev_console_slider_rect(param);
    float t = (mouse.x - slider.x) / slider.width;
    t = clamp_float(t, 0.0f, 1.0f);
    float value = bg_music_param_min(param) + t * (bg_music_param_max(param) - bg_music_param_min(param));
    bg_music_set_param_value(&console->draft_music, param, value);
}

void dev_console_update(DevConsoleState *console, GameAudioState *audio, float dt) {
    if (!console->open) {
        return;
    }

    if (console->message_timer > 0.0f) {
        console->message_timer -= dt;
        if (console->message_timer <= 0.0f) {
            console->message_timer = 0.0f;
            console->message[0] = 0;
        }
    }

    dev_console_process_text(console);

    if (console->mode != DEV_CONSOLE_BG_AUDIO) {
        return;
    }

    Vector2 mouse = GetMousePosition();
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        Rectangle apply_button = dev_console_apply_button_rect();
        Rectangle export_button = dev_console_export_button_rect();
        Rectangle random_button = dev_console_random_button_rect();
        Rectangle mutate_button = dev_console_mutate_button_rect();
        if (CheckCollisionPointRec(mouse, apply_button)) {
            dev_console_apply_music(console, audio);
            return;
        }
        if (CheckCollisionPointRec(mouse, export_button)) {
            dev_console_export_music(console);
            return;
        }
        if (CheckCollisionPointRec(mouse, random_button)) {
            dev_console_randomize_music(console);
            return;
        }
        if (CheckCollisionPointRec(mouse, mutate_button)) {
            dev_console_mutate_music(console);
            return;
        }

        for (uint32_t param = 0; param < BGM_PARAM_COUNT; param++) {
            Rectangle slider = dev_console_slider_rect(param);
            Rectangle hit_rect = {slider.x - 8.0f, slider.y - 10.0f, slider.width + 16.0f, 28.0f};
            if (CheckCollisionPointRec(mouse, hit_rect)) {
                console->active_param = (int32_t)param;
                dev_console_update_slider_from_mouse(console, param, mouse);
                break;
            }
        }
    }

    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) &&
        console->active_param >= 0 &&
        console->active_param < BGM_PARAM_COUNT) {
        dev_console_update_slider_from_mouse(console, (uint32_t)console->active_param, mouse);
    }

    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
        console->active_param = -1;
    }
}

void draw_dev_console_button(Rectangle rect, const char *label, Color color) {
    Vector2 mouse = GetMousePosition();
    bool hot = CheckCollisionPointRec(mouse, rect);
    DrawRectangleRec(rect, hot ? Fade(color, 0.78f) : Fade(color, 0.55f));
    DrawRectangleLines((int)rect.x, (int)rect.y, (int)rect.width, (int)rect.height, color);
    int text_width = MeasureText(label, 18);
    DrawText(label,
             (int)(rect.x + rect.width / 2.0f - (float)text_width / 2.0f),
             (int)(rect.y + 8.0f),
             18,
             RAYWHITE);
}

void draw_dev_console_slider(const DevConsoleState *console, uint32_t param) {
    Rectangle slider = dev_console_slider_rect(param);
    float value = bg_music_get_param_value(&console->draft_music, param);
    float min_value = bg_music_param_min(param);
    float max_value = bg_music_param_max(param);
    float t = (value - min_value) / (max_value - min_value);
    t = clamp_float(t, 0.0f, 1.0f);
    int knob_x = (int)(slider.x + t * slider.width);
    int knob_y = (int)(slider.y + slider.height / 2.0f);
    char value_text[32] = {0};
    bg_music_param_text(&console->draft_music, param, value_text, sizeof(value_text));

    Color text_color = (console->active_param == (int32_t)param) ? GOLD : RAYWHITE;
    DrawText(bg_music_param_label(param), (int)(slider.x - 150.0f), (int)(slider.y - 7.0f), 18, text_color);
    DrawRectangleRec(slider, Fade(LIGHTGRAY, 0.35f));
    DrawRectangle((int)slider.x, (int)slider.y, knob_x - (int)slider.x, (int)slider.height, LIME);
    DrawCircle(knob_x, knob_y, 8, text_color);
    DrawText(value_text, (int)(slider.x + slider.width + 20.0f), (int)(slider.y - 7.0f), 18, LIGHTGRAY);
}

void draw_dev_console(const DevConsoleState *console) {
    if (!console->open) {
        return;
    }

    if (console->mode == DEV_CONSOLE_COMMAND) {
        int box_x = 40;
        int box_y = SCREEN_HEIGHT - 105;
        int box_w = SCREEN_WIDTH - 80;
        int box_h = 75;
        DrawRectangle(box_x + 3, box_y + 3, box_w, box_h, Fade(BLACK, 0.35f));
        DrawRectangle(box_x, box_y, box_w, box_h, Fade(BLACK, 0.82f));
        DrawRectangleLines(box_x, box_y, box_w, box_h, Fade(LIME, 0.75f));
        DrawText("dev console", box_x + 15, box_y + 10, 18, LIME);
        DrawText(TextFormat("> %s", console->command), box_x + 15, box_y + 38, 20, RAYWHITE);
        if (console->message[0] != 0) {
            DrawText(console->message, box_x + 410, box_y + 12, 18, LIGHTGRAY);
        }
        return;
    }

    Rectangle panel = dev_console_panel_rect();
    DrawRectangle((int)panel.x + 4, (int)panel.y + 4, (int)panel.width, (int)panel.height, Fade(BLACK, 0.35f));
    DrawRectangleRec(panel, Fade(BLACK, 0.86f));
    DrawRectangleLines((int)panel.x, (int)panel.y, (int)panel.width, (int)panel.height, Fade(SKYBLUE, 0.80f));
    DrawText("bg.audio", (int)panel.x + 25, (int)panel.y + 22, 30, SKYBLUE);
    DrawText("cute arcade / fast chiptune search", (int)panel.x + 25, (int)panel.y + 58, 18, LIGHTGRAY);
    DrawText("drag sliders, A applies, E exports, R randomizes, M mutates", (int)panel.x + 25, (int)panel.y + 78, 16, GRAY);

    for (uint32_t param = 0; param < BGM_PARAM_COUNT; param++) {
        draw_dev_console_slider(console, param);
    }

    draw_dev_console_button(dev_console_apply_button_rect(), "Apply", LIME);
    draw_dev_console_button(dev_console_export_button_rect(), "Export", GOLD);
    draw_dev_console_button(dev_console_random_button_rect(), "Random", SKYBLUE);
    draw_dev_console_button(dev_console_mutate_button_rect(), "Mutate", ORANGE);

    DrawText(BGM_EXPORT_PATH, (int)panel.x + 25, (int)panel.y + 500, 18, LIGHTGRAY);
    DrawText("Esc closes", (int)panel.x + 500, (int)panel.y + 500, 18, GRAY);
    if (console->message[0] != 0) {
        DrawText(console->message, (int)panel.x + 300, (int)panel.y + 420, 18, RAYWHITE);
    }
}

Rectangle title_easy_mode_button_rect(void) {
    return (Rectangle){SCREEN_WIDTH / 2.0f - 230.0f, SCREEN_HEIGHT / 2.0f - 52.0f, 190.0f, 34.0f};
}

Rectangle title_rocks_button_rect(void) {
    return (Rectangle){SCREEN_WIDTH / 2.0f + 40.0f, SCREEN_HEIGHT / 2.0f - 52.0f, 210.0f, 34.0f};
}

Rectangle title_audio_button_rect(void) {
    return (Rectangle){SCREEN_WIDTH - 195.0f, SCREEN_HEIGHT - 50.0f, 165.0f, 34.0f};
}

Rectangle upgrade_choice_rect(uint32_t index) {
    return (Rectangle){SCREEN_WIDTH / 2.0f - 285.0f,
                       168.0f + (float)index * 82.0f,
                       570.0f,
                       66.0f};
}

Rectangle boss_reward_choice_rect(uint32_t index) {
    return (Rectangle){SCREEN_WIDTH / 2.0f - 285.0f,
                       210.0f + (float)index * 88.0f,
                       570.0f,
                       70.0f};
}

Rectangle stat_reward_choice_rect(uint32_t index) {
    return (Rectangle){SCREEN_WIDTH / 2.0f - 285.0f,
                       235.0f + (float)index * 95.0f,
                       570.0f,
                       76.0f};
}

void draw_title_option_button(Rectangle rect, const char *label, bool enabled) {
    Vector2 mouse = GetMousePosition();
    bool hot = CheckCollisionPointRec(mouse, rect);
    Color border = enabled ? LIME : RED;
    Color text_color = enabled ? RAYWHITE : LIGHTGRAY;
    int box_size = 20;
    int box_x = (int)rect.x + 10;
    int box_y = (int)(rect.y + rect.height / 2.0f - (float)box_size / 2.0f);

    DrawRectangleRec(rect, hot ? Fade(BLACK, 0.45f) : Fade(BLACK, 0.24f));
    DrawRectangleLines((int)rect.x, (int)rect.y, (int)rect.width, (int)rect.height, Fade(border, 0.75f));
    DrawRectangleLines(box_x, box_y, box_size, box_size, border);
    if (enabled) {
        DrawLine(box_x + 4, box_y + 11, box_x + 8, box_y + 16, LIME);
        DrawLine(box_x + 8, box_y + 16, box_x + 17, box_y + 4, LIME);
    } else {
        DrawLine(box_x + 4, box_y + 4, box_x + 16, box_y + 16, RED);
        DrawLine(box_x + 16, box_y + 4, box_x + 4, box_y + 16, RED);
    }

    DrawText(label, box_x + 32, (int)rect.y + 8, 18, text_color);
}

void draw_upgrade_choice(const GameState *game, uint32_t index) {
    UpgradeType type = game->upgrade_choices[index];
    if (type == UPGRADE_NONE || type >= UPGRADE_COUNT) {
        return;
    }

    Rectangle rect = upgrade_choice_rect(index);
    Vector2 mouse = GetMousePosition();
    bool hot = CheckCollisionPointRec(mouse, rect);
    bool selected = index == game->reward_selection_index;
    const UpgradeDef *def = &UPGRADE_DEFS[type];
    uint32_t next_level = game->upgrade_levels[type] + 1;

    DrawRectangle((int)rect.x + 3, (int)rect.y + 3, (int)rect.width, (int)rect.height, Fade(BLACK, 0.30f));
    DrawRectangleRec(rect, (hot || selected) ? Fade(DARKGREEN, 0.60f) : Fade(BLACK, 0.72f));
    DrawRectangleLines((int)rect.x, (int)rect.y, (int)rect.width, (int)rect.height, (hot || selected) ? GOLD : LIME);
    if (selected) {
        DrawRectangle((int)rect.x, (int)rect.y, 6, (int)rect.height, GOLD);
    }
    DrawText(TextFormat("%u", index + 1), (int)rect.x + 18, (int)rect.y + 19, 28, selected ? RAYWHITE : GOLD);
    DrawText(def->name, (int)rect.x + 58, (int)rect.y + 10, 23, RAYWHITE);
    DrawText(def->description, (int)rect.x + 58, (int)rect.y + 38, 18, LIGHTGRAY);
    DrawText(TextFormat("%u/%u", next_level, def->max_level),
             (int)(rect.x + rect.width - 68.0f),
             (int)rect.y + 23,
             18,
             GRAY);
}

void draw_boss_reward_choice(const GameState *game, uint32_t index) {
    BossRewardType type = game->boss_reward_choices[index];
    if (type == BOSS_REWARD_NONE || type >= BOSS_REWARD_COUNT) {
        return;
    }

    Rectangle rect = boss_reward_choice_rect(index);
    Vector2 mouse = GetMousePosition();
    bool hot = CheckCollisionPointRec(mouse, rect);
    bool selected = index == game->reward_selection_index;
    const BossRewardDef *def = &BOSS_REWARD_DEFS[type];

    DrawRectangle((int)rect.x + 3, (int)rect.y + 3, (int)rect.width, (int)rect.height, Fade(BLACK, 0.35f));
    DrawRectangleRec(rect, (hot || selected) ? Fade(BROWN, 0.72f) : Fade(BLACK, 0.76f));
    DrawRectangleLines((int)rect.x, (int)rect.y, (int)rect.width, (int)rect.height, (hot || selected) ? GOLD : ORANGE);
    if (selected) {
        DrawRectangle((int)rect.x, (int)rect.y, 6, (int)rect.height, GOLD);
    }
    DrawText(TextFormat("%u", index + 1), (int)rect.x + 18, (int)rect.y + 20, 28, selected ? RAYWHITE : GOLD);
    DrawText(def->name, (int)rect.x + 58, (int)rect.y + 12, 24, RAYWHITE);
    DrawText(def->description, (int)rect.x + 58, (int)rect.y + 42, 18, LIGHTGRAY);
}

void draw_stat_reward_choice(const GameState *game, uint32_t index) {
    SnakeStatType type = game->stat_reward_choices[index];
    if (type == STAT_NONE || type >= STAT_COUNT) {
        return;
    }

    Rectangle rect = stat_reward_choice_rect(index);
    Vector2 mouse = GetMousePosition();
    bool hot = CheckCollisionPointRec(mouse, rect);
    bool selected = index == game->reward_selection_index;
    const SnakeStatDef *def = &SNAKE_STAT_DEFS[type];

    DrawRectangle((int)rect.x + 3, (int)rect.y + 3, (int)rect.width, (int)rect.height, Fade(BLACK, 0.35f));
    DrawRectangleRec(rect, (hot || selected) ? Fade(DARKBLUE, 0.68f) : Fade(BLACK, 0.76f));
    DrawRectangleLines((int)rect.x, (int)rect.y, (int)rect.width, (int)rect.height, (hot || selected) ? SKYBLUE : GOLD);
    if (selected) {
        DrawRectangle((int)rect.x, (int)rect.y, 6, (int)rect.height, SKYBLUE);
    }
    DrawText(TextFormat("%u", index + 1), (int)rect.x + 18, (int)rect.y + 22, 30, selected ? RAYWHITE : GOLD);
    DrawText(def->name, (int)rect.x + 62, (int)rect.y + 14, 25, RAYWHITE);
    DrawText(def->description, (int)rect.x + 62, (int)rect.y + 46, 18, LIGHTGRAY);
}

void draw_game_hud(const GameState *game) {
    DrawRectangle(0, 0, SCREEN_WIDTH, HUD_HEIGHT, Fade(BLACK, 0.55f));
    DrawLine(0, HUD_HEIGHT - 1, SCREEN_WIDTH, HUD_HEIGHT - 1, Fade(LIME, 0.55f));
    DrawText(TextFormat("Score: %u", game->score), 15, 10, 20, WHITE);
    DrawText(TextFormat("HP: %u.%u/%u.%u",
                        game->stats.hp_shards / HP_SHARDS_PER_HEART,
                        (game->stats.hp_shards % HP_SHARDS_PER_HEART) * 2,
                        game->stats.max_hp_shards / HP_SHARDS_PER_HEART,
                        (game->stats.max_hp_shards % HP_SHARDS_PER_HEART) * 2),
             150, 10, 20, RAYWHITE);
    DrawText(TextFormat("Horses: %u", game->horses_collected), 300, 10, 20, WHITE);
    DrawText(TextFormat("Shield: %u", game->rock_medal_charges), 430, 10, 20, WHITE);
    if (game->current_screen == SCREEN_BOSS) {
        DrawText(TextFormat("Jockey HP: %u", game->boss.hp), 540, 10, 20, GOLD);
    }

    uint32_t boost_seconds = game_boost_seconds_remaining(game);
    Color boost_color = boost_seconds == 0 ? GRAY : (boost_seconds <= 3 ? RED : SKYBLUE);
    if (boost_seconds > 0) {
        DrawText(TextFormat("Boost: %us", boost_seconds), 675, 10, 20, boost_color);
    } else {
        DrawText("Boost: --", 675, 10, 20, boost_color);
    }
}

void draw_pause_overlay(void) {
    const char *text = "PAUSED";
    int text_w = MeasureText(text, 42);
    DrawRectangle(0, HUD_HEIGHT, SCREEN_WIDTH, SCREEN_HEIGHT - HUD_HEIGHT, Fade(BLACK, 0.38f));
    DrawText(text, SCREEN_WIDTH / 2 - text_w / 2, HUD_HEIGHT + 245, 42, RAYWHITE);
    DrawText("press P to resume", SCREEN_WIDTH / 2 - 82, HUD_HEIGHT + 292, 20, LIGHTGRAY);
}

void game_update(GameState *game, float dt) {
    if (!game_screen_runs_simulation(game->current_screen)) return;
    if (game->paused) return;

    if (dt > 0.25f) {
        dt = 0.25f;
    }

    game_update_boost_timer(game, dt);
    game_update_damage_grace_timer(game, dt);
    game_update_rocks(game, dt);
    if (!game_screen_runs_simulation(game->current_screen)) {
        return;
    }

    game_update_jockey_boss(game, dt);
    if (!game_screen_runs_simulation(game->current_screen)) {
        return;
    }

    game->update_timer += dt;
    while (game->update_timer >= game_tick_interval(game)) {
        game->update_timer -= game_tick_interval(game);
        game_tick(game);
        
        if (!game_screen_runs_simulation(game->current_screen)) {
            break;
        }
    }
}

void game_draw(const GameState *game, const GameAudioState *audio) {
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
            draw_title_option_button(title_easy_mode_button_rect(), "Easy mode", game->easy_mode);
            draw_title_option_button(title_rocks_button_rect(), "Falling rocks", game->rocks_enabled);
            draw_title_option_button(title_audio_button_rect(), "Audio", !audio->muted);

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
        case SCREEN_BOSS:
            draw_game_hud(game);
            for (int i = 0; i < GRID_COLS; i++) {
                DrawLine(i * GRID_SIZE, HUD_HEIGHT, i * GRID_SIZE, SCREEN_HEIGHT, GRAY);
            }
            for (int i = 0; i < GRID_ROWS; i++) {
                int y = HUD_HEIGHT + i * GRID_SIZE;
                DrawLine(0, y, SCREEN_WIDTH, y, GRAY);
            }

            draw_jockey_boss_telegraph(game);
            draw_rocks(game);

            draw_fruit(&game->fruit);
            draw_collectible(&game->collectible);
            draw_jockey_boss(game);

            draw_snake(game);

            if (game->paused) {
                draw_pause_overlay();
            }
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

        case SCREEN_UPGRADE: {
            const char *title = "Choose an upgrade";
            const char *sub = "fruit milestone reached";
            int t_w = MeasureText(title, 38);
            int s_w = MeasureText(sub, 20);
            uint32_t choice_count = game_upgrade_choice_count(game);

            DrawText(title, SCREEN_WIDTH / 2 - t_w / 2, 105, 38, GOLD);
            DrawText(sub, SCREEN_WIDTH / 2 - s_w / 2, 150, 20, LIGHTGRAY);
            for (uint32_t i = 0; i < choice_count; i++) {
                draw_upgrade_choice(game, i);
            }
            DrawText(choice_count > UPGRADE_CHOICES_PER_ROLL
                     ? "up/down + enter, or press 1 / 2 / 3 / 4"
                     : "up/down + enter, or press 1 / 2 / 3",
                     choice_count > UPGRADE_CHOICES_PER_ROLL ? SCREEN_WIDTH / 2 - 190 : SCREEN_WIDTH / 2 - 162,
                     520,
                     20,
                     GRAY);
            break;
        }

        case SCREEN_BOSS_REWARD: {
            const char *title = "Jockey beaten";
            const char *sub = "choose a boss reward";
            int t_w = MeasureText(title, 38);
            int s_w = MeasureText(sub, 20);

            DrawText(title, SCREEN_WIDTH / 2 - t_w / 2, 105, 38, GOLD);
            DrawText(sub, SCREEN_WIDTH / 2 - s_w / 2, 150, 20, LIGHTGRAY);
            for (uint32_t i = 0; i < BOSS_REWARD_CHOICES; i++) {
                draw_boss_reward_choice(game, i);
            }
            DrawText("up/down + enter, or press 1 / 2 / 3", SCREEN_WIDTH / 2 - 162, 510, 20, GRAY);
            break;
        }

        case SCREEN_STAT_REWARD: {
            const char *title = "Chest found";
            const char *sub = "choose a snake stat";
            int t_w = MeasureText(title, 38);
            int s_w = MeasureText(sub, 20);

            DrawText(title, SCREEN_WIDTH / 2 - t_w / 2, 125, 38, GOLD);
            DrawText(sub, SCREEN_WIDTH / 2 - s_w / 2, 170, 20, LIGHTGRAY);
            for (uint32_t i = 0; i < CHEST_REWARD_CHOICES; i++) {
                draw_stat_reward_choice(game, i);
            }
            DrawText("up/down + enter, or press 1 / 2", SCREEN_WIDTH / 2 - 140, 475, 20, GRAY);
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
    GameAudioState audio = {0};
    game_audio_init(&audio);
    DevConsoleState dev_console = {0};
    dev_console_init(&dev_console, &audio.settings);
    TraceLog(LOG_INFO, "Initialized game with seed: %u", game.run_seed);

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        game_audio_update(&audio);

        // 1. Gather input from Raylib's key queue
        GameInput input = {0};
        int key;
        while ((key = GetKeyPressed()) != 0) {
            if (key == KEY_GRAVE) {
                bool was_open = dev_console.open;
                dev_console_toggle(&dev_console, &audio.settings);
                if (was_open && !dev_console.open) {
                    game_close_dev_console_pause(&game);
                } else if (!was_open && dev_console.open) {
                    game_open_dev_console_pause(&game);
                }
                continue;
            }

            if (dev_console.open) {
                bool was_open = dev_console.open;
                dev_console_process_key(&dev_console, &audio, &game, key);
                if (was_open && !dev_console.open) {
                    game_close_dev_console_pause(&game);
                }
                continue;
            }

            switch (key) {
                case KEY_ONE:
                    if (game.current_screen == SCREEN_UPGRADE) {
                        input.upgrade_choice_pressed = true;
                        input.upgrade_choice_index = 0;
                    } else if (game.current_screen == SCREEN_BOSS_REWARD) {
                        input.boss_reward_choice_pressed = true;
                        input.boss_reward_choice_index = 0;
                    } else if (game.current_screen == SCREEN_STAT_REWARD) {
                        input.stat_reward_choice_pressed = true;
                        input.stat_reward_choice_index = 0;
                    }
                    break;
                case KEY_TWO:
                    if (game.current_screen == SCREEN_UPGRADE) {
                        input.upgrade_choice_pressed = true;
                        input.upgrade_choice_index = 1;
                    } else if (game.current_screen == SCREEN_BOSS_REWARD) {
                        input.boss_reward_choice_pressed = true;
                        input.boss_reward_choice_index = 1;
                    } else if (game.current_screen == SCREEN_STAT_REWARD) {
                        input.stat_reward_choice_pressed = true;
                        input.stat_reward_choice_index = 1;
                    }
                    break;
                case KEY_THREE:
                    if (game.current_screen == SCREEN_UPGRADE) {
                        input.upgrade_choice_pressed = true;
                        input.upgrade_choice_index = 2;
                    } else if (game.current_screen == SCREEN_BOSS_REWARD) {
                        input.boss_reward_choice_pressed = true;
                        input.boss_reward_choice_index = 2;
                    }
                    break;
                case KEY_FOUR:
                    if (game.current_screen == SCREEN_UPGRADE &&
                        game_upgrade_choice_count(&game) > UPGRADE_CHOICES_PER_ROLL) {
                        input.upgrade_choice_pressed = true;
                        input.upgrade_choice_index = 3;
                    }
                    break;
                case KEY_LEFT:  if (input.count < INPUT_QUEUE_MAX) input.moves[input.count++] = DIR_LEFT; break;
                case KEY_RIGHT: if (input.count < INPUT_QUEUE_MAX) input.moves[input.count++] = DIR_RIGHT; break;
                case KEY_UP:
                    if (game.current_screen == SCREEN_UPGRADE ||
                        game.current_screen == SCREEN_BOSS_REWARD ||
                        game.current_screen == SCREEN_STAT_REWARD) {
                        input.choice_up_pressed = true;
                    } else if (input.count < INPUT_QUEUE_MAX) {
                        input.moves[input.count++] = DIR_UP;
                    }
                    break;
                case KEY_DOWN:
                    if (game.current_screen == SCREEN_UPGRADE ||
                        game.current_screen == SCREEN_BOSS_REWARD ||
                        game.current_screen == SCREEN_STAT_REWARD) {
                        input.choice_down_pressed = true;
                    } else if (input.count < INPUT_QUEUE_MAX) {
                        input.moves[input.count++] = DIR_DOWN;
                    }
                    break;
                case KEY_R:     input.r_pressed = true; break;
                case KEY_P:     input.pause_pressed = true; break;
                case KEY_ENTER: input.enter_pressed = true; break;
            }
        }
        if (!dev_console.open &&
            game.current_screen == SCREEN_TITLE &&
            IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 mouse = GetMousePosition();
            if (CheckCollisionPointRec(mouse, title_easy_mode_button_rect())) {
                input.easy_mode_toggle_pressed = true;
            } else if (CheckCollisionPointRec(mouse, title_rocks_button_rect())) {
                input.rocks_toggle_pressed = true;
            } else if (CheckCollisionPointRec(mouse, title_audio_button_rect())) {
                game_audio_toggle_muted(&audio);
            }
        }
        if (!dev_console.open &&
            game.current_screen == SCREEN_UPGRADE &&
            IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 mouse = GetMousePosition();
            uint32_t choice_count = game_upgrade_choice_count(&game);
            for (uint32_t i = 0; i < choice_count; i++) {
                if (CheckCollisionPointRec(mouse, upgrade_choice_rect(i))) {
                    input.upgrade_choice_pressed = true;
                    input.upgrade_choice_index = i;
                    break;
                }
            }
        }
        if (!dev_console.open &&
            game.current_screen == SCREEN_BOSS_REWARD &&
            IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 mouse = GetMousePosition();
            for (uint32_t i = 0; i < BOSS_REWARD_CHOICES; i++) {
                if (CheckCollisionPointRec(mouse, boss_reward_choice_rect(i))) {
                    input.boss_reward_choice_pressed = true;
                    input.boss_reward_choice_index = i;
                    break;
                }
            }
        }
        if (!dev_console.open &&
            game.current_screen == SCREEN_STAT_REWARD &&
            IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 mouse = GetMousePosition();
            for (uint32_t i = 0; i < CHEST_REWARD_CHOICES; i++) {
                if (CheckCollisionPointRec(mouse, stat_reward_choice_rect(i))) {
                    input.stat_reward_choice_pressed = true;
                    input.stat_reward_choice_index = i;
                    break;
                }
            }
        }
        dev_console_update(&dev_console, &audio, dt);

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
        game_draw(&game, &audio);
        draw_dev_console(&dev_console);
        EndDrawing();
    }

    game_audio_shutdown(&audio);
    CloseWindow();
    return 0;
}

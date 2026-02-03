#ifdef __INTELLISENSE__
#undef __SDCC_mcs51
#endif

#include <at89x52.h>
#include <lint.h>
#include <stdbool.h>
#include <stdint.h>

// helper macro to convert integer to fixed point (6 bits integer and 2 bits fractional)
#define INT_TO_Q6_2(x) (x << 2)

// helper macro to convert fixed point (6 bits integer and 2 bits fractional) to integer
#define Q6_2_TO_INT(x) (x >> 2)

#define BIT0 0x01
#define BIT1 0x02
#define BIT2 0x04
#define BIT3 0x08
#define BIT4 0x10
#define BIT5 0x20
#define BIT6 0x40
#define BIT7 0x80

// PINS for NES controller
#define NES_CLK P1_2
#define NES_LATCH P1_0
#define NES_DATA P1_1

// Debug LED
#define LED P3_5

// Button bitmapping for NES controller
#define BTN_RIGHT BIT0
#define BTN_LEFT BIT1
#define BTN_DOWN BIT2
#define BTN_UP BIT3
#define BTN_START BIT4
#define BTN_SELECT BIT5
#define BTN_B BIT6
#define BTN_A BIT7

// Colors
#define BLK 0x00
#define WHT 0x3F
#define RED 0x30
#define GRN 0x0C
#define BLU 0x03
#define YEL 0x3C
#define MAG 0x33
#define CYN 0x0F
#define ORG 0x38
#define RED_LIGHT 0x10
#define GRN_LIGHT 0x04

// LED Matrix Pixel Size
#define WIDTH 32
#define HEIGHT 16

// Pixel Buffer for Matrix LED Display
__xdata uint8_t framebuffer[HEIGHT][WIDTH];

// Pointer to external LED Matrix
// highest address is 16*7+8 = 120, so 8-bit addressing mode is sufficient
__pdata uint8_t* matrix = (__pdata uint8_t*) 0x00;

// tick counter for game code timings (one tick every 10 ms)
uint16_t tick;

// current and previous controller state
uint8_t last_controller = 0;
uint8_t controller      = 0;

// checks if a button is currently pressed
#define PRESSED(btn) (controller & (btn))

// checks if a button was pressed once (rising edge)
#define JUST_PRESSED(btn) ((controller & btn) && !(last_controller & btn))

// checks if two areas intersect anywhere
#define INTERSECT_AREA(x1, y1, w1, h1, x2, y2, w2, h2) ((y2) + (h2) >= (y1) && (y2) < (y1) + (h1) && (x2) + (w2) >= (x1) && (x2) < (x1) + (w1))

// checks if a point is within an area
#define INTERSECT_POINT(x1, y1, w1, h1, x2, y2) ((x2) >= (x1) && (x2) < (x1) + (w1) && (y2) >= (y1) && (y2) < (y1) + (h1))

__code uint8_t font_A[] = {
    0b01100000,
    0b10010000,
    0b11110000,
    0b10010000,
    0b10010000,
};

__code uint8_t font_E[] = {
    0b11110000,
    0b10000000,
    0b11100000,
    0b10000000,
    0b11110000,
};

__code uint8_t font_G[] = {
    0b01110000,
    0b10000000,
    0b10110000,
    0b10010000,
    0b01110000,
};

__code uint8_t font_M[] = {
    0b10001000,
    0b11011000,
    0b10101000,
    0b10001000,
    0b10001000,
};

__code uint8_t font_N[] = {
    0b10001000,
    0b11001000,
    0b10101000,
    0b10011000,
    0b10001000,
};

__code uint8_t font_O[] = {
    0b01100000,
    0b10010000,
    0b10010000,
    0b10010000,
    0b01100000,
};

__code uint8_t font_R[] = {
    0b11100000,
    0b10010000,
    0b11100000,
    0b10100000,
    0b10010000,
};

__code uint8_t font_S[] = {
    0b01110000,
    0b10000000,
    0b01100000,
    0b00010000,
    0b11100000,
};

__code uint8_t font_T[] = {
    0b11100000,
    0b01000000,
    0b01000000,
    0b01000000,
    0b01000000,
};

__code uint8_t font_V[] = {
    0b10100000,
    0b10100000,
    0b10100000,
    0b01000000,
    0b01000000,
};

__code uint8_t font_W[] = {
    0b10101000,
    0b10101000,
    0b10101000,
    0b10101000,
    0b01010000,
};

// renders a single letter. does not check for framebuffer array bounds.
void render_font(__code uint8_t* bitmap, uint8_t x0, uint8_t y, uint8_t color) {
    uint8_t height = 5;
    do {
        uint8_t bitmask = *bitmap++;

        __xdata uint8_t* f_ptr = framebuffer[y++] + x0;

        do {
            if (bitmask & BIT7) {
                *f_ptr = color;
            }
            bitmask <<= 1;
            f_ptr++;
        } while (bitmask != 0);
    } while (--height > 0);  // djnz loop
}

typedef struct {
    uint8_t x;
    uint8_t y;
    uint8_t z;
    uint8_t a;
} rng_t;

rng_t rng = {0, 0, 0, 1};

void seed(uint8_t seed) {
    rng.x = seed;
    rng.y = seed ^ 0xA5;
    rng.z = (seed << 1) | (seed >> 7);
    rng.a = seed | BIT0;  // ensure non-zero
}

// Pseudo Random Number Generator
// Source: github.com/edrosten/8bit_rng
uint8_t rand() {
    uint8_t t = rng.x ^ (rng.x << 4);

    rng.x = rng.y;
    rng.y = rng.z;
    rng.z = rng.a;

    rng.a = rng.z ^ t ^ (rng.z >> 1) ^ (t << 1);

    return rng.a;
}

typedef enum {
    GAME_STATE_DRAW_START,     // intermediate state used to draw "Start Game" once
    GAME_STATE_START,          // waiting for user input "Start" button
    GAME_STATE_INGAME,         // main game loop
    GAME_STATE_DRAW_GAMEOVER,  // intermediate state used to draw "Game Over" once
    GAME_STATE_GAMEOVER,       // waiting for user input "Start" button
    GAME_STATE_DRAW_WON,       // intermediate state used to draw "Game Won" once
    GAME_STATE_WON             // waiting for user input "Start" button
} game_state_t;

game_state_t state = GAME_STATE_DRAW_START;

__code uint8_t asteroid_bitmap_0[] = {
    0b11000000,
    0b11000000,
};

__code uint8_t asteroid_bitmap_1[] = {
    0b00100000,
    0b01100000,
    0b11100000,
};

__code uint8_t asteroid_bitmap_2[] = {
    0b00110000,
    0b01110000,
    0b11100000,
    0b01000000,
};

__code uint8_t asteroid_bitmap_3[] = {
    0b01110000,
    0b11111000,
    0b11111100,
    0b11111100,
    0b00110000,
};

typedef struct {
    int8_t          w;
    int8_t          h;
    uint8_t         hp;
    uint8_t         score;
    __code uint8_t* bitmap;
} asteroid_type_t;

// these are constant, but stored in RAM for fast access
__idata asteroid_type_t asteroid_types[] = {
    {
        .w      = 2,
        .h      = 2,
        .hp     = 1,
        .score  = 1,
        .bitmap = asteroid_bitmap_0,
    },
    {
        .w      = 3,
        .h      = 3,
        .hp     = 1,
        .score  = 1,
        .bitmap = asteroid_bitmap_1,
    },
    {
        .w      = 4,
        .h      = 4,
        .hp     = 2,
        .score  = 1,
        .bitmap = asteroid_bitmap_2,
    },
    {
        .w      = 6,
        .h      = 5,
        .hp     = 3,
        .score  = 2,
        .bitmap = asteroid_bitmap_3,
    },
};

typedef struct {
    __idata asteroid_type_t* type;

    // coordinates stored as fixed point. 6 bits integer, 2 bits fractional
    int8_t x;
    int8_t y;
    int8_t vx;
    int8_t vy;

    bool    visible;
    uint8_t hp;
} asteroid_t;

#define ASTEROID_COUNT 8
__idata asteroid_t asteroids[ASTEROID_COUNT];

#define ASTEROID_SPAWN_DELAY_INIT 300
#define ASTEROID_SPAWN_DELAY_MIN 40
#define ASTEROID_SPAWN_DELAY_REDUCE 5

uint16_t asteroid_last_spawn_tick = 0;
uint16_t asteroid_spawn_delay     = ASTEROID_SPAWN_DELAY_INIT;

uint8_t asteroid_get_color(__idata asteroid_t* asteroid) {
    switch (asteroid->hp) {
        case 1:
            return ORG;
        case 2:
            return YEL;
        default:
            return BLU;
    }
}

/**
 * Renders the asteroid based on the asteroid's type.
 * Checks framebuffer array bounds and renders asteroids partially at the edges.
 *
 * Uses it's own register bank, because this function is complex and called often
 *
 * @return true if any pixels have been rendered
 */
bool asteroid_render(__idata asteroid_t* asteroid, uint8_t color) __using(2) {
    __idata asteroid_type_t* type = asteroid->type;

    // convert fixed point values to just integer parts
    int8_t x = Q6_2_TO_INT(asteroid->x);
    int8_t y = Q6_2_TO_INT(asteroid->y);

    // asteroid bitmap area bounds on screen
    int8_t x_start = x;
    int8_t x_end   = x + type->w;
    int8_t y_start = y;
    int8_t y_end   = y + type->h;

    // constrain asteroid area to valid framebuffer bounds
    if (x_start < 0) x_start = 0;
    if (y_start < 1) y_start = 1;  // first pixel row is reserved to health- and scorebar
    if (x_end > WIDTH) x_end = WIDTH;
    if (y_end > HEIGHT) y_end = HEIGHT;

    if (x_start >= x_end || y_start >= y_end) {
        // asteroid is not visible on screen
        return false;
    }

    __code uint8_t* bitmap_ptr = type->bitmap;

    // clip top
    bitmap_ptr += y_start - y;

    // clip horizontal
    x = x_start - x;

    // loop variables
    x_end -= x_start;
    y_end -= y_start;

    do {
        uint8_t bitmask = *bitmap_ptr++;

        // clip left
        if (x) {
            bitmask <<= x;
        }

        __xdata uint8_t* f_ptr = framebuffer[y_start++] + x_start;

        uint8_t bits = x_end;
        do {
            if (bitmask & BIT7) {
                *f_ptr = color;
            }
            bitmask <<= 1;
            f_ptr++;
        } while (--bits > 0);  // djnz loop
    } while (--y_end > 0);  // djnz loop

    return true;
}

void asteroid_update(__idata asteroid_t* asteroid) {
    if (!asteroid->visible) return;

    asteroid_render(asteroid, BLK);

    asteroid->x += asteroid->vx;
    asteroid->y += asteroid->vy;

    if (!asteroid_render(asteroid, asteroid_get_color(asteroid))) {
        asteroid->visible = false;
    }
}

/**
 * configures/generates a new asteroid based on random numbers
 */
void asteroid_generate(__idata asteroid_t* asteroid) {
    uint8_t r  = rand();
    uint8_t r2 = rand();

    asteroid->type = asteroid_types + (r & (BIT0 | BIT1));

    if (r & BIT2) {
        // spawn at right edge of screen
        asteroid->x = INT_TO_Q6_2(31);
        asteroid->y = INT_TO_Q6_2((r2 & 0xF) - (asteroid->type->h >> 2));

        // choose speed
        asteroid->vx = (r & BIT3) ? -4 : -3;

        // choose direction
        if (r & BIT4) {
            // moves straight left
            asteroid->vy = 0;
        } else {
            // moves slightly diagonal
            asteroid->vy = (r & BIT5) ? -1 : 1;
        }
    } else {
        // spawn at top or bottom edge of screen
        asteroid->x = INT_TO_Q6_2(15 + (r2 & 0x0F));

        asteroid->vx = (r & BIT3) ? -3 : -2;

        if (r & BIT4) {
            // spawn at top edge of screen, velocity must be downwards (positive)
            asteroid->y  = INT_TO_Q6_2(2 - asteroid->type->h);
            asteroid->vy = (r & BIT5) ? 1 : 2;
        } else {
            // spawn at bottom egde of screen. velocity must be upwards (negative)
            asteroid->y  = INT_TO_Q6_2(15);
            asteroid->vy = (r & BIT5) ? -1 : -2;
        }
    }

    asteroid->hp = asteroid->type->hp;
}

void asteroid_spawn() {
    if ((tick - asteroid_last_spawn_tick) > asteroid_spawn_delay) {
        // find unused asteroid using loop to keep visible asteroids in the
        // front of the array. this allows for early return in collision check loops
        __idata asteroid_t* asteroid_ptr = asteroids;
        do {
            if (!asteroid_ptr->visible) {
                asteroid_generate(asteroid_ptr);
                asteroid_ptr->visible = true;
                break;
            }
            asteroid_ptr++;
        } while (asteroid_ptr < asteroids + ASTEROID_COUNT);

        asteroid_last_spawn_tick = tick;

        // decrease spawn rate of asteroid to increase difficulity
        if (asteroid_spawn_delay > ASTEROID_SPAWN_DELAY_MIN) {
            asteroid_spawn_delay -= ASTEROID_SPAWN_DELAY_REDUCE;
        }
    }
}

typedef struct {
    uint8_t x;
    uint8_t y;
    uint8_t hp;
    uint8_t score;
} spaceship_t;

spaceship_t spaceship;

/**
 * Checks and handles if the spaceship was hit by a given asteroid.
 */
bool spaceship_hit(__idata asteroid_t* asteroid) {
    if (!asteroid->visible) {
        return false;
    }

    __idata asteroid_type_t* type = asteroid->type;

    if (INTERSECT_AREA(spaceship.x, spaceship.y, 2, 3, asteroid->x >> 2, asteroid->y >> 2, type->w, type->h)) {
        if (spaceship.hp <= 1) {
            state = GAME_STATE_DRAW_GAMEOVER;
        }
        spaceship.hp -= 1;
        return true;
    }

    return false;
}

void spaceship_clear() {
    // check if pixel is white, because it might already be drawn by an asteroid
    if (framebuffer[spaceship.y][spaceship.x] == WHT) {
        framebuffer[spaceship.y][spaceship.x] = BLK;
    }
    if (framebuffer[spaceship.y][spaceship.x + 1] == WHT) {
        framebuffer[spaceship.y][spaceship.x + 1] = BLK;
    }
    if (framebuffer[spaceship.y - 1][spaceship.x] == WHT) {
        framebuffer[spaceship.y - 1][spaceship.x] = BLK;
    }
    if (framebuffer[spaceship.y + 1][spaceship.x] == WHT) {
        framebuffer[spaceship.y + 1][spaceship.x] = BLK;
    }
}

void spaceship_render() {
    framebuffer[spaceship.y][spaceship.x]     = WHT;
    framebuffer[spaceship.y][spaceship.x + 1] = WHT;
    framebuffer[spaceship.y - 1][spaceship.x] = WHT;
    framebuffer[spaceship.y + 1][spaceship.x] = WHT;
}

/**
 * @return true if the spaceship has been hit
 */
bool spaceship_update() {
    spaceship_clear();

    if (PRESSED(BTN_UP) && spaceship.y >= 3) {
        spaceship.y -= 1;
    }

    if (PRESSED(BTN_DOWN) && spaceship.y <= 13) {
        spaceship.y += 1;
    }

    if (PRESSED(BTN_RIGHT) && spaceship.x <= 29) {
        spaceship.x += 1;
    }

    if (PRESSED(BTN_LEFT) && spaceship.x >= 1) {
        spaceship.x -= 1;
    }

    // asteroids are rendered before spaceship, so we can check pixels to detect collision
    bool hit = framebuffer[spaceship.y][spaceship.x] != BLK ||
               framebuffer[spaceship.y][spaceship.x + 1] != BLK ||
               framebuffer[spaceship.y - 1][spaceship.x] != BLK ||
               framebuffer[spaceship.y + 1][spaceship.x] != BLK;

    spaceship_render();

    if (hit) {
        // we know we hit something, but we need to find the exact asteroid we hit
        __idata asteroid_t* asteroid_ptr = asteroids;
        do {
            if (spaceship_hit(asteroid_ptr)) {
                break;
            }
            asteroid_ptr++;
        } while (asteroid_ptr < asteroids + ASTEROID_COUNT);
    }

    return hit;
}

typedef struct {
    int8_t x;
    int8_t y;
    bool   visible;
} laser_t;

#define LASER_COUNT 8

__idata laser_t  lasers[LASER_COUNT];
__idata laser_t* laser_next = lasers;

void laser_spawn() {
    if (JUST_PRESSED(BTN_A)) {
        laser_next->x       = spaceship.x + 2;
        laser_next->y       = spaceship.y;
        laser_next->visible = true;

        laser_next++;
        if (laser_next >= lasers + LASER_COUNT) {
            laser_next = lasers;
        }
    }
}

// checks and handles a laser hitting an asteroid
bool laser_hit(__idata asteroid_t* asteroid, __idata laser_t* laser) {
    if (!asteroid->visible) return false;

    __idata asteroid_type_t* type = asteroid->type;

    if (INTERSECT_POINT(Q6_2_TO_INT(asteroid->x), Q6_2_TO_INT(asteroid->y), type->w, type->h, laser->x, laser->y)) {
        asteroid->hp--;

        if (asteroid->hp == 0) {
            asteroid->visible = false;
            asteroid_render(asteroid, BLK);
            spaceship.score += type->score;
        }
        return true;
    }

    return false;
}

void laser_update(__idata laser_t* laser) {
    if (!laser->visible) return;

    // clear previous pixel, don't clear previously drawn asteroids
    if (framebuffer[laser->y][laser->x] == RED) {
        framebuffer[laser->y][laser->x] = BLK;
    }

    // update laser state
    laser->x++;
    if (laser->x >= 32) {
        laser->visible = 0;
        return;
    }

    // check for collision with asteroid based on pixels
    if (framebuffer[laser->y][laser->x] != BLK) {
        // we know we hit something, but we need to find the exact asteroid we hit
        __idata asteroid_t* asteroid_ptr = asteroids;
        do {
            if (laser_hit(asteroid_ptr, laser)) {
                break;
            }
            asteroid_ptr++;
        } while (asteroid_ptr < asteroids + ASTEROID_COUNT);

        if (spaceship.score >= 16) {
            state = GAME_STATE_DRAW_WON;
        }

        laser->visible = false;
        return;
    }

    // render laser
    framebuffer[laser->y][laser->x] = RED;
}

void healthbar_update() {
    for (uint8_t i = 0; i < 16; i++) {
        framebuffer[0][i] = (spaceship.hp > i) ? RED : BLK;
    }
}

void score_update() {
    for (uint8_t i = 0; i < 16; i++) {
        framebuffer[0][31 - i] = (spaceship.score > i) ? GRN : BLK;
    }
}

void game_reset() {
    spaceship.x     = 3;
    spaceship.y     = 8;
    spaceship.hp    = 16;
    spaceship.score = 0;

    asteroid_spawn_delay = ASTEROID_SPAWN_DELAY_INIT;

    for (uint8_t i = 0; i < ASTEROID_COUNT; i++) {
        asteroids[i].visible = false;
    }

    for (uint8_t i = 0; i < LASER_COUNT; i++) {
        lasers[i].visible = false;
    }
}

/**
 * Reads NES controller inputs
 * @return bitmask of pressed buttons
 */
uint8_t read_controller() {
    uint8_t value = 0;

    // Pulse Latch to sample buttons states in shift register
    NES_LATCH = 1;
    NES_LATCH = 0;

    // Read first button (active low)
    value = (NES_DATA == 0);

    // Loop remaining 7 buttons
    uint8_t i = 7;
    do {
        // Pulse clock to shift the next bit out
        NES_CLK = 1;
        NES_CLK = 0;

        // Make room for the next bit
        value <<= 1;

        // Read next button and store bit (active low)
        value |= (NES_DATA == 0);
    } while (--i > 0);  // djnz loop

    return value;
}

void clear_screen() {
    __xdata uint8_t* f_ptr = framebuffer;

    uint8_t x = WIDTH;
    do {
        uint8_t y = HEIGHT;
        do {
            *f_ptr++ = BLK;
        } while (--y > 0);  // djnz loop
    } while (--x > 0);  // djnz loop
}

void main() {
    // set matrix address high byte
    P2 = 0x40;

    // disable LED
    LED = 1;

    // reset controller pins to known state
    NES_LATCH = 0;
    NES_CLK   = 0;

    // interrupt config
    EA  = 1;  // enable interrupts
    ET0 = 1;  // enable timer 0 interrupt

    // timer 0 in 16-bit mode
    // timer 1 in 8-bit autoreload mode
    TMOD = M0_0 | M1_1;

    // timer 0 interrupt in 2.5 ms
    TH0 = 0xEF;
    TL0 = 0xB9;
    TR0 = 1;  // enable timer 0

    // timer 1 freerunning 0-255
    TH1 = 0x00;
    TL1 = 0x00;

    while (1) {
        // throttle main loop to 20 Hz
        static uint16_t last_render_tick = 0;
        if (tick - last_render_tick < 5) continue;

        controller = read_controller();

        switch (state) {
            case GAME_STATE_DRAW_START: {
                TR1 = 1;  // enable timer 1 for random seed sampling.

                clear_screen();

                render_font(font_S, 5, 2, BLU);
                render_font(font_T, 10, 2, BLU);
                render_font(font_A, 14, 2, BLU);
                render_font(font_R, 19, 2, BLU);
                render_font(font_T, 24, 2, BLU);

                render_font(font_G, 6, 9, BLU);
                render_font(font_A, 11, 9, BLU);
                render_font(font_M, 16, 9, BLU);
                render_font(font_E, 22, 9, BLU);

                state = GAME_STATE_START;
                break;
            }
            case GAME_STATE_START: {
                if (JUST_PRESSED(BTN_START)) {
                    // use timer 1 low byte as random seed. together with the user input it's essentially random.
                    // timer 0 can't be used, it's value is synced with the game loop and won't be random enough
                    seed(TL1);

                    TR1 = 0;  // disable timer 1. only used for random seed sampling.

                    clear_screen();
                    game_reset();
                    state = GAME_STATE_INGAME;
                }
                break;
            }
            case GAME_STATE_INGAME: {
                // The order is important here. Asteroids must be rendered first, so that
                // lasers and the spaceship can check for collisions based on the
                // framebuffer.

                asteroid_spawn();

                // update all asteroids
                __idata asteroid_t* asteroid_ptr = asteroids;
                do {
                    asteroid_update(asteroid_ptr);
                    asteroid_ptr++;
                } while (asteroid_ptr < asteroids + ASTEROID_COUNT);

                spaceship_update();

                laser_spawn();

                // update all lasers
                __idata laser_t* laser_ptr = lasers;
                do {
                    laser_update(laser_ptr);
                    laser_ptr++;
                } while (laser_ptr < lasers + LASER_COUNT);

                // update UI
                healthbar_update();
                score_update();

                break;
            }
            case GAME_STATE_DRAW_GAMEOVER: {
                clear_screen();

                render_font(font_G, 6, 2, RED);
                render_font(font_A, 11, 2, RED);
                render_font(font_M, 16, 2, RED);
                render_font(font_E, 22, 2, RED);

                render_font(font_O, 7, 9, RED);
                render_font(font_V, 12, 9, RED);
                render_font(font_E, 16, 9, RED);
                render_font(font_R, 21, 9, RED);

                state = GAME_STATE_GAMEOVER;
                break;
            }
            case GAME_STATE_GAMEOVER: {
                if (JUST_PRESSED(BTN_START)) {
                    clear_screen();
                    state = GAME_STATE_DRAW_START;
                }

                break;
            }
            case GAME_STATE_DRAW_WON: {
                clear_screen();

                render_font(font_G, 6, 2, GRN);
                render_font(font_A, 11, 2, GRN);
                render_font(font_M, 16, 2, GRN);
                render_font(font_E, 22, 2, GRN);

                render_font(font_W, 8, 9, GRN);
                render_font(font_O, 14, 9, GRN);
                render_font(font_N, 19, 9, GRN);

                state = GAME_STATE_WON;
                break;
            }
            case GAME_STATE_WON: {
                if (JUST_PRESSED(BTN_START)) {
                    clear_screen();
                    state = GAME_STATE_DRAW_START;
                }

                break;
            }
        }

        last_controller  = controller;
        last_render_tick = tick;
    }
}

void timer0() __interrupt(TF0_VECTOR) __using(1) {
    // timer 0 interrupt in 2.5 ms
    TH0 = 0xEF;
    TL0 = 0xB9;

    // remember scan line across interrupt calls
    static uint8_t line = 0;

    uint8_t i;
    uint8_t cmd;

    __pdata uint8_t* m_ptr;
    __xdata uint8_t* f_ptr;

    // prepare matrix cmd to write line
    cmd = line | BIT3;

    // base pointers to matrix and framebuffer
    m_ptr = matrix;
    f_ptr = framebuffer;

    // shift framebuffer pointer to the currently scanned line
    f_ptr += (line << 5);

    // loop through first 4 matrix modules
    i = 4;
    do {
        // clear line command
        *(m_ptr + 8) = 0;

        // write 8 pixels from RAM to Matrix
        *m_ptr++ = *f_ptr++;
        *m_ptr++ = *f_ptr++;
        *m_ptr++ = *f_ptr++;
        *m_ptr++ = *f_ptr++;
        *m_ptr++ = *f_ptr++;
        *m_ptr++ = *f_ptr++;
        *m_ptr++ = *f_ptr++;
        *m_ptr++ = *f_ptr++;

        // send matrix cmd to write line
        *m_ptr = cmd;

        m_ptr += 8;
    } while (--i > 0);  // djnz loop

    // wrap framebuffer pointer to send row (32 * 8) - 32
    f_ptr += 224;

    // loop through send 4 matrix modules
    i = 4;
    do {
        // clear line command
        *(m_ptr + 8) = 0;

        // write 8 pixels from RAM to Matrix
        *m_ptr++ = *f_ptr++;
        *m_ptr++ = *f_ptr++;
        *m_ptr++ = *f_ptr++;
        *m_ptr++ = *f_ptr++;
        *m_ptr++ = *f_ptr++;
        *m_ptr++ = *f_ptr++;
        *m_ptr++ = *f_ptr++;
        *m_ptr++ = *f_ptr++;

        // send matrix cmd to write line
        *m_ptr = cmd;

        m_ptr += 8;
    } while (--i > 0);  // djnz loop

    // advance to next scan line for the next ISR call
    line++;
    if (line >= 8) line = 0;

    if (line == 0 || line == 3) tick++;
}

void timer1() __interrupt(TF1_VECTOR) __using(0) {
}

void timer2() __interrupt(TF2_VECTOR) __using(0) {
}

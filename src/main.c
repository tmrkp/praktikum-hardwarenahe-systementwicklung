#ifdef __INTELLISENSE__
#undef __SDCC_mcs51
#endif

#include <at89x52.h>
#include <lint.h>
#include <stdbool.h>
#include <stdint.h>

// PINS for NES controller
#define NES_CLK P1_2
#define NES_LATCH P1_0
#define NES_DATA P1_1

// Debug LED
#define LED P3_5

// Button bitmapping for NES controller
#define BTN_RIGHT 0x01
#define BTN_LEFT 0x02
#define BTN_DOWN 0x04
#define BTN_UP 0x08
#define BTN_START 0x10
#define BTN_SELECT 0x20
#define BTN_B 0x40
#define BTN_A 0x80

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

// checks if a pixel is within the frame buffer
#define IN_BOUNDS(x, y) ((uint8_t) (x) < WIDTH && (uint8_t) (y) < HEIGHT)

// set a pixel in the framebuffer with bounds check
#define SET_PIXEL(x, y, color)           \
    if (IN_BOUNDS(x, y)) {               \
        framebuffer[(y)][(x)] = (color); \
    }

#define INTERSECT_AREA(x1, y1, w1, h1, x2, y2, w2, h2) ((y2) + (h2) >= (y1) && (y2) < (y1) + (h1) && (x2) + (w2) >= (x1) && (x2) < (x1) + (w1))

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

void render_font(__code uint8_t* glyph, uint8_t x0, uint8_t y, uint8_t color) {
    uint8_t i = 5;
    do {
        uint8_t          bitmask = *glyph++;
        __xdata uint8_t* f_ptr   = framebuffer[y++] + x0;
        do {
            if (bitmask & 0x80) {
                *f_ptr = color;
            }
            bitmask <<= 1;
            f_ptr++;
        } while (bitmask != 0);
    } while (--i > 0);
}

// Pseudo Random Number Generator
// Source: github.com/edrosten/8bit_rng

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
    rng.a = seed | 1;  // ensure non-zero
}

uint8_t rand() {
    uint8_t t = rng.x ^ (rng.x << 4);

    rng.x = rng.y;
    rng.y = rng.z;
    rng.z = rng.a;

    rng.a = rng.z ^ t ^ (rng.z >> 1) ^ (t << 1);

    return rng.a;
}

void clear_screen() {
    for (uint8_t x = 0; x < WIDTH; x++) {
        for (uint8_t y = 0; y < HEIGHT; y++) {
            framebuffer[y][x] = BLK;
        }
    }
}

typedef enum {
    GAME_STATE_DRAW_START,
    GAME_STATE_START,
    GAME_STATE_INGAME,
    GAME_STATE_DRAW_GAMEOVER,
    GAME_STATE_GAMEOVER,
    GAME_STATE_DRAW_WON,
    GAME_STATE_WON
} game_state_t;

game_state_t state = GAME_STATE_DRAW_START;

__code uint8_t asteroid_bitmap_0[] = {
    0b00100000,
    0b01100000,
    0b11100000,
};

__code uint8_t asteroid_bitmap_1[] = {
    0b00110000,
    0b01110000,
    0b11100000,
    0b01000000,
};

__code uint8_t asteroid_bitmap_2[] = {
    0b01110000,
    0b11111000,
    0b11111100,
    0b11111100,
    0b00110000,
};

__code uint8_t asteroid_bitmap_3[] = {
    0b11000000,
    0b11000000,
};

typedef struct {
    int8_t          w;
    int8_t          h;
    uint8_t         hp;
    uint8_t         damage;
    uint8_t         score;
    __code uint8_t* bitmap;
} asteroid_type_t;

__idata asteroid_type_t asteroid_types[] = {
    {
        .w      = 2,
        .h      = 2,
        .hp     = 1,
        .damage = 1,
        .score  = 1,
        .bitmap = asteroid_bitmap_3,
    },
    {
        .w      = 3,
        .h      = 3,
        .hp     = 1,
        .damage = 1,
        .score  = 1,
        .bitmap = asteroid_bitmap_0,
    },
    {
        .w      = 4,
        .h      = 4,
        .hp     = 2,
        .damage = 1,
        .score  = 2,
        .bitmap = asteroid_bitmap_1,
    },
    {
        .w      = 6,
        .h      = 5,
        .hp     = 3,
        .damage = 2,
        .score  = 3,
        .bitmap = asteroid_bitmap_2,
    },
};

typedef struct {
    __idata asteroid_type_t* type;

    int8_t  x;
    int8_t  y;
    int8_t  vx;
    int8_t  vy;
    bool    visible;
    uint8_t hp;
} asteroid_t;

__idata asteroid_t asteroids[8];
uint8_t            asteroids_index = 0;

typedef struct {
    uint8_t x;
    uint8_t y;
    uint8_t hp;
    uint8_t score;
} spaceship_t;

spaceship_t spaceship;

typedef struct {
    uint8_t x;
    uint8_t y;
    bool    visible;
} laser_t;

__idata laser_t laser[8];
uint8_t         laser_index = 0;

bool spaceship_checkhit(__idata asteroid_t* asteroid) {
    if (!asteroid->visible) {
        return false;
    }

    __idata asteroid_type_t* type = asteroid->type;

    return INTERSECT_AREA(spaceship.x, spaceship.y, 2, 3, asteroid->x >> 2, asteroid->y >> 2, type->w, type->h);
}

void spaceship_hit(__idata asteroid_t* asteroid) {
    __idata asteroid_type_t* type = asteroid->type;

    if (spaceship.hp <= type->damage) {
        state = GAME_STATE_DRAW_GAMEOVER;
        return;
    }

    spaceship.hp -= type->damage;
}

inline bool fits_on_screen(int8_t x, int8_t y, int8_t w, int8_t h) {
    return (x + w >= 0) && (x < WIDTH) && (y + h >= 0) && (y <= HEIGHT);
}

bool render_asteroid_shape(__idata asteroid_t* asteroid, uint8_t color) {
    __idata asteroid_type_t* type = asteroid->type;

    int8_t x = asteroid->x >> 2;
    int8_t y = asteroid->y >> 2;

    // asteroid bitmap bounds
    int8_t sx = x;
    int8_t ex = x + type->w;
    int8_t sy = y;
    int8_t ey = y + type->h;

    // apply framebuffer bounds
    if (sx < 0) sx = 0;
    if (sy < 1) sy = 1;
    if (ex > WIDTH) ex = WIDTH;
    if (ey > HEIGHT) ey = HEIGHT;

    // asteroid is not in frame
    if (sx >= ex || sy >= ey) {
        return false;
    }

    __code uint8_t* bitmap = type->bitmap;

    // clip top
    bitmap += sy - y;

    // clip horizontal
    x = sx - x;

    // loop variables
    ex -= sx;
    ey -= sy;
    do {
        uint8_t bitmask = *bitmap++;

        if (x) {
            bitmask <<= x;
        }

        __xdata uint8_t* f_ptr = framebuffer[sy++] + sx;

        uint8_t bits = ex;
        do {
            if (bitmask & 0x80) {
                *f_ptr = color;
            }
            bitmask <<= 1;
            f_ptr++;
        } while (--bits > 0);
    } while (--ey > 0);

    return true;
}

void render_asteroid(__idata asteroid_t* asteroid) {
    if (!asteroid->visible) {
        return;
    }
    render_asteroid_shape(asteroid, BLK);
    asteroid->x += asteroid->vx;
    asteroid->y += asteroid->vy;

    uint8_t color;

    switch (asteroid->hp) {
        case 1: {
            color = ORG;
            break;
        }
        case 2: {
            color = YEL;
            break;
        }
        default: {
            color = BLU;
            break;
        }
    }

    if (!render_asteroid_shape(asteroid, color)) {
        asteroid->visible = false;
    }
}

bool asteroid_checkhit(__idata asteroid_t* asteroid, int8_t x, int8_t y) {
    if (!asteroid->visible) return false;

    __idata asteroid_type_t* type = asteroid->type;

    return INTERSECT_POINT(asteroid->x >> 2, asteroid->y >> 2, type->w, type->h, x, y);
}

void asteroid_hit(__idata asteroid_t* asteroid) {
    __idata asteroid_type_t* type = asteroid->type;

    asteroid->hp--;

    if (asteroid->hp == 0) {
        spaceship.score += type->score;
        if (spaceship.score >= 16) {
            state = GAME_STATE_DRAW_WON;
        }
        asteroid->visible = false;
        render_asteroid_shape(asteroid, BLK);
    }
}

void asteroid_spawn(__idata asteroid_t* asteroid) {
    uint8_t r  = rand();
    uint8_t r2 = rand();

    asteroid->type = asteroid_types + (r & 0b00000011);

    if (r & 0b00000100) {
        // right edge
        asteroid->x = 31 << 2;
        asteroid->y = ((r2 & 0x3F) - (asteroid->type->h * 2));

        asteroid->vx = (r & 0b00001000) ? -4 : -3;

        if (r & 0b00010000) {
            asteroid->vy = 0;
        } else {
            asteroid->vy = (r & 0b00100000) ? -1 : 1;
        }
    } else {
        // top or bottom edge
        asteroid->x = 60 + (r2 & 0x3F);

        asteroid->vx = (r & 0b00001000) ? -3 : -2;

        if (r & 0b0001000) {
            // top edge
            asteroid->y  = (2 - asteroid->type->h) << 2;
            asteroid->vy = (r2 & 0x01) ? 1 : 2;
        } else {
            // bottom egde
            asteroid->y  = 15 << 2;
            asteroid->vy = (r2 & 0x01) ? -1 : -2;
        }
    }

    asteroid->visible = true;
    asteroid->hp      = asteroid->type->hp;
}

void render_spaceship() {
    if (framebuffer[spaceship.y][spaceship.x] != BLK ||
        framebuffer[spaceship.y][spaceship.x + 1] != BLK ||
        framebuffer[spaceship.y - 1][spaceship.x] != BLK ||
        framebuffer[spaceship.y + 1][spaceship.x] != BLK) {
        if (spaceship_checkhit(asteroids + 0)) {
            spaceship_hit(asteroids + 0);
        } else if (spaceship_checkhit(asteroids + 1)) {
            spaceship_hit(asteroids + 1);
        } else if (spaceship_checkhit(asteroids + 2)) {
            spaceship_hit(asteroids + 2);
        } else if (spaceship_checkhit(asteroids + 3)) {
            spaceship_hit(asteroids + 3);
        } else if (spaceship_checkhit(asteroids + 4)) {
            spaceship_hit(asteroids + 4);
        } else if (spaceship_checkhit(asteroids + 5)) {
            spaceship_hit(asteroids + 5);
        } else if (spaceship_checkhit(asteroids + 6)) {
            spaceship_hit(asteroids + 6);
        } else if (spaceship_checkhit(asteroids + 7)) {
            spaceship_hit(asteroids + 7);
        }
    }

    framebuffer[spaceship.y][spaceship.x]     = WHT;
    framebuffer[spaceship.y][spaceship.x + 1] = WHT;
    framebuffer[spaceship.y - 1][spaceship.x] = WHT;
    framebuffer[spaceship.y + 1][spaceship.x] = WHT;
}

void clear_spaceship() {
    framebuffer[spaceship.y][spaceship.x]     = BLK;
    framebuffer[spaceship.y][spaceship.x + 1] = BLK;
    framebuffer[spaceship.y - 1][spaceship.x] = BLK;
    framebuffer[spaceship.y + 1][spaceship.x] = BLK;
}

void render_laser(__idata laser_t* laser) {
    if (!laser->visible) {
        return;
    }
    framebuffer[laser->y][laser->x] = BLK;
    laser->x++;
    if (laser->x >= 32) {
        laser->visible = 0;
        return;
    }

    if (framebuffer[laser->y][laser->x] != BLK) {
        // we know we hit something, but we need to find the exact asteroid we hit
        if (asteroid_checkhit(asteroids + 0, laser->x, laser->y)) {
            asteroid_hit(asteroids + 0);
        } else if (asteroid_checkhit(asteroids + 1, laser->x, laser->y)) {
            asteroid_hit(asteroids + 1);
        } else if (asteroid_checkhit(asteroids + 2, laser->x, laser->y)) {
            asteroid_hit(asteroids + 2);
        } else if (asteroid_checkhit(asteroids + 3, laser->x, laser->y)) {
            asteroid_hit(asteroids + 3);
        } else if (asteroid_checkhit(asteroids + 4, laser->x, laser->y)) {
            asteroid_hit(asteroids + 4);
        } else if (asteroid_checkhit(asteroids + 5, laser->x, laser->y)) {
            asteroid_hit(asteroids + 5);
        } else if (asteroid_checkhit(asteroids + 6, laser->x, laser->y)) {
            asteroid_hit(asteroids + 6);
        } else if (asteroid_checkhit(asteroids + 7, laser->x, laser->y)) {
            asteroid_hit(asteroids + 7);
        }

        laser->visible = 0;
        return;
    }

    framebuffer[laser->y][laser->x] = RED;
}

uint8_t last_rendered_hp = 0xFF;

void render_healthbar() {
    if (last_rendered_hp != spaceship.hp) {
        for (uint8_t i = 0; i < 16; i++) {
            framebuffer[0][i] = BLK;
        }

        for (uint8_t i = 0; i < spaceship.hp; i++) {
            framebuffer[0][i] = RED;
        }

        last_rendered_hp = spaceship.hp;
    }
}

uint8_t last_rendered_score = 0xFF;

void render_score() {
    if (last_rendered_score != spaceship.score) {
        for (uint8_t i = 0; i < spaceship.score; i++) {
            framebuffer[0][31 - i] = GRN;
        }

        last_rendered_score = spaceship.score;
    }
}

void game_reset() {
    spaceship.x     = 3;
    spaceship.y     = 8;
    spaceship.hp    = 16;
    spaceship.score = 0;

    last_rendered_hp    = 0xFF;
    last_rendered_score = 0xFF;

    for (uint8_t i = 0; i < 8; i++) {
        asteroids[i].visible = false;
    }

    asteroids_index = 0;

    for (uint8_t i = 0; i < 8; i++) {
        laser[i].visible = false;
    }

    laser_index = 0;
}

// Reads NES controller inputs and returns button states as 8-bit bitmask
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
    } while (--i > 0);

    return value;
}

void main() {
    // timer 0 in 16-bit mode
    // timer 1 in 8-bit autoreload mode
    TMOD = M0_0 | M1_1;

    // timer 0 interrupt in 2.5 ms
    TH0 = 0xEF;
    TL0 = 0xB9;

    // timer 1 freerunning 0-255
    TH1 = 0x00;
    TL1 = 0x00;

    // interrupt config
    EA  = 1;  // enable interrupts
    ET0 = 1;  // enable timer 0 interrupt

    TR0 = 1;  // enable timer 0
    TR1 = 1;  // enable timer 1

    LED = 1;  // disable LED

    P2 = 0x40;  // matrix address high byte

    NES_LATCH = 0;
    NES_CLK   = 0;

    while (1) {
        // throttle main loop to 20 Hz
        static uint16_t last_render_tick = 0;
        if (tick - last_render_tick < 5) continue;

        static uint8_t last_controller = 0;
        uint8_t        controller      = read_controller();

        switch (state) {
            case GAME_STATE_DRAW_START: {
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
                if ((controller & BTN_START) && !(last_controller & BTN_START)) {
                    // use timer 1 low byte as random seed
                    // timer 0 can't be used, it's synced with the game loop
                    seed(TL1);

                    clear_screen();
                    game_reset();
                    state = GAME_STATE_INGAME;
                }
                break;
            }
            case GAME_STATE_INGAME: {
                if ((controller & BTN_UP) && spaceship.y >= 3) {
                    clear_spaceship();
                    spaceship.y -= 1;
                }

                if ((controller & BTN_DOWN) && spaceship.y <= 13) {
                    clear_spaceship();
                    spaceship.y += 1;
                }

                if ((controller & BTN_RIGHT) && spaceship.x <= 29) {
                    clear_spaceship();
                    spaceship.x += 1;
                }

                if ((controller & BTN_LEFT) && spaceship.x >= 1) {
                    clear_spaceship();
                    spaceship.x -= 1;
                }

                static uint16_t last_asteroid_tick = 0;
                if (tick - last_asteroid_tick > 100) {
                    asteroid_spawn(asteroids + asteroids_index);
                    asteroids_index++;
                    if (asteroids_index >= 8) asteroids_index = 0;

                    last_asteroid_tick = tick;
                }

                render_asteroid(asteroids + 0);
                render_asteroid(asteroids + 1);
                render_asteroid(asteroids + 2);
                render_asteroid(asteroids + 3);
                render_asteroid(asteroids + 4);
                render_asteroid(asteroids + 5);
                render_asteroid(asteroids + 6);
                render_asteroid(asteroids + 7);

                if ((controller & BTN_A) && !(last_controller & BTN_A)) {
                    laser[laser_index].x       = spaceship.x + 2;
                    laser[laser_index].y       = spaceship.y;
                    laser[laser_index].visible = true;
                    laser_index++;
                    if (laser_index >= 8) laser_index = 0;
                }

                render_laser(laser + 0);
                render_laser(laser + 1);
                render_laser(laser + 2);
                render_laser(laser + 3);
                render_laser(laser + 4);
                render_laser(laser + 5);
                render_laser(laser + 6);
                render_laser(laser + 7);

                render_healthbar();
                render_score();
                render_spaceship();

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
                if ((controller & BTN_START) && !(last_controller & BTN_START)) {
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
                if ((controller & BTN_START) && !(last_controller & BTN_START)) {
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
    cmd = line | 0x8;

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
    } while (--i > 0);

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
    } while (--i > 0);

    // advance to next scan line for the next ISR call
    line++;
    if (line >= 8) line = 0;

    if (line == 0 || line == 3) tick++;
}

void timer1() __interrupt(TF1_VECTOR) __using(0) {
}

void timer2() __interrupt(TF2_VECTOR) __using(0) {
}

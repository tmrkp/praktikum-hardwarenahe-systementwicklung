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
#define BLK (0x00)
#define WHT (0x3F)
#define RED (0x30)
#define GRN (0x0C)
#define BLU (0x03)
#define YEL (0x3C)
#define MAG (0x33)
#define CYN (0x0F)
#define ORG (0x38)
#define RED_LIGHT (0x10)

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

__code uint8_t font_O[] = {
    0b01110000,
    0b10001000,
    0b10001000,
    0b10001000,
    0b01110000,
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

void render_font(__code uint8_t* glyph, uint8_t x0, uint8_t y, uint8_t color) {
    uint8_t i = 5;
    do {
        uint8_t bitmask = *glyph++;
        uint8_t x       = x0;
        do {
            if (bitmask & 0x80) {
                framebuffer[y][x] = color;
            }
            bitmask <<= 1;
            x++;
        } while (bitmask != 0);
        y++;
    } while (--i > 0);
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

typedef enum {
    ASTEROID_SHAPE_BIG = 0,
    ASTEROID_SHAPE_SMALL,
    ASTEROID_SHAPE_MIDDLE,
} asteroid_shape_t;

typedef struct {
    int8_t           x;
    int8_t           y;
    asteroid_shape_t shape;
    bool             visible;
    uint8_t          hp;
} asteroid_t;

asteroid_t asteroids[8];
uint8_t    asteroids_index = 0;

typedef struct {
    uint8_t x;
    uint8_t y;
    uint8_t hp;
} spaceship_t;

spaceship_t spaceship;

typedef struct {
    uint8_t x;
    uint8_t y;
    bool    visible;
} laser_t;

laser_t laser[8];
uint8_t laser_index = 0;

void game_reset() {
    spaceship.x  = 3;
    spaceship.y  = 8;
    spaceship.hp = 16;

    for (uint8_t i = 0; i < 8; i++) {
        asteroids[i].visible = false;
    }

    asteroids_index = 0;

    for (uint8_t i = 0; i < 8; i++) {
        laser[i].visible = false;
    }

    laser_index = 0;
}

bool spaceship_hit(uint8_t index) {
    if (!asteroids[index].visible) {
        return false;
    }
    switch (asteroids[index].shape) {
        case ASTEROID_SHAPE_BIG: {
            if (INTERSECT_AREA(spaceship.x, spaceship.y, 2, 3, asteroids[index].x, asteroids[index].y, 6, 5)) {
                if (spaceship.hp <= 2) {
                    state = GAME_STATE_DRAW_GAMEOVER;
                    return true;
                };
                spaceship.hp -= 2;
                return true;
            }
            break;
        }
        case ASTEROID_SHAPE_MIDDLE: {
            if (INTERSECT_AREA(spaceship.x, spaceship.y, 2, 3, asteroids[index].x, asteroids[index].y, 4, 4)) {
                if (spaceship.hp <= 1) {
                    state = GAME_STATE_DRAW_GAMEOVER;
                    return true;
                };
                spaceship.hp -= 1;
                return true;
            }
            break;
        }
        case ASTEROID_SHAPE_SMALL: {
            if (INTERSECT_AREA(spaceship.x, spaceship.y, 2, 3, asteroids[index].x, asteroids[index].y, 3, 3)) {
                if (spaceship.hp <= 1) {
                    state = GAME_STATE_DRAW_GAMEOVER;
                    return true;
                };
                spaceship.hp -= 1;
                return true;
            }
            break;
        }
    }
    return false;
}

inline bool fits_on_screen(int8_t x, int8_t y, int8_t w, int8_t h) {
    return (x + w >= 0) && (x < WIDTH) && (y + h >= 0) && (y <= HEIGHT);
}

bool render_asteroid_shape(uint8_t index, uint8_t color) {
    uint8_t x = asteroids[index].x;
    uint8_t y = asteroids[index].y;

    switch (asteroids[index].shape) {
        case ASTEROID_SHAPE_SMALL: {
            if (!fits_on_screen(x, y, 3, 3)) return true;

            SET_PIXEL(x + 0, y + 0, color);
            SET_PIXEL(x + 0, y + 1, color);
            SET_PIXEL(x + 0, y + 2, color);

            SET_PIXEL(x + 1, y + 1, color);
            SET_PIXEL(x + 1, y + 2, color);

            SET_PIXEL(x + 2, y + 2, color);
            break;
        }
        case ASTEROID_SHAPE_MIDDLE: {
            if (!fits_on_screen(x, y, 4, 4)) return true;

            SET_PIXEL(x + 0, y + 2, color);

            SET_PIXEL(x + 1, y + 1, color);
            SET_PIXEL(x + 1, y + 2, color);
            SET_PIXEL(x + 1, y + 3, color);

            SET_PIXEL(x + 2, y + 0, color);
            SET_PIXEL(x + 2, y + 1, color);
            SET_PIXEL(x + 2, y + 2, color);

            SET_PIXEL(x + 3, y + 0, color);
            SET_PIXEL(x + 3, y + 1, color);
            break;
        }
        case ASTEROID_SHAPE_BIG: {
            if (!fits_on_screen(x, y, 6, 5)) return true;

            SET_PIXEL(x + 0, y + 1, color);
            SET_PIXEL(x + 0, y + 2, color);
            SET_PIXEL(x + 0, y + 3, color);

            SET_PIXEL(x + 1, y + 0, color);
            SET_PIXEL(x + 1, y + 1, color);
            SET_PIXEL(x + 1, y + 2, color);
            SET_PIXEL(x + 1, y + 3, color);

            SET_PIXEL(x + 2, y + 0, color);
            SET_PIXEL(x + 2, y + 1, color);
            SET_PIXEL(x + 2, y + 2, color);
            SET_PIXEL(x + 2, y + 3, color);
            SET_PIXEL(x + 2, y + 4, color);

            SET_PIXEL(x + 3, y + 0, color);
            SET_PIXEL(x + 3, y + 1, color);
            SET_PIXEL(x + 3, y + 2, color);
            SET_PIXEL(x + 3, y + 3, color);
            SET_PIXEL(x + 3, y + 4, color);

            SET_PIXEL(x + 4, y + 1, color);
            SET_PIXEL(x + 4, y + 2, color);
            SET_PIXEL(x + 4, y + 3, color);

            SET_PIXEL(x + 5, y + 2, color);
            SET_PIXEL(x + 5, y + 3, color);

            break;
        }
    }

    return false;
}

void render_asteroid(uint8_t index) {
    if (!asteroids[index].visible) {
        return;
    }
    render_asteroid_shape(index, BLK);
    asteroids[index].x -= 1;

    uint8_t color;

    switch (asteroids[index].hp) {
        case 1: {
            color = ORG;
            break;
        }
        case 2: {
            color = YEL;
            break;
        }
        case 3: {
            color = BLU;
            break;
        }
    }

    if (render_asteroid_shape(index, color)) {
        asteroids[index].visible = false;
    }
}

bool asteroid_hit(uint8_t index, uint8_t x, uint8_t y) {
    if (!asteroids[index].visible) return false;

    uint8_t ax = asteroids[index].x;
    uint8_t ay = asteroids[index].y;
    switch (asteroids[index].shape) {
        case ASTEROID_SHAPE_BIG: {
            if (!INTERSECT_POINT(ax, ay, 6, 4, x, y)) {
                return false;
            }
            break;
        }
        case ASTEROID_SHAPE_MIDDLE: {
            if (!INTERSECT_POINT(ax, ay, 4, 4, x, y)) {
                return false;
            }
            break;
        }
        case ASTEROID_SHAPE_SMALL: {
            if (!INTERSECT_POINT(ax, ay, 3, 3, x, y)) {
                return false;
            }
            break;
        }
    }

    asteroids[index].hp--;

    if (asteroids[index].hp == 0) {
        asteroids[index].visible = false;
        render_asteroid_shape(index, BLK);
    }

    return true;
}

void render_spaceship() {
    bool spaceship_hit_bool = false;
    if (framebuffer[spaceship.y][spaceship.x] != BLK) spaceship_hit_bool = true;
    if (framebuffer[spaceship.y][spaceship.x + 1] != BLK) spaceship_hit_bool = true;
    if (framebuffer[spaceship.y - 1][spaceship.x] != BLK) spaceship_hit_bool = true;
    if (framebuffer[spaceship.y + 1][spaceship.x] != BLK) spaceship_hit_bool = true;

    if (spaceship_hit_bool) {
        if (spaceship_hit(0)) {
        } else if (spaceship_hit(1)) {
        } else if (spaceship_hit(2)) {
        } else if (spaceship_hit(3)) {
        } else if (spaceship_hit(4)) {
        } else if (spaceship_hit(5)) {
        } else if (spaceship_hit(6)) {
        } else if (spaceship_hit(7)) {
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

void render_laser(uint8_t index) {
    if (!laser[index].visible) {
        return;
    }
    framebuffer[laser[index].y][laser[index].x] = BLK;
    laser[index].x++;
    if (laser[index].x >= 32) {
        laser[index].visible = 0;
        return;
    }

    if (framebuffer[laser[index].y][laser[index].x] != BLK) {
        // we know we hit something, but we need to find the exact asteroid we hit
        if (asteroid_hit(0, laser[index].x, laser[index].y)) {
        } else if (asteroid_hit(1, laser[index].x, laser[index].y)) {
        } else if (asteroid_hit(2, laser[index].x, laser[index].y)) {
        } else if (asteroid_hit(3, laser[index].x, laser[index].y)) {
        } else if (asteroid_hit(4, laser[index].x, laser[index].y)) {
        } else if (asteroid_hit(5, laser[index].x, laser[index].y)) {
        } else if (asteroid_hit(6, laser[index].x, laser[index].y)) {
        } else if (asteroid_hit(7, laser[index].x, laser[index].y)) {
        }

        laser[index].visible = 0;
        return;
    }

    framebuffer[laser[index].y][laser[index].x] = RED;
}

void render_healthbar() {
    static uint8_t last_hp = -1;

    if (last_hp != spaceship.hp) {
        framebuffer[0][0]  = (spaceship.hp >= 1) ? RED : RED_LIGHT;
        framebuffer[0][1]  = (spaceship.hp >= 2) ? RED : RED_LIGHT;
        framebuffer[0][2]  = (spaceship.hp >= 3) ? RED : RED_LIGHT;
        framebuffer[0][3]  = (spaceship.hp >= 4) ? RED : RED_LIGHT;
        framebuffer[0][4]  = (spaceship.hp >= 5) ? RED : RED_LIGHT;
        framebuffer[0][5]  = (spaceship.hp >= 6) ? RED : RED_LIGHT;
        framebuffer[0][6]  = (spaceship.hp >= 7) ? RED : RED_LIGHT;
        framebuffer[0][7]  = (spaceship.hp >= 8) ? RED : RED_LIGHT;
        framebuffer[0][8]  = (spaceship.hp >= 9) ? RED : RED_LIGHT;
        framebuffer[0][9]  = (spaceship.hp >= 10) ? RED : RED_LIGHT;
        framebuffer[0][10] = (spaceship.hp >= 11) ? RED : RED_LIGHT;
        framebuffer[0][11] = (spaceship.hp >= 12) ? RED : RED_LIGHT;
        framebuffer[0][12] = (spaceship.hp >= 13) ? RED : RED_LIGHT;
        framebuffer[0][13] = (spaceship.hp >= 14) ? RED : RED_LIGHT;
        framebuffer[0][14] = (spaceship.hp >= 15) ? RED : RED_LIGHT;
        framebuffer[0][15] = (spaceship.hp >= 16) ? RED : RED_LIGHT;

        last_hp = spaceship.hp;
    }
}

// Random Number Generator
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
    TMOD = M0_0 | M0_1;

    // timer 0
    TH0 = 0xEF;  // 2.5ms
    TL0 = 0xB9;

    // // timer 1
    // TH1 = 0xBE;  // 10 ms
    // TL1 = 0xE5;

    // interrupt config
    EA  = 1;  // enable interrupts
    ET0 = 1;  // enable timer 0 interrupt
    // ET1 = 1;  // enable timer 1 interrupt
    // PT1 = 1;  // prioritize timer 1 interrupt

    TR0 = 1;  // enable timer 0
    // TR1  = 1;  // enable timer 1
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

                render_font(font_S, 5, 2, GRN);
                render_font(font_T, 10, 2, GRN);
                render_font(font_A, 14, 2, GRN);
                render_font(font_R, 19, 2, GRN);
                render_font(font_T, 24, 2, GRN);

                render_font(font_G, 6, 9, GRN);
                render_font(font_A, 11, 9, GRN);
                render_font(font_M, 16, 9, GRN);
                render_font(font_E, 22, 9, GRN);

                state = GAME_STATE_START;
                break;
            }
            case GAME_STATE_START: {
                if ((controller & BTN_START) && !(last_controller & BTN_START)) {
                    clear_screen();
                    game_reset();
                    state = GAME_STATE_INGAME;
                }
                break;
            }
            case GAME_STATE_INGAME: {
                if ((controller & BTN_UP) && spaceship.y >= 2) {
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
                if (tick - last_asteroid_tick > 200) {
                    int8_t y     = rand() & 0x0F;
                    int8_t shape = rand() & 0x03;
                    if (shape == 3) shape = 1;
                    asteroids[asteroids_index].y       = y - 2;
                    asteroids[asteroids_index].x       = 31;
                    asteroids[asteroids_index].visible = true;
                    asteroids[asteroids_index].shape   = shape;
                    asteroids[asteroids_index].hp      = 3;
                    asteroids_index++;
                    if (asteroids_index >= 8) asteroids_index = 0;

                    last_asteroid_tick = tick;
                }

                render_asteroid(0);
                render_asteroid(1);
                render_asteroid(2);
                render_asteroid(3);
                render_asteroid(4);
                render_asteroid(5);
                render_asteroid(6);
                render_asteroid(7);

                if ((controller & BTN_A) && !(last_controller & BTN_A)) {
                    laser[laser_index].x       = spaceship.x + 2;
                    laser[laser_index].y       = spaceship.y;
                    laser[laser_index].visible = true;
                    laser_index++;
                    if (laser_index >= 8) laser_index = 0;
                }

                render_laser(0);
                render_laser(1);
                render_laser(2);
                render_laser(3);
                render_laser(4);
                render_laser(5);
                render_laser(6);
                render_laser(7);

                render_healthbar();
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
        }

        last_controller  = controller;
        last_render_tick = tick;
    }
}

void timer0() __interrupt(TF0_VECTOR) __using(1) {
    // remember scan line across ISR calls
    static uint8_t line = 0;

    uint8_t i;
    uint8_t cmd;

    __pdata uint8_t* m_ptr;
    __xdata uint8_t* f_ptr;

    // restart timer 0
    TH0 = 0xEF;  // 2.5ms
    TL0 = 0xB9;

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

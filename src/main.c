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

void render_font_G(uint8_t x, uint8_t y, uint8_t color) {
    framebuffer[y][x + 1] = color;
    framebuffer[y][x + 2] = color;
    framebuffer[y][x + 3] = color;

    framebuffer[y + 1][x] = color;

    framebuffer[y + 2][x]     = color;
    framebuffer[y + 2][x + 2] = color;
    framebuffer[y + 2][x + 3] = color;

    framebuffer[y + 3][x]     = color;
    framebuffer[y + 3][x + 3] = color;

    framebuffer[y + 4][x + 1] = color;
    framebuffer[y + 4][x + 2] = color;
    framebuffer[y + 4][x + 3] = color;
}

void render_font_A(uint8_t x, uint8_t y, uint8_t color) {
    framebuffer[y][x + 1] = color;
    framebuffer[y][x + 2] = color;

    framebuffer[y + 1][x]     = color;
    framebuffer[y + 1][x + 3] = color;

    framebuffer[y + 2][x]     = color;
    framebuffer[y + 2][x + 1] = color;
    framebuffer[y + 2][x + 2] = color;
    framebuffer[y + 2][x + 3] = color;

    framebuffer[y + 3][x]     = color;
    framebuffer[y + 3][x + 3] = color;

    framebuffer[y + 4][x]     = color;
    framebuffer[y + 4][x + 3] = color;
}

void render_font_E(uint8_t x, uint8_t y, uint8_t color) {
    framebuffer[y][x]     = color;
    framebuffer[y][x + 1] = color;
    framebuffer[y][x + 2] = color;
    framebuffer[y][x + 3] = color;

    framebuffer[y + 1][x] = color;

    framebuffer[y + 2][x]     = color;
    framebuffer[y + 2][x + 1] = color;
    framebuffer[y + 2][x + 2] = color;

    framebuffer[y + 3][x] = color;

    framebuffer[y + 4][x]     = color;
    framebuffer[y + 4][x + 1] = color;
    framebuffer[y + 4][x + 2] = color;
    framebuffer[y + 4][x + 3] = color;
}

void render_font_M(uint8_t x, uint8_t y, uint8_t color) {
    for (int i = 0; i < 5; i++) {
        framebuffer[y + i][x]     = color;
        framebuffer[y + i][x + 4] = color;
    }

    framebuffer[y + 1][x + 1] = color;
    framebuffer[y + 2][x + 2] = color;
    framebuffer[y + 1][x + 3] = color;
}

void render_font_O(uint8_t x, uint8_t y, uint8_t color) {
    framebuffer[y][x + 1] = color;
    framebuffer[y][x + 2] = color;

    framebuffer[y + 1][x]     = color;
    framebuffer[y + 1][x + 3] = color;

    framebuffer[y + 2][x]     = color;
    framebuffer[y + 2][x + 3] = color;

    framebuffer[y + 3][x]     = color;
    framebuffer[y + 3][x + 3] = color;

    framebuffer[y + 4][x + 1] = color;
    framebuffer[y + 4][x + 2] = color;
}

void render_font_P(uint8_t x, uint8_t y, uint8_t color) {
    for (int i = 0; i < 5; i++)
        framebuffer[y + i][x] = color;

    framebuffer[y][x + 1] = color;
    framebuffer[y][x + 2] = color;

    framebuffer[y + 1][x + 3] = color;

    framebuffer[y + 2][x + 1] = color;
    framebuffer[y + 2][x + 2] = color;
}

void render_font_R(uint8_t x, uint8_t y, uint8_t color) {
    render_font_P(x, y, color);
    framebuffer[y + 3][x + 3] = color;
    framebuffer[y + 4][x + 3] = color;
}

void render_font_S(uint8_t x, uint8_t y, uint8_t color) {
    framebuffer[y][x + 1] = color;
    framebuffer[y][x + 2] = color;
    framebuffer[y][x + 3] = color;

    framebuffer[y + 1][x] = color;

    framebuffer[y + 2][x + 1] = color;
    framebuffer[y + 2][x + 2] = color;

    framebuffer[y + 3][x + 3] = color;

    framebuffer[y + 4][x]     = color;
    framebuffer[y + 4][x + 1] = color;
    framebuffer[y + 4][x + 2] = color;
}

void render_font_T(uint8_t x, uint8_t y, uint8_t color) {
    framebuffer[y][x]     = color;
    framebuffer[y][x + 1] = color;
    framebuffer[y][x + 2] = color;

    framebuffer[y + 1][x + 1] = color;

    framebuffer[y + 1][x + 2] = color;

    framebuffer[y + 1][x + 3] = color;

    framebuffer[y + 1][x + 4] = color;
}

void render_font_V(uint8_t x, uint8_t y, uint8_t color) {
    framebuffer[y][x]     = color;
    framebuffer[y][x + 2] = color;

    framebuffer[y + 1][x]     = color;
    framebuffer[y + 1][x + 2] = color;

    framebuffer[y + 2][x]     = color;
    framebuffer[y + 2][x + 2] = color;

    framebuffer[y + 3][x + 1] = color;
    framebuffer[y + 4][x + 1] = color;
}

typedef enum {
    ASTEROID_SHAPE_BIG,
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

spaceship_t spaceship = {3, 8, 8};

typedef struct {
    uint8_t x;
    uint8_t y;
    bool    visible;
} laser_t;

laser_t laser[8];
uint8_t laser_index = 0;

inline void spaceship_hit(uint8_t index) {
    switch (asteroids[index].shape) {
        case ASTEROID_SHAPE_BIG: {
            spaceship.hp -= 4;
            break;
        }
        case ASTEROID_SHAPE_MIDDLE: {
            spaceship.hp -= 2;
            break;
        }
        case ASTEROID_SHAPE_SMALL: {
            spaceship.hp -= 1;
            break;
        }
    }
}

inline bool fits_on_screen(int8_t x, int8_t y, int8_t w, int8_t h) {
    return (x + w >= 0) && (x < WIDTH) && (y + h >= 0) && (y <= HEIGHT);
}

typedef enum {
    RENDER_ASTEROID_SHAPE_RETURN_NOTHING = 0,
    RENDER_ASTEROID_SHAPE_RETURN_NOTVISIBLE,
    RENDER_ASTEROID_SHAPE_RETURN_HITSHIP
} render_asteroid_shape_return_t;

render_asteroid_shape_return_t render_asteroid_shape(uint8_t index, uint8_t color) {
    uint8_t x = asteroids[index].x;
    uint8_t y = asteroids[index].y;

    bool hitship = false;

    switch (asteroids[index].shape) {
        case ASTEROID_SHAPE_SMALL: {
            if (!fits_on_screen(x, y, 3, 3)) return RENDER_ASTEROID_SHAPE_RETURN_NOTVISIBLE;

            if (framebuffer[y + 0][x + 0] == WHT ||
                framebuffer[y + 1][x + 0] == WHT ||
                framebuffer[y + 2][x + 0] == WHT) {
                hitship = true;
            }

            SET_PIXEL(x + 0, y + 0, color);
            SET_PIXEL(x + 0, y + 1, color);
            SET_PIXEL(x + 0, y + 2, color);

            SET_PIXEL(x + 1, y + 1, color);
            SET_PIXEL(x + 1, y + 2, color);

            SET_PIXEL(x + 2, y + 2, color);
            break;
        }
        case ASTEROID_SHAPE_MIDDLE: {
            if (!fits_on_screen(x, y, 4, 4)) return RENDER_ASTEROID_SHAPE_RETURN_NOTVISIBLE;

            if (framebuffer[y + 0][x + 2] == WHT ||
                framebuffer[y + 1][x + 1] == WHT ||
                framebuffer[y + 2][x + 0] == WHT ||
                framebuffer[y + 3][x + 1] == WHT) {
                hitship = true;
            }

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
            if (!fits_on_screen(x, y, 6, 5)) return RENDER_ASTEROID_SHAPE_RETURN_NOTVISIBLE;

            if (framebuffer[y + 0][x + 3] == WHT ||
                framebuffer[y + 1][x + 2] == WHT ||
                framebuffer[y + 2][x + 1] == WHT ||
                framebuffer[y + 3][x + 0] == WHT ||
                framebuffer[y + 4][x + 1] == WHT) {
                hitship = true;
            }

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

    if (hitship) {
        return RENDER_ASTEROID_SHAPE_RETURN_HITSHIP;
    }

    return RENDER_ASTEROID_SHAPE_RETURN_NOTHING;
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

    switch (render_asteroid_shape(index, color)) {
        case RENDER_ASTEROID_SHAPE_RETURN_NOTVISIBLE: {
            asteroids[index].visible = false;
            break;
        };
        case RENDER_ASTEROID_SHAPE_RETURN_HITSHIP: {
            spaceship_hit(index);
            break;
        };
    };
}

bool asteroid_hit(uint8_t index, uint8_t x, uint8_t y) {
    if (!asteroids[index].visible) return false;

    uint8_t ax = asteroids[index].x;
    uint8_t ay = asteroids[index].y;
    switch (asteroids[index].shape) {
        case ASTEROID_SHAPE_BIG: {
            if (!(x >= ax && x <= ax + 5 && y >= ay && y <= ay + 4)) {
                return false;
            }
            break;
        }
        case ASTEROID_SHAPE_MIDDLE: {
            if (!(x >= ax && x <= ax + 3 && y >= ay && y <= ay + 3)) {
                return false;
            }
            break;
        }
        case ASTEROID_SHAPE_SMALL: {
            if (!(x >= ax && x <= ax + 2 && y >= ay && y <= ay + 2)) {
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

void render_spaceship(uint8_t color) {
    framebuffer[spaceship.y][spaceship.x]     = color;
    framebuffer[spaceship.y][spaceship.x + 1] = color;
    framebuffer[spaceship.y - 1][spaceship.x] = color;
    framebuffer[spaceship.y + 1][spaceship.x] = color;
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
        framebuffer[0][0] = (spaceship.hp >= 1) ? RED : RED_LIGHT;
        framebuffer[0][1] = (spaceship.hp >= 2) ? RED : RED_LIGHT;
        framebuffer[0][2] = (spaceship.hp >= 3) ? RED : RED_LIGHT;
        framebuffer[0][3] = (spaceship.hp >= 4) ? RED : RED_LIGHT;
        framebuffer[0][4] = (spaceship.hp >= 5) ? RED : RED_LIGHT;
        framebuffer[0][5] = (spaceship.hp >= 6) ? RED : RED_LIGHT;
        framebuffer[0][6] = (spaceship.hp >= 7) ? RED : RED_LIGHT;
        framebuffer[0][7] = (spaceship.hp >= 8) ? RED : RED_LIGHT;

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
    P3_5 = 1;  // disable LED

    P2 = 0x40;  // matrix address high byte

    NES_LATCH = 0;
    NES_CLK   = 0;

    render_spaceship(WHT);

    while (1) {
        static uint16_t last_render_tick = 0;
        if (tick - last_render_tick < 5) continue;

        static uint8_t old_controller = 0;
        uint8_t        controller     = read_controller();

        if ((controller & BTN_UP) && spaceship.y >= 2) {
            spaceship.y -= 1;
            render_spaceship(WHT);
        }

        if ((controller & BTN_DOWN) && spaceship.y <= 13) {
            render_spaceship(BLK);
            spaceship.y += 1;
            render_spaceship(WHT);
        }

        if ((controller & BTN_RIGHT) && spaceship.x <= 29) {
            render_spaceship(BLK);
            spaceship.x += 1;
            render_spaceship(WHT);
        }

        if ((controller & BTN_LEFT) && spaceship.x >= 1) {
            render_spaceship(BLK);
            spaceship.x -= 1;
            render_spaceship(WHT);
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

        if ((controller & BTN_A) && !(old_controller & BTN_A)) {
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

        old_controller = controller;

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

    // P3_5 = 1;

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

    // P3_5 = 0;
}

void timer1() __interrupt(TF1_VECTOR) __using(0) {
}

void timer2() __interrupt(TF2_VECTOR) __using(0) {
}

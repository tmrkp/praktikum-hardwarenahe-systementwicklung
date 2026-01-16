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

uint8_t old_controller = 0;

#define BLK (0x00)
#define RED (0x30)
#define GRN (0x0C)
#define BLU (0x03)
#define WHT (0x3F)

#define WIDTH 32
#define HEIGHT 16

// Array for external Matrix
__xdata uint8_t framebuffer[HEIGHT][WIDTH];

__xdata uint8_t* matrix = (__xdata uint8_t*) 0x4000;

typedef enum {
    ASTEROID_SHAPE_BIG,
    ASTEROID_SHAPE_SMALL,
    ASTEROID_SHAPE_MIDDLE,
} asteroid_shape_t;

typedef struct {
    uint8_t          x;
    uint8_t          y;
    asteroid_shape_t shape;
} asteroid_t;

typedef struct {
    uint8_t x;
    uint8_t y;
} spaceship_t;

typedef struct
{
    uint8_t x;
    uint8_t y;
    bool    visible;
} laser_t;

const uint8_t asteroid_middle[4][4] = {{BLU, BLU, BLK, BLU},
                                       {BLK, BLU, BLU, BLU},
                                       {BLU, BLU, BLU, BLU},
                                       {BLU, BLU, BLK, BLK}};

const uint8_t asteroid_big[6][5] = {{BLK, BLU, BLK, BLU, BLK},
                                    {BLU, BLU, BLU, BLU, BLU},
                                    {BLU, BLU, BLU, BLU, BLU},
                                    {BLU, BLU, BLU, BLU, BLU},
                                    {BLU, BLU, BLU, BLU, BLU},
                                    {BLK, BLU, BLK, BLU, BLK}};

const uint8_t asteroid_small[3][3] = {{BLK, BLK, BLU},
                                      {BLK, BLU, BLU},
                                      {BLU, BLU, BLU}};

asteroid_t a1 = {8, 8, ASTEROID_SHAPE_SMALL};
asteroid_t a2 = {8, 8, ASTEROID_SHAPE_SMALL};

spaceship_t spaceship = {3, 8};

laser_t laser[8];
uint8_t laser_index = 0;

void render_asteroid_shape(asteroid_t* asteroid) {
    switch (asteroid->shape) {
        case ASTEROID_SHAPE_SMALL: {
            framebuffer[asteroid->y + 1][asteroid->x - 1] = BLU;
            framebuffer[asteroid->y][asteroid->x]         = BLU;
            framebuffer[asteroid->y + 1][asteroid->x]     = BLU;
            framebuffer[asteroid->y - 1][asteroid->x + 1] = BLU;
            framebuffer[asteroid->y][asteroid->x + 1]     = BLU;
            framebuffer[asteroid->y + 1][asteroid->x + 1] = BLU;
            break;
        }
        case ASTEROID_SHAPE_MIDDLE: {
            break;
        }
        case ASTEROID_SHAPE_BIG: {
            break;
        }
    }
}

inline void render_spaceship(uint8_t color) {
    framebuffer[spaceship.y][spaceship.x]     = color;
    framebuffer[spaceship.y][spaceship.x + 1] = color;
    framebuffer[spaceship.y - 1][spaceship.x] = color;
    framebuffer[spaceship.y + 1][spaceship.x] = color;
}

void render(uint8_t framebuffer[32][16]) {
    // for (uint8_t i = 0; i < 32; i++) {
    //     for (uint8_t j = 0; j < 16; j++) {
    //         framebuffer[i][j] = BLK;
    //     }
    // }
    // render_asteroid_shape(&a2, framebuffer);
    // render_asteroid_shape(&a1, framebuffer);
    // a1.x++;
    // if (a1.x >= 32) {
    //     a1.x -= 32;
    // }

    // a2.y++;
    // if (a2.y >= 16) a2.y -= 16;
}

uint16_t tick_counter     = 0;  // one tick is 20 ms
uint16_t tick_delay_start = 0;

void tick_delay(uint16_t ticks) {
    tick_delay_start = tick_counter;
    while (tick_counter < tick_delay_start + ticks) {
        // no nothing
    }
}

uint8_t test_external_ram(uint8_t* ptr, uint16_t size) {
    uint8_t patterns[] = {0x00, 0xFF, 0xAA, 0x55};

    for (uint16_t i = 0; i < size; i++) {
        ptr[i] = patterns[i % 4];
    }

    for (uint16_t i = 0; i < size; i++) {
        uint8_t read_value = ptr[i];
        if (read_value != patterns[i % 4]) {
            return 1;
        }
    }

    return 0;
}

void msdelay(uint16_t value) {
    for (uint16_t x = 0; x < value; x++) {
        for (uint8_t y = 0; y < 200; y++);
    }
}

uint8_t read_controller() {
    uint8_t value = 0;

    NES_LATCH = 1;
    NES_LATCH = 0;

    value = (NES_DATA == 0);

    for (uint8_t i = 1; i < 8; i++) {
        NES_CLK = 1;
        NES_CLK = 0;

        value <<= 1;
        value |= (NES_DATA == 0);
    }

    return value;
}

void main() {
    // timer 0 config
    TMOD = M0_0;
    TH0  = 0xEF;  // 2.5ms
    TL0  = 0xB9;

    // interrupt config
    EA  = 1;  // enable interrupts
    ET0 = 1;  // enable timer 0 interrupt

    TR0  = 1;  // enable timer 0
    P3_5 = 1;  // disable LED

    NES_LATCH = 0;
    NES_CLK   = 0;

    render_spaceship(WHT);

    while (1) {
        uint8_t controller = read_controller();

        if ((controller & BTN_UP) && spaceship.y >= 2) {
            render_spaceship(BLK);
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

        if ((controller & BTN_A) && !(old_controller & BTN_A)) {
            laser[laser_index].x       = spaceship.x + 2;
            laser[laser_index].y       = spaceship.y;
            laser[laser_index].visible = true;
            laser_index++;
            if (laser_index >= 8) laser_index = 0;
        }

        if (laser[0].visible) {
            framebuffer[laser[0].x][laser[0].y] = BLK;
            laser[0].x++;
            if (laser[0].x >= 32) {
                laser[0].visible = 0;
            } else {
                framebuffer[laser[0].x][laser[0].y] = RED;
            }
        }
        if (laser[1].visible) {
            framebuffer[laser[1].x][laser[1].y] = BLK;
            laser[1].x++;
            if (laser[1].x >= 32) {
                laser[1].visible = 0;
            } else {
                framebuffer[laser[1].x][laser[1].y] = RED;
            }
        }
        if (laser[2].visible) {
            framebuffer[laser[2].x][laser[2].y] = BLK;
            laser[2].x++;
            if (laser[2].x >= 32) {
                laser[2].visible = 0;
            } else {
                framebuffer[laser[2].x][laser[2].y] = RED;
            }
        }
        if (laser[3].visible) {
            framebuffer[laser[3].x][laser[3].y] = BLK;
            laser[3].x++;
            if (laser[3].x >= 32) {
                laser[3].visible = 0;
            } else {
                framebuffer[laser[3].x][laser[3].y] = RED;
            }
        }
        if (laser[4].visible) {
            framebuffer[laser[4].x][laser[4].y] = BLK;
            laser[4].x++;
            if (laser[4].x >= 32) {
                laser[4].visible = 0;
            } else {
                framebuffer[laser[4].x][laser[4].y] = RED;
            }
        }
        if (laser[5].visible) {
            framebuffer[laser[5].x][laser[5].y] = BLK;
            laser[5].x++;
            if (laser[5].x >= 32) {
                laser[5].visible = 0;
            } else {
                framebuffer[laser[5].x][laser[5].y] = RED;
            }
        }
        if (laser[6].visible) {
            framebuffer[laser[6].x][laser[6].y] = BLK;
            laser[6].x++;
            if (laser[6].x >= 32) {
                laser[6].visible = 0;
            } else {
                framebuffer[laser[6].x][laser[6].y] = RED;
            }
        }
        if (laser[7].visible) {
            framebuffer[laser[7].x][laser[7].y] = BLK;
            laser[7].x++;
            if (laser[7].x >= 32) {
                laser[7].visible = 0;
            } else {
                framebuffer[laser[7].x][laser[7].y] = RED;
            }
        }

        old_controller = controller;

        msdelay(20);

        // for (uint8_t i = 0; i < 3; i++) {
        //     for (uint8_t j = 0; j < 3; j++) {
        //         uint8_t x = 8 + i - 1;  // To center the asteroid on coordinate x
        //         uint8_t y = 8 + j - 1;  // To center the asteroid on coordinate y
        //         if (x >= 0 && y >= 0 && x < 32 && y < 16) {
        //             framebuffer[x][y] = asteroid_small[i][j];
        //         }
        //     }
        // }
    }

    // while (1) {
    //     test_external_ram(buffer, 2000);
    // }

    // while (1) {
    //     if (test_external_ram(buffer, 8192) == 0) {
    //         P3_5 = 0;
    //         msdelay(1000);
    //         P3_5 = 1;
    //         msdelay(1000);
    //     } else {
    //         P3_5 = 0;
    //         msdelay(250);
    //         P3_5 = 1;
    //         msdelay(250);
    //     }
    // }

    // while (1) {
    //     P3_5 = 0;
    //     msdelay(250);
    //     P3_5 = 1;
    //     msdelay(250);
    // }
}

void timer0() __interrupt(TF0_VECTOR) __using(1) {
    static uint8_t line = 0;

    uint8_t i;
    uint8_t cmd;

    __xdata uint8_t* m_ptr;
    __xdata uint8_t* f_ptr;

    TH0 = 0xEF;  // 2.5ms
    TL0 = 0xB9;
    TR0 = 1;  // restart timer 0

    cmd = line | 0x8;

    m_ptr = matrix;
    f_ptr = framebuffer;

    P3_5 = 1;

    f_ptr += (line << 5);

    i = 4;
    do {
        *(m_ptr + 8) = 0;

        *m_ptr++ = *f_ptr++;
        *m_ptr++ = *f_ptr++;
        *m_ptr++ = *f_ptr++;
        *m_ptr++ = *f_ptr++;
        *m_ptr++ = *f_ptr++;
        *m_ptr++ = *f_ptr++;
        *m_ptr++ = *f_ptr++;
        *m_ptr++ = *f_ptr++;

        *m_ptr = cmd;

        m_ptr += 8;
    } while (--i > 0);

    f_ptr += 224;

    i = 4;
    do {
        *(m_ptr + 8) = 0;

        *m_ptr++ = *f_ptr++;
        *m_ptr++ = *f_ptr++;
        *m_ptr++ = *f_ptr++;
        *m_ptr++ = *f_ptr++;
        *m_ptr++ = *f_ptr++;
        *m_ptr++ = *f_ptr++;
        *m_ptr++ = *f_ptr++;
        *m_ptr++ = *f_ptr++;

        *m_ptr = cmd;

        m_ptr += 8;
    } while (--i > 0);

    line++;
    if (line >= 8) line = 0;

    P3_5 = 0;
}

void timer1() __interrupt(TF1_VECTOR) __using(0) {
}

void timer2() __interrupt(TF2_VECTOR) __using(0) {
}

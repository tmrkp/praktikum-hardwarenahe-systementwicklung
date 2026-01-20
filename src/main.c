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

__pdata uint8_t* matrix = (__pdata uint8_t*) 0x00;

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

inline void render_laser(uint8_t index) {
    if (laser[index].visible) {
        framebuffer[laser[index].y][laser[index].x] = BLK;
        laser[index].x++;
        if (laser[index].x >= 32) {
            laser[index].visible = 0;
        } else {
            framebuffer[laser[index].y][laser[index].x] = RED;
        }
    }
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

    uint8_t i = 7;
    do {
        NES_CLK = 1;
        NES_CLK = 0;

        value <<= 1;
        value |= (NES_DATA == 0);
    } while (--i > 0);

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

    P2 = 0x40;

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

        render_laser(0);
        render_laser(1);
        render_laser(2);
        render_laser(3);
        render_laser(4);
        render_laser(5);
        render_laser(6);
        render_laser(7);

        old_controller = controller;

        msdelay(20);
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
    TR0 = 1;

    // prepare matrix cmd to write line
    cmd = line | 0x8;

    // base pointers to matrix and framebuffer
    m_ptr = matrix;
    f_ptr = framebuffer;

    P3_5 = 1;

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

    P3_5 = 0;
}

void timer1() __interrupt(TF1_VECTOR) __using(0) {
}

void timer2() __interrupt(TF2_VECTOR) __using(0) {
}

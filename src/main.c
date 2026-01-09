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
#define BTN_A 0x01
#define BTN_B 0x02
#define BTN_SELECT 0x04
#define BTN_START 0x08
#define BTN_UP 0x10
#define BTN_DOWN 0x20
#define BTN_LEFT 0x40
#define BTN_RIGHT 0x80

#define BLK (0x00)
#define RED (0x30)
#define GRN (0x0C)
#define BLU (0x03)

// Array for external Matrix
__xdata uint8_t framebuffer[32][16] = {
    {RED, RED, RED, RED, RED, RED, RED, RED, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU},
    {RED, RED, RED, RED, RED, RED, RED, RED, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU},
    {RED, RED, RED, RED, RED, RED, RED, RED, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU},
    {RED, RED, RED, RED, RED, RED, RED, RED, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU},
    {RED, RED, RED, RED, RED, RED, RED, RED, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU},
    {RED, RED, RED, RED, RED, RED, RED, RED, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU},
    {RED, RED, RED, RED, RED, RED, RED, RED, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU},
    {RED, RED, RED, RED, RED, RED, RED, RED, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU},
    {GRN, GRN, GRN, GRN, GRN, GRN, GRN, GRN, RED, RED, RED, RED, RED, RED, RED, RED},
    {GRN, GRN, GRN, GRN, GRN, GRN, GRN, GRN, RED, RED, RED, RED, RED, RED, RED, RED},
    {GRN, GRN, GRN, GRN, GRN, GRN, GRN, GRN, RED, RED, RED, RED, RED, RED, RED, RED},
    {GRN, GRN, GRN, GRN, GRN, GRN, GRN, GRN, RED, RED, RED, RED, RED, RED, RED, RED},
    {GRN, GRN, GRN, GRN, GRN, GRN, GRN, GRN, RED, RED, RED, RED, RED, RED, RED, RED},
    {GRN, GRN, GRN, GRN, GRN, GRN, GRN, GRN, RED, RED, RED, RED, RED, RED, RED, RED},
    {GRN, GRN, GRN, GRN, GRN, GRN, GRN, GRN, RED, RED, RED, RED, RED, RED, RED, RED},
    {GRN, GRN, GRN, GRN, GRN, GRN, GRN, GRN, RED, RED, RED, RED, RED, RED, RED, RED},
    {RED, RED, RED, RED, RED, RED, RED, RED, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU},
    {RED, RED, RED, BLU, RED, RED, RED, RED, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU},
    {RED, RED, RED, RED, RED, RED, RED, RED, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU},
    {RED, RED, RED, RED, RED, RED, RED, RED, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU},
    {RED, RED, RED, RED, RED, RED, RED, RED, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU},
    {RED, RED, RED, RED, RED, RED, RED, RED, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU},
    {RED, RED, RED, RED, RED, RED, RED, RED, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU},
    {RED, RED, RED, RED, RED, RED, RED, RED, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU},
    {GRN, GRN, GRN, GRN, GRN, GRN, GRN, GRN, RED, RED, RED, RED, RED, RED, RED, RED},
    {GRN, GRN, GRN, GRN, GRN, GRN, GRN, GRN, RED, RED, RED, RED, RED, RED, RED, RED},
    {GRN, GRN, GRN, GRN, GRN, GRN, GRN, GRN, RED, RED, RED, RED, RED, RED, RED, RED},
    {GRN, GRN, GRN, BLU, GRN, GRN, GRN, GRN, RED, RED, RED, RED, RED, BLU, RED, RED},
    {GRN, GRN, GRN, GRN, GRN, GRN, GRN, GRN, RED, RED, RED, RED, RED, RED, RED, RED},
    {GRN, GRN, GRN, GRN, GRN, GRN, GRN, GRN, RED, RED, RED, RED, RED, RED, RED, RED},
    {GRN, GRN, GRN, GRN, GRN, GRN, GRN, GRN, RED, RED, RED, RED, RED, RED, RED, RED},
    {GRN, GRN, GRN, GRN, GRN, GRN, GRN, GRN, RED, RED, RED, RED, RED, RED, RED, RED},
};

// Counter for lines in framebuffer
uint8_t line_counter = 0;

__xdata uint8_t* matrix = (__xdata uint8_t*) 0x4000;

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

    bool pressed = (NES_DATA == 0);
    value |= pressed;

    for (uint8_t i = 1; i < 8; i++) {
        NES_CLK = 1;
        NES_CLK = 0;

        pressed = (NES_DATA == 0);
        value |= (pressed << i);

        // if (pressed) {
        //     matrix[i] = 0b00110000;
        // } else {
        //     matrix[i] = 0b0000000;
        // }
    }

    // matrix[8] = 0b00001000;

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

    // matrix1[0] = 0b00110000;
    // matrix1[1] = 0b00110000;
    // matrix1[2] = 0b00110000;
    // matrix1[3] = 0b00110000;
    // matrix1[4] = 0b00110000;
    // matrix1[5] = 0b00110000;
    // matrix1[6] = 0b00110000;
    // matrix1[7] = 0b00110000;
    // matrix1[8] = 0b00001000;

    

    while (1) {
        read_controller();
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

void frame_translator(uint8_t line) {
    uint8_t x = 0;
    uint8_t y = line;
    for (uint8_t i = 0; i < 8; i++) {
        matrix[16 * i + 8] = 0;
        for (uint8_t j = 0; j < 8; j++) {
            matrix[16 * i + j] = framebuffer[x][y];
            x++;
        }
        matrix[16 * i + 8] = line | 0x08;
        if (i == 3) {
            y = y + 8;
        }
        x %= 32;
    }
}

void timer0() __interrupt(TF0_VECTOR) __using(0) {
    TH0 = 0xEF;  // 2.5ms
    TL0 = 0xB9;

    TR0 = 1;  // restart timer 0

    P3_5 = !P3_5;

    frame_translator(line_counter);
    line_counter++;
    line_counter %= 8;
}

void timer1() __interrupt(TF1_VECTOR) __using(0) {
}

void timer2() __interrupt(TF2_VECTOR) __using(0) {
}

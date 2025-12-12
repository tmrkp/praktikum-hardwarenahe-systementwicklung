#ifdef __INTELLISENSE__
#undef __SDCC_mcs51
#endif

#include <at89x52.h>
#include <lint.h>
#include <stdint.h>

#define cbi(sfr, bit) ((sfr) &= ~(1 << (bit)))
#define sbi(sfr, bit) ((sfr) |= (1 << (bit)))

__xdata uint8_t buffer[8192];

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

void main() {
    // while (1) {
    //     test_external_ram(buffer, 2000);
    // }

    while (1) {
        if (test_external_ram(buffer, 8192) == 0) {
            P3_5 = 0;
            msdelay(1000);
            P3_5 = 1;
            msdelay(1000);
        } else {
            P3_5 = 0;
            msdelay(250);
            P3_5 = 1;
            msdelay(250);
        }
    }

    while (1) {
        P3_5 = 0;
        msdelay(250);
        P3_5 = 1;
        msdelay(250);
    }
}

void timer0(void) __interrupt(1) __using(0) {
}

void timer1(void) __interrupt(3) __using(0) {
}

void timer2(void) __interrupt(5) __using(0) {
}
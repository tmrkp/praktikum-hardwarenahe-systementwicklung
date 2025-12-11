#include <at89x52.h>
#include <stdint.h>

#define cbi(sfr, bit) ((sfr) &= ~(1 << (bit)))
#define sbi(sfr, bit) ((sfr) |= (1 << (bit)))

void msdelay(uint16_t value) {
    for (uint16_t x = 0; x < value; x++) {
        for (uint16_t y = 0; y < 1275; y++);
    }
}

void main() {
    while (1) {
        P1_2 = 0;
        msdelay(250);
        P1_2 = 1;
        msdelay(250);
    }
}
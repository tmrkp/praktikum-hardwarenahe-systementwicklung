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

#define WIDTH 32
#define HEIGHT 16

#define BLK (0x00)
#define RED (0x30)
#define GRN (0x0C)
#define BLU (0x03)

typedef uint8_t framebuffer_t[HEIGHT][WIDTH];

// __xdata framebuffer_t framebuffer;

__xdata framebuffer_t framebuffer = {
    /* y = 0 */
    {
        RED, RED, RED, RED, RED, RED, RED, RED, RED, RED, RED, RED, RED, RED, RED, RED,
        RED, RED, RED, RED, RED, RED, RED, RED, RED, RED, RED, RED, RED, RED, RED, RED},
    /* y = 1 */
    {
        RED, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU, RED,
        RED, BLK, BLK, BLK, BLK, BLK, BLK, BLK, BLK, BLK, BLK, BLK, BLK, BLK, BLK, RED},
    /* y = 2 */
    {
        RED, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU, RED,
        RED, BLK, GRN, BLK, BLK, BLK, BLK, BLK, BLK, BLK, BLK, BLK, BLK, BLK, BLK, RED},
    /* y = 3 */
    {
        RED, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU, RED,
        RED, BLK, BLK, GRN, BLK, BLK, BLK, BLK, BLK, BLK, BLK, BLK, BLK, BLK, BLK, RED},
    /* y = 4 */
    {
        RED, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU, RED,
        RED, BLK, BLK, BLK, GRN, BLK, BLK, BLK, BLK, BLK, BLK, BLK, BLK, BLK, BLK, RED},
    /* y = 5 */
    {
        RED, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU, RED,
        RED, BLK, BLK, BLK, BLK, GRN, BLK, BLK, BLK, BLK, BLK, BLK, BLK, BLK, BLK, RED},
    /* y = 6 */
    {
        RED, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU, RED,
        RED, BLK, BLK, BLK, BLK, BLK, GRN, BLK, BLK, BLK, BLK, BLK, BLK, BLK, BLK, RED},
    /* y = 7 */
    {
        RED, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU, RED,
        RED, BLK, BLK, BLK, BLK, BLK, BLK, GRN, BLK, BLK, BLK, BLK, BLK, BLK, BLK, RED},
    /* y = 8 */
    {
        RED, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU, RED,
        RED, BLK, BLK, BLK, BLK, BLK, BLK, BLK, GRN, BLK, BLK, BLK, BLK, BLK, BLK, RED},
    /* y = 9 */
    {
        RED, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU, RED,
        RED, BLK, BLK, BLK, BLK, BLK, BLK, BLK, BLK, GRN, BLK, BLK, BLK, BLK, BLK, RED},
    /* y = 10 */
    {
        RED, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU, RED,
        RED, BLK, BLK, BLK, BLK, BLK, BLK, BLK, BLK, BLK, GRN, BLK, BLK, BLK, BLK, RED},
    /* y = 11 */
    {
        RED, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU, RED,
        RED, BLK, BLK, BLK, BLK, BLK, BLK, BLK, BLK, BLK, BLK, GRN, BLK, BLK, BLK, RED},
    /* y = 12 */
    {
        RED, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU, RED,
        RED, BLK, BLK, BLK, BLK, BLK, BLK, BLK, BLK, BLK, BLK, BLK, GRN, BLK, BLK, RED},
    /* y = 13 */
    {
        RED, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU, RED,
        RED, BLK, BLK, BLK, BLK, BLK, BLK, BLK, BLK, BLK, BLK, BLK, BLK, GRN, BLK, RED},
    /* y = 14 */
    {
        RED, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU, BLU, RED,
        RED, BLK, BLK, BLK, BLK, BLK, BLK, BLK, BLK, BLK, BLK, BLK, BLK, BLK, GRN, RED},
    /* y = 15 */
    {
        RED, RED, RED, RED, RED, RED, RED, RED, RED, RED, RED, RED, RED, RED, RED, RED,
        RED, RED, RED, RED, RED, RED, RED, RED, RED, RED, RED, RED, RED, RED, RED, RED},
};

__xdata uint8_t* matrix = (__xdata uint8_t*) 0x4000;

uint8_t line_counter = 0;

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

// const uint8_t asteroid_middle[4][4] = {{BLU, BLU, BLK, BLU},
//                                        {BLK, BLU, BLU, BLU},
//                                        {BLU, BLU, BLU, BLU},
//                                        {BLU, BLU, BLK, BLK}};

// const uint8_t asteroid_big[6][5] = {{BLK, BLU, BLK, BLU, BLK},
//                                     {BLU, BLU, BLU, BLU, BLU},
//                                     {BLU, BLU, BLU, BLU, BLU},
//                                     {BLU, BLU, BLU, BLU, BLU},
//                                     {BLU, BLU, BLU, BLU, BLU},
//                                     {BLK, BLU, BLK, BLU, BLK}};

// const uint8_t asteroid_small[3][3] = {{BLK, BLK, BLU},
//                                       {BLK, BLU, BLU},
//                                       {BLU, BLU, BLU}};

asteroid_t a1 = {8, 8, ASTEROID_SHAPE_SMALL};
asteroid_t a2 = {8, 8, ASTEROID_SHAPE_SMALL};

void render_asteroid_shape(asteroid_t* asteroid, framebuffer_t framebuffer) {
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

void render(framebuffer_t framebuffer) {
    // for (uint8_t i = 0; i < 32; i++) {
    //     for (uint8_t j = 0; j < 16; j++) {
    //         framebuffer[i][j] = BLK;
    //     }
    // }
    render_asteroid_shape(&a2, framebuffer);
    render_asteroid_shape(&a1, framebuffer);

    a1.x++;
    if (a1.x >= 32) a1.x -= 32;

    a2.y++;
    if (a2.y >= 16) a2.y -= 16;
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
    P3_5 = 1;  // disable LED

    NES_LATCH = 0;
    NES_CLK   = 0;

    // interrupt config
    EA  = 1;  // enable interrupts
    ET0 = 1;  // enable timer 0 interrupt

    // timer0 config
    TMOD = M0_0;  // 16-bit mode
    TH0  = 0xEF;  // 2.5ms
    TL0  = 0xB9;
    TR0  = 1;  // enable timer0

    while (1) {
        __asm__("nop");
        // read_controller();

        // render(framebuffer);

        // for (uint8_t i = 0; i < 3; i++) {
        //     for (uint8_t j = 0; j < 3; j++) {
        //         uint8_t x = 8 + i - 1;  // To center the asteroid on coordinate x
        //         uint8_t y = 8 + j - 1;  // To center the asteroid on coordinate y
        //         if (x >= 0 && y >= 0 && x < 32 && y < 16) {
        //             framebuffer[x][y] = asteroid_small[i][j];
        //         }
        //     }
        // }
        // tick_delay(10);
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

void timer0() __interrupt(TF0_VECTOR) __naked {
    // iterating matrix address is kept in P2:R0
    // framebuffer address is kept in DPH:DPL

    // P2 = matrix address high byte

    // R0 = matrix address low byte (iterator)
    // R1 = matrix address low byte (for line clear)
    // R2 = framebuffer address low byte
    // R3 = framebuffer address high byte
    // R4 = matrix cmd (line | 0x08)
    // R5 = matrix segement loop
    // R6
    // R7

    // clang-format off
__asm
; Debug LED
    ;setb	_P3_5

; Prologue: select bank 1
    push    psw
    mov     psw,#0x08

; Prologue: save context
    push    acc
    push    b
    push    dpl
    push    dph
    push    p2

; Restart Timer
    mov	    _TH0,#0xef
    mov	    _TL0,#0xb9
    setb	_TR0

; Prepare matrix cmd
    mov     a,_line_counter
    orl     a,#0x08
    mov     r4,a

; Load matrix address pointers
    mov     dptr,#_matrix
    mov     a,dpl
    mov     r0,a
    add     a,#8
    mov     r1,a
    mov     a,dph
    mov     p2,a

; Load framebuffer address pointer DPTR
    mov     dptr,#_framebuffer

; Offset framebuffer address (DPTR) + line counter (R7) * 16 * 2
    mov     a,_line_counter
    swap    a
    rl      a
    add     a,dpl
    mov     dpl,a
    mov     a, dph
    addc    a,#0
    mov     dph,a

; Save framebuffer address in R3:R2
    mov     r3,dph
    mov     r2,dpl

; loop through first 4 matrix modules
    mov     r5,#4
matrix_modules_loop_0:

; clear matrix line
    mov     a,#0
    movx    @r1,a

; Read/Write 8 Pixels
    movx    a,@dptr
    inc     dptr
    movx    @r0,a
    inc     r0
    movx    a,@dptr
    inc     dptr
    movx    @r0,a
    inc     r0
    movx    a,@dptr
    inc     dptr
    movx    @r0,a
    inc     r0
    movx    a,@dptr
    inc     dptr
    movx    @r0,a
    inc     r0
    movx    a,@dptr
    inc     dptr
    movx    @r0,a
    inc     r0
    movx    a,@dptr
    inc     dptr
    movx    @r0,a
    inc     r0
    movx    a,@dptr
    inc     dptr
    movx    @r0,a
    inc     r0
    movx    a,@dptr
    inc     dptr
    movx    @r0,a
    inc     r0

; send matrix enable cmd
    mov     a,r4
    movx    @r0,a
    
; advance matrix address pointers to next module
    mov     a,r0
    add     a,#8
    mov     r0,a
    add     a,#8
    mov     r1,a

    djnz    r5,matrix_modules_loop_0

; advance to the second row in the framebuffer
    mov     dpl,r2
    mov     dph,r3
    inc     dph

; loop through second 4 matrix modules
    mov     r5,#4
matrix_modules_loop_1:

; clear matrix line
    mov     a,#0
    movx    @r1,a

; Read/Write 8 Pixels
    movx    a,@dptr
    inc     dptr
    movx    @r0,a
    inc     r0
    movx    a,@dptr
    inc     dptr
    movx    @r0,a
    inc     r0
    movx    a,@dptr
    inc     dptr
    movx    @r0,a
    inc     r0
    movx    a,@dptr
    inc     dptr
    movx    @r0,a
    inc     r0
    movx    a,@dptr
    inc     dptr
    movx    @r0,a
    inc     r0
    movx    a,@dptr
    inc     dptr
    movx    @r0,a
    inc     r0
    movx    a,@dptr
    inc     dptr
    movx    @r0,a
    inc     r0
    movx    a,@dptr
    inc     dptr
    movx    @r0,a
    inc     r0

; send matrix enable cmd
    mov     a,r4
    movx    @r0,a
    
; advance matrix address pointers to next module
    mov     a,r0
    add     a,#8
    mov     r0,a
    add     a,#8
    mov     r1,a

    djnz    r5,matrix_modules_loop_1

; increment line counter
    mov     a,_line_counter
    inc     a
    anl     a,#0x07
    mov     _line_counter,a

; Epilogue: Restore Context
    pop     p2
    pop     dph
    pop     dpl
    pop     b
    pop     acc
    pop     psw

; Debug LED
    clr	_P3_5

    reti

__endasm;
    // clang-format on
}

void timer1() __interrupt(TF1_VECTOR) __using(0) {
}

void timer2() __interrupt(TF2_VECTOR) __using(0) {
}

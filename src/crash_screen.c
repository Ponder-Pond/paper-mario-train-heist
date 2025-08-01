#include "common.h"
#include <stdarg.h>
#include "PR/os_internal_thread.h"
#include <stdio.h>
#include <string.h>
#include "dx/backtrace.h"
#include "include_asset.h"

typedef struct {
    /* 0x000 */ OSThread thread;
    /* 0x1B0 */ char stack[0x800];
    /* 0x9B0 */ OSMesgQueue queue;
    /* 0x9C8 */ OSMesg mesg;
    /* 0x9CC */ u16* frameBuf;
    /* 0x9D0 */ u16 width;
    /* 0x9D2 */ u16 height;
} CrashScreen; // size = 0x9D4

BSS CrashScreen gCrashScreen;

u8 gCrashScreencharToGlyph[128] = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 41, -1, -1, -1, 43, -1, -1, 37, 38, -1, 42,
    -1, 39, 44, -1, 0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  36, -1, -1, -1, -1, 40, -1, 10,
    11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32,
    33, 34, 35, -1, -1, -1, -1, -1, -1, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22,
    23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, -1, -1, -1, -1, -1,
};

INCLUDE_IMG("crash_screen/font.png", gCrashScreenFont);

// The font image is on 6x7 grid
#define GLYPH(x, y) (x + (y * 5))

const char* gFaultCauses[18] = {
    "Interrupt",
    "TLB modification",
    "TLB exception on load",
    "TLB exception on store",
    "Address error on load",
    "Address error on store",
    "Bus error on inst.",
    "Bus error on data",
    "System call exception",
    "Breakpoint exception",
    "Reserved instruction",
    "Coprocessor unusable",
    "Arithmetic overflow",
    "Trap exception",
    "Virtual coherency on inst.",
    "Floating point exception",
    "Watchpoint exception",
    "Virtual coherency on data",
};

const char* gFPCSRFaultCauses[6] = {
    "Unimplemented operation",
    "Invalid operation",
    "Division by zero",
    "Overflow",
    "Underflow",
    "Inexact operation",
};

char crashScreenAssertMessage[0x100] = {0};

void crash_screen_set_assert_info(const char* message) {
    strncpy(crashScreenAssertMessage, message, sizeof(crashScreenAssertMessage));
    crashScreenAssertMessage[sizeof(crashScreenAssertMessage) - 1] = '\0';
}

void crash_screen_sleep(s32 ms) {
    u64 cycles = ms * 1000LL * 46875000LL / 1000000ULL;

    osSetTime(0);

    while (osGetTime() < cycles) {
        // wait
    }
}

void crash_screen_draw_rect(s32 x, s32 y, s32 width, s32 height) {
    u16* ptr;
    s32 i;
    s32 j;

    if (gCrashScreen.width == (SCREEN_WIDTH * 2)) {
        x <<= 1;
        y <<= 1;
        width <<= 1;
        height <<= 1;
    }

    ptr = gCrashScreen.frameBuf + gCrashScreen.width * y + x;

    for (i = 0; i < height; i++) {
        for (j = 0; j < width; j++) {
            *ptr = ((*ptr & 0xE738) >> 2) | 1;
            ptr++;
        }

        ptr += gCrashScreen.width - width;
    }
}

/** @returns X advance */
s32 crash_screen_draw_glyph(s32 x, s32 y, s32 glyph) {
    s32 shift = ((glyph % 5) * 6);
    u16 width = gCrashScreen.width;
    const u32* data = &((u32*)gCrashScreenFont)[glyph / 5 * 7];
    s32 i;
    s32 j;

    switch (glyph) {
        case GLYPH(3, 10): // ;
        case GLYPH(4, 10): // ,
            y += 1;
            break;
        case GLYPH(1, 14): // g
        case GLYPH(0, 16): // p
        case GLYPH(1, 16): // q
        case GLYPH(4, 17): // y
            y += 2;
            break;
    }

    if (width == SCREEN_WIDTH) {
        u16* ptr = gCrashScreen.frameBuf + (gCrashScreen.width) * y + x;

        for (i = 0; i < 7; i++) {
            u32 bit = 0x80000000U >> shift;
            u32 rowMask = *data++;

            for (j = 0; j < 6; j++) {
                if (bit & rowMask) {
                    *ptr++ = 0xFFFF; // white
                } else {
                    ptr++; // dont draw
                }
                bit >>= 1;
            }

            ptr += gCrashScreen.width - 6;
        }
    } else if (width == (SCREEN_WIDTH * 2)) {
        u16* ptr = gCrashScreen.frameBuf + (y * 0x500) + (x * 2);

        for (i = 0; i < 7; i++) {
            u32 bit = 0x80000000U >> shift;
            u32 rowMask = *data++;

            for (j = 0; j < 6; j++) {
                u16 temp = (bit & rowMask) ? 0xFFFF : 1;

                ptr[0] = temp;
                ptr[1] = temp;
                ptr[(SCREEN_WIDTH * 2)] = temp;
                ptr[(SCREEN_WIDTH * 2) + 1] = temp;
                ptr += 2;
                bit >>= 1;
            }

            ptr += (0x9E8 / 2);
        }
    }

    // Calculate x advance by counting the width of the glyph + 1 pixel of padding
    if (glyph == GLYPH(2, 15)) return 7; // m - fucked up hack
    s32 xAdvance = 0;
    data = &((u32*)gCrashScreenFont)[glyph / 5 * 7];
    for (i = 0; i < 7; i++) { // 7 rows
        u32 bit = 0x80000000U >> shift;
        u32 rowMask = *data++;
        for (j = 1; j < 6; j++) { // 6 columns
            if (bit & rowMask) {
                if (xAdvance < j) {
                    xAdvance = j;
                }
            }
            bit >>= 1;
        }
    }
    return xAdvance + 1;
}

char* crash_screen_copy_to_buf(char* dest, const char* src, size_t size) {
    memcpy(dest, src, size);
    return dest + size;
}

/// @returns Y advance
s32 crash_screen_printf(s32 x, s32 y, const char* fmt, ...) {
    u8* ptr;
    u32 glyph;
    s32 size;
    u8 buf[0x100];
    va_list args;
    s32 ox = x;

    va_start(args, fmt);

    size = _Printf(crash_screen_copy_to_buf, (s8*)buf, fmt, args);

    if (size > 0) {
        ptr = buf;

        while (size > 0) {
            u8* charToGlyph = gCrashScreencharToGlyph;

            glyph = charToGlyph[*ptr & 0x7F];

            if (glyph != 0xFF) {
                crash_screen_draw_glyph(x, y, glyph);
            }

            x += 6;

            if (*ptr == '\n') {
                x = ox;
                y += 10;
            }

            size--;
            ptr++;
        }
    }

    // If last character was not a newline, move to the next line
    if (x != ox) {
        y += 10;
    }
    return y;

    va_end(args);
}

/// @returns Y advance
s32 crash_screen_printf_proportional(s32 x, s32 y, const char* fmt, ...) {
    u8* ptr;
    u32 glyph;
    s32 size;
    u8 buf[0x200];
    va_list args;
    s32 ox = x;

    va_start(args, fmt);

    size = _Printf(crash_screen_copy_to_buf, (s8*)buf, fmt, args);

    if (size > 0) {
        ptr = buf;

        while (size > 0) {
            u8* charToGlyph = gCrashScreencharToGlyph;

            glyph = charToGlyph[*ptr & 0x7F];

            if (glyph != 0xFF) {
                x += crash_screen_draw_glyph(x, y, glyph);
            } else {
                x += 4;
            }

            if (*ptr == '\n') {
                x = ox;
                y += 10;
            }

            size--;
            ptr++;
        }
    }

    // If last character was not a newline, move to the next line
    if (x != ox) {
        y += 10;
    }
    return y;

    va_end(args);
}

void crash_screen_print_fpr(s32 x, s32 y, s32 regNum, void* addr) {
    u32 bits = *(u32*)addr;
    s32 exponent = ((bits & 0x7F800000U) >> 0x17) - 0x7F;

    if ((exponent >= -0x7E && exponent <= 0x7F) || bits == 0) {
        crash_screen_printf(x, y, "F%02d:%+.3e", regNum, *(f32*)addr);
    } else {
        crash_screen_printf(x, y, "F%02d:---------", regNum);
    }
}

void crash_screen_print_fpcsr(u32 value) {
    s32 i;
    u32 flag = 0x20000;

    crash_screen_printf(30, 155, "FPCSR:%08XH", value);

    for (i = 0; i < 6;) {
        if (value & flag) {
            crash_screen_printf(132, 155, "(%s)", gFPCSRFaultCauses[i]);
            break;
        }

        i++;
        flag >>= 1;
    }
}

void crash_screen_draw(OSThread* faultedThread) {
    s16 causeIndex;

    s32 bt[8];
    s32 max = backtrace_thread((void**)bt, ARRAY_COUNT(bt), faultedThread);
    s32 i = 0;
    static char buf[0x200];

    causeIndex = ((faultedThread->context.cause >> 2) & 0x1F);

    if (causeIndex == 23) {
        causeIndex = 16;
    }

    if (causeIndex == 31) {
        causeIndex = 17;
    }

    osWritebackDCacheAll();

    s32 x = 10;
    s32 y = 10;

    crash_screen_draw_rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

    // Print error message
    b32 isException = FALSE;
    if (crashScreenAssertMessage[0] == '\0') {
        y += crash_screen_printf_proportional(x, y, "Exception in thread %d: %s", faultedThread->id, gFaultCauses[causeIndex]);
        isException = TRUE;
    } else {
        y += crash_screen_printf_proportional(x, y, crashScreenAssertMessage);
        i = 1; // Don't include is_debug_panic line in backtrace.
    }

    // Print register values
    // TODO: print registers relevant to the exception
    if (isException) {
        __OSThreadContext* ctx = &faultedThread->context;
        crash_screen_printf_proportional(x, y, "Registers:");
        y += 10;
        crash_screen_printf(x, y, "  a0 = 0x%08X     a1 = 0x%08X", (u32)ctx->a0, (u32)ctx->a1);
        y += 10;
        crash_screen_printf(x, y, "  a2 = 0x%08X     a3 = 0x%08X", (u32)ctx->a2, (u32)ctx->a3);
        y += 10;

        y += 10;
    }

    // Print backtrace
    crash_screen_printf_proportional(x, y, "Call stack:");
    y += 10;
    for (; i < max; i++) {
        backtrace_address_to_string(bt[i], buf);
        crash_screen_printf_proportional(x, y, "  in %s", buf);
        y += 10;
    }

    y += 10;

    osViBlack(0);
    osViRepeatLine(0);
    osViSwapBuffer(gCrashScreen.frameBuf);

    /*
    crash_screen_draw_rect(0, 30, SCREEN_WIDTH, SCREEN_HEIGHT - 30);

    crash_screen_printf(30, 35, "PC:%08XH   SR:%08XH   VA:%08XH", ctx->pc, ctx->sr, ctx->badvaddr);
    crash_screen_printf(30, 50, "AT:%08XH   V0:%08XH   V1:%08XH", (u32)ctx->at, (u32)ctx->v0, (u32)ctx->v1);
    crash_screen_printf(30, 45, "A0: %08X   A1: %08X   A2: %08X", (u32)ctx->a0, (u32)ctx->a1, (u32)ctx->a2);
    crash_screen_printf(30, 70, "A3:%08XH   T0:%08XH   T1:%08XH", (u32)ctx->a3, (u32)ctx->t0, (u32)ctx->t1);
    crash_screen_printf(30, 80, "T2:%08XH   T3:%08XH   T4:%08XH", (u32)ctx->t2, (u32)ctx->t3, (u32)ctx->t4);
    crash_screen_printf(30, 90, "T5:%08XH   T6:%08XH   T7:%08XH", (u32)ctx->t5, (u32)ctx->t6, (u32)ctx->t7);
    crash_screen_printf(30, 100, "S0:%08XH   S1:%08XH   S2:%08XH", (u32)ctx->s0, (u32)ctx->s1, (u32)ctx->s2);
    crash_screen_printf(30, 110, "S3:%08XH   S4:%08XH   S5:%08XH", (u32)ctx->s3, (u32)ctx->s4, (u32)ctx->s5);
    crash_screen_printf(30, 120, "S6:%08XH   S7:%08XH   T8:%08XH", (u32)ctx->s6, (u32)ctx->s7, (u32)ctx->t8);
    crash_screen_printf(30, 130, "T9:%08XH   GP:%08XH   SP:%08XH", (u32)ctx->t9, (u32)ctx->gp, (u32)ctx->sp);
    crash_screen_printf(30, 140, "S8:%08XH   RA:%08XH", (u32)ctx->s8, (u32)ctx->ra);

    crash_screen_print_fpcsr(ctx->fpcsr);

    crash_screen_print_fpr(30, 170, 0, &ctx->fp0.f.f_even);
    crash_screen_print_fpr(120, 170, 2, &ctx->fp2.f.f_even);
    crash_screen_print_fpr(210, 170, 4, &ctx->fp4.f.f_even);
    crash_screen_print_fpr(30, 180, 6, &ctx->fp6.f.f_even);
    crash_screen_print_fpr(120, 180, 8, &ctx->fp8.f.f_even);
    crash_screen_print_fpr(210, 180, 10, &ctx->fp10.f.f_even);
    crash_screen_print_fpr(30, 190, 12, &ctx->fp12.f.f_even);
    crash_screen_print_fpr(120, 190, 14, &ctx->fp14.f.f_even);
    crash_screen_print_fpr(210, 190, 16, &ctx->fp16.f.f_even);
    crash_screen_print_fpr(30, 200, 18, &ctx->fp18.f.f_even);
    crash_screen_print_fpr(120, 200, 20, &ctx->fp20.f.f_even);
    crash_screen_print_fpr(210, 200, 22, &ctx->fp22.f.f_even);
    crash_screen_print_fpr(30, 210, 24, &ctx->fp24.f.f_even);
    crash_screen_print_fpr(120, 210, 26, &ctx->fp26.f.f_even);
    crash_screen_print_fpr(210, 210, 28, &ctx->fp28.f.f_even);
    crash_screen_print_fpr(30, 220, 30, &ctx->fp30.f.f_even);

    crash_screen_sleep(500);

    // all of these null terminators needed to pad the rodata section for this file
    // can potentially fix this problem in another way?
    crash_screen_printf(210, 140, "MM:%08XH\0\0\0\0\0\0\0\0", *(u32*)ctx->pc);
    */
}

OSThread* crash_screen_get_faulted_thread(void) {
    OSThread* thread = __osGetActiveQueue();

    while (thread->priority != -1) {
        if (thread->priority > 0 && thread->priority < 0x7F && (thread->flags & 3)) {
            return thread;
        }

        thread = thread->tlnext;
    }

    return NULL;
}

void crash_screen_thread_entry(void* unused) {
    OSMesg mesg;
    OSThread* faultedThread;

    osSetEventMesg(OS_EVENT_CPU_BREAK, &gCrashScreen.queue, (OSMesg)1);
    osSetEventMesg(OS_EVENT_FAULT, &gCrashScreen.queue, (OSMesg)2);

    do {
        osRecvMesg(&gCrashScreen.queue, &mesg, 1);
        faultedThread = crash_screen_get_faulted_thread();
    } while (faultedThread == NULL);

    osStopThread(faultedThread);
    crash_screen_draw(faultedThread);

    while (TRUE) {}
}

void crash_screen_set_draw_info(u16* frameBufPtr, s16 width, s16 height) {
    gCrashScreen.frameBuf = (u16*)((u32)frameBufPtr | 0xA0000000);
    gCrashScreen.width = width;
    gCrashScreen.height = height;
}

void crash_screen_init(void) {
    gCrashScreen.width = SCREEN_WIDTH;
    gCrashScreen.height = 16;
    gCrashScreen.frameBuf = (u16*)((osMemSize | 0xA0000000) - ((SCREEN_WIDTH * SCREEN_HEIGHT) * 2));
    osCreateMesgQueue(&gCrashScreen.queue, &gCrashScreen.mesg, 1);
    osCreateThread(&gCrashScreen.thread, 2, crash_screen_thread_entry, NULL,
                   gCrashScreen.stack + sizeof(gCrashScreen.stack), 0x80);
    osStartThread(&gCrashScreen.thread);

    // gCrashScreencharToGlyph is hard to modify, so we'll just do it here
    u8 chars[] =
        "_[]<>"
        "|{};,"
        "\"#$&'"
        "/=@\\`"
        "abcde"
        "fghij"
        "klmno"
        "pqrst"
        "uvwxy"
        "z";
    s32 i;
    for (i = 0; i < ARRAY_COUNT(chars); i++) {
        gCrashScreencharToGlyph[chars[i]] = GLYPH(0, 9) + i;
    }
}

// unused
void crash_screen_printf_with_bg(s16 x, s16 y, const char* fmt, ...) {
    u8* ptr;
    u32 glyph;
    s32 size;
    u8 buf[0x100];
    va_list args;

    va_start(args, fmt);

    size = _Printf(crash_screen_copy_to_buf, (s8*)buf, fmt, args);

    if (size > 0) {
        crash_screen_draw_rect(x - 6, y - 6, (size + 2) * 6, 19);
        ptr = buf;

        while (size > 0) {
            u8* charToGlyph = gCrashScreencharToGlyph;

            glyph = charToGlyph[*ptr & 0x7F];

            if (glyph != 0xFF) {
                crash_screen_draw_glyph(x, y, glyph);
            }

            x += 6;
            size--;
            ptr++;
        }
    }

    va_end(args);
}

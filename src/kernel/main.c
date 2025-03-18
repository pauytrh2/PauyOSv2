#include <stdint.h>
#include "stdio.h"
#include "memory.h"
#include <hal/hal.h>
#include <arch/i686/irq.h>
#include <arch/i686/io.h>
#include <debug.h>
#include <boot/bootparams.h>
#include <arch/i686/vga_text.h>
#include </home/pauytrh/PauyOSv2/image/root/system_resources/PDE/frame.h>

#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define MAX_INPUT_LENGTH 1024  // Maximum length of typed input

extern void _init();

static int cursor_x = 0;
static int cursor_y = 1; // 2nd line

static int shift_pressed = 0;
static int esc_pressed = 0;

static char input_buffer[MAX_INPUT_LENGTH];
static int input_index = 0;

static int reroute_keyboard_input = 0;

void VGA_word(const char* string, uint8_t color) {
    for (int i = 0; string[i] != '\0'; i++) {
        VGA_putcolor(i, 0, color);
        VGA_putc(string[i]);
    }
}

void timer(Registers* regs) {
    printf(".");
}

void PIC_remap() {
    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    outb(0x21, 0x20); // IRQ0-7 to 0x20-0x27
    outb(0xA1, 0x28); // IRQ8-15 to 0x28-0x2F
    outb(0x21, 0x04);
    outb(0xA1, 0x02);
    outb(0x21, 0x01);
    outb(0xA1, 0x01);
    outb(0x21, 0x0);
    outb(0xA1, 0x0);
}

static char scancode_to_ascii[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0, ' ', 0,
};

static char scancode_to_ascii_shifted[128] = {
    0,  27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',
    0, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,
    '*', 0, ' ', 0,
};

void keyboard_handler(Registers* regs) {
    uint8_t scancode = inb(0x60); // scancode from keyboard port

    log_debug("Keyboard", "Scancode received: %x", scancode);

    if (scancode & 0x80) {
        // key release
        scancode &= 0x7F; // remove released flag
        if (scancode == 0x2A || scancode == 0x36) { // Lshift or Rshift release
            shift_pressed = 0;
        }
    } else {
        // key press
        if (scancode == 0x2A || scancode == 0x36) { // Lshift or Rshift press
            shift_pressed = 1;
        } else if (scancode == 0x0E) { // backspace
            if (reroute_keyboard_input && input_index > 0) {
                input_index--;
                input_buffer[input_index] = '\0'; // null terminate the string
            }
        } else if (scancode == 0x01) { // esc
            esc_pressed = 1;
        } else {
            char key = 0;

            if (shift_pressed) {
                key = scancode_to_ascii_shifted[scancode];
            } else {
                key = scancode_to_ascii[scancode];
            }

            if (key) {
                if (reroute_keyboard_input && input_index < MAX_INPUT_LENGTH - 1) {
                    input_buffer[input_index++] = key;
                    input_buffer[input_index] = '\0'; // null terminate the string
                } else if (!reroute_keyboard_input) {
                    printf("%c", key);
                    cursor_x += 1;
                }
            }
        }
    }
}

void notepad() {
    log_info("Main/PDE", "Notpad is now active!");
    VGA_clrscr();
    printf("Notpad *active*\n\n");
    reroute_keyboard_input = 0; // enable typing

    while (1) {
        if (esc_pressed) {
            log_info("Main/PDE/notpad", "esc key pressed, exiting notpad...");
            VGA_clrscr();
            VGA_word(pde_text, 0x0F);
            printf("Press 'n' for notpad");
            reroute_keyboard_input = 1; // disable typing
            esc_pressed = 0;
            break;
        }
    }
}

void PDE() {
    VGA_clrscr();
    VGA_word(pde_text, 0x0F); // white
    printf("Press 'n' for notpad");
    reroute_keyboard_input = 1; // disable typing

    while (1) {
        if (input_index > 0 && input_buffer[input_index - 1] == 'n') {
            VGA_clrscr();
            VGA_word(pde_text, 0x0F); // white
            log_info("Main/PDE", "'n' key pressed!");
            notepad();
            input_buffer[0] = '\0';  // null terminate buffer to reset
            input_index = 0;         // reset index to 0
        }
    }
}

void start(BootParams* bootParams) {   

    _init();
    VGA_clrscr();
    HAL_Initialize();

    log_debug("Main", "Boot device: %x", bootParams->BootDevice);
    log_debug("Main", "Memory region count: %d", bootParams->Memory.RegionCount);
    for (int i = 0; i < bootParams->Memory.RegionCount; i++) {
        log_debug("Main", "MEM: start=0x%llx length=0x%llx type=%x", 
            bootParams->Memory.Regions[i].Begin,
            bootParams->Memory.Regions[i].Length,
            bootParams->Memory.Regions[i].Type);
    }

    log_info("Main", "This is an info msg!");
    log_warn("Main", "This is a warning msg!");
    log_err("Main", "This is an error msg!");
    log_crit("Main", "This is a critical msg!");

    VGA_setcursor(0, 0);

    VGA_word("Welcome to PauyOS!\n", 0x05); // magenta

    i686_IRQ_RegisterHandler(1, keyboard_handler);
    outb(0x21, inb(0x21) & ~0x02);

    PDE();
}
#include <stdint.h>
#include "stdio.h"
#include "memory.h"
#include <hal/hal.h>
#include <arch/i686/irq.h>
#include <arch/i686/io.h>
#include <debug.h>
#include <boot/bootparams.h>
#include <arch/i686/vga_text.h>

extern void _init();

void VGA_word(const char* string, uint8_t color)
{
    for (int i = 0; string[i] != '\0'; i++) {
        VGA_putcolor(i, 0, color);
        VGA_putc(string[i]);
    }
}

void timer(Registers* regs)
{
    printf(".");
}

void PIC_remap()
{
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

void keyboard_handler(Registers* regs)
{
    uint8_t scancode = inb(0x60); // scancode from keyboard port

    log_debug("Keyboard", "Scancode received: %x", scancode);

    if (scancode & 0x80) {
        // key release 
    } else {
        char key = scancode_to_ascii[scancode];
        if (key) {
            printf("%c", key);
        }
    }
}

void start(BootParams* bootParams)
{   

    _init();
    VGA_clrscr();
    HAL_Initialize();

    log_debug("Main", "Boot device: %x", bootParams->BootDevice);
    log_debug("Main", "Memory region count: %d", bootParams->Memory.RegionCount);
    for (int i = 0; i < bootParams->Memory.RegionCount; i++) 
    {
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

    for (;;);
}
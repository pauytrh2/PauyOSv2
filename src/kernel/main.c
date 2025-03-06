#include <stdint.h>
#include "stdio.h"
#include "memory.h"
#include <hal/hal.h>
#include <arch/i686/irq.h>
#include <debug.h>
#include <boot/bootparams.h>
#include <arch/i686/vga_text.h>

extern void _init();

void crash_me();

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

void start(BootParams* bootParams)
{   
    // call global constructors
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
    //i686_IRQ_RegisterHandler(0, timer);

    //crash_me();

end:
    for (;;);
}
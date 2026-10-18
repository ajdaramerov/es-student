#include <stdio.h>
#include <stdint.h>
#include "hardware/regs/addressmap.h"
#include "pico/stdlib.h"
#include "command.h"
#include <stdlib.h>
#include "device.h"
#include "led.h"

extern char __flash_binary_start;
extern char __flash_binary_end;
extern char __boot2_start__;
extern char __boot2_end__;
extern char __etext;
extern char __data_start__;
extern char __data_end__;
extern char __bss_start__;
extern char __bss_end__;
extern char __HeapLimit;
extern char __StackBottom;
extern char __StackTop;


static void row(const char *name, uintptr_t start, uintptr_t end)
{
       printf("%-10s 0x%08x 0x%08x %8u\n",
              name, (unsigned)start, (unsigned)end, (unsigned)(end - start));
}
void mem_info(void) {
       printf("%-10s %-10s %-10s %8s\n", "area", "start", "end", "size");

       row("flash", XIP_BASE, XIP_BASE + PICO_FLASH_SIZE_BYTES); 
       row("sram", SRAM_BASE, SRAM_END);                         
       row("rom", ROM_BASE, ROM_BASE + 16 * 1024);    

       row("image", (uintptr_t)&__flash_binary_start, (uintptr_t)&__flash_binary_end);
       row("free", (uintptr_t)&__flash_binary_end, (uintptr_t)XIP_BASE + PICO_FLASH_SIZE_BYTES);
       row("boot2", (uintptr_t)&__boot2_start__, (uintptr_t)&__boot2_end__);
       row("text", (uintptr_t)&__boot2_end__, (uintptr_t)&__etext);

       row("data flash", (uintptr_t)&__etext, (uintptr_t)&__etext + (&__data_end__ - &__data_start__));
       row("data ram", (uintptr_t)&__data_start__, (uintptr_t)&__data_end__); 
       row("bss", (uintptr_t)&__bss_start__, (uintptr_t)&__bss_end__);        
       row("heap", (uintptr_t)&__bss_end__, (uintptr_t)&__HeapLimit);         
       row("stack", (uintptr_t)&__StackBottom, (uintptr_t)&__StackTop); 

       uintptr_t flash_image_size  = (uintptr_t)&__flash_binary_end - (uintptr_t)&__flash_binary_start;
       uintptr_t boot2_size        = (uintptr_t)&__boot2_end__       - (uintptr_t)&__boot2_start__;
       uintptr_t text_size         = (uintptr_t)&__etext             - (uintptr_t)&__boot2_end__;
       uintptr_t data_flash_size   = (uintptr_t)&__flash_binary_end  - (uintptr_t)&__etext;
       uintptr_t data_ram_size     = (uintptr_t)&__data_end__        - (uintptr_t)&__data_start__;
       uintptr_t bss_size          = (uintptr_t)&__bss_end__         - (uintptr_t)&__bss_start__;
       uintptr_t heap_size         = (uintptr_t)&__HeapLimit         - (uintptr_t)&__bss_end__;
       uintptr_t stack_size        = (uintptr_t)&__StackTop          - (uintptr_t)&__StackBottom;
       uintptr_t flash_free_size   = PICO_FLASH_SIZE_BYTES - flash_image_size;
       uintptr_t ram_used_size     = data_ram_size + bss_size;

       printf("\ntotal\n");
       printf("  flash image %8d = boot2 %d + text %d + data %d\n",
       flash_image_size, boot2_size, text_size, data_flash_size);           
       printf("  flash free  %8d of %d\n", flash_free_size, PICO_FLASH_SIZE_BYTES); 
       printf("  ram used    %8d = data %d + bss %d\n",
       ram_used_size, data_ram_size, bss_size);                                
       printf("  ram free    %8d for heap and %d for stack\n", heap_size, stack_size);

}

uint32_t data_variable = 100;
uint32_t bss_variable;

int main(void);

void fw_info(void) {
       data_variable++;
       bss_variable++;

       uint16_t *main_code = (uint16_t *)((uintptr_t)main & ~1u);
       uint16_t *fw_info_code = (uint16_t *)((uintptr_t)fw_info & ~1u);

       uint32_t stack_variable = 1946;
       uint32_t *heap_variable = malloc(sizeof(uint32_t));

       if (heap_variable != NULL) {
              *heap_variable = 1951;
       }

       printf("%-15s %-14s %s\n", "object", "address", "value");

       printf("%-15s 0x%08x  0x%04x\n", "main",       (unsigned)(uintptr_t)main, *main_code);
       printf("%-15s 0x%08x  0x%04x\n", "fw_info",    (unsigned)(uintptr_t)fw_info, *fw_info_code);

       printf("%-15s 0x%08x\n", "commands", (unsigned)(uintptr_t)commands);

       for (int i = 0; i < command_count; i++) {
              printf("- %-13s 0x%08x\n", commands[i].name, (unsigned)(uintptr_t)commands[i].handler);
       }

       printf("%-15s 0x%08x  %s\n", "DEVICE_PROJECT", (unsigned)(uintptr_t)DEVICE_PROJECT, DEVICE_PROJECT);
       printf("%-15s 0x%08x  %s\n", "DEVICE_BOARD",   (unsigned)(uintptr_t)DEVICE_BOARD, DEVICE_BOARD);

       printf("%-15s 0x%08x  %u\n", "data_variable",  (unsigned)(uintptr_t)&data_variable, data_variable);
       printf("%-15s 0x%08x  %u\n", "bss_variable",   (unsigned)(uintptr_t)&bss_variable,  bss_variable);

       printf("%-15s 0x%08x  %u\n", "stack_variable", (unsigned)(uintptr_t)&stack_variable, stack_variable);

       printf("%-15s 0x%08x  %u\n", "heap_variable",  (unsigned)(uintptr_t)heap_variable,   *heap_variable);

       free(heap_variable);
       }

#define VECTOR_TABLE 0x10000100

void boot_info(void) {
       const uint32_t *vectors = (const uint32_t *)VECTOR_TABLE;

       uint32_t stack_top = vectors[0];
       uint32_t reset_handler = vectors[1];
       uint32_t reset_handler_even = reset_handler & ~1u;
       volatile uint32_t *gpio_in = (uint32_t *)0xd0000004;

       uint32_t level = (*gpio_in >> led_pin()) & 1u;
       uint32_t pin = gpio_get(led_pin());


       printf("vector table   0x%08x\n", (unsigned)vectors);
       printf("  stack top    0x%08x\n", (unsigned)stack_top);
       printf("  reset        0x%08x\n", (unsigned)reset_handler);
       printf("  reset (even) 0x%08x\n", (unsigned)reset_handler_even); 

       printf("gpio in        0x%08x\n", (unsigned)gpio_in);
       printf("  led bit      %u\n", (unsigned)level);
       printf("  gpio_get     %u\n", (unsigned)pin);

}
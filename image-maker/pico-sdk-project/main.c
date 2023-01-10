#include <stdio.h>
#include "pico/stdlib.h"

void *heap[0x800];
void *stacks[0x800 * 2];

extern void *run(void *stack_A, void *stack_B, void *heap_start, void *heap_end);

int main() {
    stdio_init_all();
    
    run(NULL, NULL, NULL, NULL);

    return 0;
}

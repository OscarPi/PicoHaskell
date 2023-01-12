#include <stdio.h>
#include "pico/stdlib.h"

#define HEAP_SIZE 0x800
#define STACK_SIZE 0x800

void *heap[HEAP_SIZE];
void *stacks[STACK_SIZE * 2];

extern void *run(void *stack_A, void *stack_B, void *heap_start, void *heap_end);

int main() {
    stdio_init_all();
    
    run(&stacks[0], &stacks[(STACK_SIZE*2)-1], &heap[0], &heap[HEAP_SIZE]);

    return 0;
}

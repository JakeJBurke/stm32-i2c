
.syntax unified
.cpu cortex-m4
.thumb

.global Reset_Handler
.global _estack

.section .isr_vector, "a", %progbits
.type isr_vector, %object
isr_vector:
    .word _estack
    .word Reset_Handler
    .word 0
    .word 0
    .word 0
    .word 0
    .word 0
    .word 0
    .word 0
    .word 0
    .word 0
    .word 0
    .word 0
    .word 0
    .word 0
    .word 0

.section .text.Reset_Handler
.type Reset_Handler, %function
Reset_Handler:
    ldr r0, = _estack
    mov sp, r0
    bl main
    b .
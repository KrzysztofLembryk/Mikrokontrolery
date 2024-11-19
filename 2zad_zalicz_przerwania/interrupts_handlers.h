#ifndef INTERRUPTS_HANDLERS_H
#define INTERRUPTS_HANDLERS_H

#include <delay.h>
#include <gpio.h>
#include <stm32.h>

void init_interrupts_handlers_data();

void DMA1_Stream6_IRQHandler();

void DMA1_Stream5_IRQHandler();

void EXTI15_10_IRQHandler();

#endif // INTERRUPTS_HANDLERS_H
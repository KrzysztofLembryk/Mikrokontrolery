#include "diods.h"
#include "register_consts.h"
#include "init_funcs.h"

void init_diods()
{
    __NOP();
    RedLEDoff();
    GreenLEDoff();
    BlueLEDoff();
    Green2LEDoff();

    GPIOoutConfigure(RED_LED_GPIO,
                     RED_LED_PIN,
                     GPIO_OType_PP,
                     GPIO_Low_Speed,
                     GPIO_PuPd_NOPULL);

    GPIOoutConfigure(BLUE_LED_GPIO,
                     BLUE_LED_PIN,
                     GPIO_OType_PP,
                     GPIO_Low_Speed,
                     GPIO_PuPd_NOPULL);

    GPIOoutConfigure(GREEN_LED_GPIO,
                     GREEN_LED_PIN,
                     GPIO_OType_PP,
                     GPIO_Low_Speed,
                     GPIO_PuPd_NOPULL);

    GPIOoutConfigure(GREEN2_LED_GPIO,
                     GREEN2_LED_PIN,
                     GPIO_OType_PP,
                     GPIO_Low_Speed,
                     GPIO_PuPd_NOPULL);
}

void init_rcc()
{
    // Czym jest RCC:
    // --> RCC (Reset and Clock Control) is a peripheral
    // in STM32 microcontrollers that manages the system clocks and resets.
    // It controls the clock distribution to various peripherals and modules 
    // within the microcontroller, enabling or disabling their clocks as needed.
    // This is crucial for power management and ensuring that peripherals only 
    // consume power when they are in use.

    // Czym jest DMA:
    // --> DMA (Direct Memory Access) is a peripheral that can transfer data
    // from one memory location to another without the need for the CPU to be
    // involved in the data transfer. 
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN |
                    RCC_AHB1ENR_GPIOBEN |
                    RCC_AHB1ENR_GPIOCEN |
                    RCC_AHB1ENR_DMA1EN; // wlaczamy DMA1

    // APB1ENR - rejestr w RCC do wlaczania zegara dla peryferiow podpietych do
    // magistrali APB1, czyli np. USART, TIM, SPI, USART2
    // APB1ENR - Advanced Peripheral Bus 1 Enable Register
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;

    // Wlaczanie przerwan dla przyciskow itp czyli 
    // SYSCFG - System Configuration Controller
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

}
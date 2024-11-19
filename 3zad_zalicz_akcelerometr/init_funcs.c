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
                    RCC_AHB1ENR_GPIOCEN; 

    // APB1ENR - rejestr w RCC do wlaczania zegara dla peryferiow podpietych do
    // magistrali APB1, czyli np. USART, TIM, SPI, USART2, I2C
    // APB1ENR - Advanced Peripheral Bus 1 Enable Register
    // wlaczamy zegar dla I2C1
    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN; 
}

void init_I2C1()
{
    // Wlaczamy linie SCL akcelerometru
    GPIOafConfigure(GPIOB, 
                    8, 
                    GPIO_OType_OD,
                    GPIO_Low_Speed, 
                    GPIO_PuPd_NOPULL,
                    GPIO_AF_I2C1);

    // Wlaczamy linie SDA akcelerometru
    GPIOafConfigure(GPIOB, 
                    9, 
                    GPIO_OType_OD,
                    GPIO_Low_Speed, 
                    GPIO_PuPd_NOPULL,
                    GPIO_AF_I2C1);

    // konfigurujemy I2C1 w wersji podstawowej
    I2C1->CR1 = 0;
    // Ustawiamy odpowiednia czestotliowsc taktowania szyny I2C
    I2C1->CCR = (PCLK1_MHZ * 1000000) / (I2C_SPEED_HZ << 1);
    I2C1->CR2 = PCLK1_MHZ;
    I2C1->TRISE = PCLK1_MHZ + 1;

    // Wlaczamy interfejs
    I2C1->CR1 |= I2C_CR1_PE;

    // !! rejestry I2C1 sa 16bitowe

}

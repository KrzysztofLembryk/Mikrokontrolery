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
    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN |
                    RCC_APB1ENR_USART2EN; 
}

void init_usart2_TXD_RXD_lines()
{
    // Korzystamy z USART2
    // --> linia TXD (Transmit Data Line)
    //      uzywana jest do wysylania danych z mikrokontrolera poprzez pin PA2
    GPIOafConfigure(GPIOA,
                    2,
                    GPIO_OType_PP,
                    GPIO_Fast_Speed,
                    GPIO_PuPd_NOPULL,
                    GPIO_AF_USART2);

    // --> linia RXD (Receive Data Line)
    //      uzywana jest do odbierania danych z mikrokontrolera poprzez pin PA3
    GPIOafConfigure(GPIOA,
                    3,
                    GPIO_OType_PP,
                    GPIO_Fast_Speed,
                    GPIO_PuPd_UP,
                    GPIO_AF_USART2);
}

void init_usart2_cr_registers()
{
    // ------ Ustawienie rejestrow CR ------

    // CR1 - nieaktywny bo nie ustawiamy bitu USART_Enable
    USART2->CR1 = USART_Mode_Rx_Tx |
                  USART_WordLength_8b |
                  USART_Parity_No;

    // CR2 - ustawienie bitow stopu
    USART2->CR2 = USART_StopBits_1;

    // CR3
    USART2->CR3 = USART_FlowControl_None;

    // BRR - ustawienie predkosci transmisji danych
    USART2->BRR = (PCLK1_HZ + (BAUD / 2U)) / BAUD;

    // Teraz chcemy uaktywnic interfejs USART, wiec
    // ustawiamy bit USART_Enable w rejestrze CR1
    USART2->CR1 |= USART_Enable;
}

void init_I2C1()
{
    // Linie SCL (Serial Clock Line) i SDA (Serial Data Line) są podstawowymi 
    // liniami komunikacyjnymi w protokole I2C (Inter-Integrated Circuit)

    // SCL (Serial Clock Line): Linia zegarowa, która synchronizuje przesyłanie 
    // danych między urządzeniami master i slave.
    // Urządzenie master generuje sygnał zegarowy na linii SCL.
    // Urządzenia slave odbierają sygnał zegarowy i synchronizują swoje 
    // operacje z tym zegarem.

    // SDA (Serial Data Line): Linia danych, która przesyła dane między 
    // urządzeniami master i slave.
    // Linia SDA jest dwukierunkowa, co oznacza, że zarówno urządzenie master, 
    // jak i slave mogą wysyłać i odbierać dane.
    // Dane są przesyłane w formie bitów, zsynchronizowanych z sygnałem 
    // zegarowym na linii SCL.

    // Akceleromter podlaczony jest o magistrali I2C, a na niej do 
    // ukladu I2C1, lini SCL na pinie PB8 i SDA na pinie PB9
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

    // resetujemy I2C1 w do podstawowych ustawien
    I2C1->CR1 = 0;
    // Ustawiamy odpowiednia czestotliowsc taktowania szyny I2C
    I2C1->CCR = (PCLK1_MHZ * 1000000) / (I2C_SPEED_HZ << 1);
    I2C1->CR2 = PCLK1_MHZ;
    I2C1->TRISE = PCLK1_MHZ + 1;

    // Wlaczamy interfejs
    I2C1->CR1 |= I2C_CR1_PE;

    // !! rejestry I2C1 sa 16bitowe

}

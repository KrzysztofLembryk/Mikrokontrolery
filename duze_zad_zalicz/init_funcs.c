#include "diods.h"
#include "register_consts.h"
#include "init_funcs.h"
#include <xcat.h>

#define I2C_GPIO_N  B
#define I2C_SCL_PIN 8
#define I2C_SDA_PIN 9

#define I2C_GPIO                xcat(GPIO, I2C_GPIO_N)
#define RCC_AHB1ENR_I2C_GPIO_EN xcat3(RCC_AHB1ENR_GPIO, I2C_GPIO_N, EN)

#define I2C_SCL_MASK (1 << I2C_SCL_PIN)
#define I2C_SDA_MASK (1 << I2C_SDA_PIN)

#define I2C_SCL_HIGH() I2C_GPIO->BSRR = I2C_SCL_MASK
#define I2C_SCL_LOW()  I2C_GPIO->BSRR = I2C_SCL_MASK << 16
#define I2C_SDA_HIGH() I2C_GPIO->BSRR = I2C_SDA_MASK
#define I2C_SDA_LOW()  I2C_GPIO->BSRR = I2C_SDA_MASK << 16

#define I2C_IS_SCL_LOW() ((I2C_GPIO->IDR & I2C_SCL_MASK) == 0)
#define I2C_IS_SDA_LOW() ((I2C_GPIO->IDR & I2C_SDA_MASK) == 0)


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

    // CR3- dodajemy wysylanie i odbieranie danych przez DMA
    USART2->CR3 = USART_FlowControl_None |
                  USART_DMA_Tx_En |
                  USART_DMA_Rx_En;

    // BRR - ustawienie predkosci transmisji danych
    USART2->BRR = (PCLK1_HZ + (BAUD / 2U)) / BAUD;

    // Teraz chcemy uaktywnic interfejs USART, wiec
    // ustawiamy bit USART_Enable w rejestrze CR1
    USART2->CR1 |= USART_Enable;
}


void init_dma_cr_registers()
{
    // -----KONFIGURACJA NADAWCZEGO STRUMIENIA DMA-----
    // USART2 TX korzysta ze strumienia 6 i kanału 4, tryb bezpośredni, 
    // transfery 8-bitowe, wysoki priorytet, zwiększanie adresu pamięci po 
    // każdym przesłaniu, przerwanie po zakończeniu transmisji
    DMA1_Stream6->CR = 4U << 25 | // Channel 4
                       DMA_SxCR_PL_1 | // Priority level - high
                       DMA_SxCR_MINC | // Memory increment mode
                       DMA_SxCR_DIR_0 | // Direction - memory to peripheral
                       DMA_SxCR_TCIE; // Transfer complete interrupt enable

    // Ustawiamy adres pamieci ukladu peryferyjnego, 
    // on sie oczywiscie nie zmienia
    DMA1_Stream6->PAR = (uint32_t)&USART2->DR;
    // ------------------------------------------------

    // -----KONFIGURACJA ODBIORCZEGO STRUMIENIA DMA-----
    // numer kanalu jest zapisywany w 25-27 bitach rejestru CR, dlatego 
    // latwiej jest przesunac 4U o 25 bitow w lewo niz 1 o 27 bitow w lewo
    DMA1_Stream5->CR = 4U << 25 | // Channel 4
                       DMA_SxCR_PL_1 | // Priority level - high
                       DMA_SxCR_MINC | // Memory increment mode
                       DMA_SxCR_TCIE; // Transfer complete interrupt enable
    DMA1_Stream5->PAR = (uint32_t)&USART2->DR;
    // ------------------------------------------------
}

void init_dma_interrupts()
{
    // Wyczyszczenie znacznikow przerwan i wlaczenie przerwan dla 
    // DMA1_Stream6 i DMA1_Stream5
    DMA1->HIFCR = DMA_HIFCR_CTCIF6 | DMA_HIFCR_CTCIF5;
    NVIC_EnableIRQ(DMA1_Stream6_IRQn);
    NVIC_EnableIRQ(DMA1_Stream5_IRQn);

    // Uaktywnienie ukladow peryferyjnych
    USART2->CR1 |= USART_CR1_UE;
}

void init_I2C1()
{
    __NOP();

    /* Najpierw odwieś szynę I2C. To jest czysta heureza. */

    I2C_SCL_HIGH();
    I2C_SDA_HIGH(); // NACK
    GPIOoutConfigure(I2C_GPIO, I2C_SCL_PIN, GPIO_OType_OD, GPIO_Low_Speed,
                     GPIO_PuPd_NOPULL);
    GPIOoutConfigure(I2C_GPIO, I2C_SDA_PIN, GPIO_OType_OD, GPIO_Low_Speed,
                     GPIO_PuPd_NOPULL);

    // Taktuj szynę z częstotliwością około 12,5 kHz.
    unsigned half_cycle = (PCLK1_MHZ * 1000000) / 100000;

    // Dokończ ewentualnie niedokończoną transmisję, wystawiając NACK.
    Delay(half_cycle);
    for (unsigned i = 0; i < 9 || I2C_IS_SDA_LOW(); ++i)
    {
        I2C_SCL_LOW();
        Delay(half_cycle);
        I2C_SCL_HIGH();
        Delay(half_cycle);
    }

    Delay(half_cycle);
    I2C_SDA_LOW();
    Delay(half_cycle);
    I2C_SCL_LOW(); // START
    Delay(2 * half_cycle);
    I2C_SCL_HIGH(); // STOP
    Delay(half_cycle);
    I2C_SDA_HIGH();
    Delay(half_cycle);

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

    // Ustawiamy odpowiednia czestotliowsc taktowania szyny I2C
    // Aktywujemy przerwania za pomoca CR2 i bitow:
    // I2C_CR2_ ITBUFEN, ITEVTEN, ITERREN
    I2C1->CR2 = PCLK1_MHZ |
                I2C_CR2_ITEVTEN |
                I2C_CR2_ITERREN |
                I2C_CR2_ITBUFEN;
    I2C1->TRISE = PCLK1_MHZ + 1;

    // Wlaczamy przerwania  NVIC dla I2C1
    // Event interrupt - czyli przerwanie gdy np. skonczymy pomiar
    NVIC_EnableIRQ(I2C1_EV_IRQn);

    // Error interrupt - tego chyba nie potrzebujemy
    // NVIC_EnableIRQ(I2C1_ER_IRQn);

    // Wlaczamy interfejs
    I2C1->CR1 |= I2C_CR1_PE;

    // !! rejestry I2C1 sa 16bitowe
}

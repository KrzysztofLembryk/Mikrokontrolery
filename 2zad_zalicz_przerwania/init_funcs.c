#include "diods.h"
#include "register_consts.h"
#include "init_funcs.h"
#include "buttons.h"

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

void init_TXD_RXD_lines()
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

    // CR3 - dodajemy wysylanie i odbieranie danych przez DMA
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

void init_button_interrupts()
{
    // GPIOinConfigure - poprzez podanie do niej flag EXTI_... on sama ustawia
    // odpowiednie rejestry w SYSCFG takie jak:
    // EXTICR[i] - podlacza odpowiednie piny do linii przerwan
    // EXTI->IMR - wylacza/wlacza linie przerwan
    // EXTI->EMR - wylacza/wlacza linie przerwan dla zdarzen zewnetrznych
    // EXTI->RTSR - ustawia zbocze narastajace
    // EXTI->FTSR - ustawia zbocze opadajace

    // --------USER BUTTON--------
    GPIOinConfigure(USER_BTN_GPIO, 
                    USER_BTN_PIN, 
                    GPIO_PuPd_UP,
                    EXTI_Mode_Interrupt,
                    EXTI_Trigger_Rising_Falling);

    // -------AT MODE BUTTON-------
    GPIOinConfigure(AT_BTN_GPIO, 
                    AT_BTN_PIN, 
                    GPIO_PuPd_UP,
                    EXTI_Mode_Interrupt,
                    EXTI_Trigger_Rising_Falling);

    
    // -------JOYSTICK BUTTONS-------
    GPIOinConfigure(JSTICK_BTN_GPIO, 
                    JSTICK_LEFT_PIN, 
                    GPIO_PuPd_UP,
                    EXTI_Mode_Interrupt,
                    EXTI_Trigger_Rising_Falling);
    GPIOinConfigure(JSTICK_BTN_GPIO,
                    JSTICK_RIGHT_PIN,
                    GPIO_PuPd_UP,
                    EXTI_Mode_Interrupt,
                    EXTI_Trigger_Rising_Falling);
    GPIOinConfigure(JSTICK_BTN_GPIO,
                    JSTICK_UP_PIN,
                    GPIO_PuPd_UP,
                    EXTI_Mode_Interrupt,
                    EXTI_Trigger_Rising_Falling);
    GPIOinConfigure(JSTICK_BTN_GPIO,
                    JSTICK_DOWN_PIN,
                    GPIO_PuPd_UP,
                    EXTI_Mode_Interrupt,
                    EXTI_Trigger_Rising_Falling);
    GPIOinConfigure(JSTICK_BTN_GPIO,
                    JSTICK_CENTER_PIN,
                    GPIO_PuPd_UP,
                    EXTI_Mode_Interrupt,
                    EXTI_Trigger_Rising_Falling);

    // Wyczyszczenie znacznikow oczekujacych przerwan dla naszych linii
    EXTI->PR = EXTI_PR_PR13 | EXTI_PR_PR0 | EXTI_PR_PR3 | EXTI_PR_PR4 | EXTI_PR_PR5 | EXTI_PR_PR6 | EXTI_PR_PR10;

    // Ustawienie priorytetow przerwan - niepotrzebne, jest ok gdy maja te same priorytety
    // NVIC_SetPriority(EXTI15_10_IRQn, 2);
    // NVIC_SetPriority(EXTI0_IRQn, 3);
    // NVIC_SetPriority(EXTI3_IRQn, 4);
    // NVIC_SetPriority(EXTI4_IRQn, 4);
    // NVIC_SetPriority(EXTI9_5_IRQn, 4);

    // Wlaczenie przerwan dla linii:
    // 13 - USER
    // 0 - AT MODE 
    // 3, 4, 5, 6, 10 - JOYSTICK 
    NVIC_EnableIRQ(EXTI15_10_IRQn);
    NVIC_EnableIRQ(EXTI0_IRQn);
    NVIC_EnableIRQ(EXTI3_IRQn);
    NVIC_EnableIRQ(EXTI4_IRQn);
    NVIC_EnableIRQ(EXTI9_5_IRQn);

}
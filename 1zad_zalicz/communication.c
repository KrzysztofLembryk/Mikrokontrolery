#include <delay.h>
#include <gpio.h>
#include <stm32.h>
#include <inttypes.h>
#include <stdbool.h>

// KOMENDA WGRYWJACA PROGRAM NA PLYTKE
// /opt/arm/stm32/ocd/qfn4
// komenda do komunikacj z plytka to minicom

// Przyciski na plytce i ich piny/porty sa na drugim wykladzie pod koniec

// -----------DIODY LED-----------
// GPIOA - General Purpose Input/Output Port A - kontroluje piny A
// RED_LED_GPIO GPIOA - to znaczy ze czerwona dioda jest podpieta do pinu na
// porcie A
#define RED_LED_GPIO GPIOA
#define GREEN_LED_GPIO GPIOA
#define BLUE_LED_GPIO GPIOB
#define GREEN2_LED_GPIO GPIOA
#define RED_LED_PIN 6
#define GREEN_LED_PIN 7
#define BLUE_LED_PIN 0
#define GREEN2_LED_PIN 5

// BSRR -Bit Set Reset Register- sluzy do ustawiania i resetowania bitow atomowo
// To 32-bitowy rejestr, w ktorym 16 najmłodszych bitow to bity ustawiania,
// a 16 najstarszych bitow to bity resetowania.
#define RedLEDon() \
    RED_LED_GPIO->BSRR = 1 << (RED_LED_PIN + 16)
#define RedLEDoff() \
    RED_LED_GPIO->BSRR = 1 << RED_LED_PIN

#define BlueLEDon() \
    BLUE_LED_GPIO->BSRR = 1 << (BLUE_LED_PIN + 16)
#define BlueLEDoff() \
    BLUE_LED_GPIO->BSRR = 1 << BLUE_LED_PIN

#define GreenLEDon() \
    GREEN_LED_GPIO->BSRR = 1 << (GREEN_LED_PIN + 16)
#define GreenLEDoff() \
    GREEN_LED_GPIO->BSRR = 1 << GREEN_LED_PIN

#define Green2LEDon() \
    GREEN2_LED_GPIO->BSRR = 1 << GREEN2_LED_PIN
#define Green2LEDoff() \
    GREEN2_LED_GPIO->BSRR = 1 << (GREEN2_LED_PIN + 16)

// -----------DIODY LED-----------

// -----------PRZYCISKI-----------

// -----------USER-----------
// stan aktywny przycisku to 0
// trzeba dodac \r\n na koncu stringa bo inaczej robia sie schodki w minicomie
#define USER_BTN_GPIO GPIOC
#define USER_BTN_PIN 13
#define USER_PRESSED "USER PRESSED\r\n"
#define USER_RELEASED "USER RELEASED\r\n"

// robimy dla przyciskow funkcje, a nie makra bo chcemy zrobic tablice pointerow
// do tych funkcji i wywolywac je w petli
uint16_t UserBtnPressed() 
{
    return !(USER_BTN_GPIO->IDR & (1 << USER_BTN_PIN));
}

// -----------AT MODE-----------
// stan aktywny przycisku to 1
#define AT_BTN_GPIO GPIOA
#define AT_BTN_PIN 0
#define AT_PRESSED "MODE PRESSED\r\n"
#define AT_RELEASED "MODE RELEASED\r\n"

uint16_t ATBtnPressed()
{
    return (AT_BTN_GPIO->IDR & (1 << AT_BTN_PIN));
}

// -----------JOYSTICK-----------
// stan aktywny przycisku to 0
#define JSTICK_BTN_GPIO GPIOB
#define JSTICK_LEFT_PIN 3
#define JSTICK_RIGHT_PIN 4
#define JSTICK_UP_PIN 5
#define JSTICK_DOWN_PIN 6
#define JSTICK_CENTER_PIN 10
#define JSTICK_LEFT_PRSSD "LEFT PRESSED\r\n"
#define JSTICK_RIGHT_PRSSD "RIGHT PRESSED\r\n"
#define JSTICK_UP_PRSSD "UP PRESSED\r\n"
#define JSTICK_DOWN_PRSSD "DOWN PRESSED\r\n"
#define JSTICK_CENTER_PRSSD "FIRE PRESSED\r\n"
#define JSTICK_LEFT_RLSD "LEFT RELEASED\r\n"
#define JSTICK_RIGHT_RLSD "RIGHT RELEASED\r\n"
#define JSTICK_UP_RLSD "UP RELEASED\r\n"
#define JSTICK_DOWN_RLSD "DOWN RELEASED\r\n"
#define JSTICK_CENTER_RLSD "FIRE RELEASED\r\n"

#define JstickLeftPressed() \
    !(JSTICK_BTN_GPIO->IDR & (1 << JSTICK_LEFT_PIN))

#define JstickRightPressed() \
    !(JSTICK_BTN_GPIO->IDR & (1 << JSTICK_RIGHT_PIN))

#define JstickUpPressed() \
    !(JSTICK_BTN_GPIO->IDR & (1 << JSTICK_UP_PIN))

#define JstickDownPressed() \
    !(JSTICK_BTN_GPIO->IDR & (1 << JSTICK_DOWN_PIN))

#define JstickCenterPressed() \
    !(JSTICK_BTN_GPIO->IDR & (1 << JSTICK_CENTER_PIN))

uint16_t JstickPressed()
{
    if (JstickLeftPressed())
        return JSTICK_LEFT_PIN;
    if (JstickRightPressed())
        return JSTICK_RIGHT_PIN;
    if (JstickUpPressed())
        return JSTICK_UP_PIN;
    if (JstickDownPressed())
        return JSTICK_DOWN_PIN;
    if (JstickCenterPressed())
        return JSTICK_CENTER_PIN;
    return 0;
}

// -----------PRZYCISKI-----------

// USART stands for Universal Synchronous/Asynchronous Receiver/Transmitter. It is a hardware communication protocol used in microcontrollers for serial communication.
// ### Common Uses:
// - Communication between microcontrollers and peripheral devices.
// - Serial communication with computers.
// - Data logging and debugging.

// CR - Control Register dla USART
// -----------REJESTR CR1-----------
// Uzywany do podstawowej konfiguracji parametrow USART

// Tryb pracy
#define USART_Mode_Rx_Tx (USART_CR1_RE | USART_CR1_TE)
#define USART_Enable USART_CR1_UE

// Przesylane slowo
#define USART_WordLength_8b 0x0000
#define USART_WordLength_9b USART_CR1_M

// Bit parzystosci
#define USART_Parity_No 0x0000
#define USART_Parity_Even USART_CR1_PCE
#define USART_Parity_Odd (USART_CR1_PCE | USART_CR1_PS)
// -----------REJESTR CR1-----------

// -----------REJESTR CR2-----------
// Uzywany do bardziej zaawansowanej konfiguracji parametrow USART,
// takich jak bity stopu czy ustawienia zegara

// Bity stopu
#define USART_StopBits_1 0x0000
#define USART_StopBits_0_5 0x1000
#define USART_StopBits_2 0x2000
#define USART_StopBits_1_5 0x3000
// -----------REJESTR CR2-----------

// -----------REJESTR CR3-----------
// Uzywany do dodatkowej konfiguracji parametrow USART,
// takich jak hardware flow control, DMA(Direct Memory Access)

// Sterowanie przeplywem
#define USART_FlowControl_None 0x0000
#define USART_FlowControl_RTS USART_CR3_RTSE
#define USART_FlowControl_CTS USART_CR3_CTSE
// -----------REJESTR CR3-----------

// BRR - Baud Rate Register
// -----------REJESTR BRR-----------
// Uzywany do konfiguracji predkosci transmisji danych dla USART

#define HSI_HZ 16000000U
#define PCLK1_HZ HSI_HZ
#define BAUD 9600U
// -----------REJESTR BRR-----------

// -----------INNE STALE-----------
#define COLOR_IDX 1
#define STATE_IDX 2
#define INPUT_MSG_SIZE 3
#define QUEUE_SIZE 250
#define MAX_STR_LEN 16

char read_input(char input[3])
{
    static uint8_t i = 0;
    // USART2->SR - Status Register - zawiera rozne flagi statusu USART,
    // takie jak:
    // RXNE - Receive Data Register Not Empty - ktora jest ustawiana, gdy dane
    // sa dostepne do odczytu z rejestru DR
    // TXE - Transmit Data Register Empty - ustawiana gdy mozemy wyslac dane
    if (USART2->SR & USART_SR_RXNE)
    {
        char c = USART2->DR;
        switch (i)
        {
        case 0:
            if (c == 'L')
            {
                input[i] = c;
                i++;
            }
            else
            {
                i = 0;
                return 0;
            }
            break;
        case 1:
            if (c == 'R' || c == 'G' || c == 'B' || c == 'g')
            {
                input[i] = c;
                i++;
            }
            else
            {
                i = 0;
                return 0;
            }
            break;
        case 2:
            if (c == '1' || c == '0' || c == 'T')
            {
                input[i] = c;
                i = 0;
                return 1;
            }
            else
            {
                i = 0;
                return 0;
            }
            break;

        default:
            i = 0;
            return 0;
            break;
        }
    }

    return 0;
}

void handle_input(char color, char state)
{
    // !!!UWAGA!!! - Green2 a pozostale diody maja inny stan wylaczenia
    // czyli jesli state Green2 to 1 to dioda jest wlaczona
    // a dla innych diod stan 1 to dioda wylaczona
    uint16_t pin_state;

    switch (color)
    {
    case 'R':
        if (state == '1')
        {
            RedLEDon();
        }
        else if (state == '0')
        {
            RedLEDoff();
        }
        else // Toggle
        {
            // Odczytujemy stan diody, czyli z 1 = 00000001 robimy dla czerwonej diody 01000000
            // przesuwamy te jedynke na miejsce pinu czerwonej diody i robiac bitowe AND z rejestrem ODR otrzymujemy stan diody
            pin_state = RED_LED_GPIO->ODR & (1 << RED_LED_PIN);
            if (pin_state)
                RedLEDon();
            else
                RedLEDoff();
        }
        break;
    case 'G':
        if (state == '1')
        {
            GreenLEDon();
        }
        else if (state == '0')
        {
            GreenLEDoff();
        }
        else
        {
            pin_state = GREEN_LED_GPIO->ODR & (1 << GREEN_LED_PIN);
            if (pin_state)
                GreenLEDon();
            else
                GreenLEDoff();
        }
        break;
    case 'B':
        if (state == '1')
        {
            BlueLEDon();
        }
        else if (state == '0')
        {
            BlueLEDoff();
        }
        else
        {
            pin_state = BLUE_LED_GPIO->ODR & (1 << BLUE_LED_PIN);
            if (pin_state)
                BlueLEDon();
            else
                BlueLEDoff();
        }
        break;
    case 'g':
        if (state == '1')
        {
            Green2LEDon();
        }
        else if (state == '0')
        {
            Green2LEDoff();
        }
        else
        {
            pin_state = GREEN2_LED_GPIO->ODR & (1 << GREEN2_LED_PIN);
            if (pin_state)
                Green2LEDoff();
            else
                Green2LEDon();
        }
        break;
    default:
        break;
    }
}

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
    // It controls the clock distribution to various peripherals and modules within
    // the microcontroller, enabling or disabling their clocks as needed.
    // This is crucial for power management and ensuring that peripherals only consume power
    // when they are in use.

    // !!! UWAGA !!!
    // TO SA DWA ROZNE REJESTRY ROZNIACE SIE JEDNA LITERA
    // --> zeby niebieska dioda dzialala musimy wlaczyc zegar dla portu GPIOB
    // AHB1ENR - rejestr w RCC do wlaczania zegara dla portow GPIO, i ogolnie
    // rzeczy podpietych do magistrali AHB1 (Advanced High-performance Bus 1)
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN |
                    RCC_AHB1ENR_GPIOBEN |
                    RCC_AHB1ENR_GPIOCEN; // wlaczamy zegar dla portu GPIOC na
                                         // ktorym jest przycisk USER

    // APB1ENR - rejestr w RCC do wlaczania zegara dla peryferiow podpietych do
    // magistrali APB1, czyli np. USART, TIM, SPI, USART2
    // APB1ENR - Advanced Peripheral Bus 1 Enable Register
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
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

    // CR3
    USART2->CR3 = USART_FlowControl_None;

    // BRR - ustawienie predkosci transmisji danych
    USART2->BRR = (PCLK1_HZ + (BAUD / 2U)) / BAUD;

    // Teraz chcemy uaktywnic interfejs USART, wiec
    // ustawiamy bit USART_Enable w rejestrze CR1
    USART2->CR1 |= USART_Enable;
}

void init_buttons()
{
    // Jesli trzeba to dopisac nastepne buttony trzeba tutaj
    // CHYBA - nie mam jak przetestowac tego
    // Musimy przyciski skonfigurowac jako wejscia
    // przycisk USER 
    // GPIOinConfigure(USER_BTN_GPIO,
    //                 USER_BTN_PIN,
    //                 GPIO_OType_PP,
    //                 GPIO_Low_Speed,
    //                 GPIO_PuPd_NOPULL);
}

typedef struct QInfo
{
    uint8_t front;
    uint8_t end;
    uint8_t q_size;
    uint8_t nbr_of_elements; 
} QInfo;

// !!!!!!!przeniesc char [] queue do QInfo!!!!!!!

void init_QInfo(QInfo *q_info, uint8_t q_size)
{
    q_info->front = 0;
    q_info->end = 0;
    q_info->q_size = q_size;
    q_info->nbr_of_elements = 0;
}

bool q_add(char c,
           char queue[],
           QInfo *q_info)
{
    if (q_info->nbr_of_elements >= q_info->q_size)
        return false;

    queue[q_info->end] = c;
    q_info->end = (q_info->end + 1) % q_info->q_size;
    q_info->nbr_of_elements++;

    return true;
}

bool q_remove(char queue[],
              char *c,
              QInfo *q_info)
{
    if (q_info->nbr_of_elements <= 0)
        return false;

    *c = queue[q_info->front];
    q_info->front = (q_info->front + 1) % q_info->q_size;
    q_info->nbr_of_elements--;

    return true;
}

bool q_check_if_enough_space(uint8_t str_len,
                             QInfo *q_info)
{
    if (q_info->q_size - q_info->nbr_of_elements < str_len)
        return false;
    return true;
}

uint8_t get_str_len(char *str)
{
    // Zakladamy ze kazda wiadomosc konczy sie znakiem nowej linii '\n
    uint8_t str_len = 0;
    while (*str != '\n')
    {
        str_len++;
        str++;
        if (str_len > MAX_STR_LEN)
            return 0;
    }
    str_len++;
    return str_len;
}

bool q_add_str(char *str,
               char queue[],
               QInfo *q_info)
{
    uint8_t str_len = get_str_len(str);
    uint8_t i = 0;

    // jesli str_len == 0 to znaczy ze nie ma znaku nowej linii w stringu
    // bo wszystkie wysylane komendy maja <= 15 znakow
    if (!q_check_if_enough_space(str_len, q_info) || str_len == 0)
        return false;

    while (*str != '\n' && i < str_len)
    {
        if (!q_add(*str, queue, q_info))
            return false;
        str++;
        i++;
    }

    if (!q_add(*str, queue, q_info))
        return false;

    return true;
}



void handle_jstick_rlsd(uint16_t jstick_rlsd, char queue[], QInfo *q_info)
{
    switch (jstick_rlsd)
    {
    case JSTICK_LEFT_PIN:
        q_add_str(JSTICK_LEFT_RLSD, queue, q_info);
        break;
    case JSTICK_RIGHT_PIN:
        q_add_str(JSTICK_RIGHT_RLSD, queue, q_info);
        break;
    case JSTICK_UP_PIN:
        q_add_str(JSTICK_UP_RLSD, queue, q_info);
        break;
    case JSTICK_DOWN_PIN:
        q_add_str(JSTICK_DOWN_RLSD, queue, q_info);
        break;
    case JSTICK_CENTER_PIN:
        q_add_str(JSTICK_CENTER_RLSD, queue, q_info);
        break;
    default:
        q_add_str("DEFAULT BREAK\r\n", queue, q_info);
        break;
    }
}

// Funkcja najpierw sprawdza czy poprzedni przycisk byl wcisniety, jesli tak to
// dodaje do kolejki komunikat o jego zwolnieniu, a nastepnie dodaje do kolejki
// komunikat o wcisnieciu nowego przycisku i aktualizuje wartosc 
// prev_jstick_pressed
void add_jstick_msgs_to_queue(uint16_t *prev_jstick_pressed, 
                                uint16_t curr_jstick_pressed, 
                                char queue[], 
                                QInfo *q_info, 
                                char *msg)
{
    if (*prev_jstick_pressed)
    {
        if (curr_jstick_pressed != *prev_jstick_pressed)    
        {
            handle_jstick_rlsd(*prev_jstick_pressed, queue, q_info);
            q_add_str(msg, queue, q_info);
            *prev_jstick_pressed = curr_jstick_pressed;
        }
    }
    else 
    {
        q_add_str(msg, queue, q_info);
            *prev_jstick_pressed = curr_jstick_pressed;
    }
    
}

// Funkcja sprawdza jaki joystick zostal wcisniety i dodaje do kolejki
// odpowiedni komunikat. Jesli zaden przycisk joysticka nie zostal wcisniety to
// to sprawdza czy jest jakis przycisk ktory byl wcisniety wczesniej i jesli tak
// to wypisuje komunikat o jego zwolnieniu
void handle_jstick(uint16_t jstick_pressed, char queue[], QInfo *q_info)
{
    static uint16_t prev_jstick_pressed = 0;
    switch (jstick_pressed)
    {
    case JSTICK_LEFT_PIN:
        add_jstick_msgs_to_queue(&prev_jstick_pressed, 
                                    jstick_pressed, 
                                    queue, 
                                    q_info, 
                                    JSTICK_LEFT_PRSSD);
        break;
    case JSTICK_RIGHT_PIN:
        add_jstick_msgs_to_queue(&prev_jstick_pressed, 
                                    jstick_pressed, 
                                    queue, 
                                    q_info, 
                                    JSTICK_RIGHT_PRSSD);
        break;
    case JSTICK_UP_PIN:
        add_jstick_msgs_to_queue(&prev_jstick_pressed, 
                                    jstick_pressed, 
                                    queue, 
                                    q_info, 
                                    JSTICK_UP_PRSSD);
        break;
    case JSTICK_DOWN_PIN:
        add_jstick_msgs_to_queue(&prev_jstick_pressed, 
                                    jstick_pressed, 
                                    queue, 
                                    q_info, 
                                    JSTICK_DOWN_PRSSD);
        break;
    case JSTICK_CENTER_PIN:
        add_jstick_msgs_to_queue(&prev_jstick_pressed, 
                                    jstick_pressed, 
                                    queue, 
                                    q_info, 
                                    JSTICK_CENTER_PRSSD);
        break;
    default:
        if (prev_jstick_pressed != 0)
        {
            handle_jstick_rlsd(prev_jstick_pressed, queue, q_info);
            prev_jstick_pressed = 0;
        }
        break;
    }
}

void handle_user(uint16_t user_pressed, char queue[], QInfo *q_info)
{
    static bool was_pressed = false;
    if (!user_pressed)
    {
        if (was_pressed)
        {
            q_add_str(USER_RELEASED, queue, q_info);
            was_pressed = false;
        }
    }
    else 
    {
        if (!was_pressed)
        {
            was_pressed = true;
            q_add_str(USER_PRESSED, queue, q_info);
        }
    }
}

void handle_at(uint16_t at_pressed, char queue[], QInfo *q_info)
{
    static bool was_pressed = false;
    if (!at_pressed)
    {
        if (was_pressed)
        {
            q_add_str(AT_RELEASED, queue, q_info);
            was_pressed = false;
        }
    }
    else 
    {
        if (!was_pressed)
        {
            was_pressed = true;
            q_add_str(AT_PRESSED, queue, q_info);
        }
    }
}

typedef uint16_t (*BtnPrssdFuncArr_t)(void);
typedef void (*BtnHandleFuncArr_t)(uint16_t, char[], QInfo *);

void handle_buttons(char queue[], QInfo *q_info)
{
    static BtnPrssdFuncArr_t prssd_funcs[] = {UserBtnPressed, 
                                                ATBtnPressed, 
                                                JstickPressed};
    static BtnHandleFuncArr_t handle_funcs[] = {handle_user, 
                                                handle_at, 
                                                handle_jstick};
    for (int i = 0; i < 3; i++)
    {
        uint16_t pressed = prssd_funcs[i](); // = func()
        handle_funcs[i](pressed, queue, q_info);
    }
}


int main()
{
    // Inicjalizacja zegara zeby odpowiednie piny byly wlaczone
    init_rcc();
    // Konfiguracja diod, zeby mogly dzialac
    init_diods();
    init_TXD_RXD_lines();
    init_usart2_cr_registers();

    static char input[INPUT_MSG_SIZE] = {0};
    char queue[QUEUE_SIZE] = {0};
    QInfo q_info;
    init_QInfo(&q_info, QUEUE_SIZE);

    while (1)
    {
        // ----- Odczytywanie komend -----
        if (read_input(input))
        {
            char color = input[COLOR_IDX];
            char state = input[STATE_IDX];
            handle_input(color, state);
        }
        // --------------------------------

        // Wstawiamy do kolejki komunikatow o wcisnieciu przyciskow
        handle_buttons(queue, &q_info);

        // ------- Wysylanie znakow -------
        // Wysylamy po kolei znaki znajdujace sie w kolejce, gdy mozemy je
        // wyslac, czyli gdy flaga TXE jest ustawiona
        if (USART2->SR & USART_SR_TXE)
        {
            char c;
            if (q_remove(queue, &c, &q_info))
                USART2->DR = c;
        }
        // --------------------------------
    }
}
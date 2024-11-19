#include <inttypes.h>
#include <stdbool.h>
#include "button_handlers.h"
#include "buttons.h"
#include "diods.h"
#include "init_funcs.h"
#include "other_consts.h"
#include "queue.h"
#include "register_consts.h"
#include "interrupts_handlers.h"

// KOMENDA WGRYWJACA PROGRAM NA PLYTKE
// /opt/arm/stm32/ocd/qfn4
// komenda do komunikacj z plytka to minicom

// Przyciski na plytce i ich piny/porty sa na drugim wykladzie pod koniec

// USART stands for Universal Synchronous/Asynchronous Receiver/Transmitter. It is a hardware communication protocol used in microcontrollers for serial communication.
// ### Common Uses:
// - Communication between microcontrollers and peripheral devices.
// - Serial communication with computers.
// - Data logging and debugging.

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

int main()
{
    init_rcc();
    init_diods();
    init_TXD_RXD_lines();
    init_usart2_cr_registers();
    init_dma_cr_registers();
    init_dma_interrupts();
    init_button_interrupts();
    init_interrupts_handlers_data();

    static char input[INPUT_MSG_SIZE] = {0};
    // QInfo q_info;
    // init_QInfo(&q_info, QUEUE_SIZE);

    while (1)
    {
        // ----- Odczytywanie komend -----
        // if (read_input(input))
        // {
        //     char color = input[COLOR_IDX];
        //     char state = input[STATE_IDX];
        //     handle_input(color, state);
        // }
        // --------------------------------
        // Wstawiamy do kolejki komunikatow o wcisnieciu przyciskow
        // handle_buttons(&q_info);

        // ------- Wysylanie znakow PRZERWANIA -------
        // Czekamy na przerwanie
        // __WFI();


        // ------- Wysylanie znakow BEZ PRZERWAN -------
        // Wysylamy po kolei znaki znajdujace sie w kolejce, gdy mozemy je
        // wyslac, czyli gdy flaga TXE jest ustawiona
        // if (USART2->SR & USART_SR_TXE)
        // {
        //     char c;
        //     if (q_remove(&c, &q_info))
        //         USART2->DR = c;
        // }
        // --------------------------------
    }
}
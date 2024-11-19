#ifndef DIODS_H
#define DIODS_H

#include <delay.h>
#include <gpio.h>
#include <stm32.h>

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


#endif // DIODS_H
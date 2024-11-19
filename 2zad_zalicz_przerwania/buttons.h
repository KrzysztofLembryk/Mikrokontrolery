#ifndef BUTTONS_H
#define BUTTONS_H

#include <delay.h>
#include <gpio.h>
#include <stm32.h>

// -----------USER-----------
// stan aktywny przycisku to 0
// trzeba dodac \r\n na koncu stringa bo inaczej robia sie schodki w minicomie
#define USER_BTN_GPIO GPIOC
#define USER_BTN_PIN 13
#define USER_PRESSED "USER PRESSED\r\n"
#define USER_RELEASED "USER RELEASED\r\n"

// robimy dla przyciskow funkcje, a nie makra bo chcemy zrobic tablice pointerow
// do tych funkcji i wywolywac je w petli
uint16_t UserBtnPressed();

// -----------AT MODE-----------
// stan aktywny przycisku to 1
#define AT_BTN_GPIO GPIOA
#define AT_BTN_PIN 0
#define AT_PRESSED "MODE PRESSED\r\n"
#define AT_RELEASED "MODE RELEASED\r\n"

uint16_t ATBtnPressed();

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

uint16_t check_JstickPressed();


uint16_t check_JstickLeftPressed();

uint16_t check_JstickRightPressed();

uint16_t check_JstickUpPressed();

uint16_t check_JstickDownPressed();

uint16_t check_JstickCenterPressed();

#endif // BUTTONS_H
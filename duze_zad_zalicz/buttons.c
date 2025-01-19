#include "buttons.h"

uint16_t UserBtnPressed() 
{
    return !(USER_BTN_GPIO->IDR & (1 << USER_BTN_PIN));
}

uint16_t ATBtnPressed()
{
    return (AT_BTN_GPIO->IDR & (1 << AT_BTN_PIN));
}

uint16_t check_JstickPressed()
{
    if (JstickCenterPressed())
        return JSTICK_CENTER_PIN;
    if (JstickLeftPressed())
        return JSTICK_LEFT_PIN;
    if (JstickRightPressed())
        return JSTICK_RIGHT_PIN;
    if (JstickUpPressed())
        return JSTICK_UP_PIN;
    if (JstickDownPressed())
        return JSTICK_DOWN_PIN;
    return 0;
}

uint16_t check_JstickLeftPressed()
{
    if (JstickLeftPressed())
        return JSTICK_LEFT_PIN;
    return 0;
}

uint16_t check_JstickRightPressed()
{
    if (JstickRightPressed())
        return JSTICK_RIGHT_PIN;
    return 0;
}

uint16_t check_JstickUpPressed()
{
    if (JstickUpPressed())
        return JSTICK_UP_PIN;
    return 0;
}

uint16_t check_JstickDownPressed()
{
    if (JstickDownPressed())
        return JSTICK_DOWN_PIN;
    return 0;
}

uint16_t check_JstickCenterPressed()
{
    if (JstickCenterPressed())
        return JSTICK_CENTER_PIN;
    return 0;
}

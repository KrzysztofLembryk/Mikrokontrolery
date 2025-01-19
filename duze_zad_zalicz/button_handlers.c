#include "button_handlers.h"
#include "buttons.h"
#include "lcd_handlers.h"

void handle_LCD_position(uint16_t jstick_prsd, QInfo *q_info)
{
    switch (jstick_prsd)
    {
    case JSTICK_LEFT_PIN:
        calc_new_lcd_pos(-1, 0, q_info);
        break;
    case JSTICK_RIGHT_PIN:
        calc_new_lcd_pos(1, 0, q_info);
        break;
    case JSTICK_UP_PIN:
        calc_new_lcd_pos(0, -1, q_info);
        break;
    case JSTICK_DOWN_PIN:
        calc_new_lcd_pos(0, 1, q_info);
        break;
    case JSTICK_CENTER_PIN:
        q_add_str(JSTICK_CENTER_PRSSD, q_info);
        break;
    }
}

// Funkcja najpierw sprawdza czy poprzedni przycisk byl wcisniety, jesli tak to
// dodaje do kolejki komunikat o jego zwolnieniu, a nastepnie dodaje do kolejki
// komunikat o wcisnieciu nowego przycisku i aktualizuje wartosc
// prev_jstick_pressed
void handle_jstick_press(uint16_t *prev_jstick_pressed,
                              uint16_t curr_jstick_pressed,
                              QInfo *q_info)
{
    if (*prev_jstick_pressed)
    {
        if (curr_jstick_pressed != *prev_jstick_pressed)
        {
            handle_LCD_position(curr_jstick_pressed, q_info);
            *prev_jstick_pressed = curr_jstick_pressed;
        }
    }
    else
    {
        handle_LCD_position(curr_jstick_pressed, q_info);
        *prev_jstick_pressed = curr_jstick_pressed;
    }
}

// Funkcja sprawdza jaki joystick zostal wcisniety i dodaje do kolejki
// odpowiedni komunikat. Jesli zaden przycisk joysticka nie zostal wcisniety to
// to sprawdza czy jest jakis przycisk ktory byl wcisniety wczesniej i jesli tak
// to wypisuje komunikat o jego zwolnieniu
void handle_jstick(uint16_t jstick_pressed, QInfo *q_info)
{
    static uint16_t prev_jstick_pressed = 0;
    static bool is_center_prsd = false;

    if (is_center_prsd)
    {
        return;
    }

    switch (jstick_pressed)
    {
    case JSTICK_LEFT_PIN:
        handle_jstick_press(&prev_jstick_pressed,
                                 jstick_pressed,
                                 q_info);
        break;
    case JSTICK_RIGHT_PIN:
        handle_jstick_press(&prev_jstick_pressed,
                                 jstick_pressed,
                                 q_info);
        break;
    case JSTICK_UP_PIN:
        handle_jstick_press(&prev_jstick_pressed,
                                 jstick_pressed,
                                 q_info);
        break;
    case JSTICK_DOWN_PIN:
        handle_jstick_press(&prev_jstick_pressed,
                                 jstick_pressed,
                                 q_info);
        break;
    case JSTICK_CENTER_PIN:
        handle_jstick_press(&prev_jstick_pressed,
                                 jstick_pressed,
                                 q_info);
        is_center_prsd = true;
        break;
    default:
        if (prev_jstick_pressed != 0)
        {
            prev_jstick_pressed = 0;
        }
        break;
    }
}
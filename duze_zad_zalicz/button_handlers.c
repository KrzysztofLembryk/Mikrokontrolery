#include "button_handlers.h"
#include "buttons.h"

void handle_jstick_rlsd(uint16_t jstick_rlsd, QInfo *q_info)
{
    switch (jstick_rlsd)
    {
    case JSTICK_LEFT_PIN:
        q_add_str(JSTICK_LEFT_RLSD, q_info);
        break;
    case JSTICK_RIGHT_PIN:
        q_add_str(JSTICK_RIGHT_RLSD, q_info);
        break;
    case JSTICK_UP_PIN:
        q_add_str(JSTICK_UP_RLSD, q_info);
        break;
    case JSTICK_DOWN_PIN:
        q_add_str(JSTICK_DOWN_RLSD, q_info);
        break;
    case JSTICK_CENTER_PIN:
        q_add_str(JSTICK_CENTER_RLSD, q_info);
        break;
    default:
        q_add_str("DEFAULT BREAK\r\n", q_info);
        break;
    }
}

// Funkcja najpierw sprawdza czy poprzedni przycisk byl wcisniety, jesli tak to
// dodaje do kolejki komunikat o jego zwolnieniu, a nastepnie dodaje do kolejki
// komunikat o wcisnieciu nowego przycisku i aktualizuje wartosc 
// prev_jstick_pressed
void add_jstick_msgs_to_queue(uint16_t *prev_jstick_pressed, 
                                uint16_t curr_jstick_pressed, 
                                QInfo *q_info, 
                                char *msg)
{
    if (*prev_jstick_pressed)
    {
        if (curr_jstick_pressed != *prev_jstick_pressed)    
        {
            handle_jstick_rlsd(*prev_jstick_pressed, q_info);
            q_add_str(msg, q_info);
            *prev_jstick_pressed = curr_jstick_pressed;
        }
    }
    else 
    {
        q_add_str(msg, q_info);
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
    switch (jstick_pressed)
    {
    case JSTICK_LEFT_PIN:
        add_jstick_msgs_to_queue(&prev_jstick_pressed, 
                                    jstick_pressed, 
                                    q_info, 
                                    JSTICK_LEFT_PRSSD);
        break;
    case JSTICK_RIGHT_PIN:
        add_jstick_msgs_to_queue(&prev_jstick_pressed, 
                                    jstick_pressed, 
                                    q_info, 
                                    JSTICK_RIGHT_PRSSD);
        break;
    case JSTICK_UP_PIN:
        add_jstick_msgs_to_queue(&prev_jstick_pressed, 
                                    jstick_pressed, 
                                    q_info, 
                                    JSTICK_UP_PRSSD);
        break;
    case JSTICK_DOWN_PIN:
        add_jstick_msgs_to_queue(&prev_jstick_pressed, 
                                    jstick_pressed, 
                                    q_info, 
                                    JSTICK_DOWN_PRSSD);
        break;
    case JSTICK_CENTER_PIN:
        add_jstick_msgs_to_queue(&prev_jstick_pressed, 
                                    jstick_pressed, 
                                    q_info, 
                                    JSTICK_CENTER_PRSSD);
        break;
    default:
        if (prev_jstick_pressed != 0)
        {
            handle_jstick_rlsd(prev_jstick_pressed, q_info);
            prev_jstick_pressed = 0;
        }
        break;
    }
}

void handle_buttons(QInfo *q_info)
{
    uint16_t jstick_prsd = check_JstickPressed();
    handle_jstick(jstick_prsd, q_info);
}

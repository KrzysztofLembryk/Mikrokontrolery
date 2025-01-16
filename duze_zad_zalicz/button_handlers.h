#ifndef BUTTON_HANDLERS_H
#define BUTTON_HANDLERS_H

#include "queue.h"

void handle_jstick_rlsd(uint16_t jstick_rlsd, QInfo *q_info);

// Funkcja najpierw sprawdza czy poprzedni przycisk byl wcisniety, jesli tak to
// dodaje do kolejki komunikat o jego zwolnieniu, a nastepnie dodaje do kolejki
// komunikat o wcisnieciu nowego przycisku i aktualizuje wartosc 
// prev_jstick_pressed
void add_jstick_msgs_to_queue(uint16_t *prev_jstick_pressed, 
                                uint16_t curr_jstick_pressed, 
                                QInfo *q_info, 
                                char *msg);

// Funkcja sprawdza jaki joystick zostal wcisniety i dodaje do kolejki
// odpowiedni komunikat. Jesli zaden przycisk joysticka nie zostal wcisniety to
// to sprawdza czy jest jakis przycisk ktory byl wcisniety wczesniej i jesli tak
// to wypisuje komunikat o jego zwolnieniu
void handle_jstick(uint16_t jstick_pressed, QInfo *q_info);

void handle_buttons(QInfo *q_info);

#endif // BUTTON_HANDLERS_H
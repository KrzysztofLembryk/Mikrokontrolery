#ifndef BUTTON_HANDLERS_H
#define BUTTON_HANDLERS_H

#include "queue.h"

// Funkcja sprawdza jaki joystick zostal wcisniety i dodaje do kolejki
// odpowiedni komunikat. Jesli zaden przycisk joysticka nie zostal wcisniety to
// to sprawdza czy jest jakis przycisk ktory byl wcisniety wczesniej i jesli tak
// to wypisuje komunikat o jego zwolnieniu
void handle_jstick(uint16_t jstick_pressed, QInfo *q_info);

#endif // BUTTON_HANDLERS_H
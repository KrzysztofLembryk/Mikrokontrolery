#include "button_handlers.h"
#include "buttons.h"
#include "lcd.h"

// te stale beda potrzebne zeby wiedziec kiedy wyzerowac juz nasze koordynaty
// zeby w LCD handlerach dostawac wartosci od -16 do LCD_ + 16
#define LCD_WIDTH 128
#define LCD_HEIGHT 160

void handle_LCD_position(uint16_t jstick_prsd, QInfo *q_info)
{
    static int text_font_width = 0;
    static int text_font_height = 0;
    static int pos = 0;
    static int line = 0;
    static bool is_center_prsd = false;

    if (text_font_width == 0 || text_font_height == 0)
    {
        LCD_get_text_width_height(&text_font_width, &text_font_height);
        q_add_str("text_W_H\r\n", q_info);
        q_add_xyz_big(text_font_width, 0, q_info);
        q_add_xyz_big(text_font_height, 1, q_info);
    }

    switch (jstick_prsd)
    {
    case JSTICK_LEFT_PIN:
        pos--;
        if (pos < 0 && -pos >= text_font_width)
        {
            // jak tu jestesmy to znaczy ze przeszlismy maksymalnie w lewo
            // w LCD i caly nasz '+' powinien znalezc sie po prawej stronie
            pos = LCD_WIDTH - text_font_width;
        }
        break;
    case JSTICK_RIGHT_PIN:
        // pozycja moze byc maksymalnie text_font_width - 1 na prawo od konca LCD
        // wtedy jeden piksel jest jeszcze po prawej stronie, ale reszta juz
        // po lewej
        pos = (pos + 1) % (LCD_WIDTH); //+ text_font_width);
        break;
    case JSTICK_UP_PIN:
        // !!!tutaj trzeba dodac petle while bo mozna bedzie w przyszlosci miec
        // line -= k, wiec trzeba bedzie doliczyc dalej te pozycje
        line--;
        if (line < 0 && -line >= text_font_height)
        {
            line = LCD_HEIGHT - text_font_height;
        }
        break;
    case JSTICK_DOWN_PIN:
        // NIE TRZEBA TU DODAWAC TEXT HEIGHT, bo jak jestesmy na line rowne
        // LCD_HEIGHT - 1 to to znaczy ze gorny lewy rog jest juz na samym dole i
        // po kolejnym kliknieciu caly nasz prostokat bedzie na gorze
        line = (line + 1) % (LCD_HEIGHT); //+ text_font_height);
        break;
    case JSTICK_CENTER_PIN:
        q_add_str(JSTICK_CENTER_PRSSD, q_info);
        is_center_prsd = true;
        break;
    }
    if (!is_center_prsd)
    {
        LCDclear();
        LCDgoto(line, pos);
        LCDputchar('+');
        q_add_xyz_big(pos, 0, q_info);
        q_add_xyz_big(line, 1, q_info);
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
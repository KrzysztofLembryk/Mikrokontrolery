#include "lcd_handlers.h"
#include "lcd.h"

#define LCD_WIDTH 128
#define LCD_HEIGHT 160

void calc_new_lcd_pos(int8_t pos_change, int8_t line_change, QInfo *q_info)
{
    static int text_font_width = 0;
    static int text_font_height = 0;
    static int pos = 0;
    static int line = 0;

    if (text_font_width == 0 || text_font_height == 0)
    {
        LCD_get_text_width_height(&text_font_width, &text_font_height);
    }

    // pos_change i line_change moga byc ujemne badz dodatnie wiec zawsze
    // najpierw dodajemy je do naszych zmiennych i potem odpowiednio 
    // modyfikujemy je zeby spelnialy warunki wymiarow LCD
    pos += pos_change;
    line += line_change;

    if (pos_change < 0)
    {
        // Idziemy w lewo
        // jesli pos < 0 to wyszlismy poza lewa krawedz LCD, 
        // Ale tylko jesli jestesmy dalej niz text_font_width od lewej krawedzi
        // to przechodzimy calkowicie na prawy koniec LCD.
        // Robimy modulo bo mozemy sie przeniesc o wiecej niz 1 piksel w lewo 
        // wiec poprzez modulo sprawdzamy ile razy przekroczylismy lewa krawedz
        // i ile powinnismy byc od prawej krawedzi
        if (pos < 0 && -pos >= text_font_width)
        {
            pos = (-pos) % LCD_WIDTH;
            pos = LCD_WIDTH - pos;
        }
    }
    else 
    {
        // idziemy w prawo
        pos = pos % LCD_WIDTH;
    }

    if (line_change < 0)
    {
        // idziemy w gore
        if (line < 0 && -line >= text_font_height)
        {
            line = (-line) % LCD_HEIGHT;
            line = LCD_HEIGHT - line;
        }
    }
    else 
    {
        // idziemy w dol
        line = line % LCD_HEIGHT;
    }

    LCDclear(false);
    LCDgoto(line, pos);
    LCDputchar('+');
}
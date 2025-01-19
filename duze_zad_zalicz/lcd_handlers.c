#include "lcd_handlers.h"
#include "lcd.h"

#define LCD_WIDTH 128
#define LCD_HEIGHT 160

void calc_new_lcd_pos(int pos_change, int line_change)
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
    // modyfikujemy je zeby spelnialy warunki LCD
    pos += pos_change;
    line += line_change;

    if (pos_change < 0)
    {
        // Idziemy w lewo
        // jesli pos < 0 to wyszlismy poza lewa krawedz LCD, 
        // Ale tylko jesli jestesmy dalej niz text_font_width od lewej krawedzi
        // to przechodzimy na prawy koniec LCD.
        // Wyraz (-pos - text_font_width) jest potrzebny dlatego, ze mozemy 
        // przesunac sie nie o 1 w lewo, ale 2, 3, 4 itd. i wtedy powinnismy
        // przeniesc nasz '+' jeszcze dalej od prawej krawedzi
        if (pos < 0 && -pos >= text_font_width)
        {
            pos = LCD_WIDTH - text_font_width - (-pos - text_font_width);
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
            line = LCD_HEIGHT - text_font_height - (-line - text_font_height);
        }
    }
    else 
    {
        // idziemy w dol
        line = line % LCD_HEIGHT;
    }

    LCDclear();
    LCDgoto(line, pos);
    LCDputchar('+');
}
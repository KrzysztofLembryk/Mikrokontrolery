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
        // Wyraz (-pos - text_font_width) jest potrzebny dlatego, ze mozemy 
        // przesunac sie nie o 1 w lewo, ale 2, 3, 4 itd. i wtedy powinnismy
        // przeniesc nasz '+' jeszcze dalej od prawej krawedzi
        if (pos < 0 && -pos >= text_font_width)
        {
            // while (-pos > text_font_width)
            // {
            // gdy -pos == text_font_width to po tej operacji bedzie 0 i nic nam
            // sie nie zmieni w finalnej aktualizacji pos
            // ale gdy -pos > text_font_width to bedziemy miec teraz w tym pos
            // ten dodatkowy nadmiar ktory na koncu odejmiemy od finalnego pos
            pos += text_font_width;
            // }
            
            pos = LCD_WIDTH - text_font_width - (-pos);
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
            // while (-line > text_font_height)
            // {
                line += text_font_height;
            // }
            line = LCD_HEIGHT - text_font_height - (-line);
        }
    }
    else 
    {
        // idziemy w dol
        line = line % LCD_HEIGHT;
    }

    q_add_xyz_big(pos, 0, q_info);
    q_add_xyz_big(line, 1, q_info);

    LCDclear();
    LCDgoto(line, pos);
    LCDputchar('+');
}
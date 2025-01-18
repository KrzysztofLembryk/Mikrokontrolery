#include <delay.h>
#include <fonts.h>
#include <gpio.h>
#include <lcd.h>
#include <lcd_board_def.h>

/** The simple LCD driver (only text mode) for ST7735S controller
    and STM32F2xx or STM32F4xx **/

/* Microcontroller pin definitions:
constants LCD_*_GPIO_N are port letter codes (A, B, C, ...),
constants LCD_*_PIN_N are the port output numbers (from 0 to 15),
constants GPIO_LCD_* are memory pointers,
constants PIN_LCD_* and RCC_LCD_* are bit masks. */

#define GPIO_LCD_CS xcat(GPIO, LCD_CS_GPIO_N)
#define GPIO_LCD_A0 xcat(GPIO, LCD_A0_GPIO_N)
#define GPIO_LCD_SDA xcat(GPIO, LCD_SDA_GPIO_N)
#define GPIO_LCD_SCK xcat(GPIO, LCD_SCK_GPIO_N)

#define PIN_LCD_CS (1U << LCD_CS_PIN_N)
#define PIN_LCD_A0 (1U << LCD_A0_PIN_N)
#define PIN_LCD_SDA (1U << LCD_SDA_PIN_N)
#define PIN_LCD_SCK (1U << LCD_SCK_PIN_N)

#define RCC_LCD_CS xcat3(RCC_AHB1ENR_GPIO, LCD_CS_GPIO_N, EN)
#define RCC_LCD_A0 xcat3(RCC_AHB1ENR_GPIO, LCD_A0_GPIO_N, EN)
#define RCC_LCD_SDA xcat3(RCC_AHB1ENR_GPIO, LCD_SDA_GPIO_N, EN)
#define RCC_LCD_SCK xcat3(RCC_AHB1ENR_GPIO, LCD_SCK_GPIO_N, EN)

/* Screen size in pixels, left top corner has coordinates (0, 0). */

#define LCD_PIXEL_WIDTH 128
#define LCD_PIXEL_HEIGHT 160

/* Some color definitions */

#define LCD_COLOR_WHITE 0xFFFF
#define LCD_COLOR_BLACK 0x0000
#define LCD_COLOR_GREY 0xF7DE
#define LCD_COLOR_BLUE 0x001F
#define LCD_COLOR_BLUE2 0x051F
#define LCD_COLOR_RED 0xF800
#define LCD_COLOR_MAGENTA 0xF81F
#define LCD_COLOR_GREEN 0x07E0
#define LCD_COLOR_CYAN 0x7FFF
#define LCD_COLOR_YELLOW 0xFFE0

/* Needed delay(s)  */

#define Tinit 150
#define T120ms (MAIN_CLOCK_MHZ * 120000 / 4)

/* Text mode globals */

static const font_t *CurrentFont;
static uint16_t TextColor = LCD_COLOR_BLACK;
static uint16_t BackColor = LCD_COLOR_WHITE;

/* Current character line and position, the number of lines, the
number of characters in a line, position 0 and line 0 offset on screen
in pixels */

static int Line, Position, TextHeight, TextWidth, XOffset, YOffset;

// moje - dzieki tym zmiennym bedziemy wiedziec ile poza LCD i w
// ktora strone jest wysuniety znacznik '+'
static int overflow_bits_up, overflow_bits_down;
static int overflow_bits_left, overflow_bits_right;
// koniec mojego

/** Internal functions **/

/* The following four functions are inlined and "if" statement is
eliminated during optimization if the "bit" argument is a constant. */

static void CS(uint32_t bit)
{
  if (bit)
  {
    GPIO_LCD_CS->BSRR = PIN_LCD_CS; /* Activate chip select line. */
  }
  else
  {
    GPIO_LCD_CS->BSRR = PIN_LCD_CS << 16; /* Deactivate chip select line. */
  }
}

static void A0(uint32_t bit)
{
  if (bit)
  {
    GPIO_LCD_A0->BSRR = PIN_LCD_A0; /* Set data/command line to data. */
  }
  else
  {
    GPIO_LCD_A0->BSRR = PIN_LCD_A0 << 16; /* Set data/command line to command. */
  }
}

static void SDA(uint32_t bit)
{
  if (bit)
  {
    GPIO_LCD_SDA->BSRR = PIN_LCD_SDA; /* Set data bit one. */
  }
  else
  {
    GPIO_LCD_SDA->BSRR = PIN_LCD_SDA << 16; /* Set data bit zero. */
  }
}

static void SCK(uint32_t bit)
{
  if (bit)
  {
    GPIO_LCD_SCK->BSRR = PIN_LCD_SCK; /* Rising clock edge. */
  }
  else
  {
    GPIO_LCD_SCK->BSRR = PIN_LCD_SCK << 16; /* Falling clock edge. */
  }
}

static void RCCconfigure(void)
{
  /* Enable GPIO clocks. */
  RCC->AHB1ENR |= RCC_LCD_CS | RCC_LCD_A0 | RCC_LCD_SDA | RCC_LCD_SCK;
}

static void GPIOconfigure(void)
{
  CS(1); /* Set CS inactive. */
  GPIOoutConfigure(GPIO_LCD_CS, LCD_CS_PIN_N, GPIO_OType_PP,
                   GPIO_High_Speed, GPIO_PuPd_NOPULL);

  A0(1); /* Data are sent default. */
  GPIOoutConfigure(GPIO_LCD_A0, LCD_A0_PIN_N, GPIO_OType_PP,
                   GPIO_High_Speed, GPIO_PuPd_NOPULL);

  SDA(0);
  GPIOoutConfigure(GPIO_LCD_SDA, LCD_SDA_PIN_N, GPIO_OType_PP,
                   GPIO_High_Speed, GPIO_PuPd_NOPULL);

  SCK(0); /* Data bit is written on rising clock edge. */
  GPIOoutConfigure(GPIO_LCD_SCK, LCD_SCK_PIN_N, GPIO_OType_PP,
                   GPIO_High_Speed, GPIO_PuPd_NOPULL);
}

static void LCDwriteSerial(uint32_t data, uint32_t length)
{
  uint32_t mask;

  mask = 1U << (length - 1);
  while (length > 0)
  {
    // SDA - w data mamy ciag bitow 0-1, w mask idziemy od
    // najstarszego bitu, czyli od lewej, maska ma same 0 oprocz
    // jednej 1 ktora w danym obrocie petli sprawdzamy AND
    // jesli data & mask da 1, to na miejscu i-tym w data jest 1
    // wiec zapalamy bit na LCD robiac SDA
    SDA(data & mask); /* Set bit. */
    --length;         /* Add some delay. */
    SCK(1);           /* Rising edge writes bit. */
    mask >>= 1;       /* Add some delay. */
    SCK(0);           /* Falling edge ends the bit transmission. */
  }
}

static void LCDwriteCommand(uint32_t data)
{
  // transmitujemy jaka komenda ma byc dostarczona do LCD
  // A0(0) - set line to command
  A0(0);
  LCDwriteSerial(data, 8);
  // A0(1) - set line to data
  // tutaj transmitujemy dane czyli rysujemy cos na LCD
  A0(1);
}

static void LCDwriteData8(uint32_t data)
{
  /* A0(1); is already set */
  LCDwriteSerial(data, 8);
}

static void LCDwriteData16(uint32_t data)
{
  /* A0(1); is already set */
  LCDwriteSerial(data, 16);
}

static void LCDwriteData24(uint32_t data)
{
  /* A0(1); is already set */
  LCDwriteSerial(data, 24);
}

static void LCDwriteData32(uint32_t data)
{
  /* A0(1); is already set */
  LCDwriteSerial(data, 32);
}

void LCDsetRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
  // Ta komenda chyba mowi po prostu ze ustalamy wlasnie lewy gorny
  // rog, a ta druga ze prawy dolny
  LCDwriteCommand(0x2A);
  LCDwriteData16(x1);
  LCDwriteData16(x2);

  LCDwriteCommand(0x2B);
  LCDwriteData16(y1);
  LCDwriteData16(y2);

  LCDwriteCommand(0x2C);
}

static void LCDcontrollerConfigure(void)
{
  /* Activate chip select line */
  CS(0);

  Delay(Tinit);

  /* Sleep out */
  LCDwriteCommand(0x11);

  Delay(T120ms);

  /* Frame rate */
  LCDwriteCommand(0xB1);
  LCDwriteData24(0x053C3C);
  LCDwriteCommand(0xB2);
  LCDwriteData24(0x053C3C);
  LCDwriteCommand(0xB3);
  LCDwriteData24(0x053C3C);
  LCDwriteData24(0x053C3C);

  /* Dot inversion */
  LCDwriteCommand(0xB4);
  LCDwriteData8(0x03);

  /* Power sequence */
  LCDwriteCommand(0xC0);
  LCDwriteData24(0x280804);
  LCDwriteCommand(0xC1);
  LCDwriteData8(0xC0);
  LCDwriteCommand(0xC2);
  LCDwriteData16(0x0D00);
  LCDwriteCommand(0xC3);
  LCDwriteData16(0x8D2A);
  LCDwriteCommand(0xC4);
  LCDwriteData16(0x8DEE);

  /* VCOM */
  LCDwriteCommand(0xC5);
  LCDwriteData8(0x1A);

  /* Memory and color write direction */
  LCDwriteCommand(0x36);
  LCDwriteData8(0xC0);

  /* Color mode 16 bit per pixel */
  LCDwriteCommand(0x3A);
  LCDwriteData8(0x05);

  /* Gamma sequence */
  LCDwriteCommand(0xE0);
  LCDwriteData32(0x0422070A);
  LCDwriteData32(0x2E30252A);
  LCDwriteData32(0x28262E3A);
  LCDwriteData32(0x00010313);
  LCDwriteCommand(0xE1);
  LCDwriteData32(0x0416060D);
  LCDwriteData32(0x2D262327);
  LCDwriteData32(0x27252D3B);
  LCDwriteData32(0x00010413);

  /* Display on */
  LCDwriteCommand(0x29);

  /* Deactivate chip select */
  CS(1);
}

static void LCDsetFont(const font_t *font)
{
  CurrentFont = font;
  TextHeight = LCD_PIXEL_HEIGHT / CurrentFont->height;
  TextWidth = LCD_PIXEL_WIDTH / CurrentFont->width;
  XOffset = (LCD_PIXEL_WIDTH - TextWidth * CurrentFont->width) / 2;
  YOffset = (LCD_PIXEL_HEIGHT - TextHeight * CurrentFont->height) / 2;
}

static void LCDsetColors(uint16_t text, uint16_t back)
{
  TextColor = text;
  BackColor = back;
}

typedef struct RectCoords
{
  char pos;
  uint16_t up_l_x;
  uint16_t up_l_y;
  uint16_t down_r_x;
  uint16_t down_r_y;
} RectCoords;

void init_RectangleCoords(RectCoords *rect)
{
  // X oznacza ze brak tego prostokata
  rect->pos = 'X';
  rect->up_l_x = 0;
  rect->up_l_y = 0;
  rect->down_r_x = 0;
  rect->down_r_y = 0;
}

void set_UD_rect_coords(RectCoords *main_rect, RectCoords *overflow_rect_y)
{
  // Teraz ustalamy prostokat ktory zostaje po czesciowym przejsciu
  // poza LCD w pionie
  // jednoczesnie tylko jedno z tych zdarzen moze zajsc
  if (overflow_bits_up != 0)
  {
    main_rect->up_l_y = 0;
    main_rect->down_r_y = TextHeight - overflow_bits_up;

    overflow_rect_y->pos = 'U';
    overflow_rect_y->up_l_y = LCD_PIXEL_HEIGHT - overflow_bits_up;
    overflow_rect_y->down_r_y = LCD_PIXEL_HEIGHT - 1;  
  }
  else if (overflow_bits_down != 0)
  {
    main_rect->up_l_y = Line;
    main_rect->down_r_y = LCD_PIXEL_HEIGHT - 1;

    overflow_rect_y->pos = 'D';
    overflow_rect_y->up_l_y = 0;
    overflow_rect_y->down_r_y = overflow_bits_down - 1;  
  }
  // to mozemy ustawic tutaj bo nawet jak jest pos X to najwyzej tego nie 
  // uzyjemy
    overflow_rect_y->up_l_x = main_rect->up_l_x;
    overflow_rect_y->down_r_x = main_rect->down_r_x;
}

void set_LR_rect_coords(
  RectCoords *main_rect, 
  RectCoords *overflow_rect_y,
  RectCoords *overflow_rect_x)
{

  if (overflow_bits_left != 0)
  {
    main_rect->up_l_x = 0;
    main_rect->down_r_x = TextWidth - overflow_bits_left;

    overflow_rect_x->pos = 'L';
    overflow_rect_x->up_l_x = LCD_PIXEL_WIDTH - overflow_bits_left;
    overflow_rect_x->down_r_x = LCD_PIXEL_WIDTH - 1;
    overflow_rect_x->up_l_y = main_rect->up_l_y;
    overflow_rect_x->down_r_y = main_rect->down_r_y;
    // if (overflow_rect_y->pos == 'X')
    // {
    //   overflow_rect_x->up_l_x = LCD_PIXEL_WIDTH - overflow_bits_left;
    //   overflow_rect_x->down_r_x = LCD_PIXEL_WIDTH - 1;
    //   overflow_rect_x->up_l_y = main_rect->up_l_y;
    //   overflow_rect_x->down_r_y = main_rect->down_r_y;
    // }
    // else
    // {
    //   overflow_rect_x->up_l_x = LCD_PIXEL_WIDTH - overflow_bits_left;
    //   overflow_rect_x->down_r_x = LCD_PIXEL_WIDTH - 1;
    //   overflow_rect_x->up_l_y = overflow_rect_y.up_l_y;
    //   overflow_rect_x->down_r_y = overflow_rect_y.down_r_y;
    // }
  }
  else if (overflow_bits_right != 0)
  {
    main_rect->up_l_x = Position;
    main_rect->down_r_x = LCD_PIXEL_WIDTH - 1;

    overflow_rect_x->pos = 'R';
    overflow_rect_x->up_l_x = 0;
    overflow_rect_x->down_r_x = overflow_bits_right - 1;
    overflow_rect_x->up_l_y = main_rect.up_l_y;
    overflow_rect_x->down_r_y = main_rect.down_r_y;
    // if (overflow_rect_y->pos == 'X')
    // {
    //   overflow_rect_x->up_l_x = 0;
    //   overflow_rect_x->down_r_x = overflow_bits_right - 1;
    //   overflow_rect_x->up_l_y = main_rect.up_l_y;
    //   overflow_rect_x->down_r_y = main_rect.down_r_y;
    // }
  }
}

void handle_drawing_when_overflow(unsigned c)
{
    uint16_t const *p;
    // uint16_t up_l_x, up_l_y, down_r_x, down_r_y, w;
    uint16_t w;
    int i, j;
    RectCoords main_rect, overflow_rect_y, overflow_rect_x;

    init_RectangleCoords(&main_rect);
    init_RectangleCoords(&overflow_rect_y);
    init_RectangleCoords(&overflow_rect_x);

    main_rect.pos = 'M';
    main_rect.up_l_x = Position;
    main_rect.up_l_y = Line;
    main_rect.down_r_x = main_rect.up_l_x + CurrentFont->width - 1;
    main_rect.down_r_y = main_rect.up_l_y + CurrentFont->height - 1;

    set_UD_rect_coords(&main_rect, &overflow_rect_y);
    set_LR_rect_coords(&main_rect, &overflow_rect_y, &overflow_rect_x);


  // Nie ma zadnego overflow czyli robimy zwykle rysowanie
  // LCDsetRectangle(x, y, x + CurrentFont->width - 1,
  //                 y + CurrentFont->height - 1);

    p = &CurrentFont->table[(c - FIRST_CHAR) * CurrentFont->height];

    if (overflow_rect_y.pos == 'U')
    {
        LCDsetRectangle(
            overflow_rect_y.up_l_x, 
            overflow_rect_y.up_l_y, 
            overflow_rect_y.down_r_x, 
            overflow_rect_y.down_r_y);
    }

    for (i = 0; i < CurrentFont->height; ++i)
    {
        if (i == overflow_bits_up)
        {
            LCDsetRectangle(
                main_rect.up_l_x, 
                main_rect.up_l_y, 
                main_rect.down_r_x, 
                main_rect.down_r_y);
        }
        for (j = 0, w = p[i]; j < CurrentFont->width; ++j, w >>= 1)
        {
        LCDwriteData16(w & 1 ? TextColor : BackColor);
        }
    }
}

static void LCDdrawChar(unsigned c)
{
  uint16_t const *p;
  uint16_t x, y, w;
  int i, j;

  // CS(0) - deactivate chip select line
  CS(0);

  // setRectangle - ustawia prostokat w ktorym bedziemy pisac
  // dzieki czemu nasze n * 16bitowe obrazki
  // beda sie dobrze zawijaly, bo nie piszemy po calym LCD tylko
  // po tym wydzielonym malym prostokacie
  //
  // Offset tez raczej niepotrzebny bo on byl po to zeby sie
  // calkowita liczba prostokatow zmiescila w jednej linii
  //
  // y = YOffset + CurrentFont->height * Line;
  // x = XOffset + CurrentFont->width  * Position;

  if (overflow_bits_up == 0 &&
      overflow_bits_down == 0 &&
      overflow_bits_left == 0 &&
      overflow_bits_right == 0)
  {
    y = Line;
    x = Position;
    // Nie ma zadnego overflow czyli robimy zwykle rysowanie
    LCDsetRectangle(x, y, x + CurrentFont->width - 1,
                    y + CurrentFont->height - 1);

    p = &CurrentFont->table[(c - FIRST_CHAR) * CurrentFont->height];

    for (i = 0; i < CurrentFont->height; ++i)
    {
      for (j = 0, w = p[i]; j < CurrentFont->width; ++j, w >>= 1)
      {
        LCDwriteData16(w & 1 ? TextColor : BackColor);
      }
    }
  }
  else
  {
    handle_drawing_when_overflow(c);
  }
  // to co piszemy uzywajac LCDwriteData16 to piszemy 16bitow i gdy
  // skonczy sie miejsce na LCD rectangle to automatyczni on
  // przechodzi do nowej linii
  // --------------------------------------------------------------
  // UWAGA - zeby zaprogramowac warunki brzegowe, czyli zeby srodek
  // + mogl dojsc do brzegu i zeby dalsza czesc + pojawila sie z
  // drugiej strony, trzeba bedzie zmienic funkcje piszaca
  CS(1);
}

/** Public interface implementation **/

void LCDconfigure()
{
  /* See Errata, 2.1.6 Delay after an RCC peripheral clock enabling */
  RCCconfigure();
  /* Initialize global variables. */
  LCDsetFont(&LCD_DEFAULT_FONT);
  LCDsetColors(LCD_COLOR_WHITE, LCD_COLOR_BLUE);
  /* Initialize hardware. */
  GPIOconfigure();
  LCDcontrollerConfigure();
  LCDclear();

  // Moja inicjalizacja zmiennych sledzacych polozenie znacznika
  // gdy wyjdzie on poza ekran
  overflow_bits_up = 0;
  overflow_bits_down = 0;
  overflow_bits_left = 0;
  overflow_bits_right = 0;
}

void LCDclear()
{
  int i, j;

  CS(0);
  LCDsetRectangle(0, 0, LCD_PIXEL_WIDTH - 1, LCD_PIXEL_HEIGHT - 1);
  for (i = 0; i < LCD_PIXEL_WIDTH; ++i)
  {
    for (j = 0; j < LCD_PIXEL_HEIGHT; ++j)
    {
      LCDwriteData16(BackColor);
    }
  }
  CS(1);

  LCDgoto(0, 0);
}

void calc_overflow_bits(int textLine, int charPos)
{
  if (textLine >= LCD_PIXEL_HEIGHT)
  {
    // zeszlismy w dol poza LCD, maksymalnie TectHeight - 1
    // bitow, wiec obliczamy ile dokladnie bitow w dol jestesmy
    // gdy textLine = LCD_PIXEL_HEIGHT to znaczy ze jestesmy
    // dokladnie JEDEN bit w dol poza LCD
    overflow_bits_down = textLine - (LCD_PIXEL_HEIGHT - 1);
  }
  else if (textLine < 0)
  {
    // jak idziemy w gore to mamy ujemne wartosci od -1 az do
    // -(TextHeight - 1) wiec zeby dostac liczbe bitow wystarczy
    // przemnozyc to razy -1
    overflow_bits_up = -textLine;
  }

  if (charPos >= LCD_PIXEL_WIDTH)
  {
    // poszlismy w prawo poza LCD, maksymalnie TextWidth - 1
    overflow_bits_right = charPos - (LCD_PIXEL_WIDTH - 1);
  }
  else if (charPos < 0)
  {
    // jak idziemy w lewo to mamy ujemne wartosci od -1 az do
    // -(TextWidth - 1)
    overflow_bits_left = -charPos;
  }
}

void LCDgoto(int textLine, int charPos)
{
  // dodane przeze mnie - okresowe warunki brzegowe z obsluga wyjscia
  // za LCD czesciowo
  if (textLine >= 0 && textLine < LCD_PIXEL_HEIGHT &&
      charPos >= 0 && charPos < LCD_PIXEL_WIDTH)
  {

    Line = textLine % LCD_PIXEL_HEIGHT;
    Position = charPos % LCD_PIXEL_WIDTH;

    // jestesmy wewnatrz LCD wiec zerujemy bity overflow
    overflow_bits_up = 0;
    overflow_bits_down = 0;
    overflow_bits_left = 0;
    overflow_bits_right = 0;
  }
  else
  {
    // wyszlismy w jakims stopniu poza LCD wiec ustawiamy
    // bity overflow
    calc_overflow_bits(textLine, charPos);
  }
  // STARE
  // while (textLine < 0)
  //	textLine += LCD_PIXEL_HEIGHT; // bylo TextHeight

  // while (charPos < 0)
  //	charPos += LCD_PIXEL_WIDTH;
  //  koniec dodanego przeze mnie

  // aktualizujemy Line i Position, ale w obsludze overflow bitow
  // raczej nie bedziemy juz uzywac ponizszych wartosci
  Line = textLine;    //% LCD_PIXEL_HEIGHT;
  Position = charPos; //% LCD_PIXEL_WIDTH;
}

void LCDputchar(char c)
{
  if (c == '\n')
    LCDgoto(Line + 1, 0); /* line feed */
  else if (c == '\r')
    LCDgoto(Line, 0); /* carriage return */
  else if (c == '\t')
    LCDgoto(Line, (Position + 8) & ~7); /* tabulator */
  else
  {
    // ten defaultowy if dziala przy zalozeniu ze znaki sa w swoich
    // kwadratach i te kwadraty nie nachodza na siebie, wiec jest
    // ich tylko kilka, wiec jak bysmy pisali kolejne znaki to one
    // bylyby ladnie ulozone obok siebie na LCD,
    // ALE u nas mamy tylko jeden znak ktory przesuwamy wiec chcemy
    // moc go przesuwac o jeden piksel a nie o caly kwadrat w prawo
    // if (c >= FIRST_CHAR && c <= LAST_CHAR &&
    //   Line >= 0 && Line < TextHeight &&
    //  Position >= 0 && Position < TextWidth) {
    // LCDdrawChar(c);
    //}
    if (c >= FIRST_CHAR && c <= LAST_CHAR &&
        Line > -TextHeight && Line < LCD_PIXEL_HEIGHT + TextHeight &&
        Position > -TextWidth && Position < LCD_PIXEL_WIDTH + TextWidth)
    {
      LCDdrawChar(c);
    }
    // nie potrzebujemy isc do nastepnej kolumny bo i tak w glownej
    // petli sami ustawiamy gdzie bedzie nastepna pozycja
    // LCDgoto(Line, Position + 1);
  }
}

void LCDputcharWrap(char c)
{
  /* Check if, there is room for the next character,
  but does not wrap on white character. */
  if (Position >= TextWidth &&
      c != '\t' && c != '\r' && c != '\n' && c != ' ')
  {
    LCDputchar('\n');
  }
  LCDputchar(c);
}

void LCD_get_text_width_height(int *w, int *h)
{
  *w = TextWidth;
  *h = TextHeight;
}

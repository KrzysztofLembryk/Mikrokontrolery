#include <inttypes.h>
#include <gpio.h>
#include <delay.h>
#include <stm32.h>
#include "diods.h"
#include "init_funcs.h"
#include "i2c1_handlers.h"
#include "queue.h"

// KOMENDA WGRYWJACA PROGRAM NA PLYTKE
// /opt/arm/stm32/ocd/qfn4
// dokumentacja: /opt/arm/stm32/doc
// komenda do komunikacj z plytka to minicom
void int8_to_string(int8_t value, char *str)
{
    char *ptr = str;
    bool is_negative = false;

    // Handle negative values
    if (value < 0)
    {
        is_negative = true;
        value = -value;
    }

    // Null-terminate the string
    *ptr = '\r';
    ptr++;
    *ptr = '\n';
    ptr++;

    // Convert the integer to a string
    do
    {
        *ptr = '0' + (value % 10);
        ptr++;
        value /= 10;
    } while (value > 0);

    if (is_negative)
    {
        *ptr = '-';
        ptr++;
    }


    // Reverse the string
    for (char *start = str, *end = ptr - 1; start < end; ++start, --end)
    {
        char temp = *start;
        *start = *end;
        *end = temp;
    }
}


void add_xyz_to_q(int8_t x, int8_t y, int8_t z, QInfo *q_info)
{
    char dec_str[10]; 
    dec_str[0] = 'x';
    dec_str[1] = ':';
    int8_to_string(x, dec_str + 2);
    q_add_str(dec_str, q_info);

    dec_str[0] = 'y';
    int8_to_string(y, dec_str + 2);
    q_add_str(dec_str, q_info);

    dec_str[0] = 'z';
    int8_to_string(z, dec_str + 2);
    q_add_str(dec_str, q_info);
}

int main()
{
    init_rcc();
    init_usart2_TXD_RXD_lines();
    init_usart2_cr_registers();
    init_diods();
    init_I2C1();

    QInfo q_info;
    init_QInfo(&q_info, QUEUE_SIZE);
    
    if (!I2C1_send_power_en())
    {
        RedLEDon();
        while(1);
    }

    int8_t x = 0, y = 0, z = 0;
    while (1)
    {
        I2C1_recv(&x, &y, &z);
        add_xyz_to_q(x, y, z, &q_info);
        
        if (USART2->SR & USART_SR_TXE)
        {
            char c;
            if (q_remove(&c, &q_info))
                USART2->DR = c;
        }
    }
}
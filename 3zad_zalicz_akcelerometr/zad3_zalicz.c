#include <inttypes.h>
#include <gpio.h>
#include <delay.h>
#include <stm32.h>
#include "diods.h"
#include "init_funcs.h"
#include "handlers.h"


// KOMENDA WGRYWJACA PROGRAM NA PLYTKE
// /opt/arm/stm32/ocd/qfn4
// komenda do komunikacj z plytka to minicom

int main()
{
    init_rcc();
    init_usart2_TXD_RXD_lines();
    init_usart2_cr_registers();
    init_diods();
    init_I2C1();

    int what_to_send = 0;
    int x = 0, y = 0, z = 0;
    while (1)
    {
        if (what_to_send == 0)
            handle_I2C1_recv(&x, &y, &z);
        else 
            {
                if (USART2->SR & USART_SR_RXNE)
                {
                    USART2->DR = 'x';
                }
            }
        if (USART2->SR & USART_SR_TXE)
        {
            char c = 'a';
            if (what_to_send == 0)
            {
                USART2->DR = x;
                what_to_send = 1;
            }
            else if (what_to_send == 1)
            {
                c = '\r';
                USART2->DR = c;
                what_to_send = 2;
            }
            else if (what_to_send == 2)
            {
                c = '\n';
                USART2->DR = c;
                what_to_send = 0;
            }
        }
    }
}
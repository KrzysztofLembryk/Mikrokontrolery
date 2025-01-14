#include <inttypes.h>
#include <gpio.h>
#include <delay.h>
#include <stm32.h>
#include "diods.h"
#include "init_funcs.h"
#include "i2c1_interrupts_handlers.h"

// KOMENDA WGRYWJACA PROGRAM NA PLYTKE
// /opt/arm/stm32/ocd/qfn4
// dokumentacja: /opt/arm/stm32/doc
// komenda do komunikacj z plytka to minicom

int main()
{
    init_rcc();
    init_usart2_TXD_RXD_lines();
    init_usart2_cr_registers();
    init_diods();
    init_I2C1();
    init_I2C1_interrupts_handlers_data();
    init_I2C1_accelerometer_transmission();
    
    while (1)
    {
        if (USART2->SR & USART_SR_TXE)
        {
            char c;
            if (q_remove(&c, &data_queue))
                USART2->DR = c;
        }
    }
}
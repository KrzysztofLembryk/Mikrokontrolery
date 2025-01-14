#include <inttypes.h>
#include <gpio.h>
#include <delay.h>
#include <stm32.h>
#include "diods.h"
#include "init_funcs.h"
#include "i2c1_handlers.h"
#include "queue.h"
#include "i2c1_interrupts_handlers.h"

// KOMENDA WGRYWJACA PROGRAM NA PLYTKE
// /opt/arm/stm32/ocd/qfn4
// dokumentacja: /opt/arm/stm32/doc
// komenda do komunikacj z plytka to minicom
#define REG_X_TYPE 0
#define REG_Y_TYPE 1
#define REG_Z_TYPE 2

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
    int counter = 0;
    while (1)
    {
        if (counter % 19 == 0)
        {
            I2C1_recv(&x, &y, &z);
            q_add_xyz(x, REG_X_TYPE, &q_info);
            q_add_xyz(y, REG_Y_TYPE, &q_info);
            q_add_xyz(z, REG_Z_TYPE, &q_info);
            counter = 0;
        }
        
        if (USART2->SR & USART_SR_TXE)
        {
            char c;
            if (q_remove(&c, &q_info))
                USART2->DR = c;
            
            counter++;
        }
    }
}
#include <inttypes.h>
#include <stdbool.h>
#include <gpio.h>
#include <delay.h>
#include <stm32.h>


// KOMENDA WGRYWJACA PROGRAM NA PLYTKE
// /opt/arm/stm32/ocd/qfn4
// komenda do komunikacj z plytka to minicom

int main()
{
    init_rcc();
    init_diods();

    while (1)
    {
    }
}
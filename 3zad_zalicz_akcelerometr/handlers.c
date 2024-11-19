#include "handlers.h"
#include <gpio.h>
#include <delay.h>
#include <stm32.h>
#include <inttypes.h>
#include <stdbool.h>

// Nasz akcelerometr to LIS35DE 
// w dokumentacji w opt/arm/docs pod LIS35DE na stronie 25 mamy opisane 
// do jakich rejestrow co musimy wpisac, zeby wlaczyc akcelerometr

// --> Czyli najpierw musimy zaimplementowac pisanie po I2C zeby wpisac te 
// rzeczy do rejestrów, a dopiero potem bedziemy mogli odczytywac dane z
// akcelerometru

#define LIS35DE_ADDR 0x1C

#define OUT_X 0x29
#define OUT_Y 0x2B
#define OUT_Z 0x2D

#define ACCELEROMETER_REG_NBR_1 0x1C
#define ACCELEROMETER_REG_NBR_2 0x1D

void wait_timeout()
{
    volatile int i = 0;
    while(i < 1500000)
    {
        i++;
        __NOP();
    }
}


bool init_start_transmission()
{
    // inicjalizacja transmisji sygnalu START
    I2C1->CR1 |= I2C_CR1_START;

    // czekamy na ustawienie bitu SB - Start Bit
    // jesli bedzie ustawiony to SR1 na odpowiednim miejscu bedzie 1 
    // czyli warunek petli bedzie 1, dlatego dajemy !
    bool check = false;
    do
    {
        if (check)
        {
            I2C1->CR1 |= I2C_CR1_STOP;
            return false;
        }
        // TIMEOUT
        wait_timeout();
        check = true;
    }while(!(I2C1->SR1 & I2C_SR1_SB));

    // Wysylamy 7-bitowy adresu slave'a (urzadzenia)
    // Moze to byc albo 0x1C albo 0x1D
    I2C1->DR = LIS35DE_ADDR << 1;

    // czekamy na ustawienie bitu ADDR - Address sent
    check = false;
    do
    {
        if (check)
        {
            return false;
        }
        // TIMEOUT
        wait_timeout();
        check = true;
    
    } while(!(I2C1->SR1 & I2C_SR1_ADDR));
    // odczytujemy SR2, aby wyczyscic bit ADDR, jest to wymagane aby zakonczyc 
    // procedure adresowania i przejsc do nastepnego etapu transmisji
    I2C1->SR2;

    return true;
}

int generate_repeated_start(int reg)
{
    // Wysylamy numer rejestru z ktorego chcemy odczytac dane
    I2C1->DR = reg;

    // czekamy na ustawienie bitu BTF - Byte Transfer Finished
    bool check = false;
    do
    {
        if (check)
        {
            return 0;
        }
        // TIMEOUT
        wait_timeout();
        check = true;

    } while(!(I2C1->SR1 & I2C_SR1_BTF)); 

    // inicjujemy transmisje sygnalu REPEATED START
    I2C1->CR1 |= I2C_CR1_START;

    // czekamy na ustawienie bitu SB - Start Bit
    check = false;
    do
    {
        if (check)
        {
            return 0;
        }
        // TIMEOUT
        wait_timeout();
        check = true;

    } while(!(I2C1->SR1 & I2C_SR1_SB));

    // Zainicjuj wysyłanie 7-bitowego adresu slave’a, tryb MR
    I2C1->DR = LIS35DE_ADDR << 1 | 1;

    // Ponieważ ma być odebrany tylko jeden bajt, ustaw wysłanie
    // sygnału NACK, zerując bit ACK
    I2C1->CR1 &= ~I2C_CR1_ACK;

    // czekamy na zakonczenie transmisji
    check = false;
    do
    {
        if (check)
        {
            return 0;
        }
        // TIMEOUT
        wait_timeout();
        check = true;

    } while(!(I2C1->SR1 & I2C_SR1_ADDR));

    // kasujemy bit ADDR
    I2C1->SR2;

    // Czekaj na ustawienie bitu RXNE (ang. receiver data register not empty)
    check = false;
    do
    {
        if (check)
        {
            return 0;
        }
        // TIMEOUT
        wait_timeout();
        check = true;

    } while(!(I2C1->SR1 & I2C_SR1_RXNE));

    int value = I2C1->DR;
    return value;
}

void end_transmission()
{
    // Zainicjuj transmisję sygnału STOP, aby został wysłany po
    // odebraniu ostatniego bajtu
    I2C1->CR1 |= I2C_CR1_STOP;
}

void handle_I2C1_recv(int *x_val, int *y_val, int *z_val)
{
    if (!init_start_transmission())
    {
        return;
    }
    *x_val = generate_repeated_start(OUT_X);
    *y_val = generate_repeated_start(OUT_Y);
    *z_val = generate_repeated_start(OUT_Z);
    end_transmission();
}
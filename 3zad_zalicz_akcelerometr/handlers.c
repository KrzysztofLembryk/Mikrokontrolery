#include "handlers.h"
#include <gpio.h>
#include <delay.h>
#include <stm32.h>

// Nasz akcelerometr to LIS35DE 


#define OUT_X 0x29
#define OUT_Y 0x2B
#define OUT_Z 0x2D

#define ACCELEROMETER_REG_NBR_1 0x1C
#define ACCELEROMETER_REG_NBR_2 0x1D

void init_start_transmission()
{
    // inicjalizacja transmisji sygnalu START
    I2C1->CR1 |= I2C_CR1_START;

    // czekamy na ustawienie bitu SB - Start Bit
    // jesli bedzie ustawiony to SR1 na odpowiednim miejscu bedzie 1 
    // czyli warunek petli bedzie 1, dlatego dajemy !
    while(!(I2C1->SR1 & I2C_SR1_SB)) 
    {
        // TIMEOUT
    }

    // Wysylamy 7-bitowy adresu slave'a (urzadzenia)
    I2C1->DR = LIS35DE_ADDR << 1;

    // czekamy na ustawienie bitu ADDR - Address sent
    while(!(I2C1->SR1 & I2C_SR1_ADDR)) 
    {
        // TIMEOUT
    }
    // odczytujemy SR2, aby wyczyscic bit ADDR, jest to wymagane aby zakonczyc 
    // procedure adresowania i przejsc do nastepnego etapu transmisji
    I2C1->SR2;

}

int generate_repeated_start(int reg)
{
    // Wysylamy numer rejestru z ktorego chcemy odczytac dane
    I2C1->DR = reg;

    // czekamy na ustawienie bitu BTF - Byte Transfer Finished
    while(!(I2C1->SR1 & I2C_SR1_BTF)) 
    {
        // TIMEOUT
    }

    // inicjujemy transmisje sygnalu REPEATED START
    I2C1->CR1 |= I2C_CR1_START;

    // czekamy na ustawienie bitu SB - Start Bit
    while(!(I2C1->SR1 & I2C_SR1_SB)) 
    {
        // TIMEOUT
    }

    // Zainicjuj wysyłanie 7-bitowego adresu slave’a, tryb MR
    I2C1->DR = LIS35DE_ADDR << 1 | 1;

    // Ponieważ ma być odebrany tylko jeden bajt, ustaw wysłanie
    // sygnału NACK, zerując bit ACK
    I2C1->CR1 &= ~I2C_CR1_ACK;

    // czekamy na zakonczenie transmisji
    while(!(I2C1->SR1 & I2C_SR1_ADDR)) 
    {
        // TIMEOUT
    }

    // kasujemy bit ADDR
    I2C1->SR2;

    // Czekaj na ustawienie bitu RXNE (ang. receiver data register not empty)
    while(!(I2C1->SR1 & I2C_SR1_RXNE)) 
    {
        // TIMEOUT
    }

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
    init_start_transmission();
    *x_val = generate_repeated_start(OUT_X);
    *y_val = generate_repeated_start(OUT_Y);
    *z_val = generate_repeated_start(OUT_Z);
    end_transmission();
}
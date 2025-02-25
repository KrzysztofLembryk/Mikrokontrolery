#include "i2c1_handlers.h"
#include <gpio.h>
#include <delay.h>
#include <stm32.h>
#include "diods.h"

// Nasz akcelerometr to LIS35DE 
// w dokumentacji w opt/arm/docs pod LIS35DE na stronie 25 mamy opisane 
// do jakich rejestrow co musimy wpisac, zeby wlaczyc akcelerometr

// --> Czyli najpierw musimy zaimplementowac pisanie po I2C zeby wpisac te 
// rzeczy do rejestrów, a dopiero potem bedziemy mogli odczytywac dane z
// akcelerometru

#define LIS35DE_ADDR 0x1C
#define CTRL_REG1 0x20
#define PD_EN 0b01000111 // Power Down Enable, 7th bit = 64 = 0x40

#define OUT_X 0x29
#define OUT_Y 0x2B
#define OUT_Z 0x2D

#define ACCELEROMETER_REG_NBR_1 0x1C
#define ACCELEROMETER_REG_NBR_2 0x1D

void wait_timeout()
{
    volatile int i = 0;
    while(i < 15000)
    {
        i++;
        __NOP();
    }
}

void end_transmission()
{
    // Zainicjuj transmisję sygnału STOP
    I2C1->CR1 |= I2C_CR1_STOP;
}

bool wait_for_bit_set(uint32_t bit)
{
    // czekamy na ustawienie bitu bit, jesli bedzie ustawiony to SR1 na 
    // odpowiednim miejscu bedzie 1 czyli warunek ifa bedzie 1, dlatego dajemy !
    wait_timeout();
    if (!(I2C1->SR1 & bit))
    {
        // jesli nie uda sie wyslac sygnalu to wysylamy sygnal STOP
        // i konczymy
        end_transmission();
        return false;
    }
    return true;
}

bool init_start_transmission()
{
    // inicjalizacja transmisji sygnalu START, tego nie robimy w przerwaniu
    // to robimy przed i dopiero jak wyslemy to start to w przerwaniach reszte
    // rzeczy, bo dopiero wtedy one tak jakby dzialaja
    I2C1->CR1 |= I2C_CR1_START;

    // czekamy na ustawienie bitu SB - Start Bit
    if (!wait_for_bit_set(I2C_SR1_SB))
        return false;

    // Wysylamy 7-bitowy adresu slave'a (urzadzenia)
    // Moze to byc albo 0x1C albo 0x1D
    I2C1->DR = LIS35DE_ADDR << 1;

    // czekamy na ustawienie bitu ADDR - Address sent
    if (!wait_for_bit_set(I2C_SR1_ADDR))
        return false;

    // odczytujemy SR2, aby wyczyscic bit ADDR, jest to wymagane aby zakonczyc 
    // procedure adresowania i przejsc do nastepnego etapu transmisji
    I2C1->SR2;

    return true;
}

bool generate_repeated_start_recv(int reg, int8_t *res_val)
{
    // Wysylamy numer rejestru z ktorego chcemy odczytac dane
    I2C1->DR = reg;

    // czekamy na ustawienie bitu BTF - Byte Transfer Finished
    if (!wait_for_bit_set(I2C_SR1_BTF))
        return false;

    // inicjujemy transmisje sygnalu REPEATED START
    I2C1->CR1 |= I2C_CR1_START;

    // czekamy na ustawienie bitu SB - Start Bit
    if (!wait_for_bit_set(I2C_SR1_SB))
        return false;

    // Zainicjuj wysyłanie 7-bitowego adresu slave’a, tryb MR
    I2C1->DR = LIS35DE_ADDR << 1 | 1;

    // Ponieważ ma być odebrany tylko jeden bajt, ustaw wysłanie
    // sygnału NACK, zerując bit ACK
    I2C1->CR1 &= ~I2C_CR1_ACK;

    // czekamy na zakonczenie transmisji
    if (!wait_for_bit_set(I2C_SR1_ADDR))
        return false;

    // kasujemy bit ADDR
    I2C1->SR2;
    // Zainicjuj transmisję sygnału STOP, aby został wysłany po
    // odebraniu ostatniego (w tym przypadku jedynego) bajtu
    end_transmission();


    // Czekaj na ustawienie bitu RXNE (ang. receiver data register not empty)
    if (!wait_for_bit_set(I2C_SR1_RXNE))
        return false;

    *res_val = I2C1->DR;

    return true;
}

/**
 * Funkcja wysyła dane do akcelerometru, czyli co w danym rejestrze 
 * akcelerometru ma być zapisane, np w rejestrze CTRL_REG1 ma być wartość PD_EN,
 * która uruchamia akcelerometr i osie X, Y, Z
 */
bool send_data_to_accelerometer(uint8_t reg_addr, uint8_t reg_val)
{
    I2C1->DR = reg_addr;

    if (!wait_for_bit_set(I2C_SR1_TXE))
        return false;
    
    I2C1->DR = reg_val;

    if (!wait_for_bit_set(I2C_SR1_BTF))
        return false;
    
    return true;
}

bool I2C1_send_power_en()
{
    if (!init_start_transmission())
        return false;
    
    if (!send_data_to_accelerometer(CTRL_REG1, PD_EN))
        return false;
    
    end_transmission();
    
    return true;
}

bool I2C1_recv(int8_t *x_val, int8_t *y_val, int8_t *z_val)
{
    int8_t prev_x = *x_val;
    int8_t prev_y = *y_val;
    int8_t prev_z = *z_val;

    if (!init_start_transmission())
    {
        return false;
    }

    bool x_success = generate_repeated_start_recv(OUT_X, x_val);
    if (!x_success)
    {
        RedLEDon();
        Delay(3000);
        RedLEDoff();
        *x_val = prev_x;
        return false;
    }

    if (!init_start_transmission())
    {
        return false;
    }
    bool y_success = generate_repeated_start_recv(OUT_Y, y_val);
    if (!y_success)
    {
        BlueLEDon();
        Delay(3000);
        BlueLEDoff();

        *x_val = prev_x;
        *y_val = prev_y;
        return false;
    }

    if (!init_start_transmission())
    {
        return false;
    }
    bool z_success = generate_repeated_start_recv(OUT_Z, z_val);
    if (!z_success)
    {
        GreenLEDon();
        Delay(3000);
        GreenLEDoff();
        *x_val = prev_x;
        *y_val = prev_y;
        *z_val = prev_z;
        return false;
    }
    
    return true;
}
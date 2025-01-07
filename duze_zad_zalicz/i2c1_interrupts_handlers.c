#include "i2c1_interrupts_handlers.h"
#include "queue.h"

#define LIS35DE_ADDR 0x1C
#define CTRL_REG1 0x20
#define PD_EN 0b01000111 // Power Down Enable, 7th bit = 64 = 0x40

#define OUT_X 0x29
#define OUT_Y 0x2B
#define OUT_Z 0x2D

#define ACCELEROMETER_REG_NBR_1 0x1C
#define ACCELEROMETER_REG_NBR_2 0x1D

// TYPY BAJTOW, CZYLI KONTROLA KOLEJNOSCI/MOMENTU W KTORYM JESTESMY
// #define INIT_POWER_EN_RECV_SR1_SB 0
#define INIT_POWER_EN_SEND_SLAVE_ADDR 0

// #define INIT_POWER_EN_RECV_SR1_ADDR 1
#define INIT_POWER_EN_SEND_CTRL_REG_ADDR 1

// #define INIT_POWER_EN_RECV_TXE 2
#define INIT_POWER_EN_SEND_CTRL_REG_DATA 2

#define INIT_POWER_EN_END_TRANSMISSION 3

#define START_TRANSSMISION_SEND_SLAVE_ADDR 4

#define SEND_READ_REG_ADDR 5

#define SEND_REPEATED_START 6

#define SEND_SLAVE_ADDR_MR 7

#define END_SENDING 8

#define RECEIVE_DATA 9

// UWAGA - trzeba zakolejkowac jakie operacje mamy wykonywac, czyli jak robimy
// init power_en i potem robimy pierwsze odczyty, to zapisujemy te informacje 
// na kolejce, i jak np skonczymy incjacje INIT_POWER_EN to sprawdzamy kolejke
// zdejmujemy z niej operacje i sprawdzamy czy jesczcze jakas zostala, jesli tak
// to w przerwaniu inicjalizujemy kolejna operacje. 
// Analogicznie, trzeba bedzie obsluzyc i zainicjalizowac przerwania zewnetrzne 
// akcelerometru, w tym kolejka tez pomoze!!! 


QInfo q_info;

void init_interrupts_handlers_data()
{
    init_QInfo(&q_info, QUEUE_SIZE);
}

bool send_next_byte(uint32_t *byte_type, uint32_t read_reg_addr)
{
    switch (*byte_type)
    {
    case INIT_POWER_EN_SEND_SLAVE_ADDR:
        *byte_type += 1;
        I2C1->DR = LIS35DE_ADDR << 1;
        return true;
    case INIT_POWER_EN_SEND_CTRL_REG_ADDR:
        // odczytujemy SR2, aby wyczyscic bit ADDR, jest to wymagane aby
        // zakonczyc procedure adresowania i przejsc do nastepnego etapu
        // transmisji
        I2C1->SR2;
        *byte_type += 1;
        I2C1->DR = CTRL_REG1;
        return true;
    case INIT_POWER_EN_SEND_CTRL_REG_DATA:
        *byte_type += 1;
        I2C1->DR = PD_EN;
        return true;
    case INIT_POWER_EN_END_TRANSMISSION:
        *byte_type += 1;
        I2C1->CR1 |= I2C_CR1_STOP;
        // Nie mamy juz nic do wyslania wiec wylaczamy przerwania bo
        // ciagle by sie one generowaly bo nic nie dodalismy do DR
        I2C1->CR2 &= ~I2C_CR2_ITBUFEN;
        return true;
    case START_TRANSSMISION_SEND_SLAVE_ADDR:
        *byte_type += 1;
        I2C1->DR = LIS35DE_ADDR << 1;
        return true;
    case SEND_READ_REG_ADDR:
        *byte_type += 1;
        I2C1->SR2;
        I2C1->DR = read_reg_addr;
        return true;
    case SEND_REPEATED_START:
        *byte_type += 1;
        I2C1->CR1 |= I2C_CR1_START;
        // Musimy chwilowo wylaczyc przerwania bo nic nie wysylamy teraz, ale
        // jak wrocimy z obslugi przerwania to od razu to wlaczymy
        I2C1->CR2 &= ~I2C_CR2_ITBUFEN;
        return true;
    case SEND_SLAVE_ADDR_MR:
        *byte_type += 1;
        I2C1->DR = LIS35DE_ADDR << 1 | 1;
        // Ponieważ ma być odebrany tylko jeden bajt, ustaw wysłanie
        // sygnału NACK, zerując bit ACK
        I2C1->CR1 &= ~I2C_CR1_ACK;
        return true;
    case END_SENDING:
        *byte_type += 1;
        // kasujemy bit ADDR
        I2C1->SR2;
        // Zainicjuj transmisję sygnału STOP
        I2C1->CR1 |= I2C_CR1_STOP;
        // WYlaczamy przerwanie TxE bo nic nie wysylamy
        I2C1->CR2 &= ~I2C_CR2_ITBUFEN;
        return true;
    default:
        return false;
    }
}

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
    *ptr = '\n';
    ptr++;
    *ptr = '\r';
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

void add_xyz_to_q(int8_t read_val, uint32_t reg_type, QInfo *q_info)
{
    static char dec_str[10];
    dec_str[1] = ':';

    if (reg_type == 0)
        dec_str[0] = 'x';
    else if (reg_type == 1)
        dec_str[0] = 'y';
    else
        (reg_type == 2)
            dec_str[0] = 'z';

    int8_to_string(read_val, dec_str + 2);
    q_add_str(dec_str, q_info);
}

void I2C1_EV_IRQHandler()
{
    static uint32_t byte_type = INIT_POWER_EN_SEND_SLAVE_ADDR;
    static uint32_t read_reg_type = 0;
    uint32_t read_reg_addr;

    if (read_reg_type == 0)
        read_reg_addr = OUT_X;
    else if (read_reg_type == 1)
        read_reg_addr = OUT_Y;
    else if (read_reg_type == 2)
        read_reg_addr = OUT_Z;

    // Sprawdzamy, czy przerwanie TxE jest zgłoszone, jesli tak mozemy wyslac
    // kolejny bajt
    if (I2C1->SR1 & I2C_SR1_TXE)
    {
        send_next_byte(&byte_type, read_reg_addr);
    }

    if (I2C1->SR1 & I2C_SR1_RXNE)
    {
        int8_t received_byte = I2C1->DR;

        add_xyz_to_q(received_byte, read_reg_type, &q_info);

        read_reg_type = (read_reg_type + 1) % 3;
        byte_type = START_TRANSSMISION_SEND_SLAVE_ADDR;
    }
}
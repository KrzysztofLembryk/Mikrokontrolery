#include "i2c1_interrupts_handlers.h"
#include "queue.h"
#include "diods.h"
#include <delay.h>

#define LIS35DE_ADDR 0x1C
#define CTRL_REG1 0x20
#define PD_EN 0b01000111 // Power Down Enable, 7th bit = 64 = 0x40

#define OUT_X_REG 0x29
#define OUT_Y_REG 0x2B
#define OUT_Z_REG 0x2D

#define X_REG_TYPE 0
#define Y_REG_TYPE 1
#define Z_REG_TYPE 2

#define ACCELEROMETER_REG_NBR_1 0x1C
#define ACCELEROMETER_REG_NBR_2 0x1D

//------TYPY BAJTOW, CZYLI KONTROLA KOLEJNOSCI/MOMENTU W KTORYM JESTESMY------

// INICJACJA
#define INIT_POWER_EN_SEND_SLAVE_ADDR 0

#define INIT_POWER_EN_SEND_CTRL_REG_ADDR 1

#define INIT_POWER_EN_SEND_CTRL_REG_DATA 2

#define INIT_POWER_EN_END_TRANSMISSION 3

// REPEATED START, TRYB MR
#define START_TRANSSMISION_SEND_SLAVE_ADDR 4

#define SEND_READ_REG_ADDR 5

#define SEND_REPEATED_START 6

#define SEND_SLAVE_ADDR_MR 7

#define END_SENDING 8

#define RECEIVE_DATA 9

// KODY BŁĘDÓW
#define OK 0
#define SB_FLAG_NOT_SET 1
#define ADDR_FLAG_NOT_SET 2
#define TXE_FLAG_NOT_SET 3
#define BTF_FLAG_NOT_SET 4
#define OTHER_ERROR 5

// OPERACJE NA KOLEJCE
#define INIT_POWER_EN_OPERATION 'I'
#define REPEATED_START_OPERATION 'R'

// UWAGA - trzeba zakolejkowac jakie operacje mamy wykonywac, czyli jak robimy
// init power_en i potem robimy pierwsze odczyty, to zapisujemy te informacje 
// na kolejce, i jak np skonczymy incjacje INIT_POWER_EN to sprawdzamy kolejke
// zdejmujemy z niej operacje i sprawdzamy czy jesczcze jakas zostala, jesli tak
// to w przerwaniu inicjalizujemy kolejna operacje. 
// Analogicznie, trzeba bedzie obsluzyc i zainicjalizowac przerwania zewnetrzne 
// akcelerometru, w tym kolejka tez pomoze!!! 

QInfo op_queue;
QInfo data_queue;

void init_I2C1_interrupts_handlers_data()
{
    init_QInfo(&data_queue, QUEUE_SIZE);
    init_QInfo(&op_queue, QUEUE_SIZE);
    // W op_queue trzymamy operacje ktore maja sie wykonac, najpierw 
    // inicjalizacja, ale potem musimy jeszcze zainicjalizowac repeated start
    // zeby moc odbierac dane, wiec jak skonczy sie inicjalizacja, to w 
    // obsludze przerwania sprwadzamy te kolejke, widzimy, ze jest jeszcze cos 
    // na kolejce, wiec inicjalizujemy kolejna operacje.
    q_add(INIT_POWER_EN_OPERATION, &op_queue);
    q_add(REPEATED_START_OPERATION, &op_queue);
}

int handle_init_power_en(uint32_t *byte_type, uint32_t SR1)
{
    // Musimy czekac az kolejka nadawcza bedzie pusta, zeby czegos co chcielismy
    // wyslac nie nadpisac. Moze sie zdarzyc tak ze SB ustawiony ale TXE jeszcze
    // nie pusty czy cos takiego
    if (!(SR1 & I2C_SR1_TXE))
        return TXE_FLAG_NOT_SET;

    switch (*byte_type)
    {
    case INIT_POWER_EN_SEND_SLAVE_ADDR:
        if (!(SR1 & I2C_SR1_SB))
            return SB_FLAG_NOT_SET;

        *byte_type = INIT_POWER_EN_SEND_CTRL_REG_ADDR;
        I2C1->DR = LIS35DE_ADDR << 1;

        return OK;
    case INIT_POWER_EN_SEND_CTRL_REG_ADDR:
        if (!(SR1 & I2C_SR1_ADDR))
            return ADDR_FLAG_NOT_SET;

        *byte_type = INIT_POWER_EN_SEND_CTRL_REG_DATA;
        // odczytujemy SR2, aby wyczyscic bit ADDR, jest to wymagane aby
        // zakonczyc procedure adresowania i przejsc do nastepnego etapu
        // transmisji
        I2C1->SR2;
        I2C1->DR = CTRL_REG1;

        return OK;
    case INIT_POWER_EN_SEND_CTRL_REG_DATA:
        // Tutaj wystarczy nam że flaga TXE jest ustawiona 
        *byte_type = INIT_POWER_EN_END_TRANSMISSION;
        I2C1->DR = PD_EN;

        return OK;
    case INIT_POWER_EN_END_TRANSMISSION:
        if (!(SR1 & I2C_SR1_BTF))
            return BTF_FLAG_NOT_SET;
        *byte_type = START_TRANSSMISION_SEND_SLAVE_ADDR;

        // Nie mamy juz nic do wyslania bo koniec inicjacji, wiec wylaczamy 
        // przerwania bo ciagle by sie one generowaly bo nic nie dodalismy do DR
        I2C1->CR1 |= I2C_CR1_STOP;
        I2C1->CR2 &= ~I2C_CR2_ITBUFEN;

        // zdejmujemy operacje inicjalizacji z kolejki
        char op_type; 
        q_remove(&op_type, &op_queue);
        if (op_type != INIT_POWER_EN_OPERATION)
            return OTHER_ERROR;

        // Sprawdzamy czy jest jeszcze cos na kolejce, jesli tak to 
        // inicjalizujemy
        op_type = q_front(&op_queue);
        if (op_type != REPEATED_START_OPERATION)
            return OTHER_ERROR;

        // Inicjalizujemy transmisje sygnalu start zeby wlaczyc tryb MR
        I2C1->CR1 |= I2C_CR1_START;

        return OK;
    default:
        return OTHER_ERROR;
    }
}

int handle_MR_mode(uint32_t read_reg_addr, uint32_t *byte_type, uint32_t SR1)
{
    switch (*byte_type)
    {
    case START_TRANSSMISION_SEND_SLAVE_ADDR:
        if (!(SR1 & I2C_SR1_SB))
            return SB_FLAG_NOT_SET;
        
        *byte_type = SEND_READ_REG_ADDR;
        // wlaczamy przerwania TXE bo cos wysylamy
        I2C1->CR2 |= I2C_CR2_ITBUFEN;
        I2C1->DR = LIS35DE_ADDR << 1;

        return OK;
    case SEND_READ_REG_ADDR:
        // Najpierw sprawdzamy czy poprzedni bajt zostal wyslany i czy mamy 
        // pusta kolejke nadawcza
        if (!(SR1 & I2C_SR1_TXE))
            return TXE_FLAG_NOT_SET;
        // Sprawdzamy czy adres zostal wyslany, jesli tak to mozemy wyslac 
        // adres z ktorego chcemy odczytac dane
        if (!(SR1 & I2C_SR1_ADDR))
            return ADDR_FLAG_NOT_SET;

        *byte_type = SEND_REPEATED_START;
        I2C1->SR2;
        I2C1->DR = read_reg_addr;

        return OK;
    case SEND_REPEATED_START:
        if (!(SR1 & I2C_SR1_TXE))
            return TXE_FLAG_NOT_SET;
        if (!(SR1 & I2C_SR1_BTF))
            return BTF_FLAG_NOT_SET;

        *byte_type = SEND_SLAVE_ADDR_MR;
        I2C1->CR1 |= I2C_CR1_START;
        // Musimy chwilowo wylaczyc przerwania bo nic nie wysylamy teraz, ale
        // jak wrocimy z obslugi przerwania to od razu to wlaczymy
        I2C1->CR2 &= ~I2C_CR2_ITBUFEN;
        return OK;
    case SEND_SLAVE_ADDR_MR:
        if (!(SR1 & I2C_SR1_SB))
            return SB_FLAG_NOT_SET;

        // Wlaczamy przerwania TXE bo cos wysylamy
        I2C1->CR2 |= I2C_CR2_ITBUFEN;
        *byte_type = END_SENDING;
        I2C1->DR = LIS35DE_ADDR << 1 | 1;
        // Ponieważ ma być odebrany tylko jeden bajt, ustaw wysłanie
        // sygnału NACK, zerując bit ACK
        I2C1->CR1 &= ~I2C_CR1_ACK;
        return OK;
    case END_SENDING:
        if (!(SR1 & I2C_SR1_TXE))
            return TXE_FLAG_NOT_SET;
        if(!(SR1 & I2C1_SR1_ADDR))
            return ADDR_FLAG_NOT_SET;

        *byte_type = RECEIVE_DATA;
        // kasujemy bit ADDR
        I2C1->SR2;
        // Zainicjuj transmisję sygnału STOP
        I2C1->CR1 |= I2C_CR1_STOP;
        // WYlaczamy przerwanie TxE bo nic nie wysylamy
        I2C1->CR2 &= ~I2C_CR2_ITBUFEN;
        return OK;
    default:
        return OTHER_ERROR;
    }
}

void I2C1_EV_IRQHandler()
{
    static uint32_t byte_type = INIT_POWER_EN_SEND_SLAVE_ADDR;
    static uint32_t read_reg_type = X_REG_TYPE;
    uint32_t read_reg_addr;

    // Odczytujemy RAZ SR1 bo odczyt może zmienić bity i dwa odczyty pod rząd
    // mogą zwrócić różne wartości
    uint32_t SR1 = I2C1->SR1;

    char op_type = q_front(&op_queue);

    if (read_reg_type == X_REG_TYPE)
        read_reg_addr = OUT_X_REG;
    else if (read_reg_type == Y_REG_TYPE)
        read_reg_addr = OUT_Y_REG;
    else if (read_reg_type == Z_REG_TYPE)
        read_reg_addr = OUT_Z_REG;

    if (op_type == INIT_POWER_EN_OPERATION)
    {
        int ret_val = handle_init_power_en(&byte_type, SR1);

        if (ret_val != OK)
        {
            RedLEDon();
            delay(2000);
            RedLEDoff();
            BlueLEDon();
            delay(2000);
            BlueLEDoff();
        }
    }
    else if (op_type == REPEATED_START_OPERATION)
    {
        if (byte_type == RECEIVE_DATA)
        {
            if (SR1 & I2C_SR1_RXNE)
            {
                int8_t received_byte = I2C1->DR;

                q_add_xyz(received_byte, read_reg_type, &data_queue);

                read_reg_type = (read_reg_type + 1) % 3;

                // inicjujemy kolejna transmisje
                byte_type = START_TRANSSMISION_SEND_SLAVE_ADDR;
                I2C1->CR1 |= I2C_CR1_START;
            }
        }
        else 
        {
            int ret_val = handle_MR_mode(read_reg_addr, &byte_type, SR1);
            if (ret_val != OK)
            {
                RedLEDon();
                delay(2000);
                RedLEDoff();
                GreenLEDon();
                delay(2000);
                GreenLEDoff();
            }
        }
    }
}
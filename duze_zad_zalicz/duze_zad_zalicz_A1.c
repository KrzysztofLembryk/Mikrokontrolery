#include <stdlib.h>
#include <inttypes.h>
#include <gpio.h>
#include <delay.h>
#include <stm32.h>
#include <string.h>
#include "diods.h"
#include "init_funcs.h"
#include "queue.h"
#include "op_queue.h"
#include "buttons.h"
#include "button_handlers.h"
#include "lcd.h"
#include "lcd_handlers.h"

// KOMENDA WGRYWJACA PROGRAM NA PLYTKE
// /opt/arm/stm32/ocd/qfn4
// dokumentacja: /opt/arm/stm32/doc
// komenda do komunikacj z plytka to minicom

// na WSL2, minicom ok
// ale sudo /opt/arm/stm32/ocd/qfn4
#define ACC_GPIO GPIOA
#define ACC_INTRPT_PIN 1
#define IsAccIntrptHigh() \
    (ACC_GPIO->IDR & (1 << ACC_INTRPT_PIN))


#define ACCELEROMETER_READ_SPEED 55

#define LIS35DE_ADDR 0x1C
#define CTRL_REG1 0x20
#define CTRL_REG3 0x22
#define CTRL_REG1_ENABLE 0b01000011 // Zasilamy akcelerometr, i linie OX i OY
#define CTRL_REG3_ENABLE 0b00000100 // Chcemy przerwanie gdy Data Ready
#define CTRL_REG_DISABLE 0b00000000

#define OUT_X_REG 0x29
#define OUT_Y_REG 0x2B
#define OUT_Z_REG 0x2D

#define X_REG_TYPE 0
#define Y_REG_TYPE 1
#define Z_REG_TYPE 2

#define ACCELEROMETER_REG_NBR_1 0x1C
#define ACCELEROMETER_REG_NBR_2 0x1D

//------KONTROLA KOLEJNOSCI/MOMENTU W KTORYM JESTESMY------

// INICJACJA, TRYB MT
#define INIT_SEND_SLAVE_ADDR 0

#define INIT_SEND_CTRL_REG_ADDR 1

#define INIT_SEND_CTRL_REG_DATA 2

#define INIT_END_TRANSMISSION 3

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

#define INIT_SB_FLAG_NOT_SET 6
#define INIT_ADDR_FLAG_NOT_SET 7
#define INIT_TXE_FLAG_NOT_SET 8
#define INIT_BTF_FLAG_NOT_SET 9

// KOLEJKI

QInfo dma_queue;
OpQueue op_queue;
QInfo acc_queue;

// ------------------------------------------------------------------------- 
// ------------------------ DMA INTERRUPTS HANDLERS ------------------------
// -------------------------------------------------------------------------

void start_DMA_transmission()
{
    static char send_buffer[3 * MAX_STR_LEN];

    memset(send_buffer, 0, 3 * MAX_STR_LEN);

    // zdejmujemy jednoczesnie trzy stringi z kolejki, zeby w jednej paczce
    // wyslac x,y,z poprzez DMA 
    int rmvd_str1_len = 0;
    int rmvd_str2_len = 0;
    int rmvd_str3_len = 0;
    bool str1_rmvd = q_remove_str(send_buffer, &rmvd_str1_len, &dma_queue);

    if (str1_rmvd)
    {
        bool str2_rmvd = q_remove_str(send_buffer + rmvd_str1_len, 
                                    &rmvd_str2_len, 
                                    &dma_queue);
        if (str2_rmvd)
        {
            q_remove_str(send_buffer + rmvd_str1_len + rmvd_str2_len, 
                        &rmvd_str3_len, 
                        &dma_queue);
        }
    }
    if (str1_rmvd)
    {
        DMA1_Stream6->M0AR = (uint32_t)send_buffer;
        DMA1_Stream6->NDTR = rmvd_str1_len + rmvd_str2_len + rmvd_str3_len;
        DMA1_Stream6->CR |= DMA_SxCR_EN;
    }
}

bool DMA_is_not_busy()
{
    return (DMA1_Stream6->CR & DMA_SxCR_EN) == 0 
        && (DMA1->HISR & DMA_HISR_TCIF6) == 0;
}

void DMA1_Stream6_IRQHandler()
{
    /* Odczytaj zgłoszone przerwania DMA1. */
    // isr - interrupt status register
    uint32_t isr = DMA1->HISR;

    // Tutaj sprawdzamy czy TCIF6 (Transfer Complete Interrupt Flag Stream 6)
    // jest ustawiony
    if (isr & DMA_HISR_TCIF6)
    {
        // Jesli tak, to oznacza ze transfer zostal zakonczony
        // Wiec usstawiamy w HIFCR (High Interrupt Flag Clear Register) bit
        // CTCIF6 (Clear Transfer Complete Interrupt Flag Stream 6),
        // dzieki czemu konczymy przerwanie, i poprzez ustawienie bitu moze byc
        // ono ponownie wywolane
        DMA1->HIFCR = DMA_HIFCR_CTCIF6;
        // Jeśli jest coś do wysłania, startujemy kolejną transmisję.
        if (!q_is_empty(&dma_queue))
        {
            start_DMA_transmission();
        }
    }
}

void try_to_start_DMA_transmission()
{
    if (DMA_is_not_busy())
        start_DMA_transmission();
}

void power_diods(int ret_code)
{
    uint64_t delay_time = 400000;
    static int is_MR = false;
    if (is_MR)
        return;

    if (ret_code == SB_FLAG_NOT_SET)
    {
        RedLEDon();
        Delay(delay_time);
        RedLEDoff();
    }
    else if (ret_code == ADDR_FLAG_NOT_SET)
    {
        GreenLEDon();
        Delay(delay_time);
        GreenLEDoff();
    }
    else if (ret_code == TXE_FLAG_NOT_SET)
    {
        BlueLEDon();
        Delay(delay_time);
        BlueLEDoff();
    }
    else if (ret_code == BTF_FLAG_NOT_SET)
    {
        GreenLEDon();
        RedLEDon();
        Delay(delay_time);
        GreenLEDoff();
        RedLEDoff();
        is_MR = true;
    }
    else 
    {
        // ERROR w INIT, czyli kiedy tryb MT
        RedLEDon();
        BlueLEDon();
        GreenLEDon();
        Delay(delay_time);
        RedLEDoff();
        BlueLEDoff();
        GreenLEDoff();

        if (ret_code == INIT_SB_FLAG_NOT_SET)
        {
            RedLEDon();
            Delay(delay_time);
            RedLEDoff();
        }
        else if (ret_code == INIT_ADDR_FLAG_NOT_SET)
        {
            GreenLEDon();
            Delay(delay_time);
            GreenLEDoff();
        }
        else if (ret_code == INIT_TXE_FLAG_NOT_SET)
        {
            BlueLEDon();
            Delay(delay_time);
            BlueLEDoff();
        }
        else if (ret_code == INIT_BTF_FLAG_NOT_SET)
        {
            GreenLEDon();
            RedLEDon();
            Delay(delay_time);
            GreenLEDoff();
            RedLEDoff();
        }
    }
}

// -------------------------------------------------------------------------
// --------------------- EXTERNAL INTERRUPTS HANDLERS ----------------------
// -------------------------------------------------------------------------

// Akcelerometr

void EXTI1_IRQHandler(void)
{
    static bool prev_state = false;
    bool curr_state;
    if (EXTI->PR & EXTI_PR_PR1)
    {
        // zerujemy przyczyne przerwania
        EXTI->PR = EXTI_PR_PR1;

        curr_state = IsAccIntrptHigh();

        // symulujemy zbocze, czyli musimy sprawdzac jaki byl stan przed i po
        if (curr_state != prev_state)
        {
            prev_state = curr_state;
            if (curr_state)
            {
                // gdy curr_state to true to znaczy ze mamy nowy odczyt

                // Jesli kolejka jest pusta, to dodajemy do niej operacje a 
                // takze inicjujemy odczyt
                if (op_q_is_empty(&op_queue))
                {
                    op_q_add(REPEATED_START_OPERATION, 0, 0,
                    START_TRANSSMISION_SEND_SLAVE_ADDR, &op_queue);
                    I2C1->CR1 |= I2C_CR1_START;
                }
                else
                {
                    // Jesli kolejka nie jest pusta, to tylko dodajemy operacje 
                    // do kolejki, bo obsluga przerwania odczytujacego 
                    // zainicjuje kolejny odczyt
                    op_q_add(REPEATED_START_OPERATION, 0, 0, 
                    START_TRANSSMISION_SEND_SLAVE_ADDR, &op_queue);
                }
            }

        }
    }
}

// Joystick
void EXTI15_10_IRQHandler(void)
{
    // Sprawdzamy czy JOYSTICK Center wywołał przerwanie
    if (EXTI->PR & EXTI_PR_PR10)
    {
        EXTI->PR = EXTI_PR_PR10;

        handle_jstick(check_JstickCenterPressed(), &dma_queue);
    }
}

void EXTI3_IRQHandler(void)
{
    // Sprawdzamy czy JOYSTICK Left wywołał przerwanie
    if (EXTI->PR & EXTI_PR_PR3)
    {
        EXTI->PR = EXTI_PR_PR3;

        handle_jstick(check_JstickLeftPressed(), &dma_queue);
    }
}

void EXTI4_IRQHandler(void)
{
    // Sprawdzamy czy JOYSTICK Right wywołał przerwanie
    if (EXTI->PR & EXTI_PR_PR4)
    {
        EXTI->PR = EXTI_PR_PR4;

        handle_jstick(check_JstickRightPressed(), &dma_queue);
    }
}

void EXTI9_5_IRQHandler(void)
{
    // Sprawdzamy czy JOYSTICK Up wywołał przerwanie
    if (EXTI->PR & EXTI_PR_PR5)
    {
        EXTI->PR = EXTI_PR_PR5;

        handle_jstick(check_JstickUpPressed(), &dma_queue);
    }

    // Sprawdzamy czy JOYSTICK Down wywołał przerwanie
    if (EXTI->PR & EXTI_PR_PR6)
    {
        EXTI->PR = EXTI_PR_PR6;

        handle_jstick(check_JstickDownPressed(), &dma_queue);
    }
}


// -------------------------------------------------------------------------
// ------------------------ I2C INTERRUPT HANDLERS -------------------------
// -------------------------------------------------------------------------

int handle_MT_mode(uint32_t *comm_status, uint16_t SR1)
{
    OpQueueElem op_elem = op_q_front(&op_queue);

    switch (*comm_status)
    {
    case INIT_SEND_SLAVE_ADDR:
        if (!(SR1 & I2C_SR1_SB))
            return INIT_SB_FLAG_NOT_SET;

        *comm_status = INIT_SEND_CTRL_REG_ADDR;
        I2C1->DR = LIS35DE_ADDR << 1;

        return OK;
    case INIT_SEND_CTRL_REG_ADDR:
        if (!(SR1 & I2C_SR1_ADDR))
            return INIT_ADDR_FLAG_NOT_SET;

        *comm_status = INIT_SEND_CTRL_REG_DATA;
        I2C1->SR2;
        I2C1->DR = op_elem.reg_addr;

        return OK;
    case INIT_SEND_CTRL_REG_DATA:
        // Tutaj wystarczy nam że flaga TXE jest ustawiona 
        if (!(SR1 & I2C_SR1_TXE))
            return INIT_TXE_FLAG_NOT_SET;

        *comm_status = INIT_END_TRANSMISSION;
        I2C1->DR = op_elem.reg_val;

        return OK;
    case INIT_END_TRANSMISSION:
        if (!(SR1 & I2C_SR1_BTF))
            return INIT_BTF_FLAG_NOT_SET;

        op_q_remove(&op_queue);
        q_add_str("i_BTF_OK\r\n", &dma_queue); 
        try_to_start_DMA_transmission();
        // Nie mamy juz nic do wyslania bo koniec inicjacji, wiec wylaczamy 
        // przerwania bo ciagle by sie one generowaly bo nic nie dodalismy do DR
        I2C1->CR1 |= I2C_CR1_STOP;
        I2C1->CR2 &= ~I2C_CR2_ITBUFEN;

        // zdejmujemy operacje inicjalizacji z kolejki
        *comm_status = op_elem.comm_status;

        // inicjujemy kolejne transmisje 
        if (!op_q_is_empty(&op_queue))
        {
            // wlaczamy przerwania TXE bo znowu bedzie cos wysylane
            I2C1->CR2 |= I2C_CR2_ITBUFEN;
            I2C1->CR1 |= I2C_CR1_START;
        }

        return OK;
    default:
        return OTHER_ERROR;
    }
}

int handle_MR_mode(uint8_t read_reg_addr, uint32_t *comm_status, uint16_t SR1)
{
    switch (*comm_status)
    {
    case START_TRANSSMISION_SEND_SLAVE_ADDR:
        if (!(SR1 & I2C_SR1_SB))
            return SB_FLAG_NOT_SET;
        
        *comm_status = SEND_READ_REG_ADDR;
        // wlaczamy przerwania TXE bo cos wysylamy
        I2C1->CR2 |= I2C_CR2_ITBUFEN;
        I2C1->DR = LIS35DE_ADDR << 1;

        return OK;
    case SEND_READ_REG_ADDR:
        // Sprawdzamy czy adres zostal wyslany, jesli tak to mozemy wyslac 
        // adres z ktorego chcemy odczytac dane
        if (!(SR1 & I2C_SR1_ADDR))
            return ADDR_FLAG_NOT_SET;

        *comm_status = SEND_REPEATED_START;
        I2C1->SR2;
        I2C1->DR = read_reg_addr;

        return OK;
    case SEND_REPEATED_START:
        if (!(SR1 & I2C_SR1_BTF))
            return BTF_FLAG_NOT_SET;

        *comm_status = SEND_SLAVE_ADDR_MR;
        I2C1->CR1 |= I2C_CR1_START;
        // Musimy chwilowo wylaczyc przerwania bo nic nie wysylamy teraz, ale
        // jak wrocimy z obslugi przerwania to od razu to wlaczymy
        I2C1->CR2 &= ~I2C_CR2_ITBUFEN;
        return OK;
    case SEND_SLAVE_ADDR_MR:
        if (!(SR1 & I2C_SR1_SB))
            return SB_FLAG_NOT_SET;

        *comm_status = END_SENDING;
        // Wlaczamy przerwania TXE bo cos wysylamy
        I2C1->CR2 |= I2C_CR2_ITBUFEN;
        I2C1->DR = LIS35DE_ADDR << 1 | 1;
        // Ponieważ ma być odebrany tylko jeden bajt, ustaw wysłanie
        // sygnału NACK, zerując bit ACK
        I2C1->CR1 &= ~I2C_CR1_ACK;
        return OK;
    case END_SENDING:
        if(!(SR1 & I2C_SR1_ADDR))
            return ADDR_FLAG_NOT_SET;

        *comm_status = RECEIVE_DATA;
        // kasujemy bit ADDR
        I2C1->SR2;
        // Zainicjuj transmisję sygnału STOP
        I2C1->CR1 |= I2C_CR1_STOP;

        // Nie mozemy wylaczyc  przerwanie TxE, bo wylaczy sie tez RxNE 
        // I2C1->CR2 &= ~I2C_CR2_ITBUFEN;

        return OK;
    default:
        return OTHER_ERROR;
    }
}

void handle_MR_recv_data(uint8_t *read_reg_type, uint32_t *comm_status, uint16_t sr1)
{
    static uint32_t timer = 0;
    if (sr1 & I2C_SR1_RXNE)
    {
        int8_t received_byte = I2C1->DR;

        // timer jest potrzebny bo obslugujemy LCD w petli while, 
        // wiec mamy skonczona ilosc miejsca na kolejce a akcelerometr
        // wykonuje bardzo duzo pomiarow na sekunde wiec zeby miec 
        // responsywne LCD to dodajemy do naszej kolejki tylko raz 
        // na ustalona empirycznie predkosc czytania 
        if (timer % ACCELEROMETER_READ_SPEED == 0)
        {
            timer = 0;
            if (q_check_if_enough_space(3, &acc_queue))
            {
                q_add(received_byte, &acc_queue);

                q_add_xyz(received_byte, *read_reg_type, &dma_queue);
                try_to_start_DMA_transmission();
            }
            if (*read_reg_type == Y_REG_TYPE)
                timer++;
        }
        else 
            timer++;

        // wylaczamy przerwania TXE bo nic nie wysylamy juz
        I2C1->CR2 &= ~I2C_CR2_ITBUFEN;


        if (*read_reg_type == X_REG_TYPE)
        {
            // musimy odczytac oba rejestry X i Y zeby kolejne 
            // przerwanie moglo zostac wywolane
            op_q_add(REPEATED_START_OPERATION, 0, 0,        
                START_TRANSSMISION_SEND_SLAVE_ADDR, &op_queue);
        }

        *read_reg_type = (*read_reg_type + 1) % 2;

        *comm_status = START_TRANSSMISION_SEND_SLAVE_ADDR;

        op_q_remove(&op_queue);

        // inicjujemy kolejna transmisje jesli cos jest jeszcze na 
        // kolejce, a w.p.p. obsluga przerwania zewnetrznego zainicjuje
        // kolejna transmisje
        if (!op_q_is_empty(&op_queue))
        {
            I2C1->CR1 |= I2C_CR1_START;
        }
    }
}


void I2C1_EV_IRQHandler()
{
    static uint32_t comm_status = INIT_SEND_SLAVE_ADDR;
    static uint8_t read_reg_type = X_REG_TYPE;
    uint8_t read_reg_addr;

    // Odczytujemy RAZ SR1 bo odczyt może zmienić bity i dwa odczyty pod rząd
    // mogą zwrócić różne wartości
    uint16_t sr1 = I2C1->SR1;

    char op_type = op_q_front(&op_queue).op_type;

    if (read_reg_type == X_REG_TYPE)
        read_reg_addr = OUT_X_REG;
    else 
        read_reg_addr = OUT_Y_REG;

    if (op_type == INIT_OPERATION)
    {
        int ret_val = handle_MT_mode(&comm_status, sr1);
    }
    else if (op_type == REPEATED_START_OPERATION)
    {
        if (comm_status == RECEIVE_DATA)
        {
            handle_MR_recv_data(&read_reg_type, &comm_status, sr1);
        }
        else 
        {
            int ret_val = handle_MR_mode(read_reg_addr, &comm_status, sr1);
        }
    }
}

void init_queues()
{
    init_QInfo(&dma_queue, QUEUE_SIZE);
    init_QInfo(&acc_queue, QUEUE_SIZE);

    // W op_queue trzymamy operacje ktore maja sie wykonac, 
    // w obsludze przerwania sprwadzamy kolejke, widzimy ze jest jeszcze cos 
    // na kolejce, wiec inicjalizujemy kolejna operacje.
    init_OpQueue(&op_queue);
}

void reset_accelerometer()
{
    op_q_add(INIT_OPERATION, CTRL_REG1, CTRL_REG_DISABLE, 
    INIT_SEND_SLAVE_ADDR, &op_queue);
    op_q_add(INIT_OPERATION, CTRL_REG3, CTRL_REG_DISABLE, 
    INIT_SEND_SLAVE_ADDR, &op_queue);
    I2C1->CR1 |= I2C_CR1_START;
}

void init_accelerometer_transmission()
{
    op_q_add(INIT_OPERATION, CTRL_REG3, CTRL_REG3_ENABLE, 
    INIT_SEND_SLAVE_ADDR, &op_queue);
    op_q_add(INIT_OPERATION, CTRL_REG1, CTRL_REG1_ENABLE, 
     START_TRANSSMISION_SEND_SLAVE_ADDR, &op_queue);
    I2C1->CR1 |= I2C_CR1_START;
}

void handle_drawing_new_lcd_pos()
{
    static int what_to_read = X_REG_TYPE;
    static bool first_read = true;
    static int8_t first_x_val = 0;
    static int8_t first_y_val = 0;
    static int8_t curr_x_val = 0;
    static int8_t curr_y_val = 0;

    if (!q_is_empty(&acc_queue))
    {
        if (first_read)
        {
            if (what_to_read == X_REG_TYPE)
            {
                char x;

                q_remove(&x, &acc_queue);

                first_x_val = abs((int8_t)x);

                if (first_x_val >= 3)
                    first_x_val = 2;

                what_to_read = Y_REG_TYPE;
            }
            else 
            {
                char y;

                q_remove(&y, &acc_queue);

                first_y_val = abs((int8_t)y);
                if (first_y_val >= 3)
                    first_y_val = 2;
                what_to_read = X_REG_TYPE;
                first_read = false;
            }
        }
        else 
        {
            if (what_to_read == X_REG_TYPE)
            {
                char x;

                q_remove(&x, &acc_queue);

                curr_x_val = (int8_t)x;
                what_to_read = Y_REG_TYPE;
            }
            else 
            {
                char y;

                q_remove(&y, &acc_queue);

                curr_y_val = (int8_t)y;
                what_to_read = X_REG_TYPE;

                // if (curr_x_val < 0)
                //     curr_x_val += first_x_val;
                // else
                //     curr_x_val -= first_x_val;
                
                // if (curr_y_val < 0)
                //     curr_y_val += first_y_val;
                // else
                //     curr_y_val -= first_y_val;

                // if (abs(curr_x_val) <= 2)
                //     curr_x_val = 0;

                // if (abs(curr_y_val) <= 2)
                //     curr_y_val = 0;                
                curr_y_val /= 2;
                curr_x_val /= 2;

                calc_new_lcd_pos(curr_x_val, curr_y_val, &dma_queue);
            }
        }
    }
}

int main()
{
    init_rcc();
    LCDconfigure();
    init_diods();
    init_usart2_TXD_RXD_lines();
    init_usart2_cr_registers();
    init_dma_cr_registers();
    init_dma_interrupts();
    init_I2C1();
    init_queues();
    reset_accelerometer();
    init_external_interrupts();

    // Petla ustawiania startowego znacznika poprzez uzywanie joysticka
    LCDclear(true);
    LCDputchar('+');
    while (1)
    {
        if (!q_is_empty(&dma_queue))
        {
            char fire_pressed = q_front(&dma_queue);
            try_to_start_DMA_transmission();
            if (fire_pressed == 'F')
                break;
        }
    }

    init_accelerometer_interrupts();
    init_accelerometer_transmission();

    // Petla odbierania danych z akcelerometru i przesylania ich do LCD
    while(1) 
    {
        handle_drawing_new_lcd_pos();
    }
}
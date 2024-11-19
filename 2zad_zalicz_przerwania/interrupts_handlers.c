#include "interrupts_handlers.h"
#include <string.h>
#include "button_handlers.h"
#include "buttons.h"

QInfo q_info;

void init_interrupts_handlers_data()
{
    init_QInfo(&q_info, QUEUE_SIZE);
}

void start_DMA_transmission()
{
    static char send_buffer[MAX_STR_LEN];

    memset(send_buffer, 0, MAX_STR_LEN);
    
    int rmvd_str_len = 0;

    if (q_remove_str(send_buffer, &rmvd_str_len, &q_info))
    {
        DMA1_Stream6->M0AR = (uint32_t)send_buffer;
        DMA1_Stream6->NDTR = rmvd_str_len;
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
        if (!q_is_empty(&q_info))
        {
            start_DMA_transmission();
        }
    }
}

void EXTI15_10_IRQHandler(void)
{
    // Sprawdzamy czy USER_BTN wywołał przerwanie
    if (EXTI->PR & EXTI_PR_PR13)
    {
        // Clear the interrupt pending bit for EXTI Line 13
        EXTI->PR = EXTI_PR_PR13;

        // Sprzet wyzeruje to przerwanie ktore ktore zostanie zapisane do EXTI->PR 
        // w EXTI_PR_PR13 mamy 1 na 13 miejscu wiec 13 przerwanie zostanie wyzerowane
        // ECTI->PR = EXTI_PR_PR13 vs ECTI->PR |= EXTI_PR_PR13
        // jak zrobilibysmy |= to moglibysmy wyzerowac wiecej przerwan

        handle_user(UserBtnPressed(), &q_info);
    }

    // Sprawdzamy czy JOYSTICK Center wywołał przerwanie
    if (EXTI->PR & EXTI_PR_PR10)
    {
        EXTI->PR = EXTI_PR_PR10;

        handle_jstick(check_JstickCenterPressed(), &q_info);
    }

    if (DMA_is_not_busy())
    {
        // Jesli nie ma transmisji w toku, czyli DMA nie jest zajete 
        // to mozemy zaczac nowa transmisje

        start_DMA_transmission();
    }
}

void EXTI0_IRQHandler(void)
{
    // Sprawdzamy czy AT_BTN wywołał przerwanie
    if (EXTI->PR & EXTI_PR_PR0)
    {
        EXTI->PR = EXTI_PR_PR0;

        handle_at(ATBtnPressed(), &q_info);
    }

    if (DMA_is_not_busy())
    {
        start_DMA_transmission();
    }
}

void EXTI3_IRQHandler(void)
{
    // Sprawdzamy czy JOYSTICK Left wywołał przerwanie
    if (EXTI->PR & EXTI_PR_PR3)
    {
        EXTI->PR = EXTI_PR_PR3;

        handle_jstick(check_JstickLeftPressed(), &q_info);
    }

    if (DMA_is_not_busy())
    {
        start_DMA_transmission();
    }
}

void EXTI4_IRQHandler(void)
{
    // Sprawdzamy czy JOYSTICK Right wywołał przerwanie
    if (EXTI->PR & EXTI_PR_PR4)
    {
        EXTI->PR = EXTI_PR_PR4;

        handle_jstick(check_JstickRightPressed(), &q_info);
    }

    if (DMA_is_not_busy())
    {
        start_DMA_transmission();
    }
}

void EXTI9_5_IRQHandler(void)
{
    // Sprawdzamy czy JOYSTICK Up wywołał przerwanie
    if (EXTI->PR & EXTI_PR_PR5)
    {
        EXTI->PR = EXTI_PR_PR5;

        handle_jstick(check_JstickUpPressed(), &q_info);
    }

    // Sprawdzamy czy JOYSTICK Down wywołał przerwanie
    if (EXTI->PR & EXTI_PR_PR6)
    {
        EXTI->PR = EXTI_PR_PR6;

        handle_jstick(check_JstickDownPressed(), &q_info);
    }

    if (DMA_is_not_busy())
    {
        start_DMA_transmission();
    }
}
#ifndef REGISTER_CONSTS_H
#define REGISTER_CONSTS_H

// CR - Control Register dla USART
// -----------REJESTR CR1-----------
// Uzywany do podstawowej konfiguracji parametrow USART

// Tryb pracy
#define USART_Mode_Rx_Tx (USART_CR1_RE | USART_CR1_TE)
#define USART_Enable USART_CR1_UE

// Przesylane slowo
#define USART_WordLength_8b 0x0000
#define USART_WordLength_9b USART_CR1_M

// Bit parzystosci
#define USART_Parity_No 0x0000
#define USART_Parity_Even USART_CR1_PCE
#define USART_Parity_Odd (USART_CR1_PCE | USART_CR1_PS)
// -----------REJESTR CR1-----------

// -----------REJESTR CR2-----------
// Uzywany do bardziej zaawansowanej konfiguracji parametrow USART,
// takich jak bity stopu czy ustawienia zegara

// Bity stopu
#define USART_StopBits_1 0x0000
#define USART_StopBits_0_5 0x1000
#define USART_StopBits_2 0x2000
#define USART_StopBits_1_5 0x3000
// -----------REJESTR CR2-----------

// -----------REJESTR CR3-----------
// Uzywany do dodatkowej konfiguracji parametrow USART,
// takich jak hardware flow control, DMA(Direct Memory Access)

// Sterowanie przeplywem
#define USART_FlowControl_None 0x0000
#define USART_FlowControl_RTS USART_CR3_RTSE
#define USART_FlowControl_CTS USART_CR3_CTSE

// DMA
#define USART_DMA_Tx_En USART_CR3_DMAT
#define USART_DMA_Rx_En USART_CR3_DMAR
// -----------REJESTR CR3-----------

// BRR - Baud Rate Register
// -----------REJESTR BRR-----------
// Uzywany do konfiguracji predkosci transmisji danych dla USART

#define HSI_HZ 16000000U
#define PCLK1_HZ HSI_HZ
#define BAUD 9600U
// -----------REJESTR BRR-----------

#endif // REGISTER_CONSTS_H
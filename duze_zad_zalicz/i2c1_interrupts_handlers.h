#ifndef I2C1_INTERRUPTS_HANDLERS_H
#define I2C1_INTERRUPTS_HANDLERS_H

#include <delay.h>
#include <gpio.h>
#include <stm32.h>
#include "queue.h"

void init_accelerometer_transmission();

void init_queues();

void I2C1_EV_IRQHandler();

extern QInfo data_queue;

#endif // I2C1_INTERRUPTS_HANDLERS_H
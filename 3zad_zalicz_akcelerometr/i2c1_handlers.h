#ifndef I2C1_HANDLERS_H
#define I2C1_HANDLERS_H

#include <stdbool.h>

bool I2C1_send(uint8_t reg_addr, uint8_t reg_val);

bool I2C1_recv(int *x_val, int *y_val, int *z_val);

#endif // I2C1_HANDLERS_H
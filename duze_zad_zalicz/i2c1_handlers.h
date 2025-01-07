#ifndef I2C1_HANDLERS_H
#define I2C1_HANDLERS_H

#include <stdbool.h>
#include <inttypes.h>

bool I2C1_send_power_en();

bool I2C1_recv(int8_t *x_val, int8_t *y_val, int8_t *z_val);

#endif // I2C1_HANDLERS_H
#ifndef I2C1_HANDLERS_H
#define I2C1_HANDLERS_H

bool handle_I2C1_send(int reg_addr, int reg_val);

bool handle_I2C1_recv(int *x_val, int *y_val, int *z_val);

#endif // I2C1_HANDLERS_H
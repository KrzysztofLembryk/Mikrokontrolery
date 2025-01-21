#ifndef OP_QUEUE_H
#define OP_QUEUE_H

#include <inttypes.h>
#include <stdbool.h>

#define OP_QUEUE_SIZE 100
#define INIT_OPERATION 'I'
#define REPEATED_START_OPERATION 'R'
#define NO_OPERATION '\0'

typedef struct 
{
    char op_type;
    uint8_t reg_addr;
    uint8_t reg_val;
    uint8_t comm_status;
    bool active;

} OpQueueElem;

typedef struct 
{
    int front;
    int end;
    int q_size;
    int nbr_of_elements; 
    OpQueueElem queue[OP_QUEUE_SIZE];
} OpQueue;


void init_OpQueue(OpQueue *op_q);

bool op_q_is_empty(OpQueue *op_q);

bool op_q_add(char op_type, uint8_t reg_addr, uint8_t reg_val, 
uint8_t comm_status, OpQueue *op_q);

OpQueueElem op_q_remove(OpQueue *op_q);

OpQueueElem op_q_front(OpQueue *op_q);

void op_q_activate_front(OpQueue *op_q);

#endif // OP_QUEUE_H
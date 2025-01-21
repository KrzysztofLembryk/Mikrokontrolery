#include "op_queue.h"


void init_OpQueue(OpQueue *op_q)
{
    op_q->front = 0;
    op_q->end = 0;
    op_q->q_size = OP_QUEUE_SIZE;
    op_q->nbr_of_elements = 0;
}

bool op_q_is_empty(OpQueue *op_q)
{
    if (op_q->nbr_of_elements == 0)
        return true;
    return false;
}

bool op_q_add(char op_type, uint8_t reg_addr, uint8_t reg_val, 
uint8_t comm_status, OpQueue *op_q)
{
    if (op_q->nbr_of_elements >= op_q->q_size)
        return false;

    OpQueueElem elem;
    elem.op_type = op_type;
    elem.reg_addr = reg_addr;
    elem.reg_val = reg_val;
    elem.comm_status = comm_status;
    elem.active = false;

    op_q->queue[op_q->end] = elem;
    op_q->end = (op_q->end + 1) % op_q->q_size;
    op_q->nbr_of_elements++;

    return true;
}

OpQueueElem op_q_remove(OpQueue *op_q)
{
    OpQueueElem elem;
    if (op_q->nbr_of_elements <= 0)
    {
        elem.op_type = NO_OPERATION;
        elem.reg_addr = 0;
        elem.reg_val = 0;
        elem.comm_status = 10;
    }
    else 
    {
        elem = op_q->queue[op_q->front];
        op_q->front = (op_q->front + 1) % op_q->q_size;
        op_q->nbr_of_elements--;
    }

    return elem;
}

OpQueueElem op_q_front(OpQueue *op_q)
{
    if (op_q_is_empty(op_q))
    {
        OpQueueElem elem;
        elem.op_type = NO_OPERATION;
        elem.reg_addr = 0;
        elem.reg_val = 0;
        elem.comm_status = 10;
        return elem;
    }
    return op_q->queue[op_q->front];
}

void op_q_activate_front(OpQueue *op_q)
{
    if (op_q_is_empty(op_q))
        return;

    op_q->queue[op_q->front].active = true;
}
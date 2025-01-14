#ifndef QUEUE_H
#define QUEUE_H

#include <inttypes.h>
#include "other_consts.h"
#include <stdbool.h>

typedef struct 
{
    int front;
    int end;
    int q_size;
    int nbr_of_elements; 
    char queue[QUEUE_SIZE];
} QInfo;

void init_QInfo(QInfo *q_info, int q_size);

bool q_is_empty(QInfo *q_info);

bool q_add(char c, QInfo *q_info);

bool q_remove(char *c, QInfo *q_info);

bool q_check_if_enough_space(int str_len, QInfo *q_info);

int get_str_len(char *str);

bool q_add_str(char *str, QInfo *q_info);

bool q_remove_str(char *rmvd_str, int *rmvd_str_len, QInfo *q_info);

void q_add_xyz(int8_t read_val, uint32_t reg_type, QInfo *q_info);

char q_front(QInfo *q_info);

#endif // QUEUE_H
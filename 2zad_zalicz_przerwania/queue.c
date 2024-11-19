#include "queue.h"

void init_QInfo(QInfo *q_info, int q_size)
{
    q_info->front = 0;
    q_info->end = 0;
    q_info->q_size = q_size;
    q_info->nbr_of_elements = 0;
    for (int i = 0; i < QUEUE_SIZE; i++)
        q_info->queue[i] = 0;
}

bool q_is_empty(QInfo *q_info)
{
    if (q_info->nbr_of_elements == 0)
        return true;
    return false;
}

bool q_add(char c,
           QInfo *q_info)
{
    if (q_info->nbr_of_elements >= q_info->q_size)
        return false;

    q_info->queue[q_info->end] = c;
    q_info->end = (q_info->end + 1) % q_info->q_size;
    q_info->nbr_of_elements++;

    return true;
}

bool q_remove(char *c,
              QInfo *q_info)
{
    if (q_info->nbr_of_elements <= 0)
        return false;

    *c = q_info->queue[q_info->front];
    q_info->front = (q_info->front + 1) % q_info->q_size;
    q_info->nbr_of_elements--;

    return true;
}

bool q_check_if_enough_space(int str_len,
                             QInfo *q_info)
{
    if (q_info->q_size - q_info->nbr_of_elements < str_len)
        return false;
    return true;
}

int get_str_len(char *str)
{
    // Zakladamy ze kazda wiadomosc konczy sie znakiem nowej linii '\n
    int str_len = 0;
    while (*str != '\n')
    {
        str_len++;
        str++;
        if (str_len > MAX_STR_LEN)
            return 0;
    }
    str_len++;
    return str_len;
}

bool q_add_str(char *str,
               QInfo *q_info)
{
    int str_len = get_str_len(str);
    int i = 0;

    // jesli str_len == 0 to znaczy ze nie ma znaku nowej linii w stringu
    // bo wszystkie wysylane komendy maja <= 15 znakow
    if (!q_check_if_enough_space(str_len, q_info) || str_len == 0)
        return false;

    while (*str != '\n' && i < str_len)
    {
        if (!q_add(*str, q_info))
            return false;
        str++;
        i++;
    }

    if (!q_add(*str, q_info))
        return false;

    return true;
}

bool q_remove_str(char *rmvd_str, int *rmvd_str_len, QInfo *q_info)
{
    if (q_is_empty(q_info))
        return false;

    int len = 0;
    char temp;

    do 
    {
        if (!q_remove(&temp, q_info))
            return false;
        rmvd_str[len] = temp;
        len++;

    } while (temp != '\n');

    *rmvd_str_len = len;

    return true;
}
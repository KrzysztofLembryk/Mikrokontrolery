#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>

#define MAX_STR_LEN 15
#define MESSEGE "MESSEGE\n"


typedef struct QInfo
{
    uint8_t front;
    uint8_t end;
    uint8_t q_size;
    uint8_t nbr_of_elements; 
} QInfo;

void init_QInfo(QInfo *q_info, uint8_t q_size)
{
    q_info->front = 0;
    q_info->end = 0;
    q_info->q_size = q_size;
    q_info->nbr_of_elements = 0;
}

bool q_add(char c,
           char queue[],
           QInfo *q_info)
{
    if (q_info->nbr_of_elements >= q_info->q_size)
        return false;

    queue[q_info->end] = c;
    q_info->end = (q_info->end + 1) % q_info->q_size;
    q_info->nbr_of_elements++;

    return true;
}

bool q_remove(char queue[],
              char *c,
              QInfo *q_info)
{
    if (q_info->nbr_of_elements <= 0)
        return false;

    *c = queue[q_info->front];
    q_info->front = (q_info->front + 1) % q_info->q_size;
    q_info->nbr_of_elements--;

    return true;
}

bool q_check_if_enough_space(uint8_t str_len,
                             QInfo *q_info)
{
    if (q_info->q_size - q_info->nbr_of_elements < str_len)
        return false;
    return true;
}

uint8_t get_str_len(char *str)
{
    // Zakladamy ze kazda wiadomosc konczy sie znakiem nowej linii '\n
    uint8_t str_len = 0;
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
               char queue[],
               QInfo *q_info)
{
    uint8_t str_len = get_str_len(str);
    uint8_t i = 0;

    // jesli str_len == 0 to znaczy ze nie ma znaku nowej linii w stringu
    // bo wszystkie wysylane komendy maja <= 15 znakow
    if (!q_check_if_enough_space(str_len, q_info) || str_len == 0)
        return false;

    while (*str != '\n' && i < str_len)
    {
        if (!q_add(*str, queue, q_info))
            return false;
        str++;
        i++;
    }

    if (!q_add(*str, queue, q_info))
        return false;

    return true;
}

#define Q_SIZE 100
#define MSG2 "THIS MSG\n"

int main()
{
    QInfo q_info;
    init_QInfo(&q_info, Q_SIZE);
    char queue[Q_SIZE];
    char str[] = "Hello\n";
    q_add_str(MESSEGE, queue, &q_info);
    q_add_str(MESSEGE, queue, &q_info);
    q_add_str(MSG2, queue, &q_info);
    q_add_str(MESSEGE, queue, &q_info);
    q_add_str(MSG2, queue, &q_info);
    char c;
    while (q_remove(queue, &c, &q_info))
    {
        printf("%c", c);
    }
    return 0;
}
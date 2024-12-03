#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>

void int8_to_string(int8_t value, char *str)
{
    char *ptr = str;
    bool is_negative = false;

    // Handle negative values
    if (value < 0)
    {
        is_negative = true;
        value = -value;
    }

    // Null-terminate the string
    *ptr = '\r';
    ptr++;
    *ptr = '\n';
    ptr++;

    // Convert the integer to a string
    do
    {
        *ptr = '0' + (value % 10);
        ptr++;
        value /= 10;
    } while (value > 0);

    if (is_negative)
    {
        *ptr = '-';
        ptr++;
    }


    // Reverse the string
    for (char *start = str, *end = ptr - 1; start < end; ++start, --end)
    {
        char temp = *start;
        *start = *end;
        *end = temp;
    }
}

int main()
{
    int8_t x;
    scanf("%" SCNd8, &x);
    char str[10];
    int8_to_string(x, str);
    for (int i = 0; i < 10; i++)
    {
        printf("%c", str[i]);
        if (str[i] == '\n')
        {
            break;
        }
    }
    return 0;
}
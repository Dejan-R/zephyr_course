/*
 * C source files (.c) contain implementations of functions and application code.
 * Header files (.h) contain declarations shared between source files.
 */
#include <zephyr/sys/printk.h>
#include "my_function.h"

int main(void)
{
    int a = 4;
    int b = 5;
    int result = my_function(a, b);
    
    printk("Sum: %d + %d = %d\n", a, b, result);
    return 0;
}

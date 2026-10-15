#include <zephyr/sys/printk.h>

/* Read two values through pointers and return their sum. */
static int add_using_pointers(int *a_ptr, int *b_ptr)
{
    return *a_ptr + *b_ptr;
}

/* Modify the original variables through pointers. */
static void swap_values(int *a_ptr, int *b_ptr)
{
    int temp = *a_ptr;

    *a_ptr = *b_ptr;
    *b_ptr = temp;
}

int main(void)
{
    int a = 3;
    int b = 5;

    /* Pass variable addresses to the function. */
    int result = add_using_pointers(&a, &b);
    printk("Sum: %d + %d = %d\n", a, b, result);

    /* Call the same function through a function pointer. */
    int (*operation)(int *, int *) = add_using_pointers;
    result = operation(&a, &b);
    printk("Sum through function pointer: %d\n", result);

    /* Modify the original variables. */
    swap_values(&a, &b);
    printk("After swap: a = %d, b = %d\n", a, b);

    return 0;
}
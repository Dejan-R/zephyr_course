#include <zephyr/sys/printk.h>

int main(void)
{
    int a = 5;
    int *a_ptr = &a;

    printk("Pointers\n");

    /* Print the variable's value and memory address. */
    printk("Value of a: %d\n", a);
    printk("Address of a: %p\n", (void *)&a);

    /* Dereference the pointer to read the value at that address. */
    printk("Value through pointer: %d\n", *a_ptr);

    /* The pointer stores an address and has its own address. */
    printk("Address stored in a_ptr: %p\n", (void *)a_ptr);
    printk("Address of a_ptr: %p\n", (void *)&a_ptr);

    /* Modify the original variable through the pointer. */
    *a_ptr = 10;
    printk("Updated value of a: %d\n", a);

    return 0;
}

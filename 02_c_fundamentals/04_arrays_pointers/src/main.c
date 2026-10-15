#include <zephyr/sys/printk.h>

int main(void)
{
    int data[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

    /* Point to the first element of the array. */
    int *p = data;

    printk("Arrays and Pointers\n");

    for (int i = 0; i < 10; i++) {
        /* Array indexing and pointer arithmetic access the same element. */
        printk("data[%d] = %d, *(p + %d) = %d\n",
               i, data[i], i, *(p + i));
    }

    return 0;
}

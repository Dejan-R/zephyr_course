#include <zephyr/sys/printk.h>
#include "shared_data.h"

int main(void)
{
    printk("Global and Extern\n");
    printk("Initial shared value: %d\n", shared_value);

    /* Modify the same variable from another source file. */
    increment_shared_value();

    printk("Updated shared value: %d\n", shared_value);

    return 0;
}

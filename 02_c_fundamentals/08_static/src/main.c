#include <zephyr/sys/printk.h>
#include "counter.h"

int main(void)
{
    printk("Static Variables\n");

    /* Call the same function three times to compare variable lifetimes. */
    demonstrate_counters();
    demonstrate_counters();
    demonstrate_counters();

    return 0;
}

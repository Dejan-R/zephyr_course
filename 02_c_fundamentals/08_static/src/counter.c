#include <zephyr/sys/printk.h>
#include "counter.h"

/* File-scope static variable: accessible only within this source file. */
static int file_counter = 0;

void demonstrate_counters(void)
{
    /* Automatic variable: initialized on every function call. */
    int automatic_counter = 0;

    /* Local static variable: initialized once and retains its value. */
    static int local_static_counter = 0;

    automatic_counter++;
    local_static_counter++;
    file_counter++;

    printk("Auto: %d, Local static: %d, File static: %d\n",
           automatic_counter, local_static_counter, file_counter);
}


#include <zephyr/sys/printk.h>

#ifndef APP_VERSION
#define APP_VERSION "undefined"
#endif

int main(void)
{
    printk("Zephyr Build System\n");
    printk("Application version: %s\n", APP_VERSION);
    printk("Build completed successfully!\n");

    return 0;
}

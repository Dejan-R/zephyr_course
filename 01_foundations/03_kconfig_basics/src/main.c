#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

int main(void)
{
    printk("Kconfig Basics\n");

#if defined(CONFIG_COURSE_FEATURE)
    printk("Feature enabled: yes\n");

    for (int i = 1; i <= CONFIG_COURSE_REPEAT_COUNT; i++) {
        printk("Message: %d\n", i);
    }
#else
    printk("Feature enabled: no\n");
#endif

    return 0;
}

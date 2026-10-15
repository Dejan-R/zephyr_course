#include <zephyr/init.h>
#include <zephyr/sys/printk.h>

#define EVENT_COUNT 5

static unsigned int init_order[EVENT_COUNT];
static unsigned int init_count;

static void record_init(unsigned int event)
{
    if (init_count < EVENT_COUNT) {
        init_order[init_count++] = event;
    }
}

static int init_pre_kernel_1(void)
{
    record_init(0);
    return 0;
}

static int init_pre_kernel_2(void)
{
    record_init(1);
    return 0;
}

static int init_post_kernel_first(void)
{
    record_init(2);
    return 0;
}

static int init_post_kernel_second(void)
{
    record_init(3);
    return 0;
}

static int init_application(void)
{
    record_init(4);
    return 0;
}

SYS_INIT(init_pre_kernel_1, PRE_KERNEL_1, 80);
SYS_INIT(init_pre_kernel_2, PRE_KERNEL_2, 80);
SYS_INIT(init_post_kernel_first, POST_KERNEL, 80);
SYS_INIT(init_post_kernel_second, POST_KERNEL, 90);
SYS_INIT(init_application, APPLICATION, 80);

int main(void)
{
    static const char *const event_names[EVENT_COUNT] = {
        "PRE_KERNEL_1 (priority 80)",
        "PRE_KERNEL_2 (priority 80)",
        "POST_KERNEL (priority 80)",
        "POST_KERNEL (priority 90)",
        "APPLICATION (priority 80)",
    };

    printk("Initialization Levels\n");

    for (unsigned int i = 0; i < init_count; ++i) {
        printk("%u: %s\n", i + 1U, event_names[init_order[i]]);
    }

    printk("main() started\n");
    return 0;
}

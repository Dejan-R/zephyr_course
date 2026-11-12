#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#define STACK_SIZE 2048
#define THREAD_PRIORITY 7

#define MSGQ_CAPACITY 10
#define PRODUCER_INTERVAL_MS 1000

/* Message queue: 10 integers, 4-byte alignment */
K_MSGQ_DEFINE(my_msgq, sizeof(int), MSGQ_CAPACITY, 4);

/* Producer thread - sends numbers */
static void producer_thread(void *arg1, void *arg2, void *arg3)
{
    ARG_UNUSED(arg1);
    ARG_UNUSED(arg2);
    ARG_UNUSED(arg3);

    int value = 0;

    while (true) {
        if (k_msgq_put(&my_msgq, &value, K_FOREVER) == 0) {
            printk("[producer] sent: %d\n", value);
            value++;
        }

        k_msleep(PRODUCER_INTERVAL_MS);
    }
}

/* Consumer thread - receives numbers */
static void consumer_thread(void *arg1, void *arg2, void *arg3)
{
    ARG_UNUSED(arg1);
    ARG_UNUSED(arg2);
    ARG_UNUSED(arg3);

    int received_value;

    while (true) {
        if (k_msgq_get(&my_msgq, &received_value, K_FOREVER) == 0) {
            printk("[consumer] received: %d\n", received_value);
        }
    }
}

/* Thread creation */
K_THREAD_DEFINE(producer_id, STACK_SIZE, producer_thread,
                NULL, NULL, NULL, THREAD_PRIORITY, 0, 0);

K_THREAD_DEFINE(consumer_id, STACK_SIZE, consumer_thread,
                NULL, NULL, NULL, THREAD_PRIORITY, 0, 0);

int main(void)
{
    printk("Message Queue Example\n");
    return 0;
}

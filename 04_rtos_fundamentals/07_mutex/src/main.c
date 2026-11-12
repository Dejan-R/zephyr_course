#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#define STACK_SIZE 2048
#define THREAD_PRIORITY 7

#define PRODUCER_INTERVAL_MS 500
#define CONSUMER_INTERVAL_MS 700

/* Shared data protected by a mutex. */
static int counter = 0;
K_MUTEX_DEFINE(counter_mutex);

/* Producer thread - updates the shared counter. */
static void producer_thread(void *arg1, void *arg2, void *arg3)
{
    ARG_UNUSED(arg1);
    ARG_UNUSED(arg2);
    ARG_UNUSED(arg3);

    while (true) {
        k_mutex_lock(&counter_mutex, K_FOREVER);
        counter++;
        k_mutex_unlock(&counter_mutex);

        k_msleep(PRODUCER_INTERVAL_MS);
    }
}

/* Consumer thread - reads the shared counter. */
static void consumer_thread(void *arg1, void *arg2, void *arg3)
{
    ARG_UNUSED(arg1);
    ARG_UNUSED(arg2);
    ARG_UNUSED(arg3);

    while (true) {
        k_mutex_lock(&counter_mutex, K_FOREVER);
        int value = counter;
        k_mutex_unlock(&counter_mutex);

        printk("[consumer] counter: %d\n", value);
        k_msleep(CONSUMER_INTERVAL_MS);
    }
}

/* Create both threads. */
K_THREAD_DEFINE(producer_id, STACK_SIZE, producer_thread,
                NULL, NULL, NULL, THREAD_PRIORITY, 0, 0);

K_THREAD_DEFINE(consumer_id, STACK_SIZE, consumer_thread,
                NULL, NULL, NULL, THREAD_PRIORITY, 0, 0);

int main(void)
{
    printk("Mutex Example\n");
    return 0;
}

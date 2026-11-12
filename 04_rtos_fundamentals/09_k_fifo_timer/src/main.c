#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#define STACK_SIZE 2048
#define THREAD_PRIORITY 7

#define TIMER_DELAY_MS 1000
#define TIMER_PERIOD_MS 1000

/* FIFO transfers pointers to data items. */
K_FIFO_DEFINE(my_fifo);

/* Timer signals the producer thread. */
K_SEM_DEFINE(timer_sem, 0, 1);

/* First field is reserved for FIFO internal use. */
struct data_item {
    void *fifo_reserved;
    int value;
};

/* Timer callback runs in interrupt context. */
static void timer_handler(struct k_timer *timer)
{
    ARG_UNUSED(timer);
    k_sem_give(&timer_sem);
}

K_TIMER_DEFINE(my_timer, timer_handler, NULL);

/* Producer: create an item after each timer event. */
static void producer_thread(void *arg1, void *arg2, void *arg3)
{
    ARG_UNUSED(arg1);
    ARG_UNUSED(arg2);
    ARG_UNUSED(arg3);

    int counter = 0;

    while (true) {
        k_sem_take(&timer_sem, K_FOREVER);

        struct data_item *item = k_malloc(sizeof(*item));
        if (item == NULL) {
            printk("[producer] allocation failed\n");
            continue;
        }

        item->value = counter++;

        /* Print before handing ownership to the consumer. */
        printk("[producer] sent: %d\n", item->value);
        k_fifo_put(&my_fifo, item);

        /* Do not access item after k_fifo_put(). */
    }
}

/* Consumer: receive an item, use it, then free its memory. */
static void consumer_thread(void *arg1, void *arg2, void *arg3)
{
    ARG_UNUSED(arg1);
    ARG_UNUSED(arg2);
    ARG_UNUSED(arg3);

    while (true) {
        struct data_item *item = k_fifo_get(&my_fifo, K_FOREVER);

        if (item != NULL) {
            printk("[consumer] received: %d\n", item->value);
            k_free(item);
        }
    }
}

K_THREAD_DEFINE(producer_id, STACK_SIZE, producer_thread,
                NULL, NULL, NULL, THREAD_PRIORITY, 0, 0);

K_THREAD_DEFINE(consumer_id, STACK_SIZE, consumer_thread,
                NULL, NULL, NULL, THREAD_PRIORITY, 0, 0);

int main(void)
{
    printk("Timer + FIFO Example\n");

    k_timer_start(&my_timer, K_MSEC(TIMER_DELAY_MS),
                  K_MSEC(TIMER_PERIOD_MS));

    return 0;
}

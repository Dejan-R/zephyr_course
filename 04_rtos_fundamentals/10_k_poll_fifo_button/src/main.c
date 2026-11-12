#include <zephyr/kernel.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/util.h>

#define LED_NODE DT_ALIAS(led1)
#define BUTTON_NODE DT_ALIAS(sw0)

#define STACK_SIZE 2048
#define THREAD_PRIORITY 7
#define TIMER_DELAY_MS 1000
#define TIMER_PERIOD_MS 1000
#define DEBOUNCE_MS 50

#if !DT_NODE_HAS_STATUS(LED_NODE, okay) || !DT_NODE_HAS_STATUS(BUTTON_NODE, okay)
#error "Define led1 and sw0 aliases in Devicetree"
#endif

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED_NODE, gpios);
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(BUTTON_NODE, gpios);
static struct gpio_callback button_cb;

K_FIFO_DEFINE(my_fifo);
K_SEM_DEFINE(timer_sem, 0, 1);
K_SEM_DEFINE(button_sem, 0, 1);

/* FIFO stores a pointer to each item. The first field is reserved. */
struct data_item {
    void *fifo_reserved;
    int value;
};

/* Timer callback and GPIO ISR only signal an event. */
static void timer_handler(struct k_timer *timer)
{
    ARG_UNUSED(timer);
    k_sem_give(&timer_sem);
}

K_TIMER_DEFINE(my_timer, timer_handler, NULL);

static void button_pressed(const struct device *port,
                           struct gpio_callback *cb,
                           gpio_port_pins_t pins)
{
    ARG_UNUSED(port);
    ARG_UNUSED(cb);
    ARG_UNUSED(pins);
    k_sem_give(&button_sem);
}

/* Producer: a timer signal generates one FIFO item. */
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
        k_fifo_put(&my_fifo, item);
        /* Ownership now belongs to the FIFO consumer. */
    }
}

/* One thread waits for either FIFO data or a button signal. */
static void poll_thread(void *arg1, void *arg2, void *arg3)
{
    ARG_UNUSED(arg1);
    ARG_UNUSED(arg2);
    ARG_UNUSED(arg3);

    bool timer_running = false;
    int64_t last_press_ms = -DEBOUNCE_MS;

    struct k_poll_event events[2] = {
        K_POLL_EVENT_INITIALIZER(K_POLL_TYPE_FIFO_DATA_AVAILABLE,
                                 K_POLL_MODE_NOTIFY_ONLY, &my_fifo),
        K_POLL_EVENT_INITIALIZER(K_POLL_TYPE_SEM_AVAILABLE,
                                 K_POLL_MODE_NOTIFY_ONLY, &button_sem),
    };

    while (true) {
        int ret = k_poll(events, ARRAY_SIZE(events), K_FOREVER);
        if (ret != 0) {
            printk("[poll] wait failed: %d\n", ret);
            continue;
        }

        /* Data event: take the item and release its memory. */
        if (events[0].state == K_POLL_STATE_FIFO_DATA_AVAILABLE) {
            struct data_item *item = k_fifo_get(&my_fifo, K_NO_WAIT);
            if (item != NULL) {
                printk("[poll] data: %d\n", item->value);
                if (gpio_pin_toggle_dt(&led) < 0) {
                    printk("[poll] LED toggle failed\n");
                }
                k_free(item);
            }
        }

        /* Button event: take the signal, then start or stop timer. */
        if (events[1].state == K_POLL_STATE_SEM_AVAILABLE) {
            if (k_sem_take(&button_sem, K_NO_WAIT) == 0) {
                int64_t now = k_uptime_get();

                /* Simple time-based debounce, in thread context. */
                if (now - last_press_ms >= DEBOUNCE_MS) {
                    last_press_ms = now;

                    if (timer_running) {
                        k_timer_stop(&my_timer);
                        timer_running = false;
                        printk("[poll] timer stopped\n");
                    } else {
                        k_timer_start(&my_timer,
                                      K_MSEC(TIMER_DELAY_MS),
                                      K_MSEC(TIMER_PERIOD_MS));
                        timer_running = true;
                        printk("[poll] timer started\n");
                    }
                }
            }
        }

        /* k_poll() reports availability; the states must be reset. */
        events[0].state = K_POLL_STATE_NOT_READY;
        events[1].state = K_POLL_STATE_NOT_READY;
    }
}

K_THREAD_DEFINE(producer_id, STACK_SIZE, producer_thread,
                NULL, NULL, NULL, THREAD_PRIORITY, 0, 0);

K_THREAD_DEFINE(poll_id, STACK_SIZE, poll_thread,
                NULL, NULL, NULL, THREAD_PRIORITY, 0, 0);

int main(void)
{
    int ret;

    if (!gpio_is_ready_dt(&led) || !gpio_is_ready_dt(&button)) {
        printk("GPIO controller is not ready\n");
        return -1;
    }

    ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);
    if (ret < 0) {
        printk("LED configuration failed: %d\n", ret);
        return ret;
    }

    /* Pull-up and active-low polarity are defined in Devicetree. */
    ret = gpio_pin_configure_dt(&button, GPIO_INPUT);
    if (ret < 0) {
        printk("Button configuration failed: %d\n", ret);
        return ret;
    }

    gpio_init_callback(&button_cb, button_pressed, BIT(button.pin));
    ret = gpio_add_callback(button.port, &button_cb);
    if (ret < 0) {
        printk("Button callback registration failed: %d\n", ret);
        return ret;
    }

    ret = gpio_pin_interrupt_configure_dt(&button, GPIO_INT_EDGE_TO_ACTIVE);
    if (ret < 0) {
        printk("Button interrupt configuration failed: %d\n", ret);
        return ret;
    }

    printk("k_poll + FIFO + Timer + Button\n");
    printk("Press GPIO27 button to start/stop timer\n");
    return 0;
}

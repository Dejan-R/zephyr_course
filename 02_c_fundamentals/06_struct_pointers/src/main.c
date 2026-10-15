#include <stdint.h>
#include <zephyr/sys/printk.h>

/* Group two values in a structure. */
typedef struct {
    uint8_t a;
    uint8_t b;
} sum_data_t;

/* Read structure members through a pointer without modifying them. */
static int calculate_sum(const sum_data_t *data)
{
    return data->a + data->b;
}

/* Update the original structure through a pointer. */
static void update_values(sum_data_t *data, uint8_t a, uint8_t b)
{
    data->a = a;
    data->b = b;
}

int main(void)
{
    sum_data_t s1 = {3, 4};
    sum_data_t s2 = {5, 5};

    printk("Struct Pointers\n");
    printk("Sum 1: %d\n", calculate_sum(&s1));
    printk("Sum 2: %d\n", calculate_sum(&s2));

    update_values(&s1, 10, 20);

    printk("Updated s1: a = %d, b = %d\n", s1.a, s1.b);
    printk("New sum 1: %d\n", calculate_sum(&s1));

    return 0;
}

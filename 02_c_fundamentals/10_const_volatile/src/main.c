#include <zephyr/sys/printk.h>
#include <stdint.h>

int main(void)
{
    /* A const object cannot be modified through its name. */
    const int limit = 50;

    int value = 10;
    int other_value = 20;

    /* The pointed-to value is read-only through this pointer. */
    const int *ptr_to_const = &value;

    /* The pointer is fixed, but the pointed-to value can change. */
    int *const const_ptr = &value;

    /* Simulated status value: no real hardware register is accessed. */
    volatile uint32_t simulated_status = 0U;

    /* Read-only access to a value with volatile semantics. */
    const volatile uint32_t *status_view = &simulated_status;

    printk("Const and Volatile\n");
    printk("Limit: %d\n", limit);

    printk("Pointer to const: %d\n", *ptr_to_const);
    ptr_to_const = &other_value;
    printk("Pointer reassigned: %d\n", *ptr_to_const);

    *const_ptr = 15;
    printk("Value after const pointer write: %d\n", value);

    printk("Simulated status before: %u\n", (unsigned int)*status_view);

    /* Simulate an external status change without using peripherals. */
    simulated_status = 1U;
    printk("Simulated status after: %u\n", (unsigned int)*status_view);

    return 0;
}

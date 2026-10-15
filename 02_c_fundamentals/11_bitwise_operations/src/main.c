#include <zephyr/sys/printk.h>
#include <zephyr/sys/util.h>
#include <stdint.h>

/* Object-like macros name individual status bits. */
#define STATUS_READY BIT(0)
#define STATUS_ERROR BIT(1)
#define STATUS_BUSY  BIT(2)

/* Function-like macro tests whether a flag is set. */
#define HAS_FLAG(value, flag) (((value) & (flag)) != 0U)

int main(void)
{
    uint32_t status = 0U;

    printk("Bitwise Operations and Macros\n");
    printk("Initial status: 0x%02X\n", (unsigned int)status);

    /* Set two bits using OR. */
    status |= STATUS_READY | STATUS_ERROR;
    printk("Set READY and ERROR: 0x%02X\n", (unsigned int)status);

    /* Test a bit using AND inside a function-like macro. */
    printk("READY set: %s\n", HAS_FLAG(status, STATUS_READY) ? "yes" : "no");

    /* Toggle a bit using XOR. */
    status ^= STATUS_ERROR;
    printk("Toggle ERROR: 0x%02X\n", (unsigned int)status);

    /* Set and then clear a bit using OR, AND and NOT. */
    status |= STATUS_BUSY;
    printk("Set BUSY: 0x%02X\n", (unsigned int)status);

    status &= ~STATUS_BUSY;
    printk("Clear BUSY: 0x%02X\n", (unsigned int)status);

    /* Shift unsigned values left and right. */
    printk("Left shift (1 << 3): %u\n", 1U << 3);
    printk("Right shift (8 >> 2): %u\n", 8U >> 2);

    return 0;
}

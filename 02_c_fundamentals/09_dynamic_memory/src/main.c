#include <zephyr/sys/printk.h>

#include <stddef.h>
#include <stdlib.h>

int main(void)
{
    /* malloc() reserves memory but does not initialize its contents. */
    int *single = malloc(sizeof(*single));

    if (single == NULL) {
        printk("malloc failed\n");
        return 0;
    }

    *single = 55;
    printk("malloc value: %d\n", *single);

    free(single);
    single = NULL;

    /* calloc() allocates an array and initializes its bytes to zero. */
    const size_t initial_count = 3;
    int *values = calloc(initial_count, sizeof(*values));

    if (values == NULL) {
        printk("calloc failed\n");
        return 0;
    }

    printk("calloc initial:");
    for (size_t i = 0; i < initial_count; i++) {
        printk(" %d", values[i]);
        values[i] = (int)(i + 1) * 10;
    }
    printk("\n");

    printk("calloc filled:");
    for (size_t i = 0; i < initial_count; i++) {
        printk(" %d", values[i]);
    }
    printk("\n");

    /* realloc() preserves existing values but does not clear new elements. */
    const size_t expanded_count = 7;
    int *resized = realloc(values, expanded_count * sizeof(*values));

    if (resized == NULL) {
        printk("realloc failed\n");
        free(values);
        return 0;
    }

    values = resized;

    /* Initialize newly added elements before reading them. */
    for (size_t i = initial_count; i < expanded_count; i++) {
        values[i] = (int)(i + 1) * 10;
    }

    printk("realloc expanded:");
    for (size_t i = 0; i < expanded_count; i++) {
        printk(" %d", values[i]);
    }
    printk("\n");

    free(values);
    values = NULL;

    printk("Memory released\n");

    return 0;
}

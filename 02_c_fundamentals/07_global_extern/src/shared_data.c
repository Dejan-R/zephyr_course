#include "shared_data.h"

/* Define and initialize the global variable exactly once. */
int shared_value = 55;

void increment_shared_value(void)
{
    shared_value++;
}

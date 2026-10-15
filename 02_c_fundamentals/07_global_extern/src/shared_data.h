#ifndef SHARED_DATA_H
#define SHARED_DATA_H

/* Declare the global variable defined in shared_data.c. */
extern int shared_value;

/* Declare a function that modifies the same global variable. */
void increment_shared_value(void);

#endif /* SHARED_DATA_H */

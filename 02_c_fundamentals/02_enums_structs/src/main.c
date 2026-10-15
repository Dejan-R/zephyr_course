#include <zephyr/sys/printk.h>

/* Enum: named values describing the sensor status. */
enum sensor_status {
    SENSOR_OK,
    SENSOR_WARNING,
    SENSOR_ERROR
};

/* Struct: related sensor data stored in one object. */
struct sensor_reading {
    int temperature_c;
    int humidity_percent;
    enum sensor_status status;
};

int main(void)
{
    /* Designated initializers assign values to named structure members. */
    struct sensor_reading reading = {
        .temperature_c = 26,
        .humidity_percent = 58,
        .status = SENSOR_OK
    };

    printk("Enums and Structs\n");
    printk("Temperature: %d C\n", reading.temperature_c);
    printk("Humidity: %d %%\n", reading.humidity_percent);

    switch (reading.status) {
    case SENSOR_OK:
        printk("Status: OK\n");
        break;
    case SENSOR_WARNING:
        printk("Status: WARNING\n");
        break;
    case SENSOR_ERROR:
        printk("Status: ERROR\n");
        break;
    default:
        printk("Status: UNKNOWN\n");
        break;
    }

    return 0;
}

#ifndef SENSORS_H
#define SENSORS_H

#include "lib/aht20/aht20.h"
#include "lib/bmp280/bmp280.h"
#include "lib/ssd1306/ssd1306.h"
#include "hardware/i2c.h"
#include <math.h>

#define I2C_PORT i2c0
#define I2C_SDA 0
#define I2C_SCL 1
#define SEA_LEVEL_PRESSURE 102100.0

typedef struct {
    float bmp_temp;
    float pressure_kpa;
    float altitude_m;
    float aht_temp;
    float humidity;
} SensorReadings;

void init_i2c_sensor();
void init_bmp280();
void init_aht20();
SensorReadings get_sensor_readings();
double calculate_altitude(double pressure);

#endif
#include "sensors.h"

static struct bmp280_calib_param bmp_params;

void init_i2c_sensor() {
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
}

void init_bmp280() {
    bmp280_init(I2C_PORT);
    bmp280_get_calib_params(I2C_PORT, &bmp_params);
}

void init_aht20() {
    aht20_reset(I2C_PORT);
    aht20_init(I2C_PORT);
}

double calculate_altitude(double pressure) {
    return 44330.0 * (1.0 - pow(pressure / SEA_LEVEL_PRESSURE, 0.1903));
}

SensorReadings get_sensor_readings() {
    SensorReadings data;
    AHT20_Data aht;

    int32_t raw_temp, raw_press;
    bmp280_read_raw(I2C_PORT, &raw_temp, &raw_press);
    int32_t temp = bmp280_convert_temp(raw_temp, &bmp_params);
    int32_t press = bmp280_convert_pressure(raw_press, raw_temp, &bmp_params);

    data.bmp_temp = temp / 100.0f;
    data.pressure_kpa = press / 1000.0f;
    data.altitude_m = calculate_altitude(press);

    if (aht20_read(I2C_PORT, &aht)) {
        data.aht_temp = aht.temperature;
        data.humidity = aht.humidity;
    } else {
        data.aht_temp = 0;
        data.humidity = 0;
    }

    //printf("BMP Temp: %.2f C, Pressure: %.2f kPa, Altitude: %.2f m\n", data.bmp_temp, data.pressure_kpa, data.altitude_m);
    //printf("AHT Temp: %.2f C, Humidity: %.2f %%\n\n", data.aht_temp, data.humidity);

    return data;
}
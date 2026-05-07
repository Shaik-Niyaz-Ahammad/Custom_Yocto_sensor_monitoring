#ifndef BME280_H
#define BME280_H

#include <stdint.h>

#define BME280_I2C_DEVICE "/dev/i2c-2"
#define BME280_I2C_ADDR   0x76

#define BME280_REG_CHIP_ID   0xD0
#define BME280_REG_CTRL_HUM  0xF2
#define BME280_REG_STATUS    0xF3
#define BME280_REG_CTRL_MEAS 0xF4
#define BME280_REG_CONFIG    0xF5
#define BME280_REG_DATA      0xF7

typedef struct {
    uint16_t dig_T1;
    int16_t  dig_T2;
    int16_t  dig_T3;

    uint16_t dig_P1;
    int16_t  dig_P2;
    int16_t  dig_P3;
    int16_t  dig_P4;
    int16_t  dig_P5;
    int16_t  dig_P6;
    int16_t  dig_P7;
    int16_t  dig_P8;
    int16_t  dig_P9;

    uint8_t  dig_H1;
    int16_t  dig_H2;
    uint8_t  dig_H3;
    int16_t  dig_H4;
    int16_t  dig_H5;
    int8_t   dig_H6;
} bme280_calib_data_t;

typedef struct {
    double temperature_c;
    double pressure_hpa;
    double humidity_pct;
} bme280_data_t;

int bme280_open(const char *device, int addr);
void bme280_close(int fd);

int bme280_init(int fd);
int bme280_read_calibration(int fd, bme280_calib_data_t *cal);
int bme280_read_data(int fd, const bme280_calib_data_t *cal, bme280_data_t *out);

#endif

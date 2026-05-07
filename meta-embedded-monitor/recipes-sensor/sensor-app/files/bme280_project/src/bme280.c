#include "../include/bme280.h"

#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>

static int32_t t_fine = 0;

static uint16_t u16_le(const uint8_t *buf) {
    return (uint16_t)(buf[0] | (buf[1] << 8));
}

static int16_t s16_le(const uint8_t *buf) {
    return (int16_t)(buf[0] | (buf[1] << 8));
}

static int i2c_write_byte(int fd, uint8_t reg, uint8_t value) {
    uint8_t buf[2] = {reg, value};
    return (write(fd, buf, 2) == 2) ? 0 : -1;
}

static int i2c_read_bytes(int fd, uint8_t reg, uint8_t *buf, size_t len) {
    if (write(fd, &reg, 1) != 1) {
        return -1;
    }
    if (read(fd, buf, len) != (ssize_t)len) {
        return -1;
    }
    return 0;
}

int bme280_open(const char *device, int addr) {
    int fd = open(device, O_RDWR);
    if (fd < 0) {
        perror("open");
        return -1;
    }

    if (ioctl(fd, I2C_SLAVE, addr) < 0) {
        perror("ioctl");
        close(fd);
        return -1;
    }

    return fd;
}

void bme280_close(int fd) {
    if (fd >= 0) {
        close(fd);
    }
}

int bme280_init(int fd) {
    uint8_t chip_id = 0;

    if (i2c_read_bytes(fd, BME280_REG_CHIP_ID, &chip_id, 1) < 0) {
        return -1;
    }

    if (chip_id != 0x60) {
        fprintf(stderr, "Unexpected BME280 chip ID: 0x%02X\n", chip_id);
        return -1;
    }

    /* Humidity oversampling x1 */
    if (i2c_write_byte(fd, BME280_REG_CTRL_HUM, 0x01) < 0) {
        return -1;
    }

    /* Temp x1, Pressure x1, Normal mode */
    if (i2c_write_byte(fd, BME280_REG_CTRL_MEAS, 0x27) < 0) {
        return -1;
    }

    /* Standby 1000 ms, filter off */
    if (i2c_write_byte(fd, BME280_REG_CONFIG, 0xA0) < 0) {
        return -1;
    }

    return 0;
}

int bme280_read_calibration(int fd, bme280_calib_data_t *cal) {
    uint8_t buf1[24];
    uint8_t buf2[7];
    uint8_t h1;

    if (i2c_read_bytes(fd, 0x88, buf1, sizeof(buf1)) < 0) return -1;
    if (i2c_read_bytes(fd, 0xE1, buf2, sizeof(buf2)) < 0) return -1;
    if (i2c_read_bytes(fd, 0xA1, &h1, 1) < 0) return -1;

    cal->dig_T1 = u16_le(&buf1[0]);
    cal->dig_T2 = s16_le(&buf1[2]);
    cal->dig_T3 = s16_le(&buf1[4]);

    cal->dig_P1 = u16_le(&buf1[6]);
    cal->dig_P2 = s16_le(&buf1[8]);
    cal->dig_P3 = s16_le(&buf1[10]);
    cal->dig_P4 = s16_le(&buf1[12]);
    cal->dig_P5 = s16_le(&buf1[14]);
    cal->dig_P6 = s16_le(&buf1[16]);
    cal->dig_P7 = s16_le(&buf1[18]);
    cal->dig_P8 = s16_le(&buf1[20]);
    cal->dig_P9 = s16_le(&buf1[22]);

    cal->dig_H1 = h1;
    cal->dig_H2 = s16_le(&buf2[0]);
    cal->dig_H3 = buf2[2];

    cal->dig_H4 = (int16_t)((buf2[3] << 4) | (buf2[4] & 0x0F));
    if (cal->dig_H4 & 0x0800) cal->dig_H4 |= 0xF000;

    cal->dig_H5 = (int16_t)((buf2[5] << 4) | (buf2[4] >> 4));
    if (cal->dig_H5 & 0x0800) cal->dig_H5 |= 0xF000;

    cal->dig_H6 = (int8_t)buf2[6];

    return 0;
}

static double compensate_temperature(int32_t adc_T, const bme280_calib_data_t *cal) {
    int32_t var1, var2, T;

    var1 = ((((adc_T >> 3) - ((int32_t)cal->dig_T1 << 1))) * ((int32_t)cal->dig_T2)) >> 11;
    var2 = (((((adc_T >> 4) - ((int32_t)cal->dig_T1)) *
             ((adc_T >> 4) - ((int32_t)cal->dig_T1))) >> 12) *
             ((int32_t)cal->dig_T3)) >> 14;

    t_fine = var1 + var2;
    T = (t_fine * 5 + 128) >> 8;

    return T / 100.0;
}

static double compensate_pressure(int32_t adc_P, const bme280_calib_data_t *cal) {
    int64_t var1, var2, p;

    var1 = ((int64_t)t_fine) - 128000;
    var2 = var1 * var1 * (int64_t)cal->dig_P6;
    var2 = var2 + ((var1 * (int64_t)cal->dig_P5) << 17);
    var2 = var2 + (((int64_t)cal->dig_P4) << 35);
    var1 = ((var1 * var1 * (int64_t)cal->dig_P3) >> 8) +
           ((var1 * (int64_t)cal->dig_P2) << 12);
    var1 = (((((int64_t)1) << 47) + var1) * (int64_t)cal->dig_P1) >> 33;

    if (var1 == 0) {
        return 0.0;
    }

    p = 1048576 - adc_P;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (((int64_t)cal->dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((int64_t)cal->dig_P8) * p) >> 19;
    p = ((p + var1 + var2) >> 8) + (((int64_t)cal->dig_P7) << 4);

    return (double)p / 25600.0;
}

static double compensate_humidity(int32_t adc_H, const bme280_calib_data_t *cal) {
    int32_t v_x1_u32r;

    v_x1_u32r = t_fine - 76800;
    v_x1_u32r = (((((adc_H << 14) - (((int32_t)cal->dig_H4) << 20) -
                   (((int32_t)cal->dig_H5) * v_x1_u32r)) + 16384) >> 15) *
                 (((((((v_x1_u32r * ((int32_t)cal->dig_H6)) >> 10) *
                      (((v_x1_u32r * ((int32_t)cal->dig_H3)) >> 11) + 32768)) >> 10) +
                    2097152) * ((int32_t)cal->dig_H2) + 8192) >> 14));

    v_x1_u32r = v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) *
                    ((int32_t)cal->dig_H1)) >> 4);

    if (v_x1_u32r < 0) v_x1_u32r = 0;
    if (v_x1_u32r > 419430400) v_x1_u32r = 419430400;

    return (double)(v_x1_u32r >> 12) / 1024.0;
}

int bme280_read_data(int fd, const bme280_calib_data_t *cal, bme280_data_t *out) {
    uint8_t data[8];
    int32_t adc_t, adc_p, adc_h;

    if (i2c_read_bytes(fd, BME280_REG_DATA, data, sizeof(data)) < 0) {
        return -1;
    }

    adc_p = (int32_t)((data[0] << 12) | (data[1] << 4) | (data[2] >> 4));
    adc_t = (int32_t)((data[3] << 12) | (data[4] << 4) | (data[5] >> 4));
    adc_h = (int32_t)((data[6] << 8) | data[7]);

    out->temperature_c = compensate_temperature(adc_t, cal);
    out->pressure_hpa  = compensate_pressure(adc_p, cal);
    out->humidity_pct  = compensate_humidity(adc_h, cal);

    return 0;
}

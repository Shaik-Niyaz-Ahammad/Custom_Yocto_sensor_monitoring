#include "../include/bme280.h"
#include "../include/config.h"

#include <stdio.h>
#include <time.h>
#include <unistd.h>

static void get_timestamp(char *buf, size_t size) {
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    strftime(buf, size, "%Y-%m-%d %H:%M:%S", tm_info);
}

int main(void) {
    int fd;
    FILE *log_fp;
    char timestamp[32];

    app_config_t cfg;
    bme280_calib_data_t cal;
    bme280_data_t data;

    config_load(DEFAULT_CONFIG_FILE, &cfg);
    

    fd = bme280_open(BME280_I2C_DEVICE, BME280_I2C_ADDR);
    if (fd < 0) {
        fprintf(stderr, "Failed to open BME280 device\n");
        return 1;
    }

    if (bme280_init(fd) < 0) {
        fprintf(stderr, "Failed to initialize BME280\n");
        bme280_close(fd);
        return 1;
    }

    if (bme280_read_calibration(fd, &cal) < 0) {
        fprintf(stderr, "Failed to read calibration data\n");
        bme280_close(fd);
        return 1;
    }

    log_fp = fopen(cfg.log_file, "a");
    if (!log_fp) {
        perror("fopen");
        bme280_close(fd);
        return 1;
    }

    while (1) {
        if (bme280_read_data(fd, &cal, &data) < 0) {
            fprintf(stderr, "Failed to read BME280 data\n");
            sleep(cfg.sample_interval);
            continue;
        }

        get_timestamp(timestamp, sizeof(timestamp));

        printf("[%s] Temp: %.2f C | Pressure: %.2f hPa | Humidity: %.2f %%\n",
               timestamp,
               data.temperature_c,
               data.pressure_hpa,
               data.humidity_pct);

        fprintf(log_fp,
                "[%s] Temp: %.2f C | Pressure: %.2f hPa | Humidity: %.2f %%\n",
                timestamp,
                data.temperature_c,
                data.pressure_hpa,
                data.humidity_pct);

        fflush(stdout);
        fflush(log_fp);

        sleep(cfg.sample_interval);
    }

    fclose(log_fp);
    bme280_close(fd);
    return 0;
}

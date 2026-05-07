#ifndef CONFIG_H
#define CONFIG_H

#define DEFAULT_SAMPLE_INTERVAL 10
#define DEFAULT_LOG_FILE "/var/log/sensor.log"
#define DEFAULT_CONFIG_FILE "/etc/sensor_app.conf"

typedef struct {
    int sample_interval;
    char log_file[256];
} app_config_t;

void config_set_defaults(app_config_t *cfg);
int config_load(const char *path, app_config_t *cfg);

#endif

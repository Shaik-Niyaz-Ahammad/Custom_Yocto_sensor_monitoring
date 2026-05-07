#include "../include/config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void config_set_defaults(app_config_t *cfg) {
    cfg->sample_interval = DEFAULT_SAMPLE_INTERVAL;
    snprintf(cfg->log_file, sizeof(cfg->log_file), "%s", DEFAULT_LOG_FILE);
}

static void trim_newline(char *s) {
    size_t len = strlen(s);
    if (len > 0 && s[len - 1] == '\n') {
        s[len - 1] = '\0';
    }
}

int config_load(const char *path, app_config_t *cfg) {
    FILE *fp;
    char line[512];

    config_set_defaults(cfg);

    fp = fopen(path, "r");
    if (!fp) {
        return -1;
    }

    while (fgets(line, sizeof(line), fp)) {
        trim_newline(line);

        if (strncmp(line, "SAMPLE_INTERVAL=", 16) == 0) {
            cfg->sample_interval = atoi(line + 16);
            if (cfg->sample_interval <= 0) {
                cfg->sample_interval = DEFAULT_SAMPLE_INTERVAL;
            }
        } else if (strncmp(line, "LOG_FILE=", 9) == 0) {
            strncpy(cfg->log_file, line + 9, sizeof(cfg->log_file) - 1);
            cfg->log_file[sizeof(cfg->log_file) - 1] = '\0';
        }
    }

    fclose(fp);
    return 0;
}

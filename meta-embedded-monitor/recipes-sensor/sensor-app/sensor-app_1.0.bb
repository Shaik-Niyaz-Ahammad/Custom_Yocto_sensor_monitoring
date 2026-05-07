SUMMARY = "BME280 sensor monitoring application with OTA"
DESCRIPTION = "A C-based BME280 monitoring application with config file, systemd service, and OTA update support"
LICENSE = "CLOSED"

inherit systemd

SRC_URI = " \
    file://bme280_project/src/main.c \
    file://bme280_project/src/bme280.c \
    file://bme280_project/src/config.c \
    file://bme280_project/include/bme280.h \
    file://bme280_project/include/config.h \
    file://bme280_project/Makefile \
    file://bme280_project/sensor_app.conf \
    file://bme280_project/sensor_app.service \
    file://bme280_project/update_sensor.sh \
    file://bme280_project/update_sensor.service \
    file://bme280_project/update_sensor.timer \
    file://bme280_project/sensor_app.version \
"

S = "${WORKDIR}/bme280_project"

SYSTEMD_SERVICE:${PN} = "sensor_app.service update_sensor.timer"
SYSTEMD_AUTO_ENABLE = "enable"

do_compile() {
    oe_runmake CC="${CC}" CFLAGS="${CFLAGS}" LDFLAGS="${LDFLAGS}"
}

do_install() {
    install -d ${D}${bindir}
    install -m 0755 sensor_app ${D}${bindir}/sensor_app
    install -m 0755 update_sensor.sh ${D}${bindir}/update_sensor.sh

    install -d ${D}${sysconfdir}
    install -m 0644 sensor_app.conf ${D}${sysconfdir}/sensor_app.conf
    install -m 0644 sensor_app.version ${D}${sysconfdir}/sensor_app.version

    install -d ${D}${systemd_system_unitdir}
    install -m 0644 sensor_app.service ${D}${systemd_system_unitdir}/sensor_app.service
    install -m 0644 update_sensor.service ${D}${systemd_system_unitdir}/update_sensor.service
    install -m 0644 update_sensor.timer ${D}${systemd_system_unitdir}/update_sensor.timer
}

FILES:${PN} += " \
    ${bindir}/sensor_app \
    ${bindir}/update_sensor.sh \
    ${sysconfdir}/sensor_app.conf \
    ${sysconfdir}/sensor_app.version \
    ${systemd_system_unitdir}/sensor_app.service \
    ${systemd_system_unitdir}/update_sensor.service \
    ${systemd_system_unitdir}/update_sensor.timer \
"

import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import CONF_ID, UNIT_EMPTY, ICON_EMPTY
from . import WolfScom, CONF_WOLF_SCOM_ID

DEPENDENCIES = ['wolf_scom']
CONF_TARGET_FLOW_TEMPERATURE = "target_flow_temperature"
CONF_OPERATION_MODE = "operation_mode"
CONF_OUTSIDE_TEMPERATURE = "outside_temperature"
CONF_DWT_OUTSIDE_TEMPERATURE = "dwt_outside_temperature"
CONF_FLOW_TEMPERATURE = "flow_temperature"
CONF_WATER_TEMPERATURE = "water_temperature"
CONF_TARGET_WATER_TEMPERATURE = "target_water_temperature"
CONF_ERROR_CODE = "error_code"
CONF_RPM = "rpm"
CONF_BOILER_STATUS = "boiler_status"

sensor_ns = cg.esphome_ns.namespace('sensor')
Sensor = sensor_ns.class_('Sensor', sensor.Sensor, cg.Nameable)

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(WolfScom),
    cv.Optional(CONF_TARGET_FLOW_TEMPERATURE): sensor.sensor_schema(UNIT_EMPTY, ICON_EMPTY, 1).extend(),
    cv.Optional(CONF_OPERATION_MODE): sensor.sensor_schema(UNIT_EMPTY, ICON_EMPTY, 1).extend(),
    cv.Optional(CONF_OUTSIDE_TEMPERATURE): sensor.sensor_schema(UNIT_EMPTY, ICON_EMPTY, 1).extend(),
    cv.Optional(CONF_DWT_OUTSIDE_TEMPERATURE): sensor.sensor_schema(UNIT_EMPTY, ICON_EMPTY, 1).extend(),
    cv.Optional(CONF_FLOW_TEMPERATURE): sensor.sensor_schema(UNIT_EMPTY, ICON_EMPTY, 1).extend(),
    cv.Optional(CONF_WATER_TEMPERATURE): sensor.sensor_schema(UNIT_EMPTY, ICON_EMPTY, 1).extend(),
    cv.Optional(CONF_TARGET_WATER_TEMPERATURE): sensor.sensor_schema(UNIT_EMPTY, ICON_EMPTY, 1).extend(),
    cv.Optional(CONF_ERROR_CODE): sensor.sensor_schema(UNIT_EMPTY, ICON_EMPTY, 1).extend(),
    cv.Optional(CONF_RPM): sensor.sensor_schema(UNIT_EMPTY, ICON_EMPTY, 1).extend(),
    cv.Optional(CONF_BOILER_STATUS): sensor.sensor_schema(UNIT_EMPTY, ICON_EMPTY, 1).extend(),
})

def to_code(config):
    paren = yield cg.get_variable(config[CONF_WOLF_SCOM_ID])
    var = cg.new_Pvariable(config[CONF_ID])
    cg.add(paren.register_sensor(var))
    yield cg.register_component(var, config)

    if CONF_TARGET_FLOW_TEMPERATURE in config:
        conf = config[CONF_TARGET_FLOW_TEMPERATURE]
        sens = yield sensor.new_sensor(conf)
        cg.add(var.set_target_flow_temperature_sensor(sens))

    if CONF_OPERATION_MODE in config:
        conf = config[CONF_OPERATION_MODE]
        sens = yield sensor.new_sensor(conf)
        cg.add(var.set_operation_mode_sensor(sens))

    if CONF_OUTSIDE_TEMPERATURE in config:
        conf = config[CONF_OUTSIDE_TEMPERATURE]
        sens = yield sensor.new_sensor(conf)
        cg.add(var.set_outside_temperature_sensor(sens))

    if CONF_DWT_OUTSIDE_TEMPERATURE in config:
        conf = config[CONF_DWT_OUTSIDE_TEMPERATURE]
        sens = yield sensor.new_sensor(conf)
        cg.add(var.set_dwt_outside_temperature_sensor(sens))
    if CONF_FLOW_TEMPERATURE in config:
        conf = config[CONF_FLOW_TEMPERATURE]
        sens = yield sensor.new_sensor(conf)
        cg.add(var.set_flow_temperature_sensor(sens))
    if CONF_WATER_TEMPERATURE in config:
        conf = config[CONF_WATER_TEMPERATURE]
        sens = yield sensor.new_sensor(conf)
        cg.add(var.set_water_temperature_sensor(sens))
    if CONF_TARGET_WATER_TEMPERATURE in config:
        conf = config[CONF_TARGET_WATER_TEMPERATURE]
        sens = yield sensor.new_sensor(conf)
        cg.add(var.set_target_water_temperature_sensor(sens))
    if CONF_ERROR_CODE in config:
        conf = config[CONF_ERROR_CODE]
        sens = yield sensor.new_sensor(conf)
        cg.add(var.set_error_code_sensor(sens))
    if CONF_RPM in config:
        conf = config[CONF_RPM]
        sens = yield sensor.new_sensor(conf)
        cg.add(var.set_rpm_sensor(sens))
    if CONF_BOILER_STATUS in config:
        conf = config[CONF_BOILER_STATUS]
        sens = yield sensor.new_sensor(conf)
        cg.add(var.set_boiler_status_sensor(sens))
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import CONF_ID, UNIT_EMPTY, ICON_FAN, ICON_EMPTY,UNIT_REVOLUTIONS_PER_MINUTE, STATE_CLASS_MEASUREMENT, UNIT_CELSIUS, DEVICE_CLASS_TEMPERATURE
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
temperature_schema = sensor.sensor_schema(
    unit_of_measurement=UNIT_CELSIUS,
    accuracy_decimals=1,
    device_class=DEVICE_CLASS_TEMPERATURE,
    state_class=STATE_CLASS_MEASUREMENT,
).extend()

empty_schema = sensor.sensor_schema(
    unit_of_measurement=UNIT_EMPTY,
    accuracy_decimals=1,
    icon=ICON_EMPTY,
    state_class=STATE_CLASS_MEASUREMENT
).extend()

rpm_schema = sensor.sensor_schema(
    unit_of_measurement=UNIT_REVOLUTIONS_PER_MINUTE,
    accuracy_decimals=1,
    icon=ICON_FAN,
    state_class=STATE_CLASS_MEASUREMENT
).extend()

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(WolfScom),
    cv.Required(CONF_WOLF_SCOM_ID): cv.use_id(WolfScom),
    cv.Optional(CONF_TARGET_FLOW_TEMPERATURE): temperature_schema,
    cv.Optional(CONF_OPERATION_MODE): empty_schema,
    cv.Optional(CONF_OUTSIDE_TEMPERATURE): temperature_schema,
    cv.Optional(CONF_DWT_OUTSIDE_TEMPERATURE): temperature_schema,
    cv.Optional(CONF_FLOW_TEMPERATURE): temperature_schema,
    cv.Optional(CONF_WATER_TEMPERATURE): temperature_schema,
    cv.Optional(CONF_TARGET_WATER_TEMPERATURE): temperature_schema,
    cv.Optional(CONF_ERROR_CODE): empty_schema,
    cv.Optional(CONF_RPM): rpm_schema,
    cv.Optional(CONF_BOILER_STATUS): empty_schema,
}).extend({cv.GenerateID(): cv.declare_id(WolfScom)})

def to_code(config):
    paren = yield cg.get_variable(config[CONF_WOLF_SCOM_ID])

    if CONF_TARGET_FLOW_TEMPERATURE in config:
        conf = config[CONF_TARGET_FLOW_TEMPERATURE]
        sens = yield sensor.new_sensor(conf)
        cg.add(paren.set_target_flow_temperature_sensor(sens))

    if CONF_OPERATION_MODE in config:
        conf = config[CONF_OPERATION_MODE]
        sens = yield sensor.new_sensor(conf)
        cg.add(paren.set_operation_mode_sensor(sens))

    if CONF_OUTSIDE_TEMPERATURE in config:
        conf = config[CONF_OUTSIDE_TEMPERATURE]
        sens = yield sensor.new_sensor(conf)
        cg.add(paren.set_outside_temperature_sensor(sens))

    if CONF_DWT_OUTSIDE_TEMPERATURE in config:
        conf = config[CONF_DWT_OUTSIDE_TEMPERATURE]
        sens = yield sensor.new_sensor(conf)
        cg.add(paren.set_dwt_outside_temperature_sensor(sens))
    if CONF_FLOW_TEMPERATURE in config:
        conf = config[CONF_FLOW_TEMPERATURE]
        sens = yield sensor.new_sensor(conf)
        cg.add(paren.set_flow_temperature_sensor(sens))
    if CONF_WATER_TEMPERATURE in config:
        conf = config[CONF_WATER_TEMPERATURE]
        sens = yield sensor.new_sensor(conf)
        cg.add(paren.set_water_temperature_sensor(sens))
    if CONF_TARGET_WATER_TEMPERATURE in config:
        conf = config[CONF_TARGET_WATER_TEMPERATURE]
        sens = yield sensor.new_sensor(conf)
        cg.add(paren.set_target_water_temperature_sensor(sens))
    if CONF_ERROR_CODE in config:
        conf = config[CONF_ERROR_CODE]
        sens = yield sensor.new_sensor(conf)
        cg.add(paren.set_error_code_sensor(sens))
    if CONF_RPM in config:
        conf = config[CONF_RPM]
        sens = yield sensor.new_sensor(conf)
        cg.add(paren.set_rpm_sensor(sens))
    if CONF_BOILER_STATUS in config:
        conf = config[CONF_BOILER_STATUS]
        sens = yield sensor.new_sensor(conf)
        cg.add(paren.set_boiler_status_sensor(sens))
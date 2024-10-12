import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor
from esphome.const import CONF_ID, UNIT_EMPTY, ICON_EMPTY
from . import WolfScom, CONF_WOLF_SCOM_ID

DEPENDENCIES = ['wolf_scom']
CONF_HEATING = "heating"
CONF_HOT_WATER = "hot_water"
CONF_HEAT = "heat"
CONF_PUMP = "pump"
CONF_THREE_WAY_VALVE = "three_way_valve"

binary_sensor_ns = cg.esphome_ns.namespace('binary_sensor')
CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(WolfScom),
    cv.Required(CONF_WOLF_SCOM_ID): cv.use_id(WolfScom),
    cv.Optional(CONF_HEATING): binary_sensor.sensor_schema(UNIT_EMPTY, ICON_EMPTY, 1).extend(),
    cv.Optional(CONF_HOT_WATER): binary_sensor.sensor_schema(UNIT_EMPTY, ICON_EMPTY, 1).extend(),
    cv.Optional(CONF_PUMP): binary_sensor.sensor_schema(UNIT_EMPTY, ICON_EMPTY, 1).extend(),
    cv.Optional(CONF_HEAT): binary_sensor.sensor_schema(UNIT_EMPTY, ICON_EMPTY, 1).extend(),
    cv.Optional(CONF_THREE_WAY_VALVE): binary_sensor.sensor_schema(UNIT_EMPTY, ICON_EMPTY, 1).extend(),
}).extend({cv.GenerateID(): cv.declare_id(WolfScom)})

def to_code(config):
    paren = yield cg.get_variable(config[CONF_WOLF_SCOM_ID])
    var = cg.new_Pvariable(config[CONF_ID])

    yield binary_sensor.register_binary_sensor(var, config)

    cg.add(paren.register_binary_sensor(var))


    paren = yield cg.get_variable(config[CONF_WOLF_SCOM_ID])
    var = cg.new_Pvariable(config[CONF_ID])
    cg.add(paren.register_sensor(var))
    yield cg.register_component(var, config)

    if CONF_HEATING in config:
        conf = config[CONF_HEATING]
        sens = yield binary_sensor.new_sensor(conf)
        cg.add(var.set_heating_day_sensor(sens))

    if CONF_HOT_WATER in config:
        conf = config[CONF_HOT_WATER]
        sens = yield binary_sensor.new_sensor(conf)
        cg.add(var.set_hot_water_day_sensor(sens))

    if CONF_PUMP in config:
        conf = config[CONF_PUMP]
        sens = yield binary_sensor.new_sensor(conf)
        cg.add(var.set_pump_sensor(sens))

    if CONF_HEAT in config:
        conf = config[CONF_HEAT]
        sens = yield binary_sensor.new_sensor(conf)
        cg.add(var.set_heat_sensor(sens))

    if CONF_THREE_WAY_VALVE in config:
        conf = config[CONF_THREE_WAY_VALVE]
        sens = yield binary_sensor.new_sensor(conf)
        cg.add(var.set_three_way_valve_sensor(sens))
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor
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
    cv.Optional(CONF_HEATING): binary_sensor.binary_sensor_schema(),
    cv.Optional(CONF_HOT_WATER): binary_sensor.binary_sensor_schema(),
    cv.Optional(CONF_PUMP): binary_sensor.binary_sensor_schema(),
    cv.Optional(CONF_HEAT): binary_sensor.binary_sensor_schema(),
    cv.Optional(CONF_THREE_WAY_VALVE): binary_sensor.binary_sensor_schema(),
}).extend({cv.GenerateID(): cv.declare_id(WolfScom)})

async def to_code(config):
    paren = await cg.get_variable(config[CONF_WOLF_SCOM_ID])

    if CONF_HEATING in config:
        conf = config[CONF_HEATING]
        sens = await binary_sensor.new_binary_sensor(conf)
        cg.add(paren.set_heating_day_sensor(sens))

    if CONF_HOT_WATER in config:
        conf = config[CONF_HOT_WATER]
        sens = await binary_sensor.new_binary_sensor(conf)
        cg.add(paren.set_hot_water_day_sensor(sens))

    if CONF_PUMP in config:
        conf = config[CONF_PUMP]
        sens = await binary_sensor.new_binary_sensor(conf)
        cg.add(paren.set_pump_sensor(sens))

    if CONF_HEAT in config:
        conf = config[CONF_HEAT]
        sens = await binary_sensor.new_binary_sensor(conf)
        cg.add(paren.set_heat_sensor(sens))

    if CONF_THREE_WAY_VALVE in config:
        conf = config[CONF_THREE_WAY_VALVE]
        sens = await binary_sensor.new_binary_sensor(conf)
        cg.add(paren.set_three_way_valve_sensor(sens))
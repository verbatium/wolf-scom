import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID
from esphome.components import uart

DEPENDENCIES = ['uart']
AUTO_LOAD = ['sensor', 'binary_sensor']
MULTI_CONF = True

CONF_WOLF_SCOM_ID = 'wolf_scom_id'

wolf_scom_ns = cg.esphome_ns.namespace('wolf_scom')

WolfScom = wolf_scom_ns.class_('WolfScom', cg.Component, uart.UARTDevice)

CONFIG_SCHEMA = (cv.Schema({
    cv.GenerateID(): cv.declare_id(WolfScom),
}).extend(cv.COMPONENT_SCHEMA).extend(uart.UART_DEVICE_SCHEMA))


def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    yield cg.register_component(var, config)
    yield uart.register_uart_device(var, config)
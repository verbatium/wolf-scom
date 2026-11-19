import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID
from esphome.components import uart

DEPENDENCIES = ['uart']
AUTO_LOAD = ['sensor', 'binary_sensor']
MULTI_CONF = True

CONF_WOLF_SCOM_ID = 'wolf_scom_id'
CONF_MASTER = 'master'

wolf_scom_ns = cg.esphome_ns.namespace('wolf_scom')

WolfScom = wolf_scom_ns.class_('WolfScom', cg.Component, uart.UARTDevice)

CONFIG_SCHEMA = (cv.Schema({
    cv.GenerateID(): cv.declare_id(WolfScom),
    cv.Optional(CONF_MASTER, default=False): cv.boolean,
}).extend(cv.COMPONENT_SCHEMA).extend(uart.UART_DEVICE_SCHEMA))


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)
    cg.add(var.set_master(config[CONF_MASTER]))

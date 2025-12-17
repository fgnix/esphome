from esphome import pins
import esphome.codegen as cg
from esphome.components import i2c
import esphome.config_validation as cv
from esphome.const import (
    CONF_EXTERNAL_CLOCK_INPUT,
    CONF_FREQUENCY,
    CONF_ID,
    CONF_INVERTED,
    CONF_MODE,
    CONF_NUMBER,
    CONF_OUTPUT,
    CONF_PHASE_BALANCER,
)

DEPENDENCIES = ["i2c"]
MULTI_CONF = True

CONF_PCA9685_ID = "pca9685_id"

pca9685_ns = cg.esphome_ns.namespace("pca9685")
PCA9685Output = pca9685_ns.class_("PCA9685Output", cg.Component, i2c.I2CDevice)
PCA9685GPIOPin = pca9685_ns.class_("PCA9685GPIOPin", cg.GPIOPin, cg.Parented.template(PCA9685Output))

phase_balancer = pca9685_ns.enum("PhaseBalancer", is_class=True)
PHASE_BALANCERS = {
    "none": phase_balancer.NONE,
    "linear": phase_balancer.LINEAR,
}


def validate_frequency(config):
    if config[CONF_EXTERNAL_CLOCK_INPUT]:
        if CONF_FREQUENCY in config:
            raise cv.Invalid(
                "Frequency cannot be set when using an external clock input"
            )
        return config
    if CONF_FREQUENCY not in config:
        raise cv.Invalid("Frequency is required")
    return config


CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(PCA9685Output),
            cv.Optional(CONF_FREQUENCY): cv.All(
                cv.frequency, cv.float_range(min=23.84, max=1525.88)
            ),
            cv.Optional(CONF_EXTERNAL_CLOCK_INPUT, default=False): cv.boolean,
            cv.Optional(CONF_PHASE_BALANCER, default="linear"): cv.enum(
                PHASE_BALANCERS
            ),
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
    .extend(i2c.i2c_device_schema(0x40)),
    validate_frequency,
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    if CONF_FREQUENCY in config:
        cg.add(var.set_frequency(config[CONF_FREQUENCY]))
    cg.add(var.set_extclk(config[CONF_EXTERNAL_CLOCK_INPUT]))
    cg.add(var.set_phase_balancer(config[CONF_PHASE_BALANCER]))
    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)

PCA9685_PIN_SCHEMA = cv.All(
    {
        cv.GenerateID(): cv.declare_id(PCA9685GPIOPin),
        cv.Required(CONF_PCA9685_ID): cv.use_id(PCA9685Output),
        cv.Required(CONF_NUMBER): cv.int_range(min=0, max=15),
        cv.Optional(CONF_MODE, default={}): cv.All(
            {
                cv.Optional(CONF_OUTPUT, default=True): cv.boolean,
            },
        ),
        cv.Optional(CONF_INVERTED, default=False): cv.boolean,
    }
)


@pins.PIN_SCHEMA_REGISTRY.register(CONF_PCA9685_ID, PCA9685_PIN_SCHEMA)
async def pca6416a_pin_to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    parent = await cg.get_variable(config[CONF_PCA9685_ID])

    cg.add(var.set_parent(parent))

    num = config[CONF_NUMBER]
    cg.add(var.set_pin(num))
    cg.add(var.set_inverted(config[CONF_INVERTED]))
    cg.add(var.pin_mode(pins.gpio_flags_expr(config[CONF_MODE])))
    return var

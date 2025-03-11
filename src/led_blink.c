// defines the compatible value for a node that is used by this driver, like:
// compatible = "led,blink-driver"
#define DT_DRV_COMPAT led_blink_driver

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/sensor.h>

LOG_MODULE_REGISTER(led_blink, LOG_LEVEL_INF);

// Get LED device from DeviceTree
//#define LED_NODE DT_ALIAS(led0)
//#define LED_GPIO DT_GPIO_CTLR(LED_NODE, led_gpios)
//#define LED_GPIO_PIN DT_GPIO_PIN(led_gpios)
#define LED_BLINK_CONFIG_PRIORITY 24

#define BLINK_DEV_IRQ  24       /* device uses IRQ 24 */
#define BLINK_DEV_PRIO  2       /* device uses interrupt priority 2 */

struct led_blink_config {
    struct gpio_dt_spec gpio;
    struct gpio_dt_spec irq_gpio;
};

struct led_blink_data {
    struct gpio_callback irq_callback;
};

static const struct sensor_driver_api led_blink_api = {
    
};

void blink_ir(const struct device *gpio_dev, struct gpio_callback *cb, uint32_t pins) {
    struct led_blink_data *data = CONTAINER_OF(cb, struct led_blink_data, irq_callback);
    const struct device *blink_dev = CONTAINER_OF((void*)data, struct device, data);
    const struct led_blink_config *conf = (const struct led_blink_config*)blink_dev->config;

    gpio_pin_toggle_dt(&conf->gpio);
}

static int led_blink_init(const struct device *dev) {
    printk("Started LED blink init!\n");

    const struct led_blink_config* conf = (const struct led_blink_config*)dev->config;
    struct led_blink_data* data = (struct led_blink_data*)dev->data;

    if (!gpio_pin_configure_dt(&conf->gpio, 0)) {
        LOG_ERR("Failed to init gpio pin in led_blink!");
        return -ENODEV;
    }

    gpio_pin_set_dt(&conf->gpio, 0);

    if (!gpio_pin_interrupt_configure_dt(&conf->irq_gpio, GPIO_INT_EDGE_TO_ACTIVE)) {
        LOG_ERR("Failed to init interrupt pin!");
        return -ENODEV;
    }

    gpio_init_callback(&data->irq_callback, blink_ir, BIT(conf->irq_gpio.pin));
    gpio_add_callback_dt(&conf->irq_gpio, &data->irq_callback);

    return 0;
}

// this given an instance i in the device tree defines an instance of the sensor
#define LED_BLINK_DEFINE(i) \
    static const struct led_blink_config config##i =  { \
        .gpio = GPIO_DT_SPEC_INST_GET(i, led_gpios), \
        .irq_gpio = GPIO_DT_SPEC_INST_GET(i, irq_gpios) \
    }; \
    \
    static struct led_blink_data data##i; \
    \
    DEVICE_DT_INST_DEFINE(i, led_blink_init, NULL, &data##i, &config##i, POST_KERNEL, LED_BLINK_CONFIG_PRIORITY, &led_blink_api);

// 23:55 in the Driver development tutorial
DT_INST_FOREACH_STATUS_OKAY(LED_BLINK_DEFINE);

// 19:49 in the Driver development tutorial
//DEVICE_DT_INST_DEFINE(led0, led_blink_init)

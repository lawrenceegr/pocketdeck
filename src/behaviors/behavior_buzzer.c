#define DT_DRV_COMPAT zmk_behavior_buzzer

#include <zephyr/device.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <drivers/behavior.h>
#include <zmk/behavior.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)

struct behavior_buzzer_config {
    struct pwm_dt_spec pwm;
    uint32_t duration_ms;
};

struct behavior_buzzer_data {
    struct k_work_delayable stop_work;
    const struct device *dev;
};

static void buzzer_silence(const struct behavior_buzzer_config *cfg) {
    pwm_set(cfg->pwm.dev, cfg->pwm.channel, NSEC_PER_SEC / 1000, 0, cfg->pwm.flags);
}

static void buzzer_stop_handler(struct k_work *work) {
    struct k_work_delayable *dwork = k_work_delayable_from_work(work);
    struct behavior_buzzer_data *data = CONTAINER_OF(dwork, struct behavior_buzzer_data, stop_work);
    const struct behavior_buzzer_config *cfg = data->dev->config;

    buzzer_silence(cfg);
}

static int on_buzzer_binding_pressed(struct zmk_behavior_binding *binding,
                                     struct zmk_behavior_binding_event event) {
    ARG_UNUSED(event);

    const struct device *dev = zmk_behavior_get_binding(binding->behavior_dev);
    const struct behavior_buzzer_config *cfg = dev->config;
    struct behavior_buzzer_data *data = dev->data;
    uint32_t freq = binding->param1;

    if (freq == 0) {
        return ZMK_BEHAVIOR_OPAQUE;
    }

    uint32_t period = NSEC_PER_SEC / freq;
    int err = pwm_set(cfg->pwm.dev, cfg->pwm.channel, period, period / 2, cfg->pwm.flags);
    if (err) {
        LOG_ERR("Failed to start buzzer (%d)", err);
        return err;
    }

    k_work_reschedule(&data->stop_work, K_MSEC(cfg->duration_ms));

    return ZMK_BEHAVIOR_OPAQUE;
}

static int on_buzzer_binding_released(struct zmk_behavior_binding *binding,
                                      struct zmk_behavior_binding_event event) {
    ARG_UNUSED(binding);
    ARG_UNUSED(event);

    return ZMK_BEHAVIOR_OPAQUE;
}

static int behavior_buzzer_init(const struct device *dev) {
    const struct behavior_buzzer_config *cfg = dev->config;
    struct behavior_buzzer_data *data = dev->data;

    if (!pwm_is_ready_dt(&cfg->pwm)) {
        LOG_ERR("Buzzer PWM device is not ready");
        return -ENODEV;
    }

    data->dev = dev;
    k_work_init_delayable(&data->stop_work, buzzer_stop_handler);
    buzzer_silence(cfg);

    return 0;
}

static const struct behavior_driver_api behavior_buzzer_driver_api = {
    .binding_pressed = on_buzzer_binding_pressed,
    .binding_released = on_buzzer_binding_released,
};

#define BUZZER_INST(n)                                                                             \
    static struct behavior_buzzer_data behavior_buzzer_data_##n;                                   \
    static const struct behavior_buzzer_config behavior_buzzer_config_##n = {                      \
        .pwm = PWM_DT_SPEC_INST_GET(n),                                                            \
        .duration_ms = DT_INST_PROP(n, duration_ms),                                               \
    };                                                                                             \
    BEHAVIOR_DT_INST_DEFINE(n, behavior_buzzer_init, NULL, &behavior_buzzer_data_##n,              \
                            &behavior_buzzer_config_##n, POST_KERNEL,                              \
                            CONFIG_ZMK_BEHAVIOR_BUZZER_INIT_PRIORITY, &behavior_buzzer_driver_api);

DT_INST_FOREACH_STATUS_OKAY(BUZZER_INST)

#endif

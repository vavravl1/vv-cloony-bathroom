#include "application.h"
#include "vv_radio.h"

#include <bc_led.h>
#include <bc_button.h>
#include <bcl.h>

void application_init(void) {
    initialize_led();
    initialize_button();
    initialize_radio();
    initialize_tags();
    initialize_external_relay();

    application.humidity_large_start = BC_TICK_INFINITY;
    application.humidity_large_stop =  BC_TICK_INFINITY;
}

void application_task(void) {
    adjust_air_ventilation();
    bc_scheduler_plan_current_relative(1000);
}

static void initialize_led() {
    bc_led_init(&application.builtin_led, BC_GPIO_LED, false, false);
    bc_led_set_mode(&application.builtin_led, BC_LED_MODE_OFF);

    bc_led_init(&application.external_led, BC_GPIO_P1, false, false);
    bc_led_set_mode(&application.external_led, BC_LED_MODE_OFF);

    bc_led_pulse(&application.builtin_led, 3000);
}

static void initialize_button() {
    bc_button_init(&application.button, BC_GPIO_P0, BC_GPIO_PULL_UP, true);
    bc_button_set_event_handler(&application.button, button_callback, NULL);

    bc_gpio_init(BC_GPIO_P4);
    bc_gpio_set_mode(BC_GPIO_P4, BC_GPIO_MODE_INPUT);
    bc_gpio_set_pull(BC_GPIO_P4, BC_GPIO_PULL_UP);
}

static void initialize_radio() {
    bc_radio_init(BC_RADIO_MODE_NODE_LISTENING);
    bc_radio_pairing_request("bathroom-controller-2", "1.0.0");
    bc_radio_set_event_handler(radio_callback, NULL);
}

static void initialize_tags() {
    bc_tag_temperature_init(&application.temperature_tag, BC_I2C_I2C0, BC_TAG_TEMPERATURE_I2C_ADDRESS_ALTERNATE);
    bc_tag_temperature_set_update_interval(&application.temperature_tag, 1000);
    bc_tag_temperature_set_event_handler(&application.temperature_tag, temperature_tag_callback, NULL);

    bc_tag_humidity_init(&application.humidity_tag, BC_TAG_HUMIDITY_REVISION_R3, BC_I2C_I2C0,
                         BC_TAG_HUMIDITY_I2C_ADDRESS_DEFAULT);
    bc_tag_humidity_set_update_interval(&application.humidity_tag, 1000);
    bc_tag_humidity_set_event_handler(&application.humidity_tag, humidity_tag_callback,
                                      application.humidity_tag._event_param);
}


static void button_callback(bc_button_t *b, bc_button_event_t e, void *p) {
    if (e == BC_BUTTON_EVENT_PRESS) {
        bc_led_pulse(&application.builtin_led, 100);
        bc_led_pulse(&application.external_led, 100);
        static uint16_t one = 1;
        bc_radio_pub_push_button(&one);
    }
}

static void humidity_tag_callback(bc_tag_humidity_t *tag, bc_tag_humidity_event_t event, void *param) {
    float humidity;
    if (bc_tag_humidity_get_humidity_percentage(tag, &humidity)) {
        bc_radio_pub_humidity(0, &humidity);
        if (humidity > 75) {
            if(application.humidity_large_start == BC_TICK_INFINITY) {
                application.humidity_large_start = bc_scheduler_get_spin_tick();
            }
            application.humidity_large_stop = BC_TICK_INFINITY;
        } else if(humidity < 65) {
            if(application.humidity_large_stop == BC_TICK_INFINITY) {
                application.humidity_large_stop = bc_scheduler_get_spin_tick();
            }
            application.humidity_large_start = BC_TICK_INFINITY;
        }
    }
}

static void temperature_tag_callback(bc_tag_temperature_t *tag, bc_tag_temperature_event_t event, void *paramt) {
    float temperature;
    if (bc_tag_temperature_get_temperature_celsius(tag, &temperature)) {
        bc_radio_pub_temperature(BC_RADIO_PUB_CHANNEL_R1_I2C0_ADDRESS_DEFAULT, &temperature);
    }
}

static void initialize_external_relay() {
    bc_gpio_init(BC_GPIO_P8);
    bc_gpio_set_mode(BC_GPIO_P8, BC_GPIO_MODE_OUTPUT);
    bc_gpio_set_output(BC_GPIO_P8, false);
}

static void adjust_air_ventilation() {
    bool ventilation_state = (bool)bc_gpio_get_output(BC_GPIO_P8);
    bool humidity_ventilation_state = false;

    if (application.humidity_large_start != BC_TICK_INFINITY) {
        humidity_ventilation_state = ventilation_state ||
                (bc_scheduler_get_spin_tick() > application.humidity_large_start + 10000);
    }

    if (application.humidity_large_stop != BC_TICK_INFINITY) {
        humidity_ventilation_state = ventilation_state &&
                (bc_scheduler_get_spin_tick() < application.humidity_large_stop + 10000);
    }

    bool external_button = !bc_gpio_get_input(BC_GPIO_P4);
    ventilation_state = humidity_ventilation_state || application.external_ventilation_request || external_button;
    bc_gpio_set_output(BC_GPIO_P8, ventilation_state);
    bc_radio_pub_bool("ventilation/-/state", &ventilation_state);
}

static void radio_callback(bc_radio_event_t event, void *param) {
    bc_led_set_mode(&application.builtin_led, BC_LED_MODE_OFF);

    if (event == BC_RADIO_EVENT_ATTACH) {
        bc_led_pulse(&application.builtin_led, 1000);
    } else if (event == BC_RADIO_EVENT_DETACH) {
        bc_led_pulse(&application.builtin_led, 1000);
    } else if (event == BC_RADIO_EVENT_INIT_DONE) {
        application.my_id = bc_radio_get_my_id();
    }
}

void bc_radio_pub_on_buffer(uint64_t *peer_device_address, uint8_t *buffer, size_t length) {
    switch (buffer[0]) {
        case VV_RADIO_STRING_STRING: {
            struct vv_radio_string_string_packet packet;
            if(vv_radio_parse_incoming_string_buffer(length, buffer, &packet)) {
                if (packet.device_address == application.my_id) {
                    bc_led_pulse(&application.builtin_led, 100);
                    process_incoming_string_packet(&packet);
                }
            }
            break;
        }
        default: {
            break;
        }
    }
}

static void process_incoming_string_packet(struct vv_radio_string_string_packet *packet) {
    if(strncmp(packet->key, "vent", 4) == 0 && strncmp(packet->value, "on", 2) == 0) {
        application.external_ventilation_request = true;
    } else if(strncmp(packet->key, "vent", 4) == 0 && strncmp(packet->value, "off", 3) == 0) {
        application.external_ventilation_request = false;
    }
}

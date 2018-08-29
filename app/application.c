#include <application.h>
#include <bc_led.h>
#include <bc_button.h>
#include <bcl.h>

// LED instance

void application_init(void) {
    initialize_led();
    initialize_button();
    initialize_radio();
    initialize_tags();
    initialize_external_relay();
}

void application_task(void) {
//    bc_i2c_transfer_t transfer;
//    uint8_t rx_buffer[5];
//    memset(rx_buffer, 0, sizeof(uint8_t) * 5);
//
//    transfer.device_address = 0xC9;
//    transfer.buffer = rx_buffer;
//    transfer.length = sizeof(uint8_t) * 5;
//
//    for (uint8_t address = 0; address < 255; address++) {
//        transfer.device_address = address;
//
//        if (bc_i2c_read(BC_I2C_I2C0, &transfer)) {
//            int to_send = address;
//            bc_radio_pub_int("dth21-address", &to_send);
//        }
//    }

//    if (bc_i2c_read(BC_I2C_I2C0, &transfer)) {
//        for(uint8_t i = 0; i < 5; i++) {
//            int val = rx_buffer[i];
//            bc_radio_pub_int("dth21-buffer", &val);
//        }

//        int scaleValue = rx_buffer[3] & 0x7F; //B01111111;
//        int signValue = rx_buffer[3] & 0x80; //B10000000;
//        float temperature =  (rx_buffer[2] + (float) scaleValue / 10);
//        if (signValue) { // negative temperature
//            temperature = -temperature;
//        }
//        bc_radio_pub_float("dth21-temperature", &temperature);
//    } else {
//        bc_led_set_mode(&builtin_led, BC_LED_MODE_BLINK_FAST);
//    }

    adjust_air_ventilation();
    bc_scheduler_plan_current_relative(1000);
}

static void initialize_led() {
    bc_led_init(&application.builtin_led, BC_GPIO_LED, false, false);
    bc_led_set_mode(&application.builtin_led, BC_LED_MODE_OFF);

    bc_led_init(&application.external_led, BC_GPIO_P1, false, false);
    bc_led_set_mode(&application.external_led, BC_LED_MODE_OFF);
}


static void initialize_button() {
    bc_button_init(&application.button, BC_GPIO_P0, BC_GPIO_PULL_UP, true);
    bc_button_set_event_handler(&application.button, button_callback, NULL);
}

static void initialize_radio() {
    bc_radio_init(BC_RADIO_MODE_NODE_LISTENING);
    bc_radio_pairing_request("bathroom-controller", "0.0.1");
}

static void initialize_tags() {
    bc_tag_temperature_init(&application.temperature_tag, BC_I2C_I2C0, BC_TAG_TEMPERATURE_I2C_ADDRESS_ALTERNATE);
    bc_tag_temperature_set_update_interval(&application.temperature_tag, 1000);
    bc_tag_temperature_set_event_handler(&application.temperature_tag, temperature_tag_callback, NULL);

    bc_tag_humidity_init(&application.humidity_tag, BC_TAG_HUMIDITY_REVISION_R2, BC_I2C_I2C0,
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
        if (humidity > 65) {
            application.humidity_too_large_timestamp = bc_scheduler_get_spin_tick();
        } else {
            application.humidity_too_large_timestamp = BC_TICK_INFINITY;
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
    if (application.humidity_too_large_timestamp == BC_TICK_INFINITY) {
        bc_gpio_set_output(BC_GPIO_P8, false);
    } else {
        bool humidity_too_large_for_too_long = bc_scheduler_get_spin_tick() >
                                               application.humidity_too_large_timestamp + 10000;
        bc_gpio_set_output(BC_GPIO_P8, humidity_too_large_for_too_long);
    }
}
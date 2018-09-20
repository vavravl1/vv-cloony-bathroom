#ifndef _APPLICATION_H
#define _APPLICATION_H

#ifndef VERSION
#define VERSION "vdev"
#endif

#include <bcl.h>
#include "vv_radio.h"

struct {
    bc_led_t builtin_led;
    bc_led_t external_led;
    bc_button_t button;

    bc_tag_humidity_t humidity_tag;
    bc_tag_temperature_t temperature_tag;

    bc_tick_t humidity_large_start;
    bc_tick_t humidity_large_stop;
    bc_tick_t last_humidity_pub;

    uint64_t my_id;

    bool external_ventilation_request;
} application;

static void initialize_led();
static void initialize_button();
static void initialize_radio();
static void initialize_tags();
static void initialize_external_relay();

static void adjust_air_ventilation();

static void process_incoming_string_packet(struct vv_radio_string_string_packet *);

static void radio_callback(bc_radio_event_t, void *);
static void button_callback(bc_button_t *, bc_button_event_t, void *);
static void humidity_tag_callback(bc_tag_humidity_t *, bc_tag_humidity_event_t, void *);
static void temperature_tag_callback(bc_tag_temperature_t *, bc_tag_temperature_event_t, void *);

void pub_humidity_in_time(float *humidity);

#endif // _APPLICATION_H

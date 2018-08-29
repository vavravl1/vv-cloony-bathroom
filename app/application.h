#ifndef _APPLICATION_H
#define _APPLICATION_H

#ifndef VERSION
#define VERSION "vdev"
#endif

#include <bcl.h>

struct {
    bc_led_t builtin_led;
    bc_led_t external_led;
    bc_button_t button;

    bc_tag_humidity_t humidity_tag;
    bc_tag_temperature_t temperature_tag;

    bc_tick_t humidity_too_large_timestamp;
} application;

static void initialize_led();
static void initialize_button();
static void initialize_radio();
static void initialize_tags();
static void initialize_external_relay();

static void adjust_air_ventilation();

static void button_callback(bc_button_t *, bc_button_event_t, void *);
static void humidity_tag_callback(bc_tag_humidity_t *, bc_tag_humidity_event_t, void *);
static void temperature_tag_callback(bc_tag_temperature_t *, bc_tag_temperature_event_t, void *);

#endif // _APPLICATION_H

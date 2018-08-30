
#ifndef VV_RADIO_H
#define VV_RADIO_H

#include <bc_common.h>
#include <bcl.h>

#define VV_RADIO_SINGLE_FLOAT 0xf0
#define VV_RADIO_STRING_STRING 0xf1

// [BUFFER_TYPE NODE_ADDRESS DATA_TYPE NEW_VALUE]
#define VV_RADIO_TYPE             (0)
#define VV_RADIO_ADDRESS          (VV_RADIO_TYPE + sizeof(uint8_t))
#define VV_RADIO_DATA_TYPE        (VV_RADIO_ADDRESS + sizeof(uint64_t))
#define VV_RADIO_VALUE            (VV_RADIO_DATA_TYPE + sizeof(uint8_t))
#define VV_RADIO_MESSAGE_SIZE     (VV_RADIO_VALUE + sizeof(float))


// [BUFFER_TYPE NODE_ADDRESS KEY VALUE]
#define VV_RADIO_STRING_KEY_SIZE (10)
#define VV_RADIO_STRING_VALUE_SIZE (20)
 
#define VV_RADIO_STRING_TYPE             (0)
#define VV_RADIO_STRING_ADDRESS          (VV_RADIO_STRING_TYPE + sizeof(uint8_t))
#define VV_RADIO_STRING_KEY              (VV_RADIO_STRING_ADDRESS + sizeof(uint64_t))
#define VV_RADIO_STRING_VALUE            (VV_RADIO_STRING_KEY + sizeof(char[VV_RADIO_STRING_KEY_SIZE]))
#define VV_RADIO_STRING_MESSAGE_SIZE     (VV_RADIO_STRING_VALUE + sizeof(char[VV_RADIO_STRING_VALUE_SIZE]))

struct vv_radio_single_float_packet {
    uint64_t device_address; // When is Gateway->Node then this is addressee's address. When from Node->Gateway then is it source address
    uint8_t type;
    float value;
};

struct vv_radio_string_string_packet {
    uint64_t device_address; // When is Gateway->Node then this is addressee's address. When from Node->Gateway then is it source address
    char key[VV_RADIO_STRING_KEY_SIZE];
    char value[VV_RADIO_STRING_VALUE_SIZE];
};

void vv_radio_listening_init();

void vv_radio_parse_incoming_buffer(size_t length, uint8_t *buffer, struct vv_radio_single_float_packet *target);
void vv_radio_send_update(struct vv_radio_single_float_packet *source);


bool vv_radio_parse_incoming_string_buffer(size_t length, const uint8_t *buffer, struct vv_radio_string_string_packet *target);
void vv_radio_send_string(struct vv_radio_string_string_packet *source);

#endif


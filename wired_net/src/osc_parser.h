#ifndef OSC_H
#define OSC_H

#include <stdint.h>

#define OSC_MAX_ARGS 8

typedef enum {
    OSC_ARG_INT32,
    OSC_ARG_FLOAT32,
    OSC_ARG_STRING,
} osc_arg_type_t;

typedef struct {
    osc_arg_type_t type;
    union {
        int32_t i;
        float f;
        const char *s;
    } value;
} osc_arg_t;

typedef struct {
    char address[64];
    int arg_count;
    osc_arg_t args[OSC_MAX_ARGS];
} osc_message_t;

// Parses a raw incoming OSC packet. Returns 1 on success, 0 on malformed/unsupported input.
int osc_parse_message(const uint8_t *buf, int len, osc_message_t *msg);

// Builds a minimal outgoing OSC message with a single int32 arg (used for relay/chain triggers).
// Returns the byte length written, or -1 if out_buf_size is too small.
int osc_build_int_message(uint8_t *out_buf, int out_buf_size, const char *address, int32_t value);

#endif
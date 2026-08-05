#include "osc_parser.h"
#include <string.h>

static int pad4(int len) {
    return ((len / 4) + 1) * 4;
}

int osc_parse_message(const uint8_t *buf, int len, osc_message_t *msg) {
    if (len < 4 || buf[0] != '/') return 0;

    int addr_len = strnlen((const char *)buf, len);
    if (addr_len == 0 || addr_len >= (int)sizeof(msg->address)) return 0;

    memcpy(msg->address, buf, addr_len);
    msg->address[addr_len] = '\0';

    int addr_block = pad4(addr_len);
    if (addr_block >= len) return 0;

    const uint8_t *type_tags = buf + addr_block;
    int remaining = len - addr_block;
    if (remaining < 1 || type_tags[0] != ',') return 0;

    int tag_len = strnlen((const char *)type_tags, remaining);
    int tag_block = pad4(tag_len);
    if (tag_block > remaining) return 0;

    const char *tags = (const char *)type_tags + 1; // skip leading ','
    int num_tags = tag_len - 1;
    if (num_tags > OSC_MAX_ARGS) num_tags = OSC_MAX_ARGS;
    if (num_tags < 0) num_tags = 0;

    const uint8_t *arg_ptr = buf + addr_block + tag_block;
    const uint8_t *buf_end = buf + len;

    msg->arg_count = 0;
    for (int i = 0; i < num_tags; i++) {
        char t = tags[i];
        osc_arg_t *arg = &msg->args[msg->arg_count];

        if (t == 'i') {
            if (arg_ptr + 4 > buf_end) break;
            int32_t val = (arg_ptr[0] << 24) | (arg_ptr[1] << 16) | (arg_ptr[2] << 8) | arg_ptr[3];
            arg->type = OSC_ARG_INT32;
            arg->value.i = val;
            arg_ptr += 4;
        } else if (t == 'f') {
            if (arg_ptr + 4 > buf_end) break;
            uint32_t raw = (arg_ptr[0] << 24) | (arg_ptr[1] << 16) | (arg_ptr[2] << 8) | arg_ptr[3];
            float f;
            memcpy(&f, &raw, 4);
            arg->type = OSC_ARG_FLOAT32;
            arg->value.f = f;
            arg_ptr += 4;
        } else if (t == 's') {
            int s_len = strnlen((const char *)arg_ptr, buf_end - arg_ptr);
            int s_block = pad4(s_len);
            if (arg_ptr + s_block > buf_end) break;
            arg->type = OSC_ARG_STRING;
            arg->value.s = (const char *)arg_ptr;
            arg_ptr += s_block;
        } else {
            break; // unsupported type tag — stop parsing further args
        }

        msg->arg_count++;
    }

    return 1;
}

int osc_build_int_message(uint8_t *out_buf, int out_buf_size, const char *address, int32_t value) {
    int addr_len = strlen(address);
    int addr_block = pad4(addr_len);
    int total = addr_block + 4 /* ",i\0\0" */ + 4 /* int32 */;
    if (total > out_buf_size) return -1;

    memset(out_buf, 0, total);
    memcpy(out_buf, address, addr_len);

    out_buf[addr_block]     = ',';
    out_buf[addr_block + 1] = 'i';

    uint8_t *arg = out_buf + addr_block + 4;
    arg[0] = (value >> 24) & 0xFF;
    arg[1] = (value >> 16) & 0xFF;
    arg[2] = (value >> 8) & 0xFF;
    arg[3] = value & 0xFF;

    return total;
}
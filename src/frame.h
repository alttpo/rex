
#ifndef REX_FRAME_H
#define REX_FRAME_H

#include <stdint.h>
#include <stdbool.h>

typedef uint8_t u8;

#define BUFFER_SIZE 64

typedef struct {
    u8 data[1+BUFFER_SIZE];
    u8 index;
    void (*deliver)(u8 len, const u8 *data);
} Frame;

bool frame_init(Frame *frame, void (*deliver)(u8 len, const u8 *data));

void frame_deliver(Frame *frame);

void frame_set_final(Frame *frame, bool isFinal);

void frame_append_u8(Frame *frame, u8 byte);

void frame_append_buf(Frame *frame, unsigned len, const u8 *bytes);

#endif //REX_FRAME_H

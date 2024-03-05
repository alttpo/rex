#include <assert.h>
#include "frame.h"

bool frame_init(Frame *frame, void (*deliver)(u8 len, const u8 *data)) {
    if (!frame) {
        return false;
    }
    if (!deliver) {
        return false;
    }

    frame->data[0] = 0; // Initialize header
    frame->index = 0;
    frame->deliver = deliver;

    return true;
}

void frame_deliver(Frame *frame) {
    u8 len = (frame->data[0] & 127);
    assert (len <= 64);
    frame->deliver(1 + len, frame->data);
    frame->data[0] = 0; // Reset header for next data
    frame->index = 0;
}

void frame_set_final(Frame *frame, bool isFinal) {
    frame->data[0] = ((u8)(isFinal&1) << 7) | (frame->data[0] & 0x7F);
}

static void frame_set_length(Frame *frame, u8 len) {
    assert (len <= 64);
    frame->data[0] = (frame->data[0] & 0x80) | (len & 0x7F);
}

void frame_append_u8(Frame *frame, u8 byte) {
    frame->data[1 + frame->index++] = byte;
    if (frame->index == BUFFER_SIZE) {
        frame_set_length(frame, BUFFER_SIZE);
        frame_deliver(frame);
    } else {
        frame_set_length(frame, frame->index);
    }
}

void frame_append_buf(Frame *frame, unsigned len, const u8 *bytes) {
    while (len > 0) {
        for (; len > 0 && frame->index < BUFFER_SIZE; frame->index++, len--) {
            frame->data[1 + frame->index] = *bytes++;
        }
        if (frame->index == BUFFER_SIZE) {
            frame_set_length(frame, BUFFER_SIZE);
            frame_deliver(frame);
        } else {
            frame_set_length(frame, frame->index);
        }
    }
}


#ifndef REX_FRAME_H
#define REX_FRAME_H

#include <stdint.h>
#include <stdbool.h>

typedef uint8_t u8;

enum frame_error {
    FRAME_ERR_SUCCESS,
    FRAME_ERR_NULL_FRAME_ARG,
    FRAME_ERR_NULL_FRAME_DELIVERER,
    FRAME_ERR_NULL_BUFFER_ARG,
    FRAME_ERR_DELIVERY_FAILURE
};

// frame-delivery delegate:
typedef struct {
    void *ctx;
    int (*deliver_frame)(void *ctx, unsigned len, const u8 *data);
} FrameDeliverer;

typedef struct {
    // current frame including header byte at data[0]:
    u8 data[64];

    // index into data[] of next write:
    unsigned index;

    // delegate to invoke when delivering a frame:
    FrameDeliverer deliverer;
} Frame;

enum frame_error frame_init(Frame *f, FrameDeliverer deliverer);

enum frame_error frame_reset(Frame *f);

enum frame_error frame_deliver(Frame *f);

enum frame_error frame_set_final(Frame *f, bool isFinal);

enum frame_error frame_set_delimiter(Frame *f, u8 delim);

enum frame_error frame_append_u8(Frame *f, u8 byte);

enum frame_error frame_append_buf(Frame *f, unsigned len, const u8 *bytes);

#endif //REX_FRAME_H

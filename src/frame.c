#include <assert.h>
#include "frame.h"

void internal_frame_reset(Frame *f) {
    assert(f);
    f->data[0] = 0;
    f->index = 1;
}

enum frame_error frame_init(Frame *f, FrameDeliverer deliverer) {
    if (!f) {
        return FRAME_ERR_NULL_FRAME_ARG;
    }
    if (!deliverer.deliver_frame) {
        return FRAME_ERR_NULL_FRAME_DELIVERER;
    }

    f->deliverer = deliverer;
    internal_frame_reset(f);

    return FRAME_ERR_SUCCESS;
}

enum frame_error frame_reset(Frame *f) {
    if (!f) {
        return FRAME_ERR_NULL_FRAME_ARG;
    }

    internal_frame_reset(f);

    return FRAME_ERR_SUCCESS;
}

enum frame_error frame_deliver(Frame *f) {
    unsigned len;

    if (!f) {
        return FRAME_ERR_NULL_FRAME_ARG;
    }

    if (f->data[0] & 0x40) {
        // delimiter:
        len = 0;
    } else {
        // frame data:
        len = (f->data[0] & 63);
    }

    assert(len < 64);

    // attempt delivery:
    enum frame_error ret = f->deliverer.deliver_frame(f->deliverer.ctx, 1 + len, f->data);

    internal_frame_reset(f);

    return ret;
}

enum frame_error frame_set_final(Frame *f, bool isFinal) {
    if (!f) {
        return FRAME_ERR_NULL_FRAME_ARG;
    }

    f->data[0] = ((u8)(isFinal & 1) << 7) | (f->data[0] & 0x7F);

    return FRAME_ERR_SUCCESS;
}

enum frame_error frame_set_delimiter(Frame *f, u8 delim) {
    if (!f) {
        return FRAME_ERR_NULL_FRAME_ARG;
    }

    // set delimiter bit; set delimiter value; maintain isFinal bit:
    f->data[0] = (delim & 0x3F) | (0x40) | (f->data[0] & 0x80);

    return FRAME_ERR_SUCCESS;
}

static void frame_set_length(Frame *f, unsigned len) {
    assert(f);
    assert(len < 64);

    f->data[0] = (f->data[0] & (0x80 | 0x40)) | (len & 0x3F);
}

static enum frame_error frame_attempt_delivery(Frame *f) {
    assert(f);
    assert(f->index >= 1);
    assert(f->index <= 64);

    if (f->index == 64) {
        frame_set_length(f, 64-1);
        return frame_deliver(f);
    } else {
        frame_set_length(f, f->index-1);
    }

    return FRAME_ERR_SUCCESS;
}

enum frame_error frame_append_u8(Frame *f, u8 byte) {
    f->data[f->index++] = byte;
    return frame_attempt_delivery(f);
}

enum frame_error frame_append_buf(Frame *f, unsigned len, const u8 *bytes) {
    if (!f) {
        return FRAME_ERR_NULL_FRAME_ARG;
    }
    if (!bytes) {
        return FRAME_ERR_NULL_BUFFER_ARG;
    }

    enum frame_error ret;

    while (len > 0) {
        for (; (len > 0) && (f->index < 64); len--) {
            f->data[f->index++] = *bytes++;
        }

        ret = frame_attempt_delivery(f);
        if (ret != FRAME_ERR_SUCCESS) {
            return ret;
        }
    }

    return FRAME_ERR_SUCCESS;
}

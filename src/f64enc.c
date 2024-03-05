#include <assert.h>
#include "f64enc.h"

void f64enc_reset_internal(f64enc *f) {
    assert(f);
    f->data[0] = 0;
    f->index = 1;
}

enum f64enc_error f64enc_init(f64enc *f, f64enc_deliverer deliverer) {
    if (!f) {
        return F64ENC_ERR_NULL_FRAME_ARG;
    }
    if (!deliverer.deliver_frame) {
        return F64ENC_ERR_NULL_FRAME_DELIVERER;
    }

    f->deliverer = deliverer;
    f64enc_reset_internal(f);

    return F64ENC_ERR_SUCCESS;
}

enum f64enc_error f64enc_reset(f64enc *f) {
    if (!f) {
        return F64ENC_ERR_NULL_FRAME_ARG;
    }

    f64enc_reset_internal(f);

    return F64ENC_ERR_SUCCESS;
}

enum f64enc_error f64enc_deliver(f64enc *f) {
    unsigned len;

    if (!f) {
        return F64ENC_ERR_NULL_FRAME_ARG;
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
    enum f64enc_error ret = f->deliverer.deliver_frame(f->deliverer.ctx, 1 + len, f->data);

    f64enc_reset_internal(f);

    return ret;
}

enum f64enc_error f64enc_set_final(f64enc *f, bool isFinal) {
    if (!f) {
        return F64ENC_ERR_NULL_FRAME_ARG;
    }

    f->data[0] = ((u8)(isFinal & 1) << 7) | (f->data[0] & 0x7F);

    return F64ENC_ERR_SUCCESS;
}

enum f64enc_error f64enc_set_delimiter(f64enc *f, u8 delim) {
    if (!f) {
        return F64ENC_ERR_NULL_FRAME_ARG;
    }

    // set delimiter bit; set delimiter value; maintain isFinal bit:
    f->data[0] = (delim & 0x3F) | (0x40) | (f->data[0] & 0x80);

    return F64ENC_ERR_SUCCESS;
}

static void f64enc_set_length(f64enc *f, unsigned len) {
    assert(f);
    assert(len < 64);

    f->data[0] = (f->data[0] & (0x80 | 0x40)) | (len & 0x3F);
}

static enum f64enc_error f64enc_attempt_delivery(f64enc *f) {
    assert(f);
    assert(f->index >= 1);
    assert(f->index <= 64);

    if (f->index == 64) {
        f64enc_set_length(f, 64 - 1);
        return f64enc_deliver(f);
    } else {
        f64enc_set_length(f, f->index - 1);
    }

    return F64ENC_ERR_SUCCESS;
}

enum f64enc_error f64enc_append_u8(f64enc *f, u8 byte) {
    f->data[f->index++] = byte;
    return f64enc_attempt_delivery(f);
}

enum f64enc_error f64enc_append_buf(f64enc *f, unsigned len, const u8 *bytes) {
    if (!f) {
        return F64ENC_ERR_NULL_FRAME_ARG;
    }
    if (!bytes) {
        return F64ENC_ERR_NULL_BUFFER_ARG;
    }

    enum f64enc_error ret;

    while (len > 0) {
        for (; (len > 0) && (f->index < 64); len--) {
            f->data[f->index++] = *bytes++;
        }

        ret = f64enc_attempt_delivery(f);
        if (ret != F64ENC_ERR_SUCCESS) {
            return ret;
        }
    }

    return F64ENC_ERR_SUCCESS;
}

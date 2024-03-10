#include <assert.h>
#include "f64dec.h"

enum f64dec_error f64dec_init(f64dec *f, f64dec_reader reader, f64dec_consumer consumer) {
    if (!f) {
        return F64DEC_ERR_NULL_FRAME_ARG;
    }
    if (!reader.read) {
        return F64DEC_ERR_NULL_READER;
    }
    if (!consumer.data) {
        return F64DEC_ERR_NULL_CONSUMER_DATA;
    }
    if (!consumer.final) {
        return F64DEC_ERR_NULL_CONSUMER_FINAL;
    }
    if (!consumer.delimit) {
        return F64DEC_ERR_NULL_CONSUMER_DELIMIT;
    }

    f->reader = reader;
    f->consumer = consumer;

    return f64dec_reset(f);
}

enum f64dec_error f64dec_reset(f64dec *f) {
    f->index = 0;
    f->data[0] = 0;

    return F64DEC_ERR_SUCCESS;
}

enum f64dec_error f64dec_read(f64dec *f) {
    int ret;
    int n;
    int expect_len;
    int frame_len;
    u8 header;

    if (f->index == 0) {
        // read the header byte:
        n = f->reader.read(f->reader.ctx, 1, &f->data[0]);
        if (n < 0) {
            return n;
        }
        // no data:
        if (n == 0) {
            return F64DEC_ERR_READ_NO_DATA;
        }
        // too much?
        if (n > 1) {
            return F64DEC_ERR_READ_TOO_MUCH;
        }

        f->index = 1;

        // handle header:
        header = f->data[0];
        frame_len = (header & 0x3F);
        if (header & 0x40) {
            // handle delimiter:
            ret = f->consumer.delimit(f->consumer.ctx);
            if (ret) {
                return ret;
            }
        }

        // handle zero-byte frame:
        if (frame_len == 0) {
            if (header & 0x80) {
                // final frame:
                ret = f->consumer.final(f->consumer.ctx);
                if (ret) {
                    return ret;
                }
            }

            // reset for next frame:
            return f64dec_reset(f);
        }
    } else {
        // parse header:
        header = f->data[0];
        frame_len = (header & 0x3F);
    }

    // read the remaining frame length:
    expect_len = frame_len - (f->index - 1);

    assert(expect_len > 0);

    n = f->reader.read(f->reader.ctx, expect_len, &f->data[f->index]);
    if (n < 0) {
        return n;
    }
    // no data:
    if (n == 0) {
        return F64DEC_ERR_READ_NO_DATA;
    }
    // too much?
    if (n > expect_len) {
        return F64DEC_ERR_READ_TOO_MUCH;
    }

    assert(n + f->index < 64);

    f->index += n;

    // data available:
    if ((f->index - 1) == frame_len) {
        // all data received:
        ret = f->consumer.data(f->consumer.ctx, frame_len, &f->data[1]);
        if (ret) {
            return ret;
        }

        if (header & 0x80) {
            // final frame:
            ret = f->consumer.final(f->consumer.ctx);
            if (ret) {
                return ret;
            }
        }

        // reset for next frame:
        return f64dec_reset(f);
    }

    return F64DEC_ERR_SUCCESS;
}

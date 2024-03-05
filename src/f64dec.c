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
    // determine how much to read:
    int expect_len;
    int frame_len;

    if (f->index == 0) {
        // read just the header byte:
        frame_len = 0;
        expect_len = 1;
    } else {
        // read the remaining frame length:
        assert((f->data[0] & 0x40) == 0);
        frame_len = (f->data[0] & 0x3F);
        expect_len = frame_len - (f->index - 1);
    }

    int n = f->reader.read(f->reader.ctx, expect_len, &f->data[f->index]);
    if (n < 0) {
        return n;
    }
    if (n > expect_len) {
        return F64DEC_ERR_READ_TOO_MUCH;
    }

    // no data available:
    if (n == 0) {
        return F64DEC_ERR_SUCCESS;
    }

    assert(n + f->index < 64);

    f->index += n;

    u8 header = f->data[0];

    if (f->index == 1) {
        // header byte is available:
        frame_len = header & 0x3F;

        if (header & 0x40) {
            // handle delimiter:
            u8 delimiter = frame_len;
            int ret = f->consumer.delimit(f->consumer.ctx, delimiter);
            if (ret) {
                return ret;
            }

            // final:
            if (header & 0x80) {
                ret = f->consumer.final(f->consumer.ctx);
                if (ret) {
                    return ret;
                }
            }

            // reset for next frame:
            return f64dec_reset(f);
        } else if ((header & 0x80) && (frame_len == 0)) {
            // final 0-byte frame:
            int ret = f->consumer.final(f->consumer.ctx);
            if (ret) {
                return ret;
            }

            // reset for next frame:
            return f64dec_reset(f);
        }
    } else if (f->index > 1) {
        // data available:
        if ((f->index - 1) == frame_len) {
            // all data received:
            int ret = f->consumer.data(f->consumer.ctx, frame_len, &f->data[1]);
            if (ret) {
                return ret;
            }

            if (header & 0x80) {
                // finalize:
                ret = f->consumer.final(f->consumer.ctx);
                if (ret) {
                    return ret;
                }
            }

            // reset for next frame:
            return f64dec_reset(f);
        }
    }

    return F64DEC_ERR_SUCCESS;
}

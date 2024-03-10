
#ifndef REX_F64ENC_H
#define REX_F64ENC_H

#include <stdint.h>
#include <stdbool.h>

typedef uint8_t u8;

enum f64enc_error {
    F64ENC_ERR_SUCCESS,
    F64ENC_ERR_NULL_FRAME_ARG = -1,
    F64ENC_ERR_NULL_FRAME_WRITER = -2,
    F64ENC_ERR_NULL_BUFFER_ARG = -3,
    F64ENC_ERR_DELIMITER_CANNOT_HAVE_DATA = -4,
    F64ENC_ERR_DELIMITER_MUST_BE_6_BIT = -5,
    F64ENC_ERR_ZERO_LENGTH = -6,
    F64ENC_ERR_WRITE_FAILURE = -7
};

// frame-delivery delegate:
typedef struct {
    void *ctx;
    int (*write_frame)(void *ctx, int len, const u8 *data);
} f64enc_writer;

typedef struct {
    // current frame including header byte at data[0]:
    u8 data[64];

    // index into data[] of next write:
    unsigned index;

    // delegate to invoke to write a frame to its final destination:
    f64enc_writer writer;
} f64enc;

enum f64enc_error f64enc_init(f64enc *f, f64enc_writer writer);

enum f64enc_error f64enc_reset(f64enc *f);

enum f64enc_error f64enc_write_delimiter(f64enc *f, u8 delim);

enum f64enc_error f64enc_set_final(f64enc *f, bool isFinal);

enum f64enc_error f64enc_append_u8(f64enc *f, u8 byte);

enum f64enc_error f64enc_append_buf(f64enc *f, unsigned len, const u8 *bytes);

enum f64enc_error f64enc_write(f64enc *f);

enum f64enc_error f64enc_write_zero(f64enc *f);

#endif //REX_F64ENC_H


#ifndef REX_F64DEC_H
#define REX_F64DEC_H

#include <stdint.h>
#include <stdbool.h>

typedef uint8_t u8;

enum f64dec_error {
    F64DEC_ERR_SUCCESS,
    F64DEC_ERR_NULL_FRAME_ARG = -1,
    F64DEC_ERR_NULL_READER = -2,
    F64DEC_ERR_NULL_CONSUMER_DATA = -3,
    F64DEC_ERR_NULL_CONSUMER_FINAL = -4,
    F64DEC_ERR_NULL_CONSUMER_DELIMIT = -5,
    F64DEC_ERR_READ_NO_DATA = -6,
    F64DEC_ERR_READ_TOO_MUCH = -7,
    F64DEC_ERR_READ_FAILURE = -8,
    F64DEC_ERR_CONSUMER_DATA_FAILURE = -9,
    F64DEC_ERR_CONSUMER_DELIMIT_FAILURE = -10,
    F64DEC_ERR_CONSUMER_FINAL_FAILURE = -11
};

typedef struct {
    void *ctx;
    int (*read)(void *ctx, int len, u8 *dest);
} f64dec_reader;

typedef struct {
    void *ctx;
    int (*data)(void *ctx, int len, const u8 *src);
    int (*delimit)(void *ctx);
    int (*final)(void *ctx);
} f64dec_consumer;

typedef struct {
    u8 data[64];

    // index into data[] where next read will write to:
    int index;

    // reader to read encoded frame data from:
    f64dec_reader reader;
    // consumer to deliver decoded message data to:
    f64dec_consumer consumer;
} f64dec;

enum f64dec_error f64dec_init(f64dec *f, f64dec_reader reader, f64dec_consumer consumer);

enum f64dec_error f64dec_reset(f64dec *f);

enum f64dec_error f64dec_read(f64dec *f);

#endif //REX_F64DEC_H

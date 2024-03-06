
#include <vector>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_range_equals.hpp>

extern "C" {
#include "f64dec.h"
#include "f64enc.h"
}

static auto frame_write(void *ctx, unsigned len, const u8 *data) -> int {
    auto v = ((std::vector<u8> *)ctx);
    v->insert(v->end(), data, data + len);
    return F64ENC_ERR_SUCCESS;
}

struct reader {
    explicit reader(std::vector<u8> &buf) : m_buf(buf) {}

    int read(int len, unsigned char *dest) {
        return 0;
    }

    std::vector<u8> &m_buf;
};

extern "C" int reader_read(void *ctx, int len, unsigned char *dest) {
    return static_cast<reader*>(ctx)->read(len, dest);
}

struct consumer {
    consumer() {
    }

    int data(int len, const u8 *src) {
        return 0;
    }
    int delimit(u8 delim) {
        return 0;
    }
    int final() {
        return 0;
    }
};

extern "C" int consumer_data(void *ctx, int len, const u8 *src) {
    return static_cast<consumer*>(ctx)->data(len, src);
}
extern "C" int consumer_delimit(void *ctx, u8 delim) {
    return static_cast<consumer*>(ctx)->delimit(delim);
}
extern "C" int consumer_final(void *ctx) {
    return static_cast<consumer*>(ctx)->final();
}

TEST_CASE( "f64enc to f64dec", "end-to-end" ) {
    SECTION( "1 byte frame", "append_u8" ) {
        f64enc e;
        f64dec d;

        std::vector<u8> framed;
        reader reader(framed);
        consumer consumer;

        REQUIRE( f64enc_init(&e, {&framed, frame_write}) == F64ENC_ERR_SUCCESS );
        REQUIRE( f64dec_init(&d, {&reader, reader_read}, {&consumer,consumer_data,consumer_delimit,consumer_final}) == F64DEC_ERR_SUCCESS );

        REQUIRE( f64enc_set_final(&e, true) == F64ENC_ERR_SUCCESS );
        REQUIRE( f64enc_append_u8(&e, 1) == F64ENC_ERR_SUCCESS );
        REQUIRE( f64enc_write(&e) == F64ENC_ERR_SUCCESS );
        REQUIRE( framed.size() == 2 );
        REQUIRE_THAT(framed, Catch::Matchers::RangeEquals(std::vector<u8>{ 0x81, 0x01 }));

        REQUIRE( f64dec_read(&d) == F64DEC_ERR_SUCCESS );

        // TODO: verify callbacks
    }
}


#include <vector>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_range_equals.hpp>

extern "C" {
#include "f64dec.h"
#include "f64enc.h"
}

using Catch::Matchers::RangeEquals;

struct e2ereader {
    explicit e2ereader(std::vector<u8> &buf) : m_buf(buf) {}

    int read(int len, unsigned char *dest) {
        if (len > m_buf.size()) {
            len = m_buf.size();
        }
        if (len == 0) {
            return 0;
        }

        auto end = m_buf.begin() + len;
        std::copy(m_buf.begin(), end, dest);
        m_buf.erase(m_buf.begin(), end);
        return len;
    }

    std::vector<u8> &m_buf;
};

struct e2econsumer {
    int data(int len, const u8 *src) {
        for (const u8* p = src; p < src + len; p++) {
            accum.push_back(*p);
        }
        return F64DEC_ERR_SUCCESS;
    }
    int delimit() {
        accum.push_back(-1);
        return F64DEC_ERR_SUCCESS;
    }
    int final() {
        accum.push_back(-2);
        return F64DEC_ERR_SUCCESS;
    }

    std::vector<int> accum;
};

extern "C" {

static auto e2eframe_write(void *ctx, int len, const u8 *data) -> int {
    auto v = ((std::vector<u8> *)ctx);
    v->insert(v->end(), data, data + len);
    return F64ENC_ERR_SUCCESS;
}

static int e2ereader_read(void *ctx, int len, unsigned char *dest) {
    return static_cast<e2ereader*>(ctx)->read(len, dest);
}

static int e2ereader_read1(void *ctx, int len, unsigned char *dest) {
    // limit reads to 1 byte at a time:
    if (len > 0) {
        len = 1;
    }

    return static_cast<e2ereader*>(ctx)->read(len, dest);
}

static int e2econsumer_data(void *ctx, int len, const u8 *src) {
    return static_cast<e2econsumer *>(ctx)->data(len, src);
}
static int e2econsumer_delimit(void *ctx) {
    return static_cast<e2econsumer *>(ctx)->delimit();
}
static int e2econsumer_final(void *ctx) {
    return static_cast<e2econsumer *>(ctx)->final();
}

}

TEST_CASE( "f64enc to f64dec", "end-to-end" ) {
    SECTION( "0 byte frame" ) {
        f64enc e;
        f64dec d;

        std::vector<u8> framed;
        e2ereader e2ereader(framed);
        e2econsumer e2econsumer;

        REQUIRE( f64enc_init(&e, {&framed, e2eframe_write}) == F64ENC_ERR_SUCCESS );
        REQUIRE( f64dec_init(&d, {&e2ereader, e2ereader_read}, {&e2econsumer,e2econsumer_data,e2econsumer_delimit,e2econsumer_final}) == F64DEC_ERR_SUCCESS );

        REQUIRE( f64enc_write_zero(&e) == F64ENC_ERR_SUCCESS );

        REQUIRE( framed.size() == 1 );
        REQUIRE_THAT( framed, RangeEquals(std::vector<u8>{ 0x00 }) );

        REQUIRE( f64dec_read(&d) == F64DEC_ERR_SUCCESS );
        REQUIRE( f64dec_read(&d) == F64DEC_ERR_READ_NO_DATA );

        REQUIRE_THAT( e2econsumer.accum, RangeEquals(std::vector<int>{ }) );
    }

    SECTION( "0 byte frame final" ) {
        f64enc e;
        f64dec d;

        std::vector<u8> framed;
        e2ereader e2ereader(framed);
        e2econsumer e2econsumer;

        REQUIRE( f64enc_init(&e, {&framed, e2eframe_write}) == F64ENC_ERR_SUCCESS );
        REQUIRE( f64dec_init(&d, {&e2ereader, e2ereader_read}, {&e2econsumer,e2econsumer_data,e2econsumer_delimit,e2econsumer_final}) == F64DEC_ERR_SUCCESS );

        REQUIRE( f64enc_set_final(&e, true) == F64ENC_ERR_SUCCESS );
        REQUIRE( f64enc_write_zero(&e) == F64ENC_ERR_SUCCESS );

        REQUIRE( framed.size() == 1 );
        REQUIRE_THAT( framed, RangeEquals(std::vector<u8>{ 0x80 }) );

        REQUIRE( f64dec_read(&d) == F64DEC_ERR_SUCCESS );
        REQUIRE( f64dec_read(&d) == F64DEC_ERR_READ_NO_DATA );

        REQUIRE_THAT( e2econsumer.accum, RangeEquals(std::vector<int>{ -2 }) );
    }

    SECTION( "1 byte frame", "append_u8" ) {
        f64enc e;
        f64dec d;

        std::vector<u8> framed;
        e2ereader e2ereader(framed);
        e2econsumer e2econsumer;

        REQUIRE( f64enc_init(&e, {&framed, e2eframe_write}) == F64ENC_ERR_SUCCESS );
        REQUIRE( f64dec_init(&d, {&e2ereader, e2ereader_read}, {&e2econsumer,e2econsumer_data,e2econsumer_delimit,e2econsumer_final}) == F64DEC_ERR_SUCCESS );

        REQUIRE( f64enc_set_final(&e, true) == F64ENC_ERR_SUCCESS );
        REQUIRE( f64enc_append_u8(&e, 1) == F64ENC_ERR_SUCCESS );
        REQUIRE( f64enc_write(&e) == F64ENC_ERR_SUCCESS );

        REQUIRE( framed.size() == 2 );
        REQUIRE_THAT( framed, RangeEquals(std::vector<u8>{ 0x81, 0x01 }) );

        REQUIRE( f64dec_read(&d) == F64DEC_ERR_SUCCESS );
        REQUIRE( f64dec_read(&d) == F64DEC_ERR_READ_NO_DATA );

        REQUIRE_THAT( e2econsumer.accum, RangeEquals(std::vector<int>{ 1, -2 }) );
    }

    SECTION( "2 byte frame - read1", "append_u8" ) {
        f64enc e;
        f64dec d;

        std::vector<u8> framed;
        e2ereader e2ereader(framed);
        e2econsumer e2econsumer;

        REQUIRE( f64enc_init(&e, {&framed, e2eframe_write}) == F64ENC_ERR_SUCCESS );
        // NOTE: only reading 1 byte per read() call:
        REQUIRE( f64dec_init(&d, {&e2ereader, e2ereader_read1}, {&e2econsumer,e2econsumer_data,e2econsumer_delimit,e2econsumer_final}) == F64DEC_ERR_SUCCESS );

        REQUIRE( f64enc_set_final(&e, true) == F64ENC_ERR_SUCCESS );
        REQUIRE( f64enc_append_u8(&e, 1) == F64ENC_ERR_SUCCESS );
        REQUIRE( f64enc_append_u8(&e, 2) == F64ENC_ERR_SUCCESS );
        REQUIRE( f64enc_write(&e) == F64ENC_ERR_SUCCESS );

        REQUIRE( framed.size() == 3 );
        REQUIRE_THAT( framed, RangeEquals(std::vector<u8>{ 0x82, 0x01, 0x02 }) );

        REQUIRE( f64dec_read(&d) == F64DEC_ERR_SUCCESS );
        REQUIRE( f64dec_read(&d) == F64DEC_ERR_SUCCESS );
        REQUIRE( f64dec_read(&d) == F64DEC_ERR_READ_NO_DATA );

        REQUIRE_THAT( e2econsumer.accum, RangeEquals(std::vector<int>{ 1, 2, -2 }) );
    }

    SECTION( "delimiter", "delimiter" ) {
        f64enc e;
        f64dec d;

        std::vector<u8> framed;
        e2ereader e2ereader(framed);
        e2econsumer e2econsumer;

        REQUIRE( f64enc_init(&e, {&framed, e2eframe_write}) == F64ENC_ERR_SUCCESS );
        REQUIRE( f64dec_init(&d, {&e2ereader, e2ereader_read}, {&e2econsumer,e2econsumer_data,e2econsumer_delimit,e2econsumer_final}) == F64DEC_ERR_SUCCESS );

        REQUIRE( f64enc_set_delimited(&e, true) == F64ENC_ERR_SUCCESS );
        REQUIRE( f64enc_write_zero(&e) == F64ENC_ERR_SUCCESS );

        REQUIRE( framed.size() == 1 );
        REQUIRE_THAT( framed, RangeEquals(std::vector<u8>{ 0x40 }) );

        REQUIRE( f64dec_read(&d) == F64DEC_ERR_SUCCESS );
        REQUIRE( f64dec_read(&d) == F64DEC_ERR_READ_NO_DATA );

        REQUIRE_THAT( e2econsumer.accum, RangeEquals(std::vector<int>{ -1 }) );
    }

    SECTION( "delimiter final msg", "delimiter" ) {
        f64enc e;
        f64dec d;

        std::vector<u8> framed;
        e2ereader e2ereader(framed);
        e2econsumer e2econsumer;

        REQUIRE( f64enc_init(&e, {&framed, e2eframe_write}) == F64ENC_ERR_SUCCESS );
        REQUIRE( f64dec_init(&d, {&e2ereader, e2ereader_read}, {&e2econsumer,e2econsumer_data,e2econsumer_delimit,e2econsumer_final}) == F64DEC_ERR_SUCCESS );

        REQUIRE( f64enc_set_delimited(&e, true) == F64ENC_ERR_SUCCESS );
        REQUIRE( f64enc_set_final(&e, true) == F64ENC_ERR_SUCCESS );
        REQUIRE( f64enc_write_zero(&e) == F64ENC_ERR_SUCCESS );

        REQUIRE( framed.size() == 1 );
        REQUIRE_THAT( framed, RangeEquals(std::vector<u8>{ 0xC0 }) );

        REQUIRE( f64dec_read(&d) == F64DEC_ERR_SUCCESS );
        REQUIRE( f64dec_read(&d) == F64DEC_ERR_READ_NO_DATA );

        REQUIRE_THAT( e2econsumer.accum, RangeEquals(std::vector<int>{ -1, -2 }) );
    }

    SECTION( "msg delimiter msg", "delimiter" ) {
        f64enc e;
        f64dec d;

        std::vector<u8> framed;
        e2ereader e2ereader(framed);
        e2econsumer e2econsumer;

        REQUIRE( f64enc_init(&e, {&framed, e2eframe_write}) == F64ENC_ERR_SUCCESS );
        REQUIRE( f64dec_init(&d, {&e2ereader, e2ereader_read}, {&e2econsumer,e2econsumer_data,e2econsumer_delimit,e2econsumer_final}) == F64DEC_ERR_SUCCESS );

        REQUIRE( f64enc_append_u8(&e, 1) == F64ENC_ERR_SUCCESS );
        REQUIRE( f64enc_write(&e) == F64ENC_ERR_SUCCESS );

        REQUIRE( f64enc_set_delimited(&e, true) == F64ENC_ERR_SUCCESS );
        REQUIRE( f64enc_set_final(&e, true) == F64ENC_ERR_SUCCESS );
        REQUIRE( f64enc_append_u8(&e, 2) == F64ENC_ERR_SUCCESS );
        REQUIRE( f64enc_write(&e) == F64ENC_ERR_SUCCESS );

        REQUIRE( framed.size() == 4 );
        REQUIRE_THAT( framed, RangeEquals(std::vector<u8>{ 0x01, 0x01, 0xC1, 0x02 }) );

        REQUIRE( f64dec_read(&d) == F64DEC_ERR_SUCCESS );
        REQUIRE( f64dec_read(&d) == F64DEC_ERR_SUCCESS );
        REQUIRE( f64dec_read(&d) == F64DEC_ERR_READ_NO_DATA );

        REQUIRE_THAT( e2econsumer.accum, RangeEquals(std::vector<int>{ 0x01, -1, 0x02, -2 }) );
    }
}

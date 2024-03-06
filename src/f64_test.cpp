
#include <vector>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_range_equals.hpp>

extern "C" {
#include "f64dec.h"
#include "f64enc.h"
}

using Catch::Matchers::RangeEquals;

static auto frame_write(void *ctx, unsigned len, const u8 *data) -> int {
    auto v = ((std::vector<u8> *)ctx);
    v->insert(v->end(), data, data + len);
    return F64ENC_ERR_SUCCESS;
}

struct reader {
    explicit reader(std::vector<u8> &buf) : m_buf(buf) {}

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

extern "C" int reader_read(void *ctx, int len, unsigned char *dest) {
    return static_cast<reader*>(ctx)->read(len, dest);
}

extern "C" int reader_read1(void *ctx, int len, unsigned char *dest) {
    // limit reads to 1 byte at a time:
    if (len > 0) {
        len = 1;
    }

    return static_cast<reader*>(ctx)->read(len, dest);
}

struct consumer {
    int data(int len, const u8 *src) {
        accum.insert(accum.end(), src, src+len);
        return F64DEC_ERR_SUCCESS;
    }
    int delimit(u8 delim) {
        accum.push_back(delim);
        return F64DEC_ERR_SUCCESS;
    }
    int final() {
        accum.push_back(0);
        return F64DEC_ERR_SUCCESS;
    }

    std::vector<u8> accum;
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
    SECTION( "0 byte frame" ) {
        f64enc e;
        f64dec d;

        std::vector<u8> framed;
        reader reader(framed);
        consumer consumer;

        REQUIRE( f64enc_init(&e, {&framed, frame_write}) == F64ENC_ERR_SUCCESS );
        REQUIRE( f64dec_init(&d, {&reader, reader_read}, {&consumer,consumer_data,consumer_delimit,consumer_final}) == F64DEC_ERR_SUCCESS );

        REQUIRE( f64enc_write_zero(&e) == F64ENC_ERR_SUCCESS );

        REQUIRE( framed.size() == 1 );
        REQUIRE_THAT( framed, RangeEquals(std::vector<u8>{ 0x00 }) );

        REQUIRE( f64dec_read(&d) == F64DEC_ERR_SUCCESS );
        REQUIRE( f64dec_read(&d) == F64DEC_ERR_READ_NO_DATA );

        REQUIRE_THAT( consumer.accum, RangeEquals(std::vector<u8>{ }) );
    }

    SECTION( "0 byte frame final" ) {
        f64enc e;
        f64dec d;

        std::vector<u8> framed;
        reader reader(framed);
        consumer consumer;

        REQUIRE( f64enc_init(&e, {&framed, frame_write}) == F64ENC_ERR_SUCCESS );
        REQUIRE( f64dec_init(&d, {&reader, reader_read}, {&consumer,consumer_data,consumer_delimit,consumer_final}) == F64DEC_ERR_SUCCESS );

        REQUIRE( f64enc_set_final(&e, true) == F64ENC_ERR_SUCCESS );
        REQUIRE( f64enc_write_zero(&e) == F64ENC_ERR_SUCCESS );

        REQUIRE( framed.size() == 1 );
        REQUIRE_THAT( framed, RangeEquals(std::vector<u8>{ 0x80 }) );

        REQUIRE( f64dec_read(&d) == F64DEC_ERR_SUCCESS );
        REQUIRE( f64dec_read(&d) == F64DEC_ERR_READ_NO_DATA );

        REQUIRE_THAT( consumer.accum, RangeEquals(std::vector<u8>{ 0 }) );
    }

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
        REQUIRE_THAT( framed, RangeEquals(std::vector<u8>{ 0x81, 0x01 }) );

        REQUIRE( f64dec_read(&d) == F64DEC_ERR_SUCCESS );
        REQUIRE( f64dec_read(&d) == F64DEC_ERR_READ_NO_DATA );

        REQUIRE_THAT( consumer.accum, RangeEquals(std::vector<u8>{ 1, 0 }) );
    }

    SECTION( "2 byte frame - read1", "append_u8" ) {
        f64enc e;
        f64dec d;

        std::vector<u8> framed;
        reader reader(framed);
        consumer consumer;

        REQUIRE( f64enc_init(&e, {&framed, frame_write}) == F64ENC_ERR_SUCCESS );
        // NOTE: only reading 1 byte per read() call:
        REQUIRE( f64dec_init(&d, {&reader, reader_read1}, {&consumer,consumer_data,consumer_delimit,consumer_final}) == F64DEC_ERR_SUCCESS );

        REQUIRE( f64enc_set_final(&e, true) == F64ENC_ERR_SUCCESS );
        REQUIRE( f64enc_append_u8(&e, 1) == F64ENC_ERR_SUCCESS );
        REQUIRE( f64enc_append_u8(&e, 2) == F64ENC_ERR_SUCCESS );
        REQUIRE( f64enc_write(&e) == F64ENC_ERR_SUCCESS );

        REQUIRE( framed.size() == 3 );
        REQUIRE_THAT( framed, RangeEquals(std::vector<u8>{ 0x82, 0x01, 0x02 }) );

        REQUIRE( f64dec_read(&d) == F64DEC_ERR_SUCCESS );
        REQUIRE( f64dec_read(&d) == F64DEC_ERR_SUCCESS );
        REQUIRE( f64dec_read(&d) == F64DEC_ERR_READ_NO_DATA );

        REQUIRE_THAT( consumer.accum, RangeEquals(std::vector<u8>{ 1, 2, 0 }) );
    }

    SECTION( "delimiter", "delimiter" ) {
        f64enc e;
        f64dec d;

        std::vector<u8> framed;
        reader reader(framed);
        consumer consumer;

        REQUIRE( f64enc_init(&e, {&framed, frame_write}) == F64ENC_ERR_SUCCESS );
        REQUIRE( f64dec_init(&d, {&reader, reader_read}, {&consumer,consumer_data,consumer_delimit,consumer_final}) == F64DEC_ERR_SUCCESS );

        REQUIRE( f64enc_delimiter(&e, 0x3F) == F64ENC_ERR_SUCCESS );

        REQUIRE( framed.size() == 1 );
        REQUIRE_THAT( framed, RangeEquals(std::vector<u8>{ 0x7F }) );

        REQUIRE( f64dec_read(&d) == F64DEC_ERR_SUCCESS );
        REQUIRE( f64dec_read(&d) == F64DEC_ERR_READ_NO_DATA );

        REQUIRE_THAT( consumer.accum, RangeEquals(std::vector<u8>{ 0x3F }) );
    }

    SECTION( "delimiter final", "delimiter" ) {
        f64enc e;
        f64dec d;

        std::vector<u8> framed;
        reader reader(framed);
        consumer consumer;

        REQUIRE( f64enc_init(&e, {&framed, frame_write}) == F64ENC_ERR_SUCCESS );
        REQUIRE( f64dec_init(&d, {&reader, reader_read}, {&consumer,consumer_data,consumer_delimit,consumer_final}) == F64DEC_ERR_SUCCESS );

        REQUIRE( f64enc_delimiter(&e, 0x3F) == F64ENC_ERR_SUCCESS );

        REQUIRE( f64enc_set_final(&e, true) == F64ENC_ERR_SUCCESS );
        REQUIRE( f64enc_write_zero(&e) == F64ENC_ERR_SUCCESS );

        REQUIRE( framed.size() == 2 );
        REQUIRE_THAT( framed, RangeEquals(std::vector<u8>{ 0x7F, 0x80 }) );

        REQUIRE( f64dec_read(&d) == F64DEC_ERR_SUCCESS );
        REQUIRE( f64dec_read(&d) == F64DEC_ERR_SUCCESS );
        REQUIRE( f64dec_read(&d) == F64DEC_ERR_READ_NO_DATA );

        REQUIRE_THAT( consumer.accum, RangeEquals(std::vector<u8>{ 0x3F, 0 }) );
    }

    SECTION( "msg delimiter msg", "delimiter" ) {
        f64enc e;
        f64dec d;

        std::vector<u8> framed;
        reader reader(framed);
        consumer consumer;

        REQUIRE( f64enc_init(&e, {&framed, frame_write}) == F64ENC_ERR_SUCCESS );
        REQUIRE( f64dec_init(&d, {&reader, reader_read}, {&consumer,consumer_data,consumer_delimit,consumer_final}) == F64DEC_ERR_SUCCESS );

        REQUIRE( f64enc_append_u8(&e, 1) == F64ENC_ERR_SUCCESS );
        REQUIRE( f64enc_write(&e) == F64ENC_ERR_SUCCESS );

        REQUIRE( f64enc_delimiter(&e, 0x3F) == F64ENC_ERR_SUCCESS );

        REQUIRE( f64enc_set_final(&e, true) == F64ENC_ERR_SUCCESS );
        REQUIRE( f64enc_append_u8(&e, 2) == F64ENC_ERR_SUCCESS );
        REQUIRE( f64enc_write(&e) == F64ENC_ERR_SUCCESS );

        REQUIRE( framed.size() == 5 );
        REQUIRE_THAT( framed, RangeEquals(std::vector<u8>{ 0x01, 0x01, 0x7F, 0x81, 0x02 }) );

        REQUIRE( f64dec_read(&d) == F64DEC_ERR_SUCCESS );
        REQUIRE( f64dec_read(&d) == F64DEC_ERR_SUCCESS );
        REQUIRE( f64dec_read(&d) == F64DEC_ERR_SUCCESS );
        REQUIRE( f64dec_read(&d) == F64DEC_ERR_READ_NO_DATA );

        REQUIRE_THAT( consumer.accum, RangeEquals(std::vector<u8>{ 0x01, 0x3F, 0x02, 0 }) );
    }
}

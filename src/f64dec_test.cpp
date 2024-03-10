
#include <vector>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_range_equals.hpp>

extern "C" {
#include "f64dec.h"
#include "f64enc.h"
}

using Catch::Matchers::RangeEquals;

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

extern "C" {

static auto frame_write(void *ctx, int len, const u8 *data) -> int {
    auto v = ((std::vector<u8> *) ctx);
    v->insert(v->end(), data, data + len);
    return F64ENC_ERR_SUCCESS;
}

static int reader_read(void *ctx, int len, unsigned char *dest) {
    return static_cast<reader *>(ctx)->read(len, dest);
}

static int reader_read1(void *ctx, int len, unsigned char *dest) {
    // limit reads to 1 byte at a time:
    if (len > 0) {
        len = 1;
    }

    return static_cast<reader *>(ctx)->read(len, dest);
}

static int consumer_data(void *ctx, int len, const u8 *src) {
    return static_cast<consumer *>(ctx)->data(len, src);
}
static int consumer_delimit(void *ctx, u8 delim) {
    return static_cast<consumer *>(ctx)->delimit(delim);
}
static int consumer_final(void *ctx) {
    return static_cast<consumer *>(ctx)->final();
}

}

TEST_CASE( "f64dec" ) {
    SECTION("64 0-byte frames") {
        f64enc e;
        f64dec d;

        std::vector<u8> framed;
        reader reader(framed);
        consumer consumer;

        REQUIRE(f64enc_init(&e, {&framed, frame_write}) == F64ENC_ERR_SUCCESS);
        REQUIRE(f64dec_init(&d, {&reader, reader_read}, {&consumer, consumer_data, consumer_delimit, consumer_final}) == F64DEC_ERR_SUCCESS);

        std::vector<u8> expected;
        for (int i = 0; i < 64; i++) {
            REQUIRE(f64enc_write_zero(&e) == F64ENC_ERR_SUCCESS);
            expected.push_back(0);
        }

        REQUIRE(framed.size() == 64);
        REQUIRE_THAT(framed, RangeEquals(expected));

        for (int i = 0; i < 64; i++) {
            REQUIRE(f64dec_read(&d) == F64DEC_ERR_SUCCESS);
        }
        REQUIRE(f64dec_read(&d) == F64DEC_ERR_READ_NO_DATA);

        REQUIRE_THAT(consumer.accum, RangeEquals(std::vector<u8>{}));
    }

    SECTION("delimiter") {
        f64enc e;
        f64dec d;

        std::vector<u8> framed;
        reader reader(framed);
        consumer consumer;

        REQUIRE(f64enc_init(&e, {&framed, frame_write}) == F64ENC_ERR_SUCCESS);
        REQUIRE(f64dec_init(&d, {&reader, reader_read}, {&consumer, consumer_data, consumer_delimit, consumer_final}) == F64DEC_ERR_SUCCESS);

        // delimiter, delim=1:
        framed.push_back(0x41);

        REQUIRE(framed.size() == 1);

        REQUIRE(f64dec_read(&d) == F64DEC_ERR_SUCCESS);
        REQUIRE(f64dec_read(&d) == F64DEC_ERR_READ_NO_DATA);

        std::vector<u8> expected;
        expected.push_back(1);
        REQUIRE_THAT(consumer.accum, RangeEquals(expected));
    }

    SECTION("delimiter with final bit") {
        f64enc e;
        f64dec d;

        std::vector<u8> framed;
        reader reader(framed);
        consumer consumer;

        REQUIRE(f64enc_init(&e, {&framed, frame_write}) == F64ENC_ERR_SUCCESS);
        REQUIRE(f64dec_init(&d, {&reader, reader_read}, {&consumer, consumer_data, consumer_delimit, consumer_final}) == F64DEC_ERR_SUCCESS);

        // delimiter, final, delim=1:
        framed.push_back(0xC1);

        REQUIRE(framed.size() == 1);

        REQUIRE(f64dec_read(&d) == F64DEC_ERR_SUCCESS);
        REQUIRE(f64dec_read(&d) == F64DEC_ERR_READ_NO_DATA);

        std::vector<u8> expected;
        expected.push_back(1);
        expected.push_back(0);
        REQUIRE_THAT(consumer.accum, RangeEquals(expected));
    }
}

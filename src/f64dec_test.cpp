
#include <vector>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_range_equals.hpp>

extern "C" {
#include "f64dec.h"
#include "f64enc.h"
}

using Catch::Matchers::RangeEquals;

struct decreader {
    explicit decreader(std::vector<u8> &buf) : m_buf(buf) {}

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

struct decconsumer {
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

static auto decframe_write(void *ctx, int len, const u8 *data) -> int {
    auto v = ((std::vector<u8> *) ctx);
    v->insert(v->end(), data, data + len);
    return F64ENC_ERR_SUCCESS;
}

static int decreader_read(void *ctx, int len, unsigned char *dest) {
    return static_cast<decreader *>(ctx)->read(len, dest);
}

static int decreader_read1(void *ctx, int len, unsigned char *dest) {
    // limit reads to 1 byte at a time:
    if (len > 0) {
        len = 1;
    }

    return static_cast<decreader *>(ctx)->read(len, dest);
}

static int decconsumer_data(void *ctx, int len, const u8 *src) {
    return static_cast<decconsumer *>(ctx)->data(len, src);
}
static int decconsumer_delimit(void *ctx) {
    return static_cast<decconsumer *>(ctx)->delimit();
}
static int decconsumer_final(void *ctx) {
    return static_cast<decconsumer *>(ctx)->final();
}

}

TEST_CASE( "f64dec" ) {
    SECTION("64 0-byte frames") {
        f64enc e;
        f64dec d;

        std::vector<u8> framed;
        decreader decreader(framed);
        decconsumer decconsumer;

        REQUIRE(f64enc_init(&e, {&framed, decframe_write}) == F64ENC_ERR_SUCCESS);
        REQUIRE(f64dec_init(&d, {&decreader, decreader_read}, {&decconsumer, decconsumer_data, decconsumer_delimit, decconsumer_final}) == F64DEC_ERR_SUCCESS);

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

        REQUIRE_THAT(decconsumer.accum, RangeEquals(std::vector<int>{}));
    }

    SECTION("delimiter") {
        f64enc e;
        f64dec d;

        std::vector<u8> framed;
        decreader decreader(framed);
        decconsumer decconsumer;

        REQUIRE(f64enc_init(&e, {&framed, decframe_write}) == F64ENC_ERR_SUCCESS);
        REQUIRE(f64dec_init(&d, {&decreader, decreader_read}, {&decconsumer, decconsumer_data, decconsumer_delimit, decconsumer_final}) == F64DEC_ERR_SUCCESS);

        // delimiter:
        framed.push_back(0x40);

        REQUIRE(framed.size() == 1);

        REQUIRE(f64dec_read(&d) == F64DEC_ERR_SUCCESS);
        REQUIRE(f64dec_read(&d) == F64DEC_ERR_READ_NO_DATA);

        std::vector<int> expected;
        expected.push_back(-1);
        REQUIRE_THAT(decconsumer.accum, RangeEquals(expected));
    }

    SECTION("delimiter with final bit") {
        f64enc e;
        f64dec d;

        std::vector<u8> framed;
        decreader decreader(framed);
        decconsumer decconsumer;

        REQUIRE(f64enc_init(&e, {&framed, decframe_write}) == F64ENC_ERR_SUCCESS);
        REQUIRE(f64dec_init(&d, {&decreader, decreader_read}, {&decconsumer, decconsumer_data, decconsumer_delimit, decconsumer_final}) == F64DEC_ERR_SUCCESS);

        // delimiter, final:
        framed.push_back(0xC0);

        REQUIRE(framed.size() == 1);

        REQUIRE(f64dec_read(&d) == F64DEC_ERR_SUCCESS);
        REQUIRE(f64dec_read(&d) == F64DEC_ERR_READ_NO_DATA);

        std::vector<int> expected;
        expected.push_back(-1);
        expected.push_back(-2);
        REQUIRE_THAT(decconsumer.accum, RangeEquals(expected));
    }
}

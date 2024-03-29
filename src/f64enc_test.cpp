
#include <vector>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_range_equals.hpp>

extern "C" {
#include "f64dec.h"
#include "f64enc.h"
}

static auto record_frame(void *ctx, int len, const u8 *data) -> int {
    ((std::vector< std::vector<u8> > *)ctx)->emplace_back(data, data + len);
    return 0;
}

TEST_CASE( "f64enc encodes", "f64enc" ) {
    SECTION( "0 byte frame write" ) {
        f64enc e;

        std::vector< std::vector<u8> > written;
        REQUIRE( f64enc_init(&e,{&written,record_frame}) == F64ENC_ERR_SUCCESS );

        REQUIRE( f64enc_write(&e) == F64ENC_ERR_ZERO_LENGTH );

        REQUIRE( written.size() == 0 );
    }

    SECTION( "0 byte frame final write" ) {
        f64enc e;

        std::vector< std::vector<u8> > written;
        REQUIRE( f64enc_init(&e,{&written,record_frame}) == F64ENC_ERR_SUCCESS );

        REQUIRE( f64enc_set_final(&e, true) == F64ENC_ERR_SUCCESS );
        REQUIRE( f64enc_write(&e) == F64ENC_ERR_ZERO_LENGTH );

        REQUIRE( written.size() == 0 );
    }

    SECTION( "0 byte frame write_zero" ) {
        f64enc e;

        std::vector< std::vector<u8> > written;
        REQUIRE( f64enc_init(&e,{&written,record_frame}) == F64ENC_ERR_SUCCESS );

        REQUIRE( f64enc_write_zero(&e) == F64ENC_ERR_SUCCESS );

        REQUIRE( written.size() == 1 );
        REQUIRE_THAT(written[0], Catch::Matchers::RangeEquals(std::vector<u8>{ 0x00 }));
    }

    SECTION( "0 byte frame write_zero" ) {
        f64enc e;

        std::vector< std::vector<u8> > written;
        REQUIRE( f64enc_init(&e,{&written,record_frame}) == F64ENC_ERR_SUCCESS );

        REQUIRE( f64enc_set_final(&e, true) == F64ENC_ERR_SUCCESS );
        REQUIRE( f64enc_write_zero(&e) == F64ENC_ERR_SUCCESS );

        REQUIRE( written.size() == 1 );
        REQUIRE_THAT(written[0], Catch::Matchers::RangeEquals(std::vector<u8>{ 0x80 }));
    }

    SECTION( "1 byte frame", "append_u8" ) {
        f64enc e;

        std::vector< std::vector<u8> > written;
        REQUIRE( f64enc_init(&e,{&written,record_frame}) == F64ENC_ERR_SUCCESS );

        REQUIRE( f64enc_set_final(&e, true) == F64ENC_ERR_SUCCESS );
        REQUIRE( f64enc_append_u8(&e, 1) == F64ENC_ERR_SUCCESS );
        REQUIRE( f64enc_write(&e) == F64ENC_ERR_SUCCESS );
        REQUIRE( written.size() == 1 );
        REQUIRE_THAT(written[0], Catch::Matchers::RangeEquals(std::vector<u8>{ 0x81, 0x01 }));
    }

    SECTION( "2 byte frame", "append_u8" ) {
        f64enc e;

        std::vector< std::vector<u8> > written;
        REQUIRE( f64enc_init(&e,{&written,record_frame}) == F64ENC_ERR_SUCCESS );

        REQUIRE( f64enc_set_final(&e, true) == F64ENC_ERR_SUCCESS );
        REQUIRE( f64enc_append_u8(&e, 1) == F64ENC_ERR_SUCCESS );
        REQUIRE( f64enc_append_u8(&e, 2) == F64ENC_ERR_SUCCESS );
        REQUIRE( f64enc_write(&e) == F64ENC_ERR_SUCCESS );
        REQUIRE( written.size() == 1 );
        REQUIRE_THAT(written[0], Catch::Matchers::RangeEquals(std::vector<u8>{ 0x82, 0x01, 0x02 }));
    }

    SECTION( "62 byte frame", "append_buf" ) {
        f64enc e;

        std::vector< std::vector<u8> > written;
        REQUIRE( f64enc_init(&e,{&written,record_frame}) == F64ENC_ERR_SUCCESS );

        std::vector<u8> tmp;
        tmp.reserve(62);
        for (int i = 1; i <= 62; i++) {
            tmp.push_back(i);
        }
        REQUIRE( tmp.size() == 62 );
        REQUIRE( f64enc_set_final(&e, true) == F64ENC_ERR_SUCCESS );
        REQUIRE( f64enc_append_buf(&e, tmp.size(), tmp.data()) == F64ENC_ERR_SUCCESS );
        REQUIRE( f64enc_write(&e) == F64ENC_ERR_SUCCESS );

        std::vector<u8> expected;
        expected.reserve(64);
        expected.push_back(0xBe);
        for (int i = 1; i <= 62; i++) {
            expected.push_back(i);
        }
        REQUIRE( expected.size() == 63 );
        REQUIRE( written.size() == 1 );
        REQUIRE_THAT(written[0], Catch::Matchers::RangeEquals(expected));
    }

    SECTION( "63 byte frame", "append_buf" ) {
        f64enc e;

        std::vector< std::vector<u8> > written;
        REQUIRE( f64enc_init(&e,{&written,record_frame}) == F64ENC_ERR_SUCCESS );

        std::vector<u8> tmp;
        tmp.reserve(63);
        for (int i = 1; i <= 63; i++) {
            tmp.push_back(i);
        }
        REQUIRE( tmp.size() == 63 );
        REQUIRE( f64enc_set_final(&e, true) == F64ENC_ERR_SUCCESS );
        REQUIRE( f64enc_append_buf(&e, tmp.size(), tmp.data()) == F64ENC_ERR_SUCCESS );

        std::vector<u8> expected;
        expected.reserve(64);
        expected.push_back(0xBF);
        for (int i = 1; i <= 63; i++) {
            expected.push_back(i);
        }
        REQUIRE( expected.size() == 64 );
        REQUIRE( written.size() == 1 );
        REQUIRE_THAT(written[0], Catch::Matchers::RangeEquals(expected));
    }

    SECTION( "delimiter", "delimiter" ) {
        f64enc e;

        std::vector< std::vector<u8> > written;
        REQUIRE( f64enc_init(&e,{&written,record_frame}) == F64ENC_ERR_SUCCESS );

        REQUIRE( f64enc_set_delimited(&e, true) == F64ENC_ERR_SUCCESS );
        REQUIRE( f64enc_write_zero(&e) == F64ENC_ERR_SUCCESS );

        std::vector<u8> expected;
        expected.reserve(1);
        expected.push_back(0x40);
        REQUIRE( expected.size() == 1 );
        REQUIRE( written.size() == 1 );
        REQUIRE_THAT(written[0], Catch::Matchers::RangeEquals(expected));
    }
}

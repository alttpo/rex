
#include <vector>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_range_equals.hpp>

extern "C" {
#include "f64dec.h"
#include "f64enc.h"
}

TEST_CASE( "f64enc encodes", "f64enc" ) {
    SECTION( "one byte frame" ) {
        std::vector<u8> written;
        f64enc e;

        REQUIRE(
            f64enc_init(
                &e,
                {
                    &written,
                    +[](void *ctx, unsigned len, const u8 *data) -> int {
                        *(std::vector<u8> *)ctx = {data, data + len};
                        return 0;
                    }
                }
            ) == F64ENC_ERR_SUCCESS
        );

        REQUIRE( f64enc_append_u8(&e, 1) == F64ENC_ERR_SUCCESS );
        REQUIRE( f64enc_set_final(&e, true) == F64ENC_ERR_SUCCESS );
        REQUIRE( f64enc_write(&e) == F64ENC_ERR_SUCCESS );
        REQUIRE_THAT(written, Catch::Matchers::RangeEquals(std::vector<u8>{ (u8)0x81, (u8)0x01 }));
    }
}

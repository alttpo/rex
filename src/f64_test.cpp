
#include <vector>

extern "C" {
#include "f64dec.h"
#include "f64enc.h"
}

int main() {
    f64enc e;
    enum f64enc_error err;

    err = f64enc_init(
        &e,
        {
            nullptr,
            +[](void *ctx, unsigned len, const u8 *data) -> int {
                for (int i = 0; i < len; i++) {
                    printf(" %02X", *data++);
                }
                printf("\n");
                return 0;
            }
        }
    );
    if (err) {
        fprintf(stderr, "%d\n", err);
        return 1;
    }

    err = f64enc_append_u8(&e, 1);
    if (err) {
        fprintf(stderr, "%d\n", err);
        return 1;
    }

    f64enc_set_final(&e, true);

    err = f64enc_write(&e);
    if (err) {
        fprintf(stderr, "%d\n", err);
        return 1;
    }

    return 0;
}

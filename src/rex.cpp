
#include "rex_generated.h"

int main() {
    flatbuffers::FlatBufferBuilder fbb;

    auto req = rex::CreateEnterProgramRequest(
        fbb,
        fbb.CreateVector({
            (uint8_t)rex::ProgramInstruction_ReadMemory
        }),
        fbb.CreateVector({
            rex::CreateReadMemory(fbb, rex::ChipID_SNES_WRAM, 0x10, 0xFF).Union()
        })
    );

    auto reqWrapper = rex::CreateRequestMessage(
        fbb,
        rex::Request_EnterProgramRequest,
        req.Union()
    );

    fbb.Finish(reqWrapper);

    // write out flatbuffer:
    auto b = fbb.GetBufferSpan();
    for (auto i = 0; i < b.size_bytes(); i++) {
        printf("%02X ", b[i]);
        if ((i & 7) == 7) printf("\n");
    }
    printf("\n$%04zX\n\n", b.size_bytes());

    // zero-packed:
    auto words = b.size_bytes() / 8;
    size_t total = 0;
    for (auto i = 0; i < words; i++) {
        uint8_t tag = 0;
        for (uint8_t j = 0; j < 8; j++) {
            tag |= (b[i * 8 + j] != 0) << j;
        }
        printf("%02X ", tag);
        total++;
        for (uint8_t j = 0; j < 8; j++) {
            uint8_t c = b[i * 8 + j];
            if (c != 0) {
                printf("%02X ", c);
                total++;
            }
        }
        printf("\n");
    }
    printf("$%04zX\n", total);

    return 0;
}

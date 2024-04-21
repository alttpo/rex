
#include "rex_generated.h"

void dump_hex(const flatbuffers::span<uint8_t> &b) {
    printf("\n");
    for (auto i = 0; i < b.size_bytes(); i++) {
        printf("%02X ", b[i]);
        if ((i & 7) == 7) printf("\n");
    }
    printf("\n$%04zX\n\n", b.size_bytes());
}

void dump_packed(const flatbuffers::span<uint8_t> &b) {
    size_t i = 0;
    size_t total = 0;
    for (; i < b.size_bytes(); i += 8) {
        size_t remain = 8;
        uint8_t tag = 0;
        if (i + remain > b.size_bytes()) {
            remain = b.size_bytes() - i;
        }
        for (size_t j = 0; j < remain; j++) {
            tag |= (b[i + j] != 0) << j;
        }
        printf("%02X ", tag);
        total++;
        for (size_t j = 0; j < remain; j++) {
            uint8_t c = b[i + j];
            if (c != 0) {
                printf("%02X ", c);
                total++;
            }
        }
        printf("\n");
    }

    printf("$%04zX\n", total);
}

int main() {
    {
        flatbuffers::FlatBufferBuilder fbb;

        auto req = rex::CreateEnterProgramRequest(
            fbb,
            fbb.CreateVector(
                {
                    (uint8_t) rex::ProgramInstruction_ReadMemory
                }
            ),
            fbb.CreateVector(
                {
                    rex::CreateReadMemory(
                        fbb,
                        rex::ChipID_SNES_WRAM,
                        0x10,
                        0x10
                    ).Union()
                }
            )
        );

        auto reqWrapper = rex::CreateRequestMessage(
            fbb,
            rex::Request_EnterProgramRequest,
            req.Union()
        );

        fbb.Finish(reqWrapper);

        // write out flatbuffer:
        auto b = fbb.GetBufferSpan();
        dump_hex(b);
        dump_packed(b);
    }

    {
        flatbuffers::FlatBufferBuilder fbb;

        auto rsp = rex::CreateExecuteProgramResponse(
            fbb,
            fbb.CreateVector(
                {
                    rex::CreateInstructionOutput(
                        fbb,
                        rex::ErrorCode_Success,
                        fbb.CreateVector<uint8_t>({
                            0x14, 1, 0, 15, 16, 17, 18, 65, 66, 67, 97, 99, 101, 106
                        })
                    )
                }
            )
        );

        auto rspWrapper = rex::CreateResponseMessage(
            fbb,
            rex::Response_ExecuteProgramResponse,
            rsp.Union()
        );

        fbb.Finish(rspWrapper);

        // write out flatbuffer:
        auto b = fbb.GetBufferSpan();
        dump_hex(b);
        dump_packed(b);
    }

    return 0;
}

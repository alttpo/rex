#ifndef REX_VERIFIER_H
#define REX_VERIFIER_H

/* Generated by flatcc 0.6.2 FlatBuffers schema compiler for C by dvide.com */

#ifndef REX_READER_H
#include "rex_reader.h"
#endif
#include "flatcc/flatcc_verifier.h"
#include "flatcc/flatcc_prologue.h"

static int rex_EnumerateChipsRequest_verify_table(flatcc_table_verifier_descriptor_t *td);
static int rex_Chip_verify_table(flatcc_table_verifier_descriptor_t *td);
static int rex_EnumerateChipsResponse_verify_table(flatcc_table_verifier_descriptor_t *td);
static int rex_WriteMemory_verify_table(flatcc_table_verifier_descriptor_t *td);
static int rex_ReadMemory_verify_table(flatcc_table_verifier_descriptor_t *td);
static int rex_SetTimeout_verify_table(flatcc_table_verifier_descriptor_t *td);
static int rex_WaitUntil_verify_table(flatcc_table_verifier_descriptor_t *td);
static int rex_WaitWhileZero_verify_table(flatcc_table_verifier_descriptor_t *td);
static int rex_EnterProgramRequest_verify_table(flatcc_table_verifier_descriptor_t *td);
static int rex_EnterProgramResponse_verify_table(flatcc_table_verifier_descriptor_t *td);
static int rex_ExecuteProgramRequest_verify_table(flatcc_table_verifier_descriptor_t *td);
static int rex_InstructionOutput_verify_table(flatcc_table_verifier_descriptor_t *td);
static int rex_ExecuteProgramResponse_verify_table(flatcc_table_verifier_descriptor_t *td);
static int rex_RequestMessage_verify_table(flatcc_table_verifier_descriptor_t *td);
static int rex_ResponseMessage_verify_table(flatcc_table_verifier_descriptor_t *td);

static int rex_ProgramInstruction_union_verifier(flatcc_union_verifier_descriptor_t *ud)
{
    switch (ud->type) {
    case 1: return flatcc_verify_union_table(ud, rex_WriteMemory_verify_table); /* WriteMemory */
    case 2: return flatcc_verify_union_table(ud, rex_ReadMemory_verify_table); /* ReadMemory */
    case 3: return flatcc_verify_union_table(ud, rex_SetTimeout_verify_table); /* SetTimeout */
    case 4: return flatcc_verify_union_table(ud, rex_WaitUntil_verify_table); /* WaitUntil */
    case 5: return flatcc_verify_union_table(ud, rex_WaitWhileZero_verify_table); /* WaitWhileZero */
    default: return flatcc_verify_ok;
    }
}

static int rex_Request_union_verifier(flatcc_union_verifier_descriptor_t *ud)
{
    switch (ud->type) {
    case 1: return flatcc_verify_union_table(ud, rex_EnumerateChipsRequest_verify_table); /* EnumerateChipsRequest */
    case 2: return flatcc_verify_union_table(ud, rex_EnterProgramRequest_verify_table); /* EnterProgramRequest */
    case 3: return flatcc_verify_union_table(ud, rex_ExecuteProgramRequest_verify_table); /* ExecuteProgramRequest */
    default: return flatcc_verify_ok;
    }
}

static int rex_Response_union_verifier(flatcc_union_verifier_descriptor_t *ud)
{
    switch (ud->type) {
    case 1: return flatcc_verify_union_table(ud, rex_EnumerateChipsResponse_verify_table); /* EnumerateChipsResponse */
    case 2: return flatcc_verify_union_table(ud, rex_EnterProgramResponse_verify_table); /* EnterProgramResponse */
    case 3: return flatcc_verify_union_table(ud, rex_ExecuteProgramResponse_verify_table); /* ExecuteProgramResponse */
    default: return flatcc_verify_ok;
    }
}

static int rex_EnumerateChipsRequest_verify_table(flatcc_table_verifier_descriptor_t *td)
{
    return flatcc_verify_ok;
}

static inline int rex_EnumerateChipsRequest_verify_as_root(const void *buf, size_t bufsiz)
{
    return flatcc_verify_table_as_root(buf, bufsiz, rex_EnumerateChipsRequest_identifier, &rex_EnumerateChipsRequest_verify_table);
}

static inline int rex_EnumerateChipsRequest_verify_as_root_with_size(const void *buf, size_t bufsiz)
{
    return flatcc_verify_table_as_root_with_size(buf, bufsiz, rex_EnumerateChipsRequest_identifier, &rex_EnumerateChipsRequest_verify_table);
}

static inline int rex_EnumerateChipsRequest_verify_as_typed_root(const void *buf, size_t bufsiz)
{
    return flatcc_verify_table_as_root(buf, bufsiz, rex_EnumerateChipsRequest_type_identifier, &rex_EnumerateChipsRequest_verify_table);
}

static inline int rex_EnumerateChipsRequest_verify_as_typed_root_with_size(const void *buf, size_t bufsiz)
{
    return flatcc_verify_table_as_root_with_size(buf, bufsiz, rex_EnumerateChipsRequest_type_identifier, &rex_EnumerateChipsRequest_verify_table);
}

static inline int rex_EnumerateChipsRequest_verify_as_root_with_identifier(const void *buf, size_t bufsiz, const char *fid)
{
    return flatcc_verify_table_as_root(buf, bufsiz, fid, &rex_EnumerateChipsRequest_verify_table);
}

static inline int rex_EnumerateChipsRequest_verify_as_root_with_identifier_and_size(const void *buf, size_t bufsiz, const char *fid)
{
    return flatcc_verify_table_as_root_with_size(buf, bufsiz, fid, &rex_EnumerateChipsRequest_verify_table);
}

static inline int rex_EnumerateChipsRequest_verify_as_root_with_type_hash(const void *buf, size_t bufsiz, flatbuffers_thash_t thash)
{
    return flatcc_verify_table_as_typed_root(buf, bufsiz, thash, &rex_EnumerateChipsRequest_verify_table);
}

static inline int rex_EnumerateChipsRequest_verify_as_root_with_type_hash_and_size(const void *buf, size_t bufsiz, flatbuffers_thash_t thash)
{
    return flatcc_verify_table_as_typed_root_with_size(buf, bufsiz, thash, &rex_EnumerateChipsRequest_verify_table);
}

static int rex_Chip_verify_table(flatcc_table_verifier_descriptor_t *td)
{
    int ret;
    if ((ret = flatcc_verify_field(td, 0, 2, 2) /* chip_id */)) return ret;
    if ((ret = flatcc_verify_field(td, 1, 4, 4) /* size */)) return ret;
    if ((ret = flatcc_verify_field(td, 2, 1, 1) /* flags */)) return ret;
    if ((ret = flatcc_verify_string_field(td, 3, 0) /* name */)) return ret;
    return flatcc_verify_ok;
}

static inline int rex_Chip_verify_as_root(const void *buf, size_t bufsiz)
{
    return flatcc_verify_table_as_root(buf, bufsiz, rex_Chip_identifier, &rex_Chip_verify_table);
}

static inline int rex_Chip_verify_as_root_with_size(const void *buf, size_t bufsiz)
{
    return flatcc_verify_table_as_root_with_size(buf, bufsiz, rex_Chip_identifier, &rex_Chip_verify_table);
}

static inline int rex_Chip_verify_as_typed_root(const void *buf, size_t bufsiz)
{
    return flatcc_verify_table_as_root(buf, bufsiz, rex_Chip_type_identifier, &rex_Chip_verify_table);
}

static inline int rex_Chip_verify_as_typed_root_with_size(const void *buf, size_t bufsiz)
{
    return flatcc_verify_table_as_root_with_size(buf, bufsiz, rex_Chip_type_identifier, &rex_Chip_verify_table);
}

static inline int rex_Chip_verify_as_root_with_identifier(const void *buf, size_t bufsiz, const char *fid)
{
    return flatcc_verify_table_as_root(buf, bufsiz, fid, &rex_Chip_verify_table);
}

static inline int rex_Chip_verify_as_root_with_identifier_and_size(const void *buf, size_t bufsiz, const char *fid)
{
    return flatcc_verify_table_as_root_with_size(buf, bufsiz, fid, &rex_Chip_verify_table);
}

static inline int rex_Chip_verify_as_root_with_type_hash(const void *buf, size_t bufsiz, flatbuffers_thash_t thash)
{
    return flatcc_verify_table_as_typed_root(buf, bufsiz, thash, &rex_Chip_verify_table);
}

static inline int rex_Chip_verify_as_root_with_type_hash_and_size(const void *buf, size_t bufsiz, flatbuffers_thash_t thash)
{
    return flatcc_verify_table_as_typed_root_with_size(buf, bufsiz, thash, &rex_Chip_verify_table);
}

static int rex_EnumerateChipsResponse_verify_table(flatcc_table_verifier_descriptor_t *td)
{
    int ret;
    if ((ret = flatcc_verify_table_vector_field(td, 0, 0, &rex_Chip_verify_table) /* chips */)) return ret;
    return flatcc_verify_ok;
}

static inline int rex_EnumerateChipsResponse_verify_as_root(const void *buf, size_t bufsiz)
{
    return flatcc_verify_table_as_root(buf, bufsiz, rex_EnumerateChipsResponse_identifier, &rex_EnumerateChipsResponse_verify_table);
}

static inline int rex_EnumerateChipsResponse_verify_as_root_with_size(const void *buf, size_t bufsiz)
{
    return flatcc_verify_table_as_root_with_size(buf, bufsiz, rex_EnumerateChipsResponse_identifier, &rex_EnumerateChipsResponse_verify_table);
}

static inline int rex_EnumerateChipsResponse_verify_as_typed_root(const void *buf, size_t bufsiz)
{
    return flatcc_verify_table_as_root(buf, bufsiz, rex_EnumerateChipsResponse_type_identifier, &rex_EnumerateChipsResponse_verify_table);
}

static inline int rex_EnumerateChipsResponse_verify_as_typed_root_with_size(const void *buf, size_t bufsiz)
{
    return flatcc_verify_table_as_root_with_size(buf, bufsiz, rex_EnumerateChipsResponse_type_identifier, &rex_EnumerateChipsResponse_verify_table);
}

static inline int rex_EnumerateChipsResponse_verify_as_root_with_identifier(const void *buf, size_t bufsiz, const char *fid)
{
    return flatcc_verify_table_as_root(buf, bufsiz, fid, &rex_EnumerateChipsResponse_verify_table);
}

static inline int rex_EnumerateChipsResponse_verify_as_root_with_identifier_and_size(const void *buf, size_t bufsiz, const char *fid)
{
    return flatcc_verify_table_as_root_with_size(buf, bufsiz, fid, &rex_EnumerateChipsResponse_verify_table);
}

static inline int rex_EnumerateChipsResponse_verify_as_root_with_type_hash(const void *buf, size_t bufsiz, flatbuffers_thash_t thash)
{
    return flatcc_verify_table_as_typed_root(buf, bufsiz, thash, &rex_EnumerateChipsResponse_verify_table);
}

static inline int rex_EnumerateChipsResponse_verify_as_root_with_type_hash_and_size(const void *buf, size_t bufsiz, flatbuffers_thash_t thash)
{
    return flatcc_verify_table_as_typed_root_with_size(buf, bufsiz, thash, &rex_EnumerateChipsResponse_verify_table);
}

static int rex_WriteMemory_verify_table(flatcc_table_verifier_descriptor_t *td)
{
    int ret;
    if ((ret = flatcc_verify_field(td, 0, 2, 2) /* chip_id */)) return ret;
    if ((ret = flatcc_verify_field(td, 1, 4, 4) /* chip_addr */)) return ret;
    if ((ret = flatcc_verify_vector_field(td, 2, 0, 1, 1, INT64_C(4294967295)) /* data */)) return ret;
    return flatcc_verify_ok;
}

static inline int rex_WriteMemory_verify_as_root(const void *buf, size_t bufsiz)
{
    return flatcc_verify_table_as_root(buf, bufsiz, rex_WriteMemory_identifier, &rex_WriteMemory_verify_table);
}

static inline int rex_WriteMemory_verify_as_root_with_size(const void *buf, size_t bufsiz)
{
    return flatcc_verify_table_as_root_with_size(buf, bufsiz, rex_WriteMemory_identifier, &rex_WriteMemory_verify_table);
}

static inline int rex_WriteMemory_verify_as_typed_root(const void *buf, size_t bufsiz)
{
    return flatcc_verify_table_as_root(buf, bufsiz, rex_WriteMemory_type_identifier, &rex_WriteMemory_verify_table);
}

static inline int rex_WriteMemory_verify_as_typed_root_with_size(const void *buf, size_t bufsiz)
{
    return flatcc_verify_table_as_root_with_size(buf, bufsiz, rex_WriteMemory_type_identifier, &rex_WriteMemory_verify_table);
}

static inline int rex_WriteMemory_verify_as_root_with_identifier(const void *buf, size_t bufsiz, const char *fid)
{
    return flatcc_verify_table_as_root(buf, bufsiz, fid, &rex_WriteMemory_verify_table);
}

static inline int rex_WriteMemory_verify_as_root_with_identifier_and_size(const void *buf, size_t bufsiz, const char *fid)
{
    return flatcc_verify_table_as_root_with_size(buf, bufsiz, fid, &rex_WriteMemory_verify_table);
}

static inline int rex_WriteMemory_verify_as_root_with_type_hash(const void *buf, size_t bufsiz, flatbuffers_thash_t thash)
{
    return flatcc_verify_table_as_typed_root(buf, bufsiz, thash, &rex_WriteMemory_verify_table);
}

static inline int rex_WriteMemory_verify_as_root_with_type_hash_and_size(const void *buf, size_t bufsiz, flatbuffers_thash_t thash)
{
    return flatcc_verify_table_as_typed_root_with_size(buf, bufsiz, thash, &rex_WriteMemory_verify_table);
}

static int rex_ReadMemory_verify_table(flatcc_table_verifier_descriptor_t *td)
{
    int ret;
    if ((ret = flatcc_verify_field(td, 0, 2, 2) /* chip_id */)) return ret;
    if ((ret = flatcc_verify_field(td, 1, 4, 4) /* chip_addr */)) return ret;
    if ((ret = flatcc_verify_field(td, 2, 2, 2) /* size */)) return ret;
    return flatcc_verify_ok;
}

static inline int rex_ReadMemory_verify_as_root(const void *buf, size_t bufsiz)
{
    return flatcc_verify_table_as_root(buf, bufsiz, rex_ReadMemory_identifier, &rex_ReadMemory_verify_table);
}

static inline int rex_ReadMemory_verify_as_root_with_size(const void *buf, size_t bufsiz)
{
    return flatcc_verify_table_as_root_with_size(buf, bufsiz, rex_ReadMemory_identifier, &rex_ReadMemory_verify_table);
}

static inline int rex_ReadMemory_verify_as_typed_root(const void *buf, size_t bufsiz)
{
    return flatcc_verify_table_as_root(buf, bufsiz, rex_ReadMemory_type_identifier, &rex_ReadMemory_verify_table);
}

static inline int rex_ReadMemory_verify_as_typed_root_with_size(const void *buf, size_t bufsiz)
{
    return flatcc_verify_table_as_root_with_size(buf, bufsiz, rex_ReadMemory_type_identifier, &rex_ReadMemory_verify_table);
}

static inline int rex_ReadMemory_verify_as_root_with_identifier(const void *buf, size_t bufsiz, const char *fid)
{
    return flatcc_verify_table_as_root(buf, bufsiz, fid, &rex_ReadMemory_verify_table);
}

static inline int rex_ReadMemory_verify_as_root_with_identifier_and_size(const void *buf, size_t bufsiz, const char *fid)
{
    return flatcc_verify_table_as_root_with_size(buf, bufsiz, fid, &rex_ReadMemory_verify_table);
}

static inline int rex_ReadMemory_verify_as_root_with_type_hash(const void *buf, size_t bufsiz, flatbuffers_thash_t thash)
{
    return flatcc_verify_table_as_typed_root(buf, bufsiz, thash, &rex_ReadMemory_verify_table);
}

static inline int rex_ReadMemory_verify_as_root_with_type_hash_and_size(const void *buf, size_t bufsiz, flatbuffers_thash_t thash)
{
    return flatcc_verify_table_as_typed_root_with_size(buf, bufsiz, thash, &rex_ReadMemory_verify_table);
}

static int rex_SetTimeout_verify_table(flatcc_table_verifier_descriptor_t *td)
{
    int ret;
    if ((ret = flatcc_verify_field(td, 0, 4, 4) /* clock_cycles */)) return ret;
    return flatcc_verify_ok;
}

static inline int rex_SetTimeout_verify_as_root(const void *buf, size_t bufsiz)
{
    return flatcc_verify_table_as_root(buf, bufsiz, rex_SetTimeout_identifier, &rex_SetTimeout_verify_table);
}

static inline int rex_SetTimeout_verify_as_root_with_size(const void *buf, size_t bufsiz)
{
    return flatcc_verify_table_as_root_with_size(buf, bufsiz, rex_SetTimeout_identifier, &rex_SetTimeout_verify_table);
}

static inline int rex_SetTimeout_verify_as_typed_root(const void *buf, size_t bufsiz)
{
    return flatcc_verify_table_as_root(buf, bufsiz, rex_SetTimeout_type_identifier, &rex_SetTimeout_verify_table);
}

static inline int rex_SetTimeout_verify_as_typed_root_with_size(const void *buf, size_t bufsiz)
{
    return flatcc_verify_table_as_root_with_size(buf, bufsiz, rex_SetTimeout_type_identifier, &rex_SetTimeout_verify_table);
}

static inline int rex_SetTimeout_verify_as_root_with_identifier(const void *buf, size_t bufsiz, const char *fid)
{
    return flatcc_verify_table_as_root(buf, bufsiz, fid, &rex_SetTimeout_verify_table);
}

static inline int rex_SetTimeout_verify_as_root_with_identifier_and_size(const void *buf, size_t bufsiz, const char *fid)
{
    return flatcc_verify_table_as_root_with_size(buf, bufsiz, fid, &rex_SetTimeout_verify_table);
}

static inline int rex_SetTimeout_verify_as_root_with_type_hash(const void *buf, size_t bufsiz, flatbuffers_thash_t thash)
{
    return flatcc_verify_table_as_typed_root(buf, bufsiz, thash, &rex_SetTimeout_verify_table);
}

static inline int rex_SetTimeout_verify_as_root_with_type_hash_and_size(const void *buf, size_t bufsiz, flatbuffers_thash_t thash)
{
    return flatcc_verify_table_as_typed_root_with_size(buf, bufsiz, thash, &rex_SetTimeout_verify_table);
}

static int rex_WaitUntil_verify_table(flatcc_table_verifier_descriptor_t *td)
{
    int ret;
    if ((ret = flatcc_verify_field(td, 0, 2, 2) /* chip_id */)) return ret;
    if ((ret = flatcc_verify_field(td, 1, 4, 4) /* chip_addr */)) return ret;
    if ((ret = flatcc_verify_field(td, 2, 1, 1) /* cmp_func */)) return ret;
    if ((ret = flatcc_verify_field(td, 3, 1, 1) /* mask */)) return ret;
    if ((ret = flatcc_verify_field(td, 4, 1, 1) /* cmp_value */)) return ret;
    return flatcc_verify_ok;
}

static inline int rex_WaitUntil_verify_as_root(const void *buf, size_t bufsiz)
{
    return flatcc_verify_table_as_root(buf, bufsiz, rex_WaitUntil_identifier, &rex_WaitUntil_verify_table);
}

static inline int rex_WaitUntil_verify_as_root_with_size(const void *buf, size_t bufsiz)
{
    return flatcc_verify_table_as_root_with_size(buf, bufsiz, rex_WaitUntil_identifier, &rex_WaitUntil_verify_table);
}

static inline int rex_WaitUntil_verify_as_typed_root(const void *buf, size_t bufsiz)
{
    return flatcc_verify_table_as_root(buf, bufsiz, rex_WaitUntil_type_identifier, &rex_WaitUntil_verify_table);
}

static inline int rex_WaitUntil_verify_as_typed_root_with_size(const void *buf, size_t bufsiz)
{
    return flatcc_verify_table_as_root_with_size(buf, bufsiz, rex_WaitUntil_type_identifier, &rex_WaitUntil_verify_table);
}

static inline int rex_WaitUntil_verify_as_root_with_identifier(const void *buf, size_t bufsiz, const char *fid)
{
    return flatcc_verify_table_as_root(buf, bufsiz, fid, &rex_WaitUntil_verify_table);
}

static inline int rex_WaitUntil_verify_as_root_with_identifier_and_size(const void *buf, size_t bufsiz, const char *fid)
{
    return flatcc_verify_table_as_root_with_size(buf, bufsiz, fid, &rex_WaitUntil_verify_table);
}

static inline int rex_WaitUntil_verify_as_root_with_type_hash(const void *buf, size_t bufsiz, flatbuffers_thash_t thash)
{
    return flatcc_verify_table_as_typed_root(buf, bufsiz, thash, &rex_WaitUntil_verify_table);
}

static inline int rex_WaitUntil_verify_as_root_with_type_hash_and_size(const void *buf, size_t bufsiz, flatbuffers_thash_t thash)
{
    return flatcc_verify_table_as_typed_root_with_size(buf, bufsiz, thash, &rex_WaitUntil_verify_table);
}

static int rex_WaitWhileZero_verify_table(flatcc_table_verifier_descriptor_t *td)
{
    int ret;
    if ((ret = flatcc_verify_field(td, 0, 2, 2) /* chip_id */)) return ret;
    if ((ret = flatcc_verify_field(td, 1, 4, 4) /* chip_addr */)) return ret;
    return flatcc_verify_ok;
}

static inline int rex_WaitWhileZero_verify_as_root(const void *buf, size_t bufsiz)
{
    return flatcc_verify_table_as_root(buf, bufsiz, rex_WaitWhileZero_identifier, &rex_WaitWhileZero_verify_table);
}

static inline int rex_WaitWhileZero_verify_as_root_with_size(const void *buf, size_t bufsiz)
{
    return flatcc_verify_table_as_root_with_size(buf, bufsiz, rex_WaitWhileZero_identifier, &rex_WaitWhileZero_verify_table);
}

static inline int rex_WaitWhileZero_verify_as_typed_root(const void *buf, size_t bufsiz)
{
    return flatcc_verify_table_as_root(buf, bufsiz, rex_WaitWhileZero_type_identifier, &rex_WaitWhileZero_verify_table);
}

static inline int rex_WaitWhileZero_verify_as_typed_root_with_size(const void *buf, size_t bufsiz)
{
    return flatcc_verify_table_as_root_with_size(buf, bufsiz, rex_WaitWhileZero_type_identifier, &rex_WaitWhileZero_verify_table);
}

static inline int rex_WaitWhileZero_verify_as_root_with_identifier(const void *buf, size_t bufsiz, const char *fid)
{
    return flatcc_verify_table_as_root(buf, bufsiz, fid, &rex_WaitWhileZero_verify_table);
}

static inline int rex_WaitWhileZero_verify_as_root_with_identifier_and_size(const void *buf, size_t bufsiz, const char *fid)
{
    return flatcc_verify_table_as_root_with_size(buf, bufsiz, fid, &rex_WaitWhileZero_verify_table);
}

static inline int rex_WaitWhileZero_verify_as_root_with_type_hash(const void *buf, size_t bufsiz, flatbuffers_thash_t thash)
{
    return flatcc_verify_table_as_typed_root(buf, bufsiz, thash, &rex_WaitWhileZero_verify_table);
}

static inline int rex_WaitWhileZero_verify_as_root_with_type_hash_and_size(const void *buf, size_t bufsiz, flatbuffers_thash_t thash)
{
    return flatcc_verify_table_as_typed_root_with_size(buf, bufsiz, thash, &rex_WaitWhileZero_verify_table);
}

static int rex_EnterProgramRequest_verify_table(flatcc_table_verifier_descriptor_t *td)
{
    int ret;
    if ((ret = flatcc_verify_union_vector_field(td, 1, 0, &rex_ProgramInstruction_union_verifier) /* instructions */)) return ret;
    return flatcc_verify_ok;
}

static inline int rex_EnterProgramRequest_verify_as_root(const void *buf, size_t bufsiz)
{
    return flatcc_verify_table_as_root(buf, bufsiz, rex_EnterProgramRequest_identifier, &rex_EnterProgramRequest_verify_table);
}

static inline int rex_EnterProgramRequest_verify_as_root_with_size(const void *buf, size_t bufsiz)
{
    return flatcc_verify_table_as_root_with_size(buf, bufsiz, rex_EnterProgramRequest_identifier, &rex_EnterProgramRequest_verify_table);
}

static inline int rex_EnterProgramRequest_verify_as_typed_root(const void *buf, size_t bufsiz)
{
    return flatcc_verify_table_as_root(buf, bufsiz, rex_EnterProgramRequest_type_identifier, &rex_EnterProgramRequest_verify_table);
}

static inline int rex_EnterProgramRequest_verify_as_typed_root_with_size(const void *buf, size_t bufsiz)
{
    return flatcc_verify_table_as_root_with_size(buf, bufsiz, rex_EnterProgramRequest_type_identifier, &rex_EnterProgramRequest_verify_table);
}

static inline int rex_EnterProgramRequest_verify_as_root_with_identifier(const void *buf, size_t bufsiz, const char *fid)
{
    return flatcc_verify_table_as_root(buf, bufsiz, fid, &rex_EnterProgramRequest_verify_table);
}

static inline int rex_EnterProgramRequest_verify_as_root_with_identifier_and_size(const void *buf, size_t bufsiz, const char *fid)
{
    return flatcc_verify_table_as_root_with_size(buf, bufsiz, fid, &rex_EnterProgramRequest_verify_table);
}

static inline int rex_EnterProgramRequest_verify_as_root_with_type_hash(const void *buf, size_t bufsiz, flatbuffers_thash_t thash)
{
    return flatcc_verify_table_as_typed_root(buf, bufsiz, thash, &rex_EnterProgramRequest_verify_table);
}

static inline int rex_EnterProgramRequest_verify_as_root_with_type_hash_and_size(const void *buf, size_t bufsiz, flatbuffers_thash_t thash)
{
    return flatcc_verify_table_as_typed_root_with_size(buf, bufsiz, thash, &rex_EnterProgramRequest_verify_table);
}

static int rex_EnterProgramResponse_verify_table(flatcc_table_verifier_descriptor_t *td)
{
    return flatcc_verify_ok;
}

static inline int rex_EnterProgramResponse_verify_as_root(const void *buf, size_t bufsiz)
{
    return flatcc_verify_table_as_root(buf, bufsiz, rex_EnterProgramResponse_identifier, &rex_EnterProgramResponse_verify_table);
}

static inline int rex_EnterProgramResponse_verify_as_root_with_size(const void *buf, size_t bufsiz)
{
    return flatcc_verify_table_as_root_with_size(buf, bufsiz, rex_EnterProgramResponse_identifier, &rex_EnterProgramResponse_verify_table);
}

static inline int rex_EnterProgramResponse_verify_as_typed_root(const void *buf, size_t bufsiz)
{
    return flatcc_verify_table_as_root(buf, bufsiz, rex_EnterProgramResponse_type_identifier, &rex_EnterProgramResponse_verify_table);
}

static inline int rex_EnterProgramResponse_verify_as_typed_root_with_size(const void *buf, size_t bufsiz)
{
    return flatcc_verify_table_as_root_with_size(buf, bufsiz, rex_EnterProgramResponse_type_identifier, &rex_EnterProgramResponse_verify_table);
}

static inline int rex_EnterProgramResponse_verify_as_root_with_identifier(const void *buf, size_t bufsiz, const char *fid)
{
    return flatcc_verify_table_as_root(buf, bufsiz, fid, &rex_EnterProgramResponse_verify_table);
}

static inline int rex_EnterProgramResponse_verify_as_root_with_identifier_and_size(const void *buf, size_t bufsiz, const char *fid)
{
    return flatcc_verify_table_as_root_with_size(buf, bufsiz, fid, &rex_EnterProgramResponse_verify_table);
}

static inline int rex_EnterProgramResponse_verify_as_root_with_type_hash(const void *buf, size_t bufsiz, flatbuffers_thash_t thash)
{
    return flatcc_verify_table_as_typed_root(buf, bufsiz, thash, &rex_EnterProgramResponse_verify_table);
}

static inline int rex_EnterProgramResponse_verify_as_root_with_type_hash_and_size(const void *buf, size_t bufsiz, flatbuffers_thash_t thash)
{
    return flatcc_verify_table_as_typed_root_with_size(buf, bufsiz, thash, &rex_EnterProgramResponse_verify_table);
}

static int rex_ExecuteProgramRequest_verify_table(flatcc_table_verifier_descriptor_t *td)
{
    return flatcc_verify_ok;
}

static inline int rex_ExecuteProgramRequest_verify_as_root(const void *buf, size_t bufsiz)
{
    return flatcc_verify_table_as_root(buf, bufsiz, rex_ExecuteProgramRequest_identifier, &rex_ExecuteProgramRequest_verify_table);
}

static inline int rex_ExecuteProgramRequest_verify_as_root_with_size(const void *buf, size_t bufsiz)
{
    return flatcc_verify_table_as_root_with_size(buf, bufsiz, rex_ExecuteProgramRequest_identifier, &rex_ExecuteProgramRequest_verify_table);
}

static inline int rex_ExecuteProgramRequest_verify_as_typed_root(const void *buf, size_t bufsiz)
{
    return flatcc_verify_table_as_root(buf, bufsiz, rex_ExecuteProgramRequest_type_identifier, &rex_ExecuteProgramRequest_verify_table);
}

static inline int rex_ExecuteProgramRequest_verify_as_typed_root_with_size(const void *buf, size_t bufsiz)
{
    return flatcc_verify_table_as_root_with_size(buf, bufsiz, rex_ExecuteProgramRequest_type_identifier, &rex_ExecuteProgramRequest_verify_table);
}

static inline int rex_ExecuteProgramRequest_verify_as_root_with_identifier(const void *buf, size_t bufsiz, const char *fid)
{
    return flatcc_verify_table_as_root(buf, bufsiz, fid, &rex_ExecuteProgramRequest_verify_table);
}

static inline int rex_ExecuteProgramRequest_verify_as_root_with_identifier_and_size(const void *buf, size_t bufsiz, const char *fid)
{
    return flatcc_verify_table_as_root_with_size(buf, bufsiz, fid, &rex_ExecuteProgramRequest_verify_table);
}

static inline int rex_ExecuteProgramRequest_verify_as_root_with_type_hash(const void *buf, size_t bufsiz, flatbuffers_thash_t thash)
{
    return flatcc_verify_table_as_typed_root(buf, bufsiz, thash, &rex_ExecuteProgramRequest_verify_table);
}

static inline int rex_ExecuteProgramRequest_verify_as_root_with_type_hash_and_size(const void *buf, size_t bufsiz, flatbuffers_thash_t thash)
{
    return flatcc_verify_table_as_typed_root_with_size(buf, bufsiz, thash, &rex_ExecuteProgramRequest_verify_table);
}

static int rex_InstructionOutput_verify_table(flatcc_table_verifier_descriptor_t *td)
{
    int ret;
    if ((ret = flatcc_verify_field(td, 0, 1, 1) /* error_code */)) return ret;
    if ((ret = flatcc_verify_vector_field(td, 1, 0, 1, 1, INT64_C(4294967295)) /* data */)) return ret;
    return flatcc_verify_ok;
}

static inline int rex_InstructionOutput_verify_as_root(const void *buf, size_t bufsiz)
{
    return flatcc_verify_table_as_root(buf, bufsiz, rex_InstructionOutput_identifier, &rex_InstructionOutput_verify_table);
}

static inline int rex_InstructionOutput_verify_as_root_with_size(const void *buf, size_t bufsiz)
{
    return flatcc_verify_table_as_root_with_size(buf, bufsiz, rex_InstructionOutput_identifier, &rex_InstructionOutput_verify_table);
}

static inline int rex_InstructionOutput_verify_as_typed_root(const void *buf, size_t bufsiz)
{
    return flatcc_verify_table_as_root(buf, bufsiz, rex_InstructionOutput_type_identifier, &rex_InstructionOutput_verify_table);
}

static inline int rex_InstructionOutput_verify_as_typed_root_with_size(const void *buf, size_t bufsiz)
{
    return flatcc_verify_table_as_root_with_size(buf, bufsiz, rex_InstructionOutput_type_identifier, &rex_InstructionOutput_verify_table);
}

static inline int rex_InstructionOutput_verify_as_root_with_identifier(const void *buf, size_t bufsiz, const char *fid)
{
    return flatcc_verify_table_as_root(buf, bufsiz, fid, &rex_InstructionOutput_verify_table);
}

static inline int rex_InstructionOutput_verify_as_root_with_identifier_and_size(const void *buf, size_t bufsiz, const char *fid)
{
    return flatcc_verify_table_as_root_with_size(buf, bufsiz, fid, &rex_InstructionOutput_verify_table);
}

static inline int rex_InstructionOutput_verify_as_root_with_type_hash(const void *buf, size_t bufsiz, flatbuffers_thash_t thash)
{
    return flatcc_verify_table_as_typed_root(buf, bufsiz, thash, &rex_InstructionOutput_verify_table);
}

static inline int rex_InstructionOutput_verify_as_root_with_type_hash_and_size(const void *buf, size_t bufsiz, flatbuffers_thash_t thash)
{
    return flatcc_verify_table_as_typed_root_with_size(buf, bufsiz, thash, &rex_InstructionOutput_verify_table);
}

static int rex_ExecuteProgramResponse_verify_table(flatcc_table_verifier_descriptor_t *td)
{
    int ret;
    if ((ret = flatcc_verify_table_vector_field(td, 0, 0, &rex_InstructionOutput_verify_table) /* outputs */)) return ret;
    return flatcc_verify_ok;
}

static inline int rex_ExecuteProgramResponse_verify_as_root(const void *buf, size_t bufsiz)
{
    return flatcc_verify_table_as_root(buf, bufsiz, rex_ExecuteProgramResponse_identifier, &rex_ExecuteProgramResponse_verify_table);
}

static inline int rex_ExecuteProgramResponse_verify_as_root_with_size(const void *buf, size_t bufsiz)
{
    return flatcc_verify_table_as_root_with_size(buf, bufsiz, rex_ExecuteProgramResponse_identifier, &rex_ExecuteProgramResponse_verify_table);
}

static inline int rex_ExecuteProgramResponse_verify_as_typed_root(const void *buf, size_t bufsiz)
{
    return flatcc_verify_table_as_root(buf, bufsiz, rex_ExecuteProgramResponse_type_identifier, &rex_ExecuteProgramResponse_verify_table);
}

static inline int rex_ExecuteProgramResponse_verify_as_typed_root_with_size(const void *buf, size_t bufsiz)
{
    return flatcc_verify_table_as_root_with_size(buf, bufsiz, rex_ExecuteProgramResponse_type_identifier, &rex_ExecuteProgramResponse_verify_table);
}

static inline int rex_ExecuteProgramResponse_verify_as_root_with_identifier(const void *buf, size_t bufsiz, const char *fid)
{
    return flatcc_verify_table_as_root(buf, bufsiz, fid, &rex_ExecuteProgramResponse_verify_table);
}

static inline int rex_ExecuteProgramResponse_verify_as_root_with_identifier_and_size(const void *buf, size_t bufsiz, const char *fid)
{
    return flatcc_verify_table_as_root_with_size(buf, bufsiz, fid, &rex_ExecuteProgramResponse_verify_table);
}

static inline int rex_ExecuteProgramResponse_verify_as_root_with_type_hash(const void *buf, size_t bufsiz, flatbuffers_thash_t thash)
{
    return flatcc_verify_table_as_typed_root(buf, bufsiz, thash, &rex_ExecuteProgramResponse_verify_table);
}

static inline int rex_ExecuteProgramResponse_verify_as_root_with_type_hash_and_size(const void *buf, size_t bufsiz, flatbuffers_thash_t thash)
{
    return flatcc_verify_table_as_typed_root_with_size(buf, bufsiz, thash, &rex_ExecuteProgramResponse_verify_table);
}

static int rex_RequestMessage_verify_table(flatcc_table_verifier_descriptor_t *td)
{
    int ret;
    if ((ret = flatcc_verify_union_field(td, 1, 0, &rex_Request_union_verifier) /* request */)) return ret;
    return flatcc_verify_ok;
}

static inline int rex_RequestMessage_verify_as_root(const void *buf, size_t bufsiz)
{
    return flatcc_verify_table_as_root(buf, bufsiz, rex_RequestMessage_identifier, &rex_RequestMessage_verify_table);
}

static inline int rex_RequestMessage_verify_as_root_with_size(const void *buf, size_t bufsiz)
{
    return flatcc_verify_table_as_root_with_size(buf, bufsiz, rex_RequestMessage_identifier, &rex_RequestMessage_verify_table);
}

static inline int rex_RequestMessage_verify_as_typed_root(const void *buf, size_t bufsiz)
{
    return flatcc_verify_table_as_root(buf, bufsiz, rex_RequestMessage_type_identifier, &rex_RequestMessage_verify_table);
}

static inline int rex_RequestMessage_verify_as_typed_root_with_size(const void *buf, size_t bufsiz)
{
    return flatcc_verify_table_as_root_with_size(buf, bufsiz, rex_RequestMessage_type_identifier, &rex_RequestMessage_verify_table);
}

static inline int rex_RequestMessage_verify_as_root_with_identifier(const void *buf, size_t bufsiz, const char *fid)
{
    return flatcc_verify_table_as_root(buf, bufsiz, fid, &rex_RequestMessage_verify_table);
}

static inline int rex_RequestMessage_verify_as_root_with_identifier_and_size(const void *buf, size_t bufsiz, const char *fid)
{
    return flatcc_verify_table_as_root_with_size(buf, bufsiz, fid, &rex_RequestMessage_verify_table);
}

static inline int rex_RequestMessage_verify_as_root_with_type_hash(const void *buf, size_t bufsiz, flatbuffers_thash_t thash)
{
    return flatcc_verify_table_as_typed_root(buf, bufsiz, thash, &rex_RequestMessage_verify_table);
}

static inline int rex_RequestMessage_verify_as_root_with_type_hash_and_size(const void *buf, size_t bufsiz, flatbuffers_thash_t thash)
{
    return flatcc_verify_table_as_typed_root_with_size(buf, bufsiz, thash, &rex_RequestMessage_verify_table);
}

static int rex_ResponseMessage_verify_table(flatcc_table_verifier_descriptor_t *td)
{
    int ret;
    if ((ret = flatcc_verify_union_field(td, 1, 0, &rex_Response_union_verifier) /* response */)) return ret;
    return flatcc_verify_ok;
}

static inline int rex_ResponseMessage_verify_as_root(const void *buf, size_t bufsiz)
{
    return flatcc_verify_table_as_root(buf, bufsiz, rex_ResponseMessage_identifier, &rex_ResponseMessage_verify_table);
}

static inline int rex_ResponseMessage_verify_as_root_with_size(const void *buf, size_t bufsiz)
{
    return flatcc_verify_table_as_root_with_size(buf, bufsiz, rex_ResponseMessage_identifier, &rex_ResponseMessage_verify_table);
}

static inline int rex_ResponseMessage_verify_as_typed_root(const void *buf, size_t bufsiz)
{
    return flatcc_verify_table_as_root(buf, bufsiz, rex_ResponseMessage_type_identifier, &rex_ResponseMessage_verify_table);
}

static inline int rex_ResponseMessage_verify_as_typed_root_with_size(const void *buf, size_t bufsiz)
{
    return flatcc_verify_table_as_root_with_size(buf, bufsiz, rex_ResponseMessage_type_identifier, &rex_ResponseMessage_verify_table);
}

static inline int rex_ResponseMessage_verify_as_root_with_identifier(const void *buf, size_t bufsiz, const char *fid)
{
    return flatcc_verify_table_as_root(buf, bufsiz, fid, &rex_ResponseMessage_verify_table);
}

static inline int rex_ResponseMessage_verify_as_root_with_identifier_and_size(const void *buf, size_t bufsiz, const char *fid)
{
    return flatcc_verify_table_as_root_with_size(buf, bufsiz, fid, &rex_ResponseMessage_verify_table);
}

static inline int rex_ResponseMessage_verify_as_root_with_type_hash(const void *buf, size_t bufsiz, flatbuffers_thash_t thash)
{
    return flatcc_verify_table_as_typed_root(buf, bufsiz, thash, &rex_ResponseMessage_verify_table);
}

static inline int rex_ResponseMessage_verify_as_root_with_type_hash_and_size(const void *buf, size_t bufsiz, flatbuffers_thash_t thash)
{
    return flatcc_verify_table_as_typed_root_with_size(buf, bufsiz, thash, &rex_ResponseMessage_verify_table);
}

#include "flatcc/flatcc_epilogue.h"
#endif /* REX_VERIFIER_H */

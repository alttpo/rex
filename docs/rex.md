REX: retro extensions
---

types
--
    [--------] denotes a bit field where bits are grouped in 8-bit bytes
        MSB is the leftmost and LSB is the rightmost bit of each 8-bit byte

    u8:  unsigned 8-bit integer
    n8:  unsigned 8-bit integer where 0 is interpreted as 256 for non-zero length purposes
    u16: unsigned 16-bit integer
    n16: unsigned 16-bit integer where 0 is interpreted as 65536 for non-zero length purposes
    u24: unsigned 24-bit integer
    u32: unsigned 32-bit integer

    buf: { // 1..256 byte buffer
        n8  length;
        u8  data[length];
    }

    str: { // 0..255 byte text string (UTF-8)
        u8  length;
        u8  text[length];
    }

BINARY PROTOCOL
--
protocol is entirely byte-oriented with little-endian byte order unless specified otherwise

initial communication MUST begin with the client sending a handshake request and the server sends back a
handshake response

    handshakeRequest: {
        u24  magic;                     // magic number 'REX' aka 0x584552 in little-endian byte order
        u8   versionMajor;              // request major version of protocol (for breaking protocol changes)
        u8   versionMinor;              // request minor version of protocol (for non-breaking protocol changes)
        u16  requestSize;               // handshake request size beyond this field (should be 0 for v1.x)

        union {
            struct {} v1_x;
        }
    }

    handshakeResponse: {
        u24  magic;                     // magic number 'REX' aka 0x584552 in little-endian byte order
        u8   versionMajor;              // max supported major version of protocol
        u8   versionMinor;              // max supported minor version of protocol
        u8   error;                     // 00 = success
                                        // FF = unrecognized request
                                        // FE = unsupported major version
                                        // FD = unsupported minor version
        u16  responseSize;              // handshake response size beyond this field (should be 16 for v1.x)

        union {
            struct {
                u8   subsystems[16];    // bitset of supported subsystem ids (little endian byte order, LSB first)
            } v1_x;
        }
    }

if the handshake response returns with error=FF then the server must close the connection

if a non-zero error is returned in the handshake response then the server expects another handshake request until
it contains a version compatible with the version the server responded with

messages are streamed in request/response pairs

every request message MUST have a corresponding response message

messages always begin with a request/response header and end with the optional message payload

    request: {
        union {
            struct {
                u8   subsystem;     // subsystem id
                u8   opcode;        // opcode of command
            } v1_x;
        } header;
        frame frames[...];          // one or more frames of request data
    }

    response: {
        union {
            struct {
                u8   subsystem;     // subsystem id
                u8   opcode;        // opcode of command
                u8   error;         // 00 = success
                                    // FF = unsupported opcode
                                    // else error code defined by subsystem and opcode
            } v1_x;
        } header;
        frame frames[...];          // one or more frames of response data
    }

requests with unsupported opcodes MUST have their request messages discarded and responded to with error=FF

reserved opcodes for future extension:

    FF
    FE

message data (after the header) is framed with the following repeatable structure:

    frame: {
        u8   header;        // [fessssss]; f=final, e=64-byte frame, s=size of data
        u8   data[size];    // data for frame, 0..64 bytes
    }

    if f = 1 then this frame terminates the current message
    if e = 0 then s is interpreted as the data length from 0..63
    if e = 1 then this frame has a 64-byte data length, s is ignored
    zero-byte frames are allowed to make it easier to finalize a message

    framing helps avoid buffering large messages in memory where RAM is at a premium like in embedded systems

SUBSYSTEM IDS (00..7F)
---
* 00: [system control](sys00-control.md)
* 01: [memory chip access](sys01-memory.md)
* 02: [filesystem access](sys02-filesystem.md)
* 03: [PPUX](sys03-ppux.md)

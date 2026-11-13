# Wire protocol

Single source of truth: `main/core/Packets.h`. If this page and that header
disagree, the header wins.

## Frame layout

```
0xAA 0x55 | cmd:u8 | payload[PayloadLenForCommand(cmd)] | checksum:u8
```

* **There is no length byte on the wire.** The parser calls
  `PayloadLenForCommand(cmd)` to learn how many payload bytes follow. A host
  that sends a length field will desync the firmware immediately — this is the
  number one integration mistake.
* **checksum** = `cmd`, XOR-ed with every payload byte.
* Header constants are `HEADER_1` = `0xAA`, `HEADER_2` = `0x55` in
  `modules/Communication.h`.

## Encoding

* Integers and floats are little-endian; `float` is 4 bytes, `int` is 4 bytes.
* Payloads are packed C structs, `memcpy`-ed verbatim — no field-by-field
  encoding. Host and device must agree on endianness and `float` size.
* `sizeof(int)` is used both by the parser's length table and by
  `executeCommand` when it validates a joint index, so a host sending a
  4-byte index matches an ESP32 (where `int` is 32-bit).

Wire structs:

| Struct | Size | Fields |
|---|---|---|
| `Vector3Data` | 12 B | `float x, y, z` |
| `QuaternionData` | 16 B | `float w, x, y, z` |
| `InputChannelsData` | 28 B | `float ch[7]` |

`Vector3Packet`, `QuaternionPacket` and `InputChannelsPacket` wrap these with
`pack()` / `unpack()` / `getPacket()` / `setPacket()`.

## Parser

`TaskSerialParse` in `modules/Communication.cpp` runs a 5-state machine:
header1 → header2 → command → payload → checksum. Both `SerialComA_Task`
(UART1) and `SerialUSB_Task` (USB CDC) run the same function with a different
port in `pvParameters`.

* Wrong second header byte → back to state 0, silently.
* Checksum mismatch → `logW(log1, "Checksum error")`, buffer cleared, back to
  state 0.
* **Unknown opcodes are not rejected.** `PayloadLenForCommand` returns 0 for
  anything outside the enum (`default: return 0`), so an unknown command is
  treated as a zero-payload command, consumes the next byte as checksum, and
  then dispatches to `executeCommand`'s `default:` case which does nothing.
  A garbage opcode with a matching XOR therefore swallows one byte and resyncs
  — it does not corrupt the stream, but it is not an error either.
* The read loop polls `port->available()` then `vTaskDelay(10ms)`; there is no
  blocking read.

## Commands

| Opcode | Name | Payload in | Reply |
|---|---|---|---|
| `0x01` | Ping | — | **none** (logs `Ping OK` only) |
| `0x10` | Set quaternion offset | 16 B | **none** |
| `0x11` | Set vector3 offset | 12 B | **none** |
| `0x12` | Set useIMU | 1 B | **none** |
| `0x13` | Set Wi-Fi SSID | 32 B | **none** |
| `0x14` | Set Wi-Fi password | 32 B | **none** |
| `0x15` | Set BT name | 32 B | **none** |
| `0x16` | Set BT password | 32 B | **none** |
| `0x17` | Set IMU quaternion offset | 16 B | **none** |
| `0x20` | Get joint position | 4 B int index | `0x20` + 12 B |
| `0x21` | Get joint quaternion | 4 B int index | `0x21` + 16 B |
| `0x22` | Get joint angle (deg) | 4 B int index | `0x22` + 4 B float |
| `0x23` | Get joint angle (rad) | 4 B int index | `0x23` + 4 B float |
| `0x24` | Get joint count | — | `0x24` + 1 B u8 |
| `0x30` | Set input channels | 28 B | **none** |
| `0x31` | Get input channels | — | `0x31` + 28 B |
| `0x40` | Read unread log | — | `0x99` + text, **max 255 B** |
| `0x41` | Read full log history | — | `0x99` + text, **max 255 B** |
| `0x50` | Power off | — | **none** |
| `0x99` | Log reply | — | *reply only, never sent by host* |

### Acknowledgements

**The setters do not acknowledge.** `0x10`–`0x17`, `0x30` and `0x50` perform
their work and return nothing on the wire. They log a line to the ring buffer
instead (`Quat Saved & Applied`, `SSID Saved & Applied`, `GoodBye`, …).

A host that waits for an ack after a setter will hang. Either poll with
`0x40` to read the log, or fire-and-forget.

### Log replies (`0x40` / `0x41`)

* Reply opcode is `0x99`, not the request opcode.
* Both commands return **at most 255 bytes**, truncated silently at 255.
* It is a **single frame, not chunked.** There is no continuation, no "more
  data" flag, and no guarantee the text ends on a newline — a hard cut at 255
  bytes can split a line in half.
* `0x40` drains the unread cursor (subsequent reads return only newer lines).
* `0x41` snapshots the whole buffer without moving the unread cursor.
* Internal buffers are 1025 B, so on a full ring buffer the reply is truncated
  to 255 regardless of how much is buffered.

### Joint index validation

`0x20`–`0x23` check `idx >= 0 && idx < Kinematics.getJointCount()`.
Out of range → **no reply at all**, silently. `0x24` exists so a host can
discover the valid range up front; use it instead of guessing `6`.

## Adding a command

1. Add the opcode to `enum class Cmd` in `core/Packets.h`.
2. Add its inbound payload length to `PayloadLenForCommand()`. Without this,
   the parser cannot know where the frame ends.
3. Handle it in `executeCommand()` in `modules/Communication.cpp`.
4. If it replies, `sendFrame(port, cmd, buffer, len)` — the reply opcode is
   the request opcode unless you are the log path.
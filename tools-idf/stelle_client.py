#!/usr/bin/env python3
"""Minimal host client for the Stelle protocol.

    python stelle_client.py COM3 ping
    python stelle_client.py COM3 angle 0
    python stelle_client.py COM3 count
    python stelle_client.py COM3 log

Frame: 0xAA 0x55 | cmd | payload | xor   (no length byte on the wire)
"""

import struct
import sys

try:
    import serial
except ImportError:
    sys.exit("pip install pyserial")

HEADER = b"\xaa\x55"

REPLY_LEN = {0x20: 12, 0x21: 16, 0x22: 4, 0x23: 4, 0x24: 1, 0x31: 28}


def build_frame(opcode, payload=b""):
    body = bytes([opcode]) + payload
    checksum = 0
    for b in body:
        checksum ^= b
    return HEADER + body + bytes([checksum])


def read_frame(ser, expected, timeout_s=1.0):
    ser.timeout = timeout_s
    state = 0
    checksum = 0
    need = REPLY_LEN.get(expected, 0)
    buf = bytearray()

    while True:
        chunk = ser.read(1)
        if not chunk:
            return None
        byte = chunk[0]

        if state == 0:
            state = 1 if byte == 0xAA else 0
        elif state == 1:
            state = 2 if byte == 0x55 else 0
        elif state == 2:
            if byte != expected:
                state = 0
                continue
            checksum = byte
            buf = bytearray()
            if expected in REPLY_LEN:
                state = 3
            else:
                state = 5  # text reply: run until a byte equals the checksum
        elif state == 3:
            buf.append(byte)
            checksum ^= byte
            if len(buf) == need:
                return bytes(buf)
        elif state == 5:
            # A text chunk ends where the next byte would validate as checksum.
            if checksum == byte:
                return bytes(buf)
            buf.append(byte)
            checksum ^= byte


def main():
    if len(sys.argv) < 3:
        sys.exit(__doc__)

    port, command = sys.argv[1], sys.argv[2].lower()

    if command == "ping":
        opcode, payload, expect = 0x01, b"", 0x01
    elif command in ("angle", "deg"):
        opcode, payload, expect = 0x22, struct.pack("<i", int(sys.argv[3])), 0x22
    elif command == "rad":
        opcode, payload, expect = 0x23, struct.pack("<i", int(sys.argv[3])), 0x23
    elif command == "pos":
        opcode, payload, expect = 0x20, struct.pack("<i", int(sys.argv[3])), 0x20
    elif command == "quat":
        opcode, payload, expect = 0x21, struct.pack("<i", int(sys.argv[3])), 0x21
    elif command == "count":
        opcode, payload, expect = 0x24, b"", 0x24
    elif command == "log":
        opcode, payload, expect = 0x40, b"", 0x99
    elif command == "logall":
        opcode, payload, expect = 0x41, b"", 0x99
    elif command == "poweroff":
        opcode, payload, expect = 0x50, b"", 0x50
    else:
        sys.exit("unknown command: " + command)

    with serial.Serial(port, 115200, timeout=1.0) as ser:
        ser.write(build_frame(opcode, payload))
        reply = read_frame(ser, expect)

    if reply is None:
        print("no reply (timeout or checksum error)")
        return

    if expect == 0x99:
        sys.stdout.write(reply.decode("utf-8", "replace"))
        sys.stdout.write("\n")
    elif len(reply) == 12:
        print("position:", struct.unpack("<3f", reply))
    elif len(reply) == 16:
        print("quaternion:", struct.unpack("<4f", reply))
    elif len(reply) == 4:
        print("angle:", struct.unpack("<f", reply)[0])
    elif len(reply) == 1:
        print("count:", reply[0])
    elif len(reply) == 28:
        print("channels:", struct.unpack("<7f", reply))
    else:
        print("reply:", reply.hex())


if __name__ == "__main__":
    main()

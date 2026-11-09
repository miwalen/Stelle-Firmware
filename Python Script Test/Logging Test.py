import sys
import time
import threading
import serial

# ====== SESUAIKAN ======
PORT = "COM12"          # Linux: "/dev/ttyUSB0"
BAUD = 115200
HEADER_1 = 0xAA        # ganti sesuai Communication.h
HEADER_2 = 0x55        # ganti sesuai Communication.h
IDLE_GAP = 0.05        # detik diam di jalur sebelum dianggap 1 paket selesai
# =======================

CMD_PING = 0x01
CMD_READ_UNREAD = 0x40
CMD_READ_ALL_HISTORY = 0x41
CMD_SHUTDOWN = 0x50
RESP_LOG = 0x99

running = True


def build_frame(cmd, payload=b""):
    chk = cmd
    for b in payload:
        chk ^= b
    return bytes([HEADER_1, HEADER_2, cmd]) + payload + bytes([chk])


def process(buf: bytes):
    """Cetak data raw dan hasil proses."""
    print(f"[RAW] ({len(buf)} byte) {buf.hex(' ')}")

    head = bytes([HEADER_1, HEADER_2])
    i = buf.find(head)
    if i < 0:
        # bukan frame protokol (mis. boot log ESP32 / teks debug)
        print(f"[TXT] {buf.decode('utf-8', errors='replace')!r}")
        return

    if i > 0:
        print(f"[PRE] {buf[:i].decode('utf-8', errors='replace')!r}")

    frame = buf[i:]
    if len(frame) < 4:
        print("[ERR] Frame terlalu pendek")
        return

    cmd = frame[2]
    body = frame[3:-1]
    chk = frame[-1]

    calc = cmd
    for b in body:
        calc ^= b
    ok = (calc == chk)
    status = "CHK OK" if ok else f"CHK ERR (hitung {calc:02X}, terima {chk:02X})"

    if cmd == RESP_LOG:
        text = body.decode("utf-8", errors="replace")
        print(f"[LOG | {status}]")
        print(text if text else "(kosong)", end="" if text.endswith("\n") else "\n")
    else:
        print(f"[CMD 0x{cmd:02X} | {status}] payload: {body.hex(' ')}")
    print()


def reader_thread(ser):
    """Baca terus-menerus; kalau jalur diam, proses buffer sebagai satu paket."""
    buf = bytearray()
    last_rx = 0.0
    while running:
        try:
            n = ser.in_waiting
            if n:
                buf += ser.read(n)
                last_rx = time.time()
            elif buf and time.time() - last_rx > IDLE_GAP:
                process(bytes(buf))
                buf.clear()
            else:
                time.sleep(0.005)
        except (serial.SerialException, OSError):
            print("[ERR] Koneksi serial terputus")
            break


def send(ser, cmd, label):
    ser.write(build_frame(cmd))
    print(f"[>] {label} dikirim")


def key_loop(ser):
    """Spasi = ping | w = read unread | s = all history | q = keluar."""
    global running

    def handle(ch):
        global running
        if ch == " ":
            send(ser, CMD_PING, "Ping (0x01)")
        elif ch in ("w", "W"):
            send(ser, CMD_READ_UNREAD, "Read Unread (0x40)")
        elif ch in ("s", "S"):
            send(ser, CMD_READ_ALL_HISTORY, "All History (0x41)")
        elif ch in ("e", "E"):
            send(ser, CMD_SHUTDOWN, "Shutdown (0x50)")
        elif ch in ("q", "Q"):
            running = False

    try:
        import msvcrt
        while running:
            if msvcrt.kbhit():
                handle(msvcrt.getwch())
            time.sleep(0.01)
    except ImportError:
        import termios, tty, select
        fd = sys.stdin.fileno()
        old = termios.tcgetattr(fd)
        try:
            tty.setcbreak(fd)
            while running:
                if select.select([sys.stdin], [], [], 0.05)[0]:
                    handle(sys.stdin.read(1))
        finally:
            termios.tcsetattr(fd, termios.TCSADRAIN, old)


def main():
    global running
    ser = serial.Serial(PORT, BAUD, timeout=0.05)
    time.sleep(2)              # tunggu ESP32 selesai reset/boot
    ser.reset_input_buffer()

    threading.Thread(target=reader_thread, args=(ser,), daemon=True).start()
    print("SPASI = ping | w = read unread | s = all history | e = power off | q = keluar\n")

    try:
        key_loop(ser)
    except KeyboardInterrupt:
        pass
    finally:
        running = False
        time.sleep(0.1)
        ser.close()
        print("\nSelesai.")


if __name__ == "__main__":
    main()
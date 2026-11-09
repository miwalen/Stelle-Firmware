import serial
import struct
import threading
import time
import queue

# ==========================================
# KONFIGURASI PROTOKOL & SERIAL
# ==========================================
# Sesuaikan port dengan OS Anda (Misal: 'COM3' untuk Windows, '/dev/ttyUSB0' untuk Linux/Mac)
SERIAL_PORT = 'COM12' 
BAUD_RATE = 115200

# Sesuaikan nilai header ini dengan definisi HEADER_1 dan HEADER_2 di Communication.h Anda
HEADER_1 = 0xAA 
HEADER_2 = 0x55

CMD_GET_DEGREE = 0x22
CMD_GET_RAD = 0x23

# State global
running = True
joint_angles = [0.0] * 6
request_queue = queue.Queue() # Untuk mencocokkan balasan dengan joint yang di-request

# ==========================================
# THREAD PEMBACA SERIAL (PARSER)
# ==========================================
def serial_reader(ser):
    global running
    state = 0
    cmd = 0
    payload_len = 0
    payload = bytearray()
    checksum_calc = 0

    while running:
        if ser.in_waiting > 0:
            b = ser.read(1)[0]

            if state == 0:
                if b == HEADER_1:
                    state = 1
            elif state == 1:
                if b == HEADER_2:
                    state = 2
                else:
                    state = 0
            elif state == 2:
                cmd = b
                checksum_calc = b
                
                # Command 0x22 (Degree) atau 0x23 (Rad) membalas dengan 4 byte float
                if cmd in [CMD_GET_DEGREE, CMD_GET_RAD]:
                    payload_len = 4
                    state = 3
                    payload = bytearray()
                else:
                    state = 0 # Abaikan command lain untuk script ini
                    
            elif state == 3:
                payload.append(b)
                checksum_calc ^= b
                if len(payload) == payload_len:
                    state = 4
                    
            elif state == 4:
                if b == checksum_calc:
                    # Packet valid, ekstrak float (little-endian)
                    angle = struct.unpack('<f', payload)[0]
                    
                    # Ambil indeks joint dari queue berdasarkan request terakhir
                    if not request_queue.empty():
                        joint_idx = request_queue.get()
                        joint_angles[joint_idx] = angle
                else:
                    print("[Error] Checksum mismatch")
                state = 0

# ==========================================
# FUNGSI PENGIRIM COMMAND
# ==========================================
def send_get_angle(ser, joint_idx, use_rad=False):
    cmd = CMD_GET_RAD if use_rad else CMD_GET_DEGREE
    
    # Pack index sebagai integer 32-bit (4 byte) little-endian
    payload = struct.pack('<i', joint_idx)
    
    # Kalkulasi checksum
    checksum = cmd
    for b in payload:
        checksum ^= b
        
    # Susun paket: [H1][H2][CMD][PAYLOAD(4 byte)][CHK]
    packet = bytearray([HEADER_1, HEADER_2, cmd]) + payload + bytearray([checksum])
    
    ser.write(packet)
    request_queue.put(joint_idx) # Masukkan ke antrian tracking

# ==========================================
# MAIN LOOP (10 Hz)
# ==========================================
if __name__ == '__main__':
    try:
        ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=0.1)
        print(f"Berhasil membuka {SERIAL_PORT}")
    except Exception as e:
        print(f"Gagal membuka port serial: {e}")
        exit()

    # Mulai thread pembaca
    reader_thread = threading.Thread(target=serial_reader, args=(ser,), daemon=True)
    reader_thread.start()

    print("Memulai request data pada 10 Hz. Tekan Ctrl+C untuk berhenti.")
    
    try:
        while running:
            loop_start = time.time()

            # Request data untuk joint 0 sampai 5
            for i in range(6):
                send_get_angle(ser, i, use_rad=False) # Ubah True jika ingin Radian
                # Delay sangat kecil untuk memberi nafas buffer serial
                time.sleep(0.002)

            # Print output frame saat ini (menimpa baris sebelumnya di console)
            angles_str = " | ".join([f"J{i}: {ang:6.2f}°" for i, ang in enumerate(joint_angles)])
            print(f"\r{angles_str}", end="")

            # Pastikan loop berjalan pas ~10 kali per detik (100ms)
            elapsed = time.time() - loop_start
            if elapsed < 0.1:
                time.sleep(0.1 - elapsed)

    except KeyboardInterrupt:
        print("\nMenghentikan script...")
        running = False
        reader_thread.join()
        ser.close()
        print("Selesai.")
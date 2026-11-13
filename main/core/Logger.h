#pragma once

#include <cstdarg>
#include <cstdio>
#include <cstring>

namespace stelle {

/**
 * Logger - ring buffer teks berukuran tetap, line-oriented.
 * Logika identik dengan versi Arduino. Tidak ada lock di dalam: pemanggil
 * yang memegang mutexLog.
 */
class Logger {
public:
    static const int BUFFER_SIZE = CONFIG_STELLE_RING_BYTES;
    static const int OUTPUT_BUFFER_SIZE = BUFFER_SIZE + 1;

private:
    char buffer[BUFFER_SIZE];

    int writeIndex = 0;
    int oldestIndex = 0;
    int unreadStartIndex = 0;
    bool bufferFull = false;
    bool unreadFull = false;

    static int advance(int index) { return (index + 1) % BUFFER_SIZE; }

    // Membuang data tertua sampai (dan termasuk) '\n' pertama.
    void discardOldestLine() {
        if (oldestIndex == writeIndex && !bufferFull) return;

        do {
            char dropped = buffer[oldestIndex];
            oldestIndex = advance(oldestIndex);
            if (dropped == '\n') break;
        } while (oldestIndex != writeIndex);
    }

    void discardUnreadLine() {
        if (unreadStartIndex == writeIndex && !unreadFull) return;

        do {
            char dropped = buffer[unreadStartIndex];
            unreadStartIndex = advance(unreadStartIndex);
            if (dropped == '\n') break;
        } while (unreadStartIndex != writeIndex);
    }

    void writeChar(char c) {
        if (bufferFull) {
            discardOldestLine();
            bufferFull = false;
        }
        if (unreadFull) {
            discardUnreadLine();
            unreadFull = false;
        }

        buffer[writeIndex] = c;
        writeIndex = advance(writeIndex);

        if (writeIndex == oldestIndex) {
            bufferFull = true;
        }

        if (writeIndex == unreadStartIndex) {
            unreadFull = true;
        }
    }

public:
    bool begin() {
        writeIndex = 0;
        oldestIndex = 0;
        unreadStartIndex = 0;
        bufferFull = false;
        unreadFull = false;
        buffer[0] = '\0';
        return true;
    }

    void log(const char* message) {
        for (int i = 0; message[i] != 0; i++) {
            writeChar(message[i]);
        }
    }

    void readUnread(char* outBuffer) {
        if (unreadStartIndex == writeIndex && !unreadFull) {
            outBuffer[0] = '\0';
            return;
        }

        int current = unreadStartIndex;
        int length = 0;

        // Gunakan do-while untuk menangani kasus unreadFull (posisi sama tapi terisi)
        do {
            outBuffer[length++] = buffer[current];
            current = advance(current);
        } while (current != writeIndex);

        outBuffer[length] = '\0';

        unreadStartIndex = writeIndex;
        unreadFull = false;
    }

    void readAllHistory(char* outBuffer) const {
        if (oldestIndex == writeIndex && !bufferFull) {
            outBuffer[0] = '\0';
            return;
        }

        int length = 0;
        int current = oldestIndex;

        do {
            outBuffer[length++] = buffer[current];
            current = advance(current);
        } while (current != writeIndex);

        outBuffer[length] = '\0';
    }
};

// Instance global, deklarasinya sama seperti pemakaian log1 di Stelle.ino
extern Logger log1;

// ============================================================
// Fungsi log bebas (bukan member class). Menerima Logger tujuan supaya
// jelas mau menulis ke instance mana, dan mengambil mutexLog sendiri.
// Awalan tiap baris: "[INFO] ...", "[WARN] ...", "[ERROR] ..."
// ============================================================
void logI(Logger& target, const char* fmt, ...) __attribute__((format(printf, 2, 3)));
void logW(Logger& target, const char* fmt, ...) __attribute__((format(printf, 2, 3)));
void logE(Logger& target, const char* fmt, ...) __attribute__((format(printf, 2, 3)));

} // namespace stelle
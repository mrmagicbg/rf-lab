#include "cc1100_raspi.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>
#include <signal.h>

// Simple CSV packet logger and Manchester decoder for TPMS

static FILE *g_logfile = NULL;
static volatile int g_running = 1;

static void signal_handler(int sig) {
    (void)sig;
    g_running = 0;
}

static FILE *open_packet_log(int mode_sel, char *outpath, int outpath_len) {
    time_t now = time(NULL);
    struct tm t;
    localtime_r(&now, &t);
    char dirname[] = "../logs"; // run from CC1101 dir
    mkdir(dirname, 0755);

    char fname[256];
    snprintf(fname, sizeof(fname), "%s/packets-mode0x%02X-%04d%02d%02d-%02d%02d%02d.csv",
             dirname, mode_sel,
             t.tm_year+1900, t.tm_mon+1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);

    FILE *f = fopen(fname, "w");
    if(!f) return NULL;
    
    if(outpath && outpath_len > 0) {
        snprintf(outpath, outpath_len, "%s", fname);
    }
    
    fprintf(f, "timestamp,mode,raw_len,raw_hex,decoded,fields\n");
    fflush(f);
    return f;
}

// Helper: hex dump
static void hex_encode(const uint8_t *data, int len, char *out, int outlen) {
    int pos = 0;
    for(int i=0;i<len && pos+3<outlen;i++) {
        pos += snprintf(out+pos, outlen-pos, "%02X", data[i]);
    }
}

// Basic Manchester decoder for TPMS raw bit buffer
// Note: This is a simplified decoder. Real TPMS may need preamble detection and sync.
// Input: raw packet bytes. Output: decoded bytes written to outbuf, returns byte length or -1 on fail.
static int manchester_decode_bytes(const uint8_t *data, int len, uint8_t *outbuf, int outbuf_len) {
    if(len < 2) return -1;
    
    // Simple byte-level Manchester decode: treat each byte pair as encoded nibbles
    // This is a placeholder - real implementation needs bit-level decoding
    int outpos = 0;
    for(int i = 0; i < len && outpos < outbuf_len; i++) {
        // For now, just pass through raw data
        // TODO: Implement proper Manchester decoding with sync word detection
        outbuf[outpos++] = data[i];
    }
    
    return outpos;
}

// Simple demo to select a CC1101 profile and dump register configuration after applying.
// Usage: ./rx_profile_demo -mTPMS | -mIoT | -mOOK | -mGFSK100
// Additional options:
//   -addr <dec>      Set node address (default 1)
//   -freq <1|2|3|4>  ISM band: 1=315,2=433,3=868,4=915 (profile overrides may retune)
//   -channel <n>     Channel number (default 0)
// Profiles added:
//   TPMS (mode 0x07) and IoT (mode 0x08)

static void usage() {
    printf("CC1101 RX Profile Demo\n");
    printf("Usage: rx_profile_demo [-mTPMS|-mIoT|-mGFSK100|-mOOK] [options]\n");
    printf("Options:\n");
    printf("  -addr <dec>       Node address (default 1)\n");
    printf("  -freq <1|2|3|4>   ISM band select (default 3=868)\n");
    printf("  -channel <n>      Channel (default 0)\n");
    printf("  -h                Help\n");
}

int main(int argc, char *argv[]) {
    uint8_t addr = 1;
    int mode = 0x03;          // default GFSK_100_kb
    int freq = 0x03;          // default 868.3 MHz
    int channel = 0;          // default channel

    for(int i=1;i<argc;i++) {
        if(strcmp(argv[i],"-h")==0) { usage(); return 0; }
        else if(strcmp(argv[i],"-mTPMS")==0) { mode = 0x07; freq = 0x02; } // TPMS uses 433.92 MHz
        else if(strcmp(argv[i],"-mIoT")==0) { mode = 0x08; freq = 0x03; }  // IoT IT+ at 868.3 MHz
        else if(strcmp(argv[i],"-mGFSK100")==0) { mode = 0x03; }
        else if(strcmp(argv[i],"-mOOK")==0) { mode = 0x06; }
        else if(strcmp(argv[i],"-addr")==0 && i+1<argc) { addr = (uint8_t)atoi(argv[++i]); }
        else if(strcmp(argv[i],"-freq")==0 && i+1<argc) { freq = atoi(argv[++i]); }
        else if(strcmp(argv[i],"-channel")==0 && i+1<argc) { channel = atoi(argv[++i]); }
        else {
            printf("Unknown argument: %s\n", argv[i]);
            usage();
            return 1;
        }
    }

    CC1100 radio;
    radio.set_debug_level(1);

    volatile uint8_t my_addr = addr;
    extern int cc1100_freq_select; cc1100_freq_select = freq;
    extern int cc1100_mode_select; cc1100_mode_select = mode;
    extern int cc1100_channel_select; cc1100_channel_select = channel;

    if(radio.begin(my_addr) == FALSE) {
        printf("Failed to init CC1101 (check wiring/SPI).\n");
        return 2;
    }

    printf("Applied profile mode=0x%02X freq_sel=%d channel=%d addr=%d\n", mode, freq, channel, addr);
    radio.show_register_settings();

    // Setup signal handler for graceful exit
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    // Open packet log
    char logpath[512] = {0};
    FILE *logf = open_packet_log(mode, logpath, sizeof(logpath));
    g_logfile = logf;
    
    if(!logf) {
        printf("Warning: could not open packet log file; continuing without logging.\n");
    } else {
        printf("Logging packets to: %s\n", logpath);
    }

    printf("Listening for packets, press Ctrl-C to exit...\n");

    while(g_running) {
        // wait up to 2000ms for a packet
        if(radio.wait_for_packet(2000) == TRUE) {
            uint8_t rxbuf[64];
            uint8_t pktlen=0;
            uint8_t myaddr = 0, sender = 0; int8_t rssi = 0; uint8_t lqi = 0;
            uint8_t got = radio.get_payload(rxbuf, pktlen, myaddr, sender, rssi, lqi);
            if(got) {
                // Log timestamp, mode, raw len, raw hex, decoded, fields
                char hexdump[256] = {0};
                hex_encode(rxbuf, pktlen, hexdump, sizeof(hexdump));
                time_t now = time(NULL);
                char timestr[64];
                strftime(timestr, sizeof(timestr), "%Y-%m-%d %H:%M:%S", localtime(&now));

                // Try TPMS decode if in TPMS mode
                char decoded[256] = "";
                char fields[256] = "";
                if(mode==0x07) {
                    // TPMS Manchester decode
                    uint8_t out[32];
                    int dlen = manchester_decode_bytes(rxbuf, pktlen, out, sizeof(out));
                    if(dlen >= 6) {
                        char idstr[64]={0};
                        // Typical TPMS: first 4 bytes ID, next bytes pressure/temp
                        snprintf(idstr, sizeof(idstr), "%02X%02X%02X%02X", out[0], out[1], out[2], out[3]);
                        snprintf(decoded, sizeof(decoded), "%d bytes", dlen);
                        snprintf(fields, sizeof(fields), "id=%s,pressure=%d,temp=%d", idstr, out[4], out[5]);
                    } else if(dlen > 0) {
                        snprintf(decoded, sizeof(decoded), "%d bytes (partial)", dlen);
                        snprintf(fields, sizeof(fields), "insufficient data");
                    } else {
                        snprintf(decoded, sizeof(decoded), "decode-fail");
                        snprintf(fields, sizeof(fields), "n/a");
                    }
                } else if(mode==0x08) {
                    // IoT OOK: just log raw hex and rssi
                    snprintf(decoded, sizeof(decoded), "raw");
                    snprintf(fields, sizeof(fields), "rssi=%d,lqi=%d", rssi, lqi);
                } else {
                    snprintf(decoded, sizeof(decoded), "raw");
                    snprintf(fields, sizeof(fields), "rssi=%d,lqi=%d", rssi, lqi);
                }

                if(logf) {
                    fprintf(logf, "%s,0x%02X,%d,%s,%s,%s\n", timestr, mode, pktlen, hexdump, decoded, fields);
                    fflush(logf);
                }

                printf("%s MODE=0x%02X LEN=%d RSSI=%d LQI=%d HEX=%s DECODE=%s %s\n", timestr, mode, pktlen, rssi, lqi, hexdump, decoded, fields);
            }
        }
    }

    printf("\nShutting down...\n");
    if(logf) {
        fclose(logf);
        printf("Log file closed: %s\n", logpath);
    }
    radio.end();
    return 0;
}

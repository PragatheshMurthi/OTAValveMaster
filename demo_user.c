#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include "comm.h"

#define MASTER_USER_PORT 46000
#define MAX_REQUESTS_PER_LINE 32
#define MAX_TOKENS 128

static int send_packet_to_master(const void *buf, size_t len)
{
    DBG_ENTRY;
    int s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s < 0) {
        perror("socket");
        print_err("[DemoUser] send_packet_to_master: socket open failed");
        DBG_EXIT;
        return -1;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = htons(MASTER_USER_PORT);

    ssize_t rc = sendto(s, buf, len, 0, (struct sockaddr *)&addr, sizeof(addr));
    close(s);

    if (rc == (ssize_t)len) {
        print_info("[DemoUser] send_packet_to_master: sent batch packet of %zu bytes to port %d", len, MASTER_USER_PORT);
        DBG_EXIT;
        return 0;
    }

    perror("sendto");
    print_err("[DemoUser] send_packet_to_master: send failed len=%zu rc=%zd", len, rc);
    DBG_EXIT;
    return -1;
}

int main(void)
{
    char line[1024]; 
    DBG_ENTRY;
    printf("Demo user: enter batch commands (e.g., 'OPEN_TIMED 1 120 CLOSED_TIMED 2 50 OPEN 3 CLOSE 4 CLOSE 5')\n");

    while (fgets(line, sizeof(line), stdin) != NULL) {
        UREQ_BUFFER request_list[MAX_REQUESTS_PER_LINE];
        uint32_t request_count = 0;
        
        // Linear array to hold pre-tokenized words
        char *tokens[MAX_TOKENS];
        int token_count = 0;

        char *saveptr = NULL;
        
        char *tok = strtok(line, " \t\r\n");
        
        // --- STEP 1: PRE-TOKENIZE THE ENTIRE INPUT INTO A FLAT LIST ---
        while (tok != NULL && token_count < MAX_TOKENS) {
            tokens[token_count++] = tok;
            tok = strtok(NULL, " \t\r\n");
        }

        // --- STEP 2: DETERMINISTIC VALUE INDEX PARSING ---
        int i = 0;
        while (i < token_count && request_count < MAX_REQUESTS_PER_LINE) {
            char *action = tokens[i];
            
            // Validate that an argument follows the command string
            if (i + 1 >= token_count) {
                print_info("[DemoUser] main: ignored trailing command without valve ID '%s'", action);
                break;
            }

            char *valve_str = tokens[i + 1];
            unsigned int vid = (unsigned int)strtoul(valve_str, NULL, 10);
            if (vid == 0) vid = 1;

            unsigned int duration = 0;
            uint8_t req_type = 0;
            int is_timed = 0;
            int valid_command = 1;

            // Differentiate matching structures cleanly
            if (strcasecmp(action, "OPEN") == 0) {
                req_type = REQUEST_VALVE_OPEN;
                is_timed = 0;
            } else if (strcasecmp(action, "CLOSE") == 0) {
                req_type = REQUEST_VALVE_CLOSE;
                is_timed = 0;
            } else if (strcasecmp(action, "STATUS") == 0) {
                req_type = REQUEST_VALVE_STATUS;
                is_timed = 0;
            } else if (strcasecmp(action, "OPEN_TIMED") == 0) {
                req_type = REQUEST_VALVE_OPEN_TIMED_CLOSE;
                is_timed = 1;
            } else if (strcasecmp(action, "CLOSE_TIMED") == 0 || strcasecmp(action, "CLOSED_TIMED") == 0) {
                req_type = REQUEST_VALVE_CLOSE_TIMED_OPEN;
                is_timed = 1;
            } else {
                printf("Unknown command: %s\n", action);
                print_info("[DemoUser] main: unknown command '%s'", action);
                valid_command = 0;
                // If the word is entirely garbled, skip 1 word to try finding alignment again
                i += 1; 
                continue;
            }

            // Extract optional parameter for timed configurations safely
            if (is_timed) {
                if (i + 2 < token_count) {
                    duration = (unsigned int)strtoul(tokens[i + 2], NULL, 10);
                    i += 3; // Step past: Command + ValveID + Duration
                } else {
                    duration = 5; // Default fallback fallback
                    i += 2; // Step past: Command + ValveID
                }
            } else {
                i += 2; // Step past: Command + ValveID
            }

            if (valid_command) {
                memset(&request_list[request_count], 0, sizeof(UREQ_BUFFER));
                request_list[request_count].u32ValveID = vid;
                request_list[request_count].u32TimerDuration = duration;
                request_list[request_count].u8RequestType = req_type;
                
                print_info("[DemoUser] main: parsed command indices %u: '%s' valve=%u duration=%u", 
                           request_count, action, vid, duration);
                request_count++;
            }
        }

        // --- STEP 3: DYNAMIC PACKET PREPARATION & NETWORK TRANSMISSION ---
        if (request_count == 0) {
            continue; 
        }

        size_t payload_bytes = request_count * sizeof(UREQ_BUFFER);
        size_t total_packet_size = sizeof(UREQ_METADATA) + payload_bytes;

        uint8_t *packet_buf = malloc(total_packet_size);
        if (!packet_buf) {
            print_err("[DemoUser] main: allocation error");
            DBG_EXIT;
            return 1;
        }
        memset(packet_buf, 0, total_packet_size);

        UREQ_METADATA meta = {0};
        meta.u32NumberOfRequests = request_count;
        meta.u32RequestSequenceNumber = (UINT32)(rand() & 0xFFFFu);
        meta.u32RequestLength = (UINT32)total_packet_size;
        meta.u32RequestCRC = 0;

        // Map memory sequentially: [Metadata Header][Request Payload Block Elements...]
        memcpy(packet_buf, &meta, sizeof(UREQ_METADATA));
        memcpy(packet_buf + sizeof(UREQ_METADATA), request_list, payload_bytes);

        // Apply dynamic calculation check across entire frame size
        ((UREQ_METADATA *)packet_buf)->u32RequestCRC = calculate_crc(packet_buf, (UINT16)total_packet_size);

        if (send_packet_to_master(packet_buf, total_packet_size) == 0) {
            printf("Successfully dispatched compound packet! Seq=%u Requests=%u TotalBytes=%zu\n", 
                    meta.u32RequestSequenceNumber, request_count, total_packet_size);
        } else {
            perror("send_packet_to_master failed");
        }

        free(packet_buf);
    }

    DBG_EXIT;
    return 0;
}

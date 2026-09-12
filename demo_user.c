#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include "comm.h"

#define MASTER_USER_PORT 46000

static int send_to_master(const void *buf, size_t len)
{
    DBG_ENTRY;
    int s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s < 0) {
        perror("socket");
        print_err("[DemoUser] send_to_master: socket open failed");
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
        print_info("[DemoUser] send_to_master: sent %zu bytes to port %d", len, MASTER_USER_PORT);
        DBG_EXIT;
        return 0;
    }

    perror("sendto");
    print_err("[DemoUser] send_to_master: send failed len=%zu rc=%zd", len, rc);
    DBG_EXIT;
    return -1;
}

int main(void)
{
    char line[128];
    DBG_ENTRY;
    printf("Demo user: enter commands like 'OPEN 1' or 'STATUS 2'\n");
    while (fgets(line, sizeof(line), stdin) != NULL) {
        unsigned int vid = 0;
        char cmd[32] = {0};
        if (sscanf(line, "%31s %u", cmd, &vid) < 1) {
            print_info("[DemoUser] main: ignored malformed input '%s'", line);
            continue;
        }

        print_info("[DemoUser] main: parsing command '%s' valve=%u", cmd, vid);
        UREQ_METADATA meta = {0};
        UREQ_BUFFER req = {0};
        size_t total = sizeof(meta) + sizeof(req);

        meta.u32NumberOfRequests = 1;
        meta.u32RequestSequenceNumber = (UINT32)(rand() & 0xFFFFu);
        meta.u32RequestLength = (UINT32)total;
        meta.u32RequestCRC = 0;

        if (vid == 0) vid = 1;
        req.u32ValveID = vid;
        if (strcasecmp(cmd, "OPEN") == 0) {
            req.u8RequestType = REQUEST_VALVE_OPEN;
            req.u32TimerDuration = 0;
        } else if (strcasecmp(cmd, "CLOSE") == 0) {
            req.u8RequestType = REQUEST_VALVE_CLOSE;
        } else if (strcasecmp(cmd, "STATUS") == 0) {
            req.u8RequestType = REQUEST_VALVE_STATUS;
        } else {
            printf("unknown command\n");
            print_info("[DemoUser] main: unknown command '%s'", cmd);
            continue;
        }

        uint8_t *buf = malloc(total);
        if (!buf) {
            print_err("[DemoUser] main: malloc failed for request buffer");
            DBG_EXIT;
            return 1;
        }
        memcpy(buf, &meta, sizeof(meta));
        memcpy(buf + sizeof(meta), &req, sizeof(req));

        /* compute CRC with CRC field zero */
        ((UREQ_METADATA *)buf)->u32RequestCRC = 0;
        ((UREQ_METADATA *)buf)->u32RequestCRC = calculate_crc(buf, (UINT16)total);

        if (send_to_master(buf, total) == 0) {
            printf("Sent request Seq=%u Vid=%u\n", meta.u32RequestSequenceNumber, req.u32ValveID);
        } else {
            perror("send_to_master");
        }

        free(buf);
    }
    DBG_EXIT;
    return 0;
}

#include "ipc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

/* Ports used for IPC communication (localhost UDP) */
#define MASTER_USER_RECV_PORT 46000
#define MASTER_SLAVE_RECV_PORT 46001
#define DEMO_USER_RECV_PORT 48000
#define SLAVE_APP_RECV_PORT 47001

static int g_master_user_fd = -1;
static int g_master_slave_fd = -1;

ERROR_CODE initialize_ipc(void)
{
    struct sockaddr_in addr;

    /* create socket to receive user requests */
    g_master_user_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (g_master_user_fd < 0) {
        perror("socket");
        return ERR_NET_IF_FAIL;
    }
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = htons(MASTER_USER_RECV_PORT);
    if (bind(g_master_user_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind user");
        close(g_master_user_fd);
        g_master_user_fd = -1;
        return ERR_NET_IF_FAIL;
    }

    /* create socket to receive slave acks */
    g_master_slave_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (g_master_slave_fd < 0) {
        perror("socket");
        close(g_master_user_fd);
        g_master_user_fd = -1;
        return ERR_NET_IF_FAIL;
    }
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = htons(MASTER_SLAVE_RECV_PORT);
    if (bind(g_master_slave_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind slave");
        close(g_master_user_fd);
        close(g_master_slave_fd);
        g_master_user_fd = -1;
        g_master_slave_fd = -1;
        return ERR_NET_IF_FAIL;
    }

    return ERR_OK;
}

static int udp_recv_into(int fd, void **ppvBuffer)
{
    if (fd < 0 || ppvBuffer == NULL) return ERR_INVALID_PARAM;
    *ppvBuffer = NULL;
    uint8_t buf[1024];
    ssize_t rc = recvfrom(fd, buf, sizeof(buf), 0, NULL, NULL);
    if (rc < 0) return ERR_TIMEOUT;
    void *p = malloc((size_t)rc);
    if (p == NULL) return ERR_MEMORY_ALLOCATION_FAIL;
    memcpy(p, buf, (size_t)rc);
    *ppvBuffer = p;
    return ERR_OK;
}

int receive_ipc_from_user(void **ppvBuffer)
{
    return udp_recv_into(g_master_user_fd, ppvBuffer);
}

int receive_ipc_from_slave(void **ppvBuffer)
{
    return udp_recv_into(g_master_slave_fd, ppvBuffer);
}

static int udp_send_to_port(int port, const void *pvBuffer, size_t uLen)
{
    if (pvBuffer == NULL || uLen == 0) return ERR_INVALID_PARAM;
    int s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s < 0) return ERR_NET_IF_FAIL;
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = htons(port);
    ssize_t rc = sendto(s, pvBuffer, uLen, 0, (struct sockaddr *)&addr, sizeof(addr));
    close(s);
    return (rc == (ssize_t)uLen) ? ERR_OK : ERR_NET_IF_FAIL;
}

int send_ipc_to_user(const void *pvBuffer)
{
    if (pvBuffer == NULL) return ERR_INVALID_PARAM;
    /* first 4 bytes expected to be length (UINT32) */
    UINT32 uLen = *((const UINT32 *)pvBuffer);
    return udp_send_to_port(DEMO_USER_RECV_PORT, pvBuffer, (size_t)uLen);
}

int send_ipc_to_slave(const void *pvBuffer)
{
    if (pvBuffer == NULL) return ERR_INVALID_PARAM;
    UINT32 uLen = *((const UINT32 *)pvBuffer);
    return udp_send_to_port(SLAVE_APP_RECV_PORT, pvBuffer, (size_t)uLen);
}

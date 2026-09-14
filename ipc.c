/* *****************************************************************************
 * @file        ipc.c
 * @brief       UDP-based inter-process communication transport.
 * @author      OTAValveMaster Contributors
 * @date        2026-09-14
 *
 * @license     Project License
 *              Copyright (c) 2026 OTAValveMaster Contributors
 *              All rights reserved.
 ******************************************************************************/

/*============================================================================*/
/*                                  INCLUDES                                  */
/*============================================================================*/
#include "ipc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

/*============================================================================*/
/*                         DEFINES & MACROS & ENUMS                           */
/*============================================================================*/

#define IPC_LOG_PREFIX "[IPC]"

/* Ports used for IPC communication */
#define MASTER_USER_RECV_PORT 46000
#define MASTER_SLAVE_RECV_PORT 46001
#define DEMO_USER_RECV_PORT 48000
#define SLAVE_APP_RECV_PORT 47001

/* Master -> Slave broadcast address */
#define SLAVE_BROADCAST_IP "255.255.255.255"

/*============================================================================*/
/*                            FUNCTION PROTOTYPES                             */
/*============================================================================*/

/*============================================================================*/
/*                          GLOBAL / STATIC VARIABLES                         */
/*============================================================================*/

/*============================================================================*/
/*                           FUNCTION DEFINITIONS                             */
/*============================================================================*/

static int g_master_user_fd = -1;
static int g_master_slave_fd = -1;

ERROR_CODE initialize_ipc(void)
{
    struct sockaddr_in addr;

    DBG_ENTRY;

    /* Create socket to receive user requests */
    g_master_user_fd = socket(AF_INET, SOCK_DGRAM, 0);

    if (g_master_user_fd < 0) {
        perror("socket");
        print_err("%s initialize_ipc: failed to open user socket",
                  IPC_LOG_PREFIX);
        DBG_EXIT;
        return ERR_NET_IF_FAIL;
    }

    memset(&addr, 0, sizeof(addr));

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = htons(MASTER_USER_RECV_PORT);

    if (bind(g_master_user_fd,
             (struct sockaddr *)&addr,
             sizeof(addr)) < 0) {

        perror("bind user");

        close(g_master_user_fd);
        g_master_user_fd = -1;

        print_err("%s initialize_ipc: bind user port %d failed",
                  IPC_LOG_PREFIX,
                  MASTER_USER_RECV_PORT);

        DBG_EXIT;
        return ERR_NET_IF_FAIL;
    }


    /* Create socket to receive slave ACKs */
    g_master_slave_fd = socket(AF_INET, SOCK_DGRAM, 0);

    if (g_master_slave_fd < 0) {

        perror("socket");

        close(g_master_user_fd);
        g_master_user_fd = -1;

        print_err("%s initialize_ipc: failed to open slave socket",
                  IPC_LOG_PREFIX);

        DBG_EXIT;
        return ERR_NET_IF_FAIL;
    }

    memset(&addr, 0, sizeof(addr));

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = htons(MASTER_SLAVE_RECV_PORT);

    if (bind(g_master_slave_fd,
             (struct sockaddr *)&addr,
             sizeof(addr)) < 0) {

        perror("bind slave");

        close(g_master_user_fd);
        close(g_master_slave_fd);

        g_master_user_fd = -1;
        g_master_slave_fd = -1;

        print_err("%s initialize_ipc: bind slave port %d failed",
                  IPC_LOG_PREFIX,
                  MASTER_SLAVE_RECV_PORT);

        DBG_EXIT;
        return ERR_NET_IF_FAIL;
    }

    print_info("%s initialize_ipc: success; "
               "user port=%d slave port=%d",
               IPC_LOG_PREFIX,
               MASTER_USER_RECV_PORT,
               MASTER_SLAVE_RECV_PORT);

    DBG_EXIT;
    return ERR_OK;
}


static int udp_recv_into(int fd, void **ppvBuffer)
{
    DBG_ENTRY;

    if (fd < 0 || ppvBuffer == NULL) {
        print_err("%s udp_recv_into: invalid fd or buffer",
                  IPC_LOG_PREFIX);
        DBG_EXIT;
        return ERR_INVALID_PARAM;
    }

    *ppvBuffer = NULL;

    uint8_t buf[1024];

    ssize_t rc = recvfrom(fd,
                          buf,
                          sizeof(buf),
                          0,
                          NULL,
                          NULL);

    if (rc < 0) {

        print_err("%s udp_recv_into: recvfrom failed rc=%zd",
                  IPC_LOG_PREFIX,
                  rc);

        DBG_EXIT;
        return ERR_TIMEOUT;
    }

    void *p = malloc((size_t)rc);

    if (p == NULL) {

        print_err("%s udp_recv_into: malloc failed for %zd bytes",
                  IPC_LOG_PREFIX,
                  rc);

        DBG_EXIT;
        return ERR_MEMORY_ALLOCATION_FAIL;
    }

    memcpy(p, buf, (size_t)rc);
    
    *ppvBuffer = p;

    print_info("%s udp_recv_into: received %zd bytes",
               IPC_LOG_PREFIX,
               rc);

    DBG_EXIT;
    return ERR_OK;
}


int receive_ipc_from_user(void **ppvBuffer)
{
    DBG_ENTRY;

    int ret = udp_recv_into(g_master_user_fd, ppvBuffer);

    print_info("%s receive_ipc_from_user: ret=%d",
               IPC_LOG_PREFIX,
               ret);

    DBG_EXIT;
    return ret;
}


int receive_ipc_from_slave(void **ppvBuffer)
{
    DBG_ENTRY;

    int ret = udp_recv_into(g_master_slave_fd, ppvBuffer);

    print_info("%s receive_ipc_from_slave: ret=%d",
               IPC_LOG_PREFIX,
               ret);

    DBG_EXIT;
    return ret;
}


/*
 * Send UDP packet to a specific IPv4 address.
 */
static int udp_send_to_address(const char *ip,
                               int port,
                               const void *pvBuffer,
                               size_t uLen,
                               int enable_broadcast)
{
    DBG_ENTRY;

    if (ip == NULL || pvBuffer == NULL || uLen == 0U) {
        print_err("%s udp_send_to_address: invalid parameters",
                  IPC_LOG_PREFIX);
        DBG_EXIT;
        return ERR_INVALID_PARAM;
    }

    int s = socket(AF_INET, SOCK_DGRAM, 0);

    if (s < 0) {
        perror("socket");

        print_err("%s udp_send_to_address: socket failed",
                  IPC_LOG_PREFIX);

        DBG_EXIT;
        return ERR_NET_IF_FAIL;
    }

    /*
     * Enable UDP broadcast when requested.
     */
    if (enable_broadcast) {

        int enable = 1;

        if (setsockopt(s,
                       SOL_SOCKET,
                       SO_BROADCAST,
                       &enable,
                       sizeof(enable)) < 0) {

            perror("setsockopt(SO_BROADCAST)");

            print_err("%s udp_send_to_address: "
                      "failed to enable broadcast",
                      IPC_LOG_PREFIX);

            close(s);

            DBG_EXIT;
            return ERR_NET_IF_FAIL;
        }
    }

    struct sockaddr_in addr;

    memset(&addr, 0, sizeof(addr));

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip, &addr.sin_addr) != 1) {

        print_err("%s udp_send_to_address: invalid IP %s",
                  IPC_LOG_PREFIX,
                  ip);

        close(s);

        DBG_EXIT;
        return ERR_INVALID_PARAM;
    }

    ssize_t rc = sendto(s,
                        pvBuffer,
                        uLen,
                        0,
                        (struct sockaddr *)&addr,
                        sizeof(addr));

    close(s);

    if (rc == (ssize_t)uLen) {

        print_info("%s udp_send_to_address: "
                   "sent %zu bytes to %s:%d",
                   IPC_LOG_PREFIX,
                   uLen,
                   ip,
                   port);

        DBG_EXIT;
        return ERR_OK;
    }

    print_err("%s udp_send_to_address: "
              "sendto failed rc=%zd target=%s:%d",
              IPC_LOG_PREFIX,
              rc,
              ip,
              port);

    DBG_EXIT;
    return ERR_NET_IF_FAIL;
}


/*
 * Master -> DemoUser
 *
 * UNICAST
 *
 *     127.0.0.1:48000
 */
int send_ipc_to_user(const void *pvBuffer)
{
    DBG_ENTRY;

    if (pvBuffer == NULL) {
        print_err("%s send_ipc_to_user: null buffer",
                  IPC_LOG_PREFIX);
        DBG_EXIT;
        return ERR_INVALID_PARAM;
    }

    /*
     * First 4 bytes contain total packet length.
     */
    UINT32 uLen = *((const UINT32 *)pvBuffer);

    print_info("%s send_ipc_to_user: "
               "sending %u bytes to 127.0.0.1:%d",
               IPC_LOG_PREFIX,
               uLen,
               DEMO_USER_RECV_PORT);

    int ret = udp_send_to_address("127.0.0.1",
                                  DEMO_USER_RECV_PORT,
                                  pvBuffer,
                                  (size_t)uLen,
                                  0);

    print_info("%s send_ipc_to_user: exit ret=%d",
               IPC_LOG_PREFIX,
               ret);

    DBG_EXIT;
    return ret;
}


/*
 * Master -> ALL Slaves
 *
 * BROADCAST
 *
 *     255.255.255.255:47001
 */
int send_ipc_to_slave(const void *pvBuffer)
{
    DBG_ENTRY;

    if (pvBuffer == NULL) {
        print_err("%s send_ipc_to_slave: null buffer",
                  IPC_LOG_PREFIX);
        DBG_EXIT;
        return ERR_INVALID_PARAM;
    }

    /*
     * First 4 bytes contain total packet length.
     */
    UINT32 uLen = *((const UINT32 *)pvBuffer);

    print_info("%s send_ipc_to_slave: "
               "broadcasting %u bytes to %s:%d",
               IPC_LOG_PREFIX,
               uLen,
               SLAVE_BROADCAST_IP,
               SLAVE_APP_RECV_PORT);

    int ret = udp_send_to_address(SLAVE_BROADCAST_IP,
                                  SLAVE_APP_RECV_PORT,
                                  pvBuffer,
                                  (size_t)uLen,
                                  1);

    print_info("%s send_ipc_to_slave: exit ret=%d",
               IPC_LOG_PREFIX,
               ret);

    DBG_EXIT;
    return ret;
}

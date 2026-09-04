#include "gen.h"

int initializeInterfaces( void ) {

// Initializing LAN networks
#ifdef ENABLE_LORA_FOR_LAN
    if (initializeLoRa() != ERR_OK) {
        return ERR_NET_IF_FAIL;
    }
#endif /* ENABLE_LORA_FOR_LAN */

#ifdef ENABLE_WIFI_FOR_WAN
    if (initializeWiFi() != ERR_OK) {
        return ERR_NET_IF_FAIL;
    }
#endif /* ENABLE_WIFI_FOR_WAN */

#ifdef ENABLE_GSM_FOR_WAN
    if (initializeGSM() != ERR_OK) {
        return ERR_NET_IF_FAIL;
    }
#endif /* ENABLE_GSM_FOR_WAN */
    return ERR_OK;
}

hal_sendAck

hal_receive
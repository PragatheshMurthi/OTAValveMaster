/* Headders */
#include "gen.h"

/* Prototypes */

// Main routine
void startWorking (void);

/* Global variables */
static MASTER_ARCHIVE *gstInformationDB;

/* Defenitions */
void setup() {
  ERROR_CODE enErrorCode = ERR_OK;

  // Create memory for the global information data base.
  gstInformationDB = malloc ( sizeof ( MASTER_ARCHIVE ));
  if ( NULL != gstInformationDB )
    memset( gstInformationDB, 0, sizeof(MASTER_ARCHIVE));

  // Initialize the network modules for com.
  enErrorCode = initializeInterfaces( gstInformationDB );
  if ( enErrorCode != ERR_OK) {
      // Handle the error here
      print_err("%s:NetModInit<KO>ERR<%d>",__FUNCTION__,enErrorCode);
      return;
  }
}

void loop() {

  // All the core listening functionalities...
  startWorking();
  print_dbg("Work halted...");

}

void startWorking (void) {

  if (gstInformationDB == NULL) {
    print_err("MASTER Archive DB not initialized/NULL");
    return;
  }
  
  while ( true ) {
    
    PVOID pvUReqBuffer = NULL;
    // Expect request from the user
    pvUReqBuffer = receive_user_request ( gstInformationDB );
    parse_request( gstInformationDB, pvUReqBuffer );
    initiate_order( gstInformationDB );
    actuate_motor ( gstInformationDB );
    post_ack ( gstInformationDB );

  }

}
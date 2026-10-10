/* *****************************************************************************
 * @file        motor.h
 * @brief       Public valve actuation interface.
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
#ifndef __MOTOR__
#define __MOTOR__

#include "gen.h"

/*============================================================================*/
/*                            FUNCTION PROTOTYPES                             */
/*============================================================================*/

/** Processes parsed requests through the valve actuator layer.
 *  @param[in] pstMasterArchive Master archive containing parsed requests.
 */
void actuate_motor(MASTER_ARCHIVE *pstMasterArchive);

#endif /* __MOTOR__ */
/**
 *  If not stated otherwise in this file or this component's LICENSE
 *  file the following copyright and licenses apply:
 *
 *  Copyright 2025 RDK Management
 *
 *  Licensed under the Apache License, Version 2.0 (the License);
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an AS IS BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

/**
 * @addtogroup AV_Clock AV Clock Module
 * @{
 */

/**
 * @addtogroup AV_Clock_Halif_Test AV Clock HALIF Tests
 * @{
 */

/**
 * @defgroup AV_Clock_Halif_Test_Init AV Clock HALIF Tests Init File
 * @{
 * @parblock
 *
 * ### Initialization functions for AV Clock VTS HALIF AIDL tests :
 *
 *
 *
 * **Pre-Conditions:**  None@n
 * **Dependencies:** None@n
 *
 * Ref to API Definition specification documentation :
 *
 * @endparblock
 */


/**
 * @file vts_avclock_init.h
 *
 */
#ifndef __VTS_AVCLOCK_INIT_H__
#define __VTS_AVCLOCK_INIT_H__

#include <ut_kvp_profile.h>
#include <ut.h>
#include <ut_log.h>
#include <ut_control_plane.h>

extern void *gAVClockTestConfiguration;
extern void *gAudioPrimeTestConfiguration;
extern void *gVideoPrimeTestConfiguration;

UT_status_t VTS_AVClock_init();


#endif //__VTS_AVCLOCK_INIT_H__
/** @} */ // End of AV_Clock_Halif_Test_Init_header
/** @} */ // End of AV_Clock_Halif_Test
/** @} */ // End of AV_Clock
/** @} */ // End of HPK

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
 * @file vts_avclock_init.cpp
 *
 */

#include "vts_avclock.h"
sp<IAVClockManager> gAVClockManagerService;

UT_status_t VTS_AVClock_init()
{
    if(getAVClockService() != UT_STATUS_OK)
    {
        return UT_STATUS_FAILURE;
    }
    /*if(vts_videodecoder_parseAVClockHFP() != UT_STATUS_OK)
    {
        UT_LOG_ERROR("Failed to process the HFP file");
        return UT_STATUS_FAILURE;
    }*/

    return UT_STATUS_OK;
}

/** @} */ // End of AV_Clock_Halif_Test_Init
/** @} */ // End of AV_Clock_Halif_Test
/** @} */ // End of AV_Clock
/** @} */ // End of HPK

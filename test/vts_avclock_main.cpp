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
 * @addtogroup HPK Hardware Porting Kit
 * @{
 * @par The Hardware Porting Kit
 * HPK is the next evolution of the well-defined Hardware Abstraction Layer
 * (HAL), but augmented with more comprehensive documentation and test suites
 * that OEM or SOC vendors can use to self-certify their ports before taking
 * them to RDKM for validation or to an operator for final integration and
 * deployment.
 */

/**
 * @addtogroup AV_CLOCK AV Clock Module
 * @{
 */

/**
 * @addtogroup AV_CLOCK_HALTEST AV Clock HAL Tests
 * @{
 */

/**
 * @defgroup AV_CLOCK_HALTEST_Main AV Clock HAL Tests helper functions main File
 * @{
 * @parblock
 *
 *
 *
 * mainfile  AV clock module test.
 *
 * **Pre-Conditions:** None @n
 * **Dependencies:** None @n
 *
 * @endparblock
 */

/**
 * @file vts_avclock_main.c
 *
 */

#include "vts_avclock.h"


int main(int argc, char **argv)
{
    UT_status_t result;
    UT_init(argc, argv);
    if(VTS_AVClock_init()!=UT_STATUS_OK)
    {
        return UT_STATUS_FAILURE;
    }

    result = UT_run_tests();
    return result;
}

/** @} */ // End of AV_CLOCK_HALTEST_Main
/** @} */ // End of AV_CLOCK_HALTEST
/** @} */ // End of AV_CLOCK
/** @} */ // End of HPK
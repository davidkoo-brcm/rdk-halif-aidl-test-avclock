/**
 *  If not stated otherwise in this file or this component's LICENSE
 *  file the following copyright and licenses apply:
 *
 *  Copyright 2024 RDK Management
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
 * deployment. The Hardware Porting Kit effectively enables an OEM and/or SOC
 * vendor to self-certify their own Video Accelerator devices, with minimal RDKM
 * assistance.
 *
 */

/**
 * @addtogroup AVCLOCK AVClock Module
 * @{
 */

/**
 * @addtogroup AVCLOCK_HALTEST AVClock HAL Tests
 * @{
 */

/**
 * @defgroup AVCLOCK_HALTEST_PARSE_TESTCONFIG_HEADER AVClock HAL Tests parse test configuration Header File
 * @{
 * @parblock
 *
 * ### Parse test configuration functions for AVClock :
 *
 * Parse test configuration functions required for the module across all vendors.
 *
 * **Pre-Conditions:**  None @n
 * **Dependencies:** None @n
 *
 * @endparblock
 *
 */

/**
 * @file vts_avclock_parse_testconfig.h
 *
 */

#ifndef __VTS_AVCLOCK_PARSE_TESTCONFIG_H__
#define __VTS_AVCLOCK_PARSE_TESTCONFIG_H__

#include <ut.h>
#include <ut_log.h>
#include "ut_kvp_profile.h"
#include "vts_helper_functions.h"
#include "com/rdk/hal/avclock/IAVClockManager.h"
#include "com/rdk/hal/avclock/BnAVClockControllerListener.h"
#include "com/rdk/hal/avclock/BnAVClockEventListener.h"

#define VTS_AVCLOCK_TEST_CONFIG_PATH "./avclock_test_configuration/vts_l3_AVClock_PCR.yaml"

using namespace std;
using namespace com::rdk::hal;

UT_status_t vts_avclock_parse_testconfig(void* , const char *, const char *, const char *testName, std::vector<int64_t>&);

#endif //__VTS_AVCLOCK_PARSE_TESTCONFIG_H__

/** @} */ // End of AVCLOCK_HALTEST_PARSE_TESTCONFIG_HEADER
/** @} */ // End of AVCLOCK_HALTEST
/** @} */ // End of AVCLOCK
/** @} */ // End of HPK
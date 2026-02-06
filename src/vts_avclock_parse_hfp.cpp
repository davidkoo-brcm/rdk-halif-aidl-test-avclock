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
 * @addtogroup AVCLOCK AV Clock Module
 * @{
 */

/**
 * @addtogroup AVCLOCK_HALTEST AV Clock HAL Tests
 * @{
 */

/**
 * @defgroup AVCLOCK_HALTEST_PARSE_HFP AV Clock HAL Tests parse Hardware feature profile
 * @{
 * @parblock
 *
 * ### Parse HFP functions for AV Clock :
 *
 * Parse HFP functions required for the module across all vendors.
 *
 * **Pre-Conditions:**  None @n
 * **Dependencies:** None @n
 *
 * @endparblock
 *
 */

/**
 * @file vts_avclock_parse_hfp.cpp
 *
 */

#include "vts_avclock_parse_hfp.h"

using namespace std;

vector<avclockmanager> gAVClock_hfp;

UT_status_t vts_avclock_parseAVClockHFP()
{
    string avclockpath = AVCLOCKMGR_IAVCLOCK;
    int32_t number_of_avclockinst;
    avclockmanager temporary;

    number_of_avclockinst = UT_KVP_PROFILE_GET_LIST_COUNT(avclockpath.c_str());
    if(number_of_avclockinst <= 0)
    {
        UT_LOG_ERROR("No avclock instances found in the HFP");
        return UT_STATUS_FAILURE;
    }

    for(int32_t i = 0; i < number_of_avclockinst; ++i)
    {
        string avclockidpath = avclockpath + "." + std::to_string(i);
        int32_t avclock_id = UT_KVP_PROFILE_GET_UINT32(avclockidpath.c_str());
        temporary.ids.push_back(avclock_id);
    }


    gAVClock_hfp.push_back(temporary);
    temporary.ids.clear();
    return UT_STATUS_OK;
}

/** @} */ // End of AVCLOCK_HALTEST_PARSE_HFP
/** @} */ // End of AVCLOCK_HALTEST
/** @} */ // End of AVCLOCK
/** @} */ // End of HPK
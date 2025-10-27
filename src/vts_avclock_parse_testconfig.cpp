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
 * @defgroup AVCLOCK_HALTEST_PARSE_TESTCONFIG Audio Decoder HAL Tests parse test configuration file
 * @{
 * @parblock
 *
 * ### Parse test configuration functions for AVClock :
 *
 * Parse configuration functions required for the module across all vendors.
 *
 * **Pre-Conditions:**  None @n
 * **Dependencies:** None @n
 *
 * @endparblock
 *
 */

/**
 * @file vts_avclock_parse_testconfig.cpp
 *
 */

#include <filesystem>
#include "vts_avclock_parse_testconfig.h"
#include "vts_avclock_helper_functions.h"

UT_status_t vts_avclock_parse_testconfig(void* pTestInstance, const char *module, const char *testGroup, const char *testName)
{
    ut_kvp_status_t status;
    ut_kvp_instance_t* kvpTestInstance = (ut_kvp_instance_t*)pTestInstance;
    filesystem::path fullPath;

    const filesystem::path currentPath = filesystem::current_path();
    const string testList = string(module) + "." + string(testGroup) + "." + string(testName);
    const string streamList = testList + "." + "Inputs" + "." + string("Streams");

    streamCount = ut_kvp_getListCount(kvpTestInstance, streamList.c_str());

    for(int32_t i = 0; i < streamCount; i++)
    {
        char resultKVP[UT_KVP_MAX_ELEMENT_SIZE] = {0};
        uint32_t streamId;

        string streamName = streamList + "." + to_string(i) + "Name";
        ut_kvp_getStringField(kvpTestInstance, streamName.c_str(), resultKVP, UT_KVP_MAX_ELEMENT_SIZE);

        for (const auto& val : streamName["PTS"]) {
                ptsValues.push_back(val.as<int64_t>());
        }
    }

    return UT_STATUS_OK;
}

/** @} */ // End of AUDIO_DECODER_HALTEST_PARSE_TESTCONFIG
/** @} */ // End of AUDIO_DECODER_HALTEST
/** @} */ // End of AUDIO_DECODER
/** @} */ // End of HPK
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

        string framePTS    = streamList + "." + to_string(i) + "." +"PTS";
        int64_t PTS = ut_kvp_getUInt64Field(kvpTestInstance, eachPTS.c_str())

        ut_kvp_getStringField(kvpTestInstance, input.c_str(), resultKVP, UT_KVP_MAX_ELEMENT_SIZE);
        fullPath = currentPath;
        fullPath.append(streamInputPath);
        fullPath.append(resultKVP);
        testDetails.input = fullPath.string();

        ut_kvp_getStringField(kvpTestInstance, reference.c_str(), resultKVP, UT_KVP_MAX_ELEMENT_SIZE);
        fullPath = currentPath;
        fullPath.append(streamRefPath);
        fullPath.append(resultKVP);
        testDetails.reference = fullPath.string();

        for(int32_t j = 0; j < testDetails.numOfFrames; j++)
        {
            bool errorFrame = false;

            string eachFrame = frameSize + "." + to_string(j);
            string eachPTS = framePTS + "." + to_string(j);
            int32_t size = ut_kvp_getUInt32Field(kvpTestInstance, eachFrame.c_str());
            int64_t PTS = ut_kvp_getUInt64Field(kvpTestInstance, eachPTS.c_str());
            if(errorFrameNum.size() > 0)
            {
                if (find(errorFrameNum.begin(), errorFrameNum.end(), j) != errorFrameNum.end())
                {
                    errorFrame = true;
                }
            }
            testDetails.frameDetails.push_back({size, PTS, errorFrame});
        }

        testDetails.rms_diff_threashold = ut_kvp_getDoubleField(kvpTestInstance, rmsDiff.c_str());
        testDetails.decodeTime = ut_kvp_getDoubleField(kvpTestInstance, decodeTime.c_str());
        testDetails.trimStartNs  = ut_kvp_getUInt64Field(kvpTestInstance, trimStart.c_str());
        testDetails.trimEndNs    = ut_kvp_getUInt64Field(kvpTestInstance, trimEnd.c_str());
        testDetails.decoderDelay = ut_kvp_getUInt32Field(kvpTestInstance, decoderDelay.c_str());
        testDetails.stopPts      = ut_kvp_getUInt64Field(kvpTestInstance, stopPts.c_str());
        testDetails.discontinuityPts = ut_kvp_getUInt64Field(kvpTestInstance, discontinuityPts.c_str());
        testDetails.flushPts     = ut_kvp_getUInt64Field(kvpTestInstance, flushPts.c_str());
        testDetails.flushRestPts = ut_kvp_getUInt64Field(kvpTestInstance, flushRestPts.c_str());
        testDetails.eosPts       = ut_kvp_getUInt64Field(kvpTestInstance, eosPts.c_str());
        testDetails.sinkEosPts   = ut_kvp_getUInt64Field(kvpTestInstance, sinkEosPts.c_str());
        testDetails.sinkFlushPts = ut_kvp_getUInt64Field(kvpTestInstance, sinkFlushPts.c_str());

        testConfiguration.push_back(testDetails);
    }

    return UT_STATUS_OK;
}

/** @} */ // End of AUDIO_DECODER_HALTEST_PARSE_TESTCONFIG
/** @} */ // End of AUDIO_DECODER_HALTEST
/** @} */ // End of AUDIO_DECODER
/** @} */ // End of HPK
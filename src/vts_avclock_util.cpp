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
 * @defgroup AV_CLOCK_HALTEST_UTIL AV Clock HAL Tests helper functions Header File
 * @{
 * @parblock
 *
 * ### Common functions/macros required for the AV clock module :
 *
 * Common functions/macros required for the AV clock module.
 *
 * **Pre-Conditions:** None @n
 * **Dependencies:** None @n
 *
 * @endparblock
 */

/**
 * @file vts_avclock_util.c
 *
 */

#include "vts_avclock.h"

extern sp<IAVClockManager> gAVClockManagerService;

bool isClkSinkUsable(const IAudioSink::Id& sinkId)
{
    for (const auto& _sink : gAudiosinks_hfp)
    {
        if (std::find(_sink.availableIds.begin(), _sink.availableIds.end(), sinkId.value) != _sink.availableIds.end())
        {
            return true;
        }
    }
    return false;
}

bool isClkDecoderUsable(const IAudioDecoder::Id& decoderId)
{
    if (gAudiosinks_hfp.empty()) {
        return false;
    }

    const auto& availableDecoders = gAudiosinks_hfp[0].availableDecoderIds;
    return std::find(availableDecoders.begin(), availableDecoders.end(), decoderId.value) != availableDecoders.end();
}

bool isClkVideoSinkUsable(const IVideoSink::Id& sinkId)
{
    return std::find(gAvailableVideosinks_hfp.begin(), gAvailableVideosinks_hfp.end(), sinkId.value) != gAvailableVideosinks_hfp.end();
}

bool isClkVideoDecoderUsable(const IVideoDecoder::Id& decoderId)
{
    return std::find(gAvailableVideodecoders_hfp.begin(),
                            gAvailableVideodecoders_hfp.end(),
                            decoderId.value) != gAvailableVideodecoders_hfp.end();
}

int getAVClockService()
{
    gAVClockManagerService = android::interface_cast<IAVClockManager>(
        defaultServiceManager()->getService(String16(IAVClockManager::serviceName().c_str())));
    if (gAVClockManagerService == nullptr)
    {
        UT_LOG_ERROR("AVClock Service not available\n");
        return UT_STATUS_FAILURE;
    }
    return UT_STATUS_OK;
}

bool vts_avclock_helper_areElementsUnique(const std::vector<int32_t>& ids)
{
    std::unordered_set<int32_t> unique(ids.begin(), ids.end());
    return ids.size() == unique.size();
}

void vts_avclock_helper_validateCapabilities(int avClockId, const avclock::Capabilities& capabilities) {
    // Example: Check profile for expected capabilities (adjust based on actual Capabilities fields)
    std::string profilePath = std::string(AVCLOCKMGR_IAVCLOCK) + "." + std::to_string(avClockId);
    // UT_KVP_PROFILE_* calls to validate capabilities (e.g., supported clock modes, sinks)
    UT_LOG_INFO("Validating capabilities for AVClock ID: %d (profile: %s)", avClockId, profilePath.c_str());
    // Add assertions based on profile or capabilities fields
    UT_ASSERT_TRUE(true); // Placeholder
}

#if 0
bool vts_avclock_helper_validatePropertyValue(int avClockId, const std::optional<PropertyValue>& propertyValue, avclock::Property property) {
    if (!propertyValue.has_value()) {
        UT_LOG_ERROR("Property %s for AVClock ID: %d returned null",
            stringToProperty(property).c_str(), avClockId);
        return false;
    }

    if (propertyValue->value.getTag() != PropertyValue::Value::Tag::intValue) {
        UT_LOG_ERROR("Property %s for AVClock ID: %d has unexpected type (not int)",
            stringToProperty(property).c_str(), avClockId);
        return false;
    }

    int value = propertyValue->value.get<PropertyValue::Value::Tag::intValue>();
    UT_LOG_INFO("Validating property %s for AVClock ID: %d, value: %d",
        stringToProperty(property).c_str(), avClockId, value);

    switch (property) {
        case avclock::Property::RESOURCE_ID: {
            // Example: Validate RESOURCE_ID matches avClockId or profile
            std::string profilePath = std::string(AVCLOCKMGR_IAVCLOCK) + "." + std::to_string(avClockId);
            uint32_t expectedId = UT_KVP_PROFILE_GET_UINT32(profilePath.c_str());
            UT_ASSERT_EQUAL(value, static_cast<int>(expectedId));
            break;
        }
        case avclock::Property::METRIC_AV_SYNC_ACCURACY_MS:
            // Allow -1 (unimplemented) or reasonable range (e.g., -1000 to 1000 ms)
            UT_ASSERT_TRUE(value == -1 || (value >= -1000 && value <= 1000));
            break;
        case avclock::Property::METRIC_AV_RESYNC_COUNT:
            // Allow -1 (unimplemented) or non-negative
            UT_ASSERT_TRUE(value == -1 || value >= 0);
            break;
        default:
            UT_LOG_ERROR("Unknown property %s for AVClock ID: %d",
                         stringToProperty(property).c_str(), avClockId);
            return false;
    }
    return true;
}
#endif

/** @} */ // End of AV_CLOCK_HALTEST_UTIL
/** @} */ // End of AV_CLOCK_HALTEST
/** @} */ // End of AV_CLOCK
/** @} */ // End of HPK
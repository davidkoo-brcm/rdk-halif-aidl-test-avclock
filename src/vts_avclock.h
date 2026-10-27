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
 * @defgroup AV_CLOCK_HALTEST_HEADER AV Clock HAL Tests helper functions Header File
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
 * @file vts_avclock.h
 *
 */

#ifndef __VTS_AVCLOCK_H__
#define __VTS_AVCLOCK_H__

#include <ut_kvp_profile.h>
#include <ut.h>
#include <ut_log.h>
#include <ut_gtest.h>
#include <ut_control_plane.h>
#include <binder/IServiceManager.h>
#include <binder/ProcessState.h>
#include <binder/IPCThreadState.h>
#include <binder/Status.h>
#include <utils/String16.h>
#include <stdio.h>
#include <vector>
#include <optional>
#include <thread>
#include <chrono>
#include <mutex>
#include <condition_variable>
#include <algorithm>
#include <unordered_set>
#include <optional> // Add for std::optional

#include "vts_audiodecoder.h"
#include "vts_videodecoder.h"
#include "vts_videosink.h"
#include "vts_avclock_init.h"
#include "vts_avclock_parse_hfp.h"

#include "com/rdk/hal/avclock/IAVClockManager.h"
#include "com/rdk/hal/audiosink/IAudioSink.h"
#include "com/rdk/hal/videosink/IVideoSink.h"
#include "com/rdk/hal/avclock/BnAVClockControllerListener.h"
#include "com/rdk/hal/avclock/BnAVClockEventListener.h"

using android::sp;
using android::String16;
using android::defaultServiceManager;
using android::binder::Status;

using namespace com::rdk::hal;
using namespace com::rdk::hal::avclock;
using namespace com::rdk::hal::videosink;

extern sp<com::rdk::hal::avclock::IAVClockManager> gAVClockManagerService;

class AudioSinkControllerListener;
class AudioSinkEventListener;
class VideoSinkControllerListener;
class VideoSinkEventListener;

namespace com {
namespace rdk {
namespace hal {
namespace audiosink {
    class IAudioSink;
}
namespace audiodecoder {
    class IAudioDecoder;
    struct Capabilities;
}
}
}
}

enum contentprimetype {
    primeaudio,
    primevideo,
    primeaudiobeforevideo,
    primevideobeforeaudio,
    primepcr
};

enum threadtype {
    audio,
    video,
    pcr
};

struct AudioConfig {
    IAudioDecoder::Id decId;
    sp<IAudioDecoder> decHandle;
    sp<AudioDecoderControllerListener> decCtrlListener;
    sp<AudioDecoderEventListener> decEventListener;
    audiodecoder::Capabilities *decCaps;
    com::rdk::hal::audiosink::IAudioSink::Id sinkId;
    sp<com::rdk::hal::audiosink::IAudioSink> sinkHandle;
    sp<AudioSinkControllerListener> sinkCtrlListener;
    sp<AudioSinkEventListener> sinkEventListener;
    bool secureDecode;
    bool tunnelMode;
    testConfiguration_t testConfig;
};

struct VideoConfig {
    IVideoDecoder::Id decId;
    sp<IVideoDecoder> decHandle;
    sp<VideoDecoderControllerListener> decCtrlListener;
    sp<VideoDecoderEventListener> decEventListener;
    videodecoder::Capabilities *decCaps;
    IVideoSink::Id sinkId;
    sp<IVideoSink> sinkHandle;
    sp<VideoSinkControllerListener> sinkCtrlListener;
    sp<VideoSinkEventListener> sinkEventListener;
    bool secureDecode;
    bool tunnelMode;
    videoDecoderTestConfiguration_t decTestConfig;
    videoDecoderTestConfiguration_t sinkTestConfig;
};

struct PcrConfig {
    int64_t pcrValue;
};

bool isClkSinkUsable(const com::rdk::hal::audiosink::IAudioSink::Id &maudioSinkId);
bool isClkDecoderUsable(const com::rdk::hal::audiodecoder::IAudioDecoder::Id &maudioDecoderId);
bool isClkVideoSinkUsable(const com::rdk::hal::videosink::IVideoSink::Id &videoSinkId);
bool isClkVideoDecoderUsable(const com::rdk::hal::videodecoder::IVideoDecoder::Id &videoDecoderId);
int getAVClockService();

// Generic function to return selected elements from a tuple at index
template <typename TupleType, std::size_t... Indices>
auto getTupleElements(const std::vector<TupleType>& list, size_t index, std::index_sequence<Indices...>)
    -> std::optional<std::tuple<std::tuple_element_t<Indices, TupleType>...>> {

    if (list.empty()) {
        UT_LOG_INFO("List is empty");
        return std::nullopt;
    }

    if (index >= list.size()) {
        UT_LOG_INFO("Index out of range. List size: %zu", list.size());
        return std::nullopt;
    }

    const auto& tuple = list[index];
    return std::make_tuple(std::get<Indices>(tuple)...);
}

void vts_avclock_helper_validateCapabilities(int avClockId, const avclock::Capabilities& capabilities);
bool vts_avclock_helper_validatePropertyValue(int avClockId, const std::optional<PropertyValue>& propertyValue, avclock::Property property);
bool vts_avclock_helper_areElementsUnique(const std::vector<int32_t>& ids);

#define MAX_TRANSITORY_STATE_WAIT_MS 10
#define MAX_FINAL_STATE_WAIT_MS 500

class AVClockControllerListener : public BnAVClockControllerListener {
    public:
        AVClockControllerListener(int32_t clockID)
           : BnAVClockControllerListener(),
         mClockID(clockID)
    {
    }
        AVClockControllerListener() = default;
        virtual ~AVClockControllerListener() = default;

        /**
         * Wait for the AVClock to reach a particular State within a given timeout.
         *
         * @param desiredState The state we want to observe.
         * @param timeout      How long to wait, e.g. std::chrono::milliseconds(500).
         * @return true if the state was reached; false if we timed out waiting.
         */
        bool waitForState(com::rdk::hal::avclock::State desiredState, std::chrono::milliseconds timeout)
        {
            std::unique_lock<std::mutex> lock(mMutex);
            // Wait until mCurrentState == desiredState or until timeout
            return mCV.wait_for(lock, timeout, [this, desiredState]() {
                return (mCurrentState == desiredState);
            });
        }

        // Accessors for test verifications if desired
        std::vector<std::pair<com::rdk::hal::avclock::State, com::rdk::hal::avclock::State>> getStateChanges() const
        {
            std::lock_guard<std::mutex> lock(mMutex);
            return mStateChanges;
        }

        void resetStateChanges()
        {
            std::lock_guard<std::mutex> lock(mMutex);
            mStateChanges.clear();
        }

        Status onStateChanged(::com::rdk::hal::avclock::State oldState, ::com::rdk::hal::avclock::State newState) override {
            UT_LOG_INFO("AVClockControllerListener: State changed from %s to %s",
                        ::com::rdk::hal::avclock::toString(oldState).c_str(),
                        ::com::rdk::hal::avclock::toString(newState).c_str());
            {
                std::lock_guard<std::mutex> lock(mMutex);
                mStateChanges.emplace_back(oldState, newState);
                mCurrentState = newState;
            }
            mCV.notify_all();
            return Status::ok();
        }

        Status onPrimed(const com::rdk::hal::avclock::ClockTime& currentClockTime) override
        {
            UT_LOG_INFO("AVClockControllerListener: Called onPrimed Event");

            if (currentClockTime.clockTimeNs == 0L && currentClockTime.sampleTimestampNs == 0L)
            {
                UT_LOG_ERROR("Invalid ClockTime received");
                return Status::fromExceptionCode(Status::EX_ILLEGAL_ARGUMENT);
            }

            {
                std::lock_guard<std::mutex> lock(mOnPrimeMutex);
                mPrimedClockTime = currentClockTime;
                mIsPrimedEventReceived = true;
                ++mPrimedEventCount;

                UT_LOG_INFO("Primed event #%zu: clockTimeNs = %lld, sampleTimestampNs = %lld",
                                 mPrimedEventCount,
                                 static_cast<long long>(currentClockTime.clockTimeNs),
                                 static_cast<long long>(currentClockTime.sampleTimestampNs));
            }

            mOnPrimeEventAvailable.notify_all();

            return Status::ok();
        }

        void resetPrimedEvent()
        {
            std::lock_guard<std::mutex> lock(mOnPrimeMutex);
            mIsPrimedEventReceived = false;
            mPrimedEventCount = 0;
        }

        bool waitForPrimedEvent(std::chrono::seconds timeout, threadtype streamType)
        {
            std::unique_lock<std::mutex> lock(mOnPrimeMutex);
            const bool eventReceived = mOnPrimeEventAvailable.wait_for(lock, timeout, [this]() {
                return mIsPrimedEventReceived;
            });
            const char* streamName = (streamType == threadtype::audio) ? "audio"
                                   : (streamType == threadtype::video) ? "video" : "pcr";
            UT_LOG_INFO("Primed event count for %s stream: %zu (event received: %s)",
                        streamName, mPrimedEventCount, eventReceived ? "true" : "false");
            return eventReceived;
        }

        bool checkStateTransition(::com::rdk::hal::avclock::State expOldState, ::com::rdk::hal::avclock::State expNewState)
        {
            std::lock_guard<std::mutex> lock(mMutex);
            bool foundClosedToOpening = false;
            for (const auto &[oldState, newState] : mStateChanges)
            {
                if (oldState == expOldState && newState == expNewState)
                {
                    foundClosedToOpening = true;
                    break;
                }
            }
            return foundClosedToOpening;
        }

        // Getter for clock time
        ClockTime getPrimedClockTime() {
            std::lock_guard<std::mutex> lock(mOnPrimeMutex);
            return mPrimedClockTime;
        }

    private:
        int32_t mClockID;
        ::com::rdk::hal::avclock::State mCurrentState;
        mutable std::mutex mMutex;
        std::condition_variable mCV;
        mutable std::mutex mOnPrimeMutex;
        std::condition_variable mOnPrimeEventAvailable;
        bool mIsPrimedEventReceived{false};
        size_t mPrimedEventCount{0};
        ClockTime mPrimedClockTime;
        std::vector<std::pair< ::com::rdk::hal::avclock::State, ::com::rdk::hal::avclock::State>> mStateChanges;
    };

class PrimePhaseSynchronizer {
public:
    PrimePhaseSynchronizer(sp<IAVClock> clockHandle,
                           sp<IAVClockController> clockController,
                           sp<AVClockControllerListener> controlListener,
                           size_t participantCount);

    void beginConfiguration(threadtype streamType, contentprimetype primetype);
    void drop(threadtype streamType);
    void finishConfiguration();

private:
    void arriveAtStart(bool isDropping, threadtype streamType);
    bool isStreamActive(threadtype streamType);

    sp<IAVClock> mClockHandle;
    sp<IAVClockController> mClockController;
    sp<AVClockControllerListener> mControlListener;
    std::mutex mMutex;
    std::condition_variable mCV;
    size_t mParticipantCount;
    size_t mStartArrivals = 0;
    size_t mFinishArrivals = 0;
    size_t mDrops = 0;
    size_t mStartGeneration = 0;
    size_t mFinishGeneration = 0;
    bool mPhaseHasWork = false;
    bool mAudioActive = true;
    bool mVideoActive = true;
    bool mPcrActive = true;
};

class AVClockEventListener : public BnAVClockEventListener {
    public:
        AVClockEventListener(int32_t clockID)
           : BnAVClockEventListener(),
                 mClockID(clockID),
                 mCurrentState(::com::rdk::hal::avclock::State::CLOSED)
    {
    }
        AVClockEventListener() = default;
        virtual ~AVClockEventListener() = default;

        /**
         * Wait for the decoder to reach a particular State within a given timeout.
         *
         * @param desiredState The state we want to observe.
         * @param timeout      How long to wait, e.g. std::chrono::milliseconds(500).
         * @return true if the state was reached; false if we timed out waiting.
         */
        bool waitForState(com::rdk::hal::avclock::State desiredState, std::chrono::milliseconds timeout)
        {
            std::unique_lock<std::mutex> lock(mMutex);
            // Wait until mCurrentState == desiredState or until timeout
            return mCV.wait_for(lock, timeout, [this, desiredState]() {
                return (mCurrentState == desiredState);
            });
        }
        // Accessors for test verifications if desired
        std::vector<std::pair<com::rdk::hal::avclock::State, com::rdk::hal::avclock::State>> getStateChanges() const
        {
            std::lock_guard<std::mutex> lock(mMutex);
            return mStateChanges;
        }

        void resetStateChanges()
        {
            std::lock_guard<std::mutex> lock(mMutex);
            mStateChanges.clear();
        }

        Status onStateChanged(::com::rdk::hal::avclock::State oldState, ::com::rdk::hal::avclock::State newState) override {
            UT_LOG_INFO("AVClockEventListener: State changed from %s to %s",
                        ::com::rdk::hal::avclock::toString(oldState).c_str(),
                        ::com::rdk::hal::avclock::toString(newState).c_str());
            {
                std::lock_guard<std::mutex> lock(mMutex);
                mStateChanges.emplace_back(oldState, newState);
                mCurrentState = newState;
            }
            mCV.notify_all();
            return Status::ok();
        }

        bool checkStateTransition(::com::rdk::hal::avclock::State expOldState, ::com::rdk::hal::avclock::State expNewState) {
            std::lock_guard<std::mutex> lock(mMutex);
            bool foundClosedToOpening = false;
            for (const auto &[oldState, newState] : mStateChanges)
            {
                if (oldState == expOldState && newState == expNewState)
                {
                    foundClosedToOpening = true;
                    break;
                }
            }
            return foundClosedToOpening;
        }

    private:
        int32_t mClockID;
        mutable std::mutex mMutex;
        std::condition_variable mCV;
        ::com::rdk::hal::avclock::State mCurrentState;
        std::vector<std::pair< ::com::rdk::hal::avclock::State, ::com::rdk::hal::avclock::State>> mStateChanges;
    };

#define AVCLOCK_MANAGER_GETAVCLOCKIDS(clockIds) \
    do { \
        ::android::binder::Status status; \
        UT_LOG_INFO("Calling getAVClockIds(OUT:clockIds:[])"); \
        status = gAVClockManagerService->getAVClockIds(&clockIds); \
        UT_LOG_INFO("Result getAVClockIds(OUT:clockIds:[size: %d]), status:[isOK:%s, exception:%s]", \
                    clockIds.size(), UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
    } while (0)

#define AVCLOCK_MANAGER_GETAVCLOCK_SUCCESS(avClockId, avClock) \
    do { \
        ::android::binder::Status status; \
        UT_LOG_INFO("Calling getAVClock(IN:avClockId:[%d], OUT:avClock[])", avClockId.value); \
        status = gAVClockManagerService->getAVClock(avClockId, &avClock); \
        UT_LOG_INFO("Result getAVClock(IN:avClockId:[%d], OUT:avClock:[%p]), status:[isOK:%s, exception:%s]", \
                    avClockId.value, static_cast<void*>(avClock.get()), \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
    } while (0)

#define AVCLOCK_GET_PROPERTY_SUCCESS(avClock, property, propertyValue) \
    do { \
        ::android::binder::Status status; \
        UT_LOG_INFO("Calling getProperty(IN:property[%s], OUT:propertyValue[])",com::rdk::hal::avclock::toString(property).c_str()); \
        status = avClock->getProperty(property, &propertyValue); \
        UT_LOG_INFO("Result getProperty(IN:property[%s], OUT:propertyValue[has_value:%s]), status:[isOK:%s, exception:%s]", \
                    com::rdk::hal::avclock::toString(property).c_str(), \
                    propertyValue.has_value() ? "true" : "false", \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_TRUE(propertyValue.has_value()); \
        UT_ASSERT_TRUE(propertyValue->value.has_value()); \
        UT_ASSERT_EQUAL(propertyValue->value.value().getTag(), com::rdk::hal::PropertyValue::Value::intValue); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
    } while (0)

#define AVCLOCK_GET_PROPERTY_FAILURE(avClock, property, propertyValue) \
    do { \
        ::android::binder::Status status; \
        UT_LOG_INFO("Calling getProperty(IN:property[%s], OUT:propertyValue[])",com::rdk::hal::avclock::toString(property).c_str()); \
        status = avClock->getProperty(property, &propertyValue); \
        UT_LOG_INFO("Result getProperty(IN:property[%s]), status:[isOK:%s, exception:%s]", \
                    com::rdk::hal::avclock::toString(property).c_str(), \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_FALSE(status.isOk()); \
        UT_ASSERT_FALSE(propertyValue.has_value()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_ILLEGAL_ARGUMENT); \
    } while (0)


#define AVCLOCK_SET_PROPERTY_SUCCESS(avClockController, property, propertyValue, result) \
    do { \
        ::android::binder::Status status; \
        UT_LOG_INFO("Calling setProperty(IN:property[%s], IN:propertyValue[has_value:%s])", \
                    com::rdk::hal::avclock::toString(property).c_str(), \
                    propertyValue.value.has_value() ? "true" : "false"); \
        status = avClockController->setProperty(property, propertyValue, &result); \
        UT_LOG_INFO("Result setProperty(IN:property[%s], IN:propertyValue[has_value:%s], OUT:result=%s), status:[isOK:%s, exception:%s]", \
                    com::rdk::hal::avclock::toString(property).c_str(), \
                    propertyValue.value.has_value() ? "true" : "false", \
                    result ? "true" : "false", \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
    } while (0)

#define AVCLOCK_SET_PROPERTY_FAILURE(avClockController, property, propertyValue,result,exception) \
    do { \
        ::android::binder::Status status; \
        UT_LOG_INFO("Calling setProperty(IN:property[%d], IN:propertyValue[has_value:%s])", \
                    property, \
                    propertyValue.value.has_value() ? "true" : "false"); \
        status = avClockController->setProperty(property, propertyValue, &result); \
        UT_LOG_INFO("Result setProperty(IN:property[%d], IN:propertyValue[has_value:%s]), status:[isOK:%s, exception:%s]", \
                    property, \
                    propertyValue.value.has_value() ? "true" : "false", \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_EQUAL(status.exceptionCode(), exception); \
        UT_ASSERT_FALSE(status.isOk()); \
    } while (0)


#define AVCLOCK_MANAGER_GETAVCLOCK_FAILURE(avClockId, avClock) \
    do { \
        ::android::binder::Status status; \
        UT_LOG_INFO("Calling getAVClock(IN:avClockId:[%d], OUT:avClock[])", avClockId.value); \
        status = gAVClockManagerService->getAVClock(avClockId, &avClock); \
        UT_LOG_INFO("Result getAVClock(IN:avClockId:[%d]), status:[isOK:%s, exception:%s]", \
                    avClockId.value, \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()); \
    } while (0)

#define AVCLOCK_GETCAPABILITIES(avClock, capabilities) \
    do { \
        ::android::binder::Status status; \
        UT_LOG_INFO("Calling getCapabilities(OUT:capabilities[])"); \
        status = avClock->getCapabilities(capabilities); \
        UT_LOG_INFO("Result getCapabilities(OUT:capabilities[]), status:[isOK:%s, exception:%s]", \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
    } while (0)

#define AVCLOCK_GET_STATE(avClock, state) \
    do { \
        ::android::binder::Status status; \
        UT_LOG_INFO("Calling getState(OUT:state[])"); \
        status = avClock->getState(&state); \
        UT_LOG_INFO("Result getState(OUT:state[%s]), status:[isOK:%s, exception:%s]", \
                    com::rdk::hal::avclock::toString(state).c_str(), \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
    } while (0)

#define AVCLOCK_WAIT_FOR_STATE(avClock, desiredState, lastState) \
    do { \
        int32_t sleep_time = 0; \
        do { \
            std::this_thread::sleep_for(std::chrono::milliseconds(MAX_TRANSITORY_STATE_WAIT_MS)); \
            sleep_time += MAX_TRANSITORY_STATE_WAIT_MS; \
            AVCLOCK_GET_STATE(avClock, lastState); \
        } while (sleep_time < MAX_FINAL_STATE_WAIT_MS && lastState != desiredState); \
    } while (0)

#define AVCLOCK_OPEN_SUCCESS(avClock, controllerListener, avClockController, stateCheck) \
    do { \
        ::android::binder::Status status; \
        UT_LOG_INFO("Calling open(IN:controllerListener[0x%p], OUT:avClockController[])", controllerListener.get()); \
        status = avClock->open(controllerListener, &avClockController); \
        UT_LOG_INFO("Result open(IN:controllerListener[0x%p], OUT:avClockController[0x%X]), status:[isOK:%s, exception:%s]", \
                    controllerListener.get(), avClockController.get(), \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        if (stateCheck) { \
            ::com::rdk::hal::avclock::State lastState; \
            AVCLOCK_WAIT_FOR_STATE(avClock, ::com::rdk::hal::avclock::State::READY, lastState); \
            UT_ASSERT_EQUAL(lastState, ::com::rdk::hal::avclock::State::READY); \
        } \
    } while (0)

#define AVCLOCK_CLOSE_SUCCESS(avClock, avClockController, stateCheck) \
    do { \
        ::android::binder::Status status; \
        bool _aidl_return = false; \
        UT_LOG_INFO("Calling close(IN:avClockController[0x%p], OUT:_aidl_return[])", avClockController.get()); \
        status = avClock->close(avClockController, &_aidl_return); \
        UT_LOG_INFO("Result close(IN:avClockController[0x%p], OUT:_aidl_return[%s]), status:[isOK:%s, exception:%s]", \
                    avClockController.get(), UT_Control_GetMapString(bool_mapTable, _aidl_return), \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        if (stateCheck) { \
            ::com::rdk::hal::avclock::State lastState; \
            AVCLOCK_WAIT_FOR_STATE(avClock, ::com::rdk::hal::avclock::State::CLOSED, lastState); \
            UT_ASSERT_EQUAL(lastState, ::com::rdk::hal::avclock::State::CLOSED); \
        } \
    } while (0)

#define AVCLOCK_OPEN_FAILURE(avClock, controllerListener, avClockController, exception) \
    do { \
        ::android::binder::Status status; \
        UT_LOG_INFO("Calling open(IN:controllerListener[0x%p], OUT:avClockController[])", controllerListener); \
        status = avClock->open(controllerListener, &avClockController); \
        UT_LOG_INFO("Result open(IN:controllerListener[0x%p]), status:[isOK:%s, exception:%s]", \
                    controllerListener, \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_FALSE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), exception); \
    } while (0)

#define AVCLOCK_CLOSE_FAILURE(avClock, avClockController, result) \
    do { \
        ::android::binder::Status status; \
        UT_LOG_INFO("Calling close(IN:avClockController:[0x%X], OUT:result)", avClockController.get()); \
        status = avClock->close(avClockController, &result); \
        UT_LOG_INFO("Result close(IN:avClockController:[0x%X]), status:[isOK:%s, exception:%s]", \
                    avClockController.get(), \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_FALSE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_ILLEGAL_STATE); \
    } while (0)

#define AVCLOCK_CLOSE_FAILURE_NULL(avClock, avClockController, result) \
    do { \
        ::android::binder::Status status; \
        UT_LOG_INFO("Calling close(IN:avClockController:[0x%X], OUT:result)", avClockController.get()); \
        status = avClock->close(avClockController, &result); \
        UT_LOG_INFO("Result close(IN:avClockController:[0x%X]), status:[isOK:%s, exception:%s]", \
                    avClockController.get(), \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_FALSE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_ILLEGAL_ARGUMENT); \
    } while (0)

#define AVCLOCK_REGISTER_EVENT_LISTENER_SUCCESS(avClock, eventListener) \
    do { \
        ::android::binder::Status status; \
        bool _aidl_return = false; \
        UT_LOG_INFO("Calling registerEventListener(IN:eventListener[0x%p], OUT:_aidl_return[])", eventListener.get()); \
        status = avClock->registerEventListener(eventListener, &_aidl_return); \
        UT_LOG_INFO("Result registerEventListener(IN:eventListener[0x%p], OUT:_aidl_return[%s]), status:[isOK:%s, exception:%s]", \
                    eventListener.get(), UT_Control_GetMapString(bool_mapTable, _aidl_return), \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_TRUE(_aidl_return); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
    } while (0)

#define AVCLOCK_REGISTER_EVENT_LISTENER_FAILURE(avClock, eventListener) \
    do { \
        ::android::binder::Status status; \
        bool _aidl_return = false; \
        UT_LOG_INFO("Calling registerEventListener(IN:eventListener[0x%p], OUT:_aidl_return[])", eventListener.get()); \
        status = avClock->registerEventListener(eventListener, &_aidl_return); \
        UT_LOG_INFO("Result registerEventListener(IN:eventListener[0x%p]), status:[isOK:%s, exception:%s]", \
                    eventListener.get(), \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_FALSE(status.isOk()); \
    } while (0)

#define AVCLOCK_REGISTER_EVENT_LISTENER_NULL_FAILURE(avClock, eventListener) \
    do { \
        ::android::binder::Status status; \
        bool _aidl_return = false; \
        UT_LOG_INFO("Calling registerEventListener(IN:eventListener[0x%p], OUT:_aidl_return[])", eventListener.get()); \
        status = avClock->registerEventListener(eventListener, &_aidl_return); \
        UT_LOG_INFO("Result registerEventListener(IN:eventListener[0x%p]), status:[isOK:%s, exception:%s]", \
                    eventListener.get(), \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_FALSE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_ILLEGAL_ARGUMENT); \
    } while (0)

#define AVCLOCK_UNREGISTER_EVENT_LISTENER_SUCCESS(avClock, eventListener) \
    do { \
        ::android::binder::Status status; \
        bool _aidl_return = false; \
        UT_LOG_INFO("Calling unregisterEventListener(IN:eventListener[0x%p], OUT:_aidl_return[])", eventListener.get()); \
        status = avClock->unregisterEventListener(eventListener, &_aidl_return); \
        UT_LOG_INFO("Result unregisterEventListener(IN:eventListener[0x%p], OUT:_aidl_return[%s]), status:[isOK:%s, exception:%s]", \
                    eventListener.get(), UT_Control_GetMapString(bool_mapTable, _aidl_return), \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_TRUE(_aidl_return); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
    } while (0)

#define AVCLOCK_UNREGISTER_EVENT_LISTENER_FAILURE(avClock, eventListener) \
    do { \
        ::android::binder::Status status; \
        bool _aidl_return = false; \
        UT_LOG_INFO("Calling unregisterEventListener(IN:eventListener[0x%p], OUT:_aidl_return[])", eventListener.get()); \
        status = avClock->unregisterEventListener(eventListener, &_aidl_return); \
        UT_LOG_INFO("Result unregisterEventListener(IN:eventListener[0x%p]), status:[isOK:%s, exception:%s]", \
                    eventListener.get(), \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_FALSE(status.isOk()); \
    } while (0)

#define AVCLOCK_UNREGISTER_EVENT_LISTENER_NULL_FAILURE(avClock, eventListener) \
    do { \
        ::android::binder::Status status; \
        bool _aidl_return = false; \
        UT_LOG_INFO("Calling unregisterEventListener(IN:eventListener[0x%p], OUT:_aidl_return[])", eventListener.get()); \
        status = avClock->unregisterEventListener(eventListener, &_aidl_return); \
        UT_LOG_INFO("Result unregisterEventListener(IN:eventListener[0x%p]), status:[isOK:%s, exception:%s]", \
                    eventListener.get(), \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_FALSE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_ILLEGAL_ARGUMENT); \
    } while (0)

#define AVCLOCK_START_SUCCESS(avClock, avClockController, stateCheck) \
    do { \
        ::android::binder::Status status; \
        UT_LOG_INFO("Calling start()"); \
        status = avClockController->start(); \
        UT_LOG_INFO("Result start(), status:[isOK:%s, exception:%s]", \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
        if(stateCheck){ \
            ::com::rdk::hal::avclock::State lastState; \
            AVCLOCK_WAIT_FOR_STATE(avClock, ::com::rdk::hal::avclock::State::STARTED, lastState); \
            UT_ASSERT_EQUAL(lastState, ::com::rdk::hal::avclock::State::STARTED); \
        } \
    } while (0)

#define AVCLOCK_START_FAILURE(avClock, avClockController) \
    do { \
        ::android::binder::Status status; \
        UT_LOG_INFO("Calling start()"); \
        status = avClockController->start(); \
        UT_LOG_INFO("Result start(), status:[isOK:%s, exception:%s]", \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_FALSE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_ILLEGAL_STATE); \
    } while (0)

#define AVCLOCK_STOP_SUCCESS(avClock, avClockController, stateCheck) \
    do { \
        ::android::binder::Status status; \
        UT_LOG_INFO("Calling stop()"); \
        status = avClockController->stop(); \
        UT_LOG_INFO("Result stop(), status:[isOK:%s, exception:%s]", \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
        if(stateCheck){ \
            ::com::rdk::hal::avclock::State lastState; \
            AVCLOCK_WAIT_FOR_STATE(avClock, ::com::rdk::hal::avclock::State::READY, lastState); \
            UT_ASSERT_EQUAL(lastState, ::com::rdk::hal::avclock::State::READY); \
        } \
    } while (0)

#define AVCLOCK_STOP_FAILURE(avClock, avClockController) \
    do { \
        ::android::binder::Status status; \
        UT_LOG_INFO("Calling stop()"); \
        status = avClockController->stop(); \
        UT_LOG_INFO("Result stop(), status:[isOK:%s, exception:%s]", \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_FALSE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_ILLEGAL_STATE); \
    } while (0)

#define AVCLOCK_SET_CLOCK_MODE_SUCCESS(avClock, avClockController, clockMode, expectedResult) \
    do { \
        ::android::binder::Status status; \
        bool _aidl_return = false; \
        UT_LOG_INFO("Calling setClockMode(IN:clockMode[%d], OUT:_aidl_return[])", static_cast<int>(clockMode)); \
        status = avClockController->setClockMode(clockMode, &_aidl_return); \
        UT_LOG_INFO("Result setClockMode(IN:clockMode[%d], OUT:_aidl_return[%s]), status:[isOK:%s, exception:%s]", \
                    static_cast<int>(clockMode), UT_Control_GetMapString(bool_mapTable, _aidl_return), \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
        UT_ASSERT_EQUAL(_aidl_return, expectedResult); \
    } while (0)

#define AVCLOCK_SET_CLOCK_MODE_FAILURE_INVALID_MODE(avClock, avClockController, clockMode) \
    do { \
        ::android::binder::Status status; \
        bool _aidl_return = false; \
        UT_LOG_INFO("Calling setClockMode(IN:clockMode[%d], OUT:_aidl_return[])", static_cast<int>(clockMode)); \
        status = avClockController->setClockMode(clockMode, &_aidl_return); \
        UT_LOG_INFO("Result setClockMode(IN:clockMode[%d], OUT:_aidl_return[%s]), status:[isOK:%s, exception:%s]", \
                    static_cast<int>(clockMode), UT_Control_GetMapString(bool_mapTable, _aidl_return), \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
    } while (0)

#define AVCLOCK_SET_CLOCK_MODE_FAILURE_STATE(avClock, avClockController, clockMode) \
    do { \
        ::android::binder::Status status; \
        bool _aidl_return = false; \
        UT_LOG_INFO("Calling setClockMode(IN:clockMode[%d], OUT:_aidl_return[])", static_cast<int>(clockMode)); \
        status = avClockController->setClockMode(clockMode, &_aidl_return); \
        UT_LOG_INFO("Result setClockMode(IN:clockMode[%d]), status:[isOK:%s, exception:%s]", \
                    static_cast<int>(clockMode), \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_FALSE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_ILLEGAL_STATE); \
    } while (0)

#define AVCLOCK_GET_CLOCK_MODE_SUCCESS(avClock, avClockController, expectedClockMode) \
    do { \
        ::android::binder::Status status; \
        com::rdk::hal::avclock::ClockMode clockMode; \
        UT_LOG_INFO("Calling getClockMode(OUT:clockMode[])"); \
        status = avClockController->getClockMode(&clockMode); \
        UT_LOG_INFO("Result getClockMode(OUT:clockMode[%d]), status:[isOK:%s, exception:%s]", \
                    static_cast<int>(clockMode), UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
        UT_ASSERT_EQUAL(static_cast<int>(clockMode), static_cast<int>(expectedClockMode)); \
    } while (0)

#define AVCLOCK_GET_CLOCK_MODE_FAILURE_STATE(avClock, avClockController) \
    do { \
        ::android::binder::Status status; \
        com::rdk::hal::avclock::ClockMode clockMode; \
        UT_LOG_INFO("Calling getClockMode(OUT:clockMode[])"); \
        status = avClockController->getClockMode(&clockMode); \
        UT_LOG_INFO("Result getClockMode(), status:[isOK:%s, exception:%s]", \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_FALSE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_ILLEGAL_STATE); \
    } while (0)

#define AVCLOCK_NOTIFY_PCR_SAMPLE_SUCCESS(avClock, avClockController, pcrTimeNs, sampleTimestampNs, expectedResult) \
    do { \
        ::android::binder::Status status; \
        bool _aidl_return = false; \
        UT_LOG_INFO("Calling notifyPCRSample(IN:pcrTimeNs[%lld], IN:sampleTimestampNs[%lld], OUT:_aidl_return[])", \
                    static_cast<long long>(pcrTimeNs), static_cast<long long>(sampleTimestampNs)); \
        status = avClockController->notifyPCRSample(pcrTimeNs, sampleTimestampNs, &_aidl_return); \
        UT_LOG_INFO("Result notifyPCRSample(IN:pcrTimeNs[%lld], IN:sampleTimestampNs[%lld], OUT:_aidl_return[%s]), status:[isOK:%s, exception:%s]", \
                    static_cast<long long>(pcrTimeNs), static_cast<long long>(sampleTimestampNs), \
                    UT_Control_GetMapString(bool_mapTable, _aidl_return), \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
        UT_ASSERT_EQUAL(_aidl_return, expectedResult); \
    } while (0)

#define AVCLOCK_NOTIFY_PCR_SAMPLE_FAILURE_INVALID_PARAMS(avClock, avClockController, pcrTimeNs, sampleTimestampNs) \
    do { \
        ::android::binder::Status status; \
        bool _aidl_return = false; \
        UT_LOG_INFO("Calling notifyPCRSample(IN:pcrTimeNs[%lld], IN:sampleTimestampNs[%lld], OUT:_aidl_return[])", \
                    static_cast<long long>(pcrTimeNs), static_cast<long long>(sampleTimestampNs)); \
        status = avClockController->notifyPCRSample(pcrTimeNs, sampleTimestampNs, &_aidl_return); \
        UT_LOG_INFO("Result notifyPCRSample(IN:pcrTimeNs[%lld], IN:sampleTimestampNs[%lld], OUT:_aidl_return[%s]), status:[isOK:%s, exception:%s]", \
                    static_cast<long long>(pcrTimeNs), static_cast<long long>(sampleTimestampNs), \
                    UT_Control_GetMapString(bool_mapTable, _aidl_return), \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
    } while (0)

#define AVCLOCK_NOTIFY_PCR_SAMPLE_FAILURE_STATE_OR_MODE(avClock, avClockController, pcrTimeNs, sampleTimestampNs) \
    do { \
        ::android::binder::Status status; \
        bool _aidl_return = false; \
        UT_LOG_INFO("Calling notifyPCRSample(IN:pcrTimeNs[%lld], IN:sampleTimestampNs[%lld], OUT:_aidl_return[])", \
                    static_cast<long long>(pcrTimeNs), static_cast<long long>(sampleTimestampNs)); \
        status = avClockController->notifyPCRSample(pcrTimeNs, sampleTimestampNs, &_aidl_return); \
        UT_LOG_INFO("Result notifyPCRSample(IN:pcrTimeNs[%lld], IN:sampleTimestampNs[%lld]), status:[isOK:%s, exception:%s]", \
                    static_cast<long long>(pcrTimeNs), static_cast<long long>(sampleTimestampNs), \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_ILLEGAL_STATE); \
    } while (0)

#define AVCLOCK_GET_CURRENT_CLOCK_TIME_SUCCESS(avClock, avClockController, clockTime) \
    do { \
        ::android::binder::Status status; \
        UT_LOG_INFO("Calling getCurrentClockTime(OUT:clockTime[])"); \
        status = avClockController->getCurrentClockTime(clockTime); \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
        if ((clockTime)->has_value()) { \
            const auto& ct = (clockTime)->value(); \
            UT_LOG_INFO("ClockTime (PRIMED): clockTimeNs=%lld, sampleTimestampNs=%lld", \
                        static_cast<long long>(ct.clockTimeNs), \
                        static_cast<long long>(ct.sampleTimestampNs)); \
            UT_ASSERT_TRUE(ct.clockTimeNs >= 0); \
            UT_ASSERT_TRUE(ct.sampleTimestampNs >= 0); \
        } else { \
            UT_LOG_INFO("ClockTime is NULL (UNPRIMED state)"); \
            UT_ASSERT_FALSE((clockTime)->has_value()); \
        } \
    } while (0)

#define AVCLOCK_GET_CURRENT_CLOCK_TIME_FAILURE_STATE(avClock, avClockController, clockTime) \
    do { \
        ::android::binder::Status status; \
        UT_LOG_INFO("Calling getCurrentClockTime(OUT:clockTime[])"); \
        status = avClockController->getCurrentClockTime(clockTime); \
        UT_LOG_INFO("Result getCurrentClockTime(), status:[isOK:%s, exception:%s]", \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_FALSE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_ILLEGAL_STATE); \
    } while (0)

#define AVCLOCK_SET_PLAYBACK_RATE_SUCCESS(avClock, avClockController, rate, expectedResult) \
    do { \
        ::android::binder::Status status; \
        bool _aidl_return = false; \
        UT_LOG_INFO("Calling setPlaybackRate(IN:rate[%.2f], OUT:_aidl_return[])", rate); \
        status = avClockController->setPlaybackRate(rate, &_aidl_return); \
        UT_LOG_INFO("Result setPlaybackRate(IN:rate[%.2f], OUT:_aidl_return[%s]), status:[isOK:%s, exception:%s]", \
                    rate, UT_Control_GetMapString(bool_mapTable, _aidl_return), \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
        UT_ASSERT_EQUAL(_aidl_return, expectedResult); \
    } while (0)

#define AVCLOCK_SET_PLAYBACK_RATE_FAILURE_INVALID_RATE(avClock, avClockController, rate) \
    do { \
        ::android::binder::Status status; \
        bool _aidl_return = false; \
        UT_LOG_INFO("Calling setPlaybackRate(IN:rate[%.2f], OUT:_aidl_return[])", rate); \
        status = avClockController->setPlaybackRate(rate, &_aidl_return); \
        UT_LOG_INFO("Result setPlaybackRate(IN:rate[%.2f], OUT:_aidl_return[%s]), status:[isOK:%s, exception:%s]", \
                    rate, UT_Control_GetMapString(bool_mapTable, _aidl_return), \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
    } while (0)

#define AVCLOCK_SET_PLAYBACK_RATE_FAILURE_STATE_OR_MODE(avClock, avClockController, rate) \
    do { \
        ::android::binder::Status status; \
        bool _aidl_return = false; \
        UT_LOG_INFO("Calling setPlaybackRate(IN:rate[%.2f], OUT:_aidl_return[])", rate); \
        status = avClockController->setPlaybackRate(rate, &_aidl_return); \
        UT_LOG_INFO("Result setPlaybackRate(IN:rate[%.2f]), status:[isOK:%s, exception:%s]", \
                    rate, \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_FALSE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_ILLEGAL_STATE); \
    } while (0)

#define AVCLOCK_GET_PLAYBACK_RATE_SUCCESS(avClock, avClockController, expectedRate) \
    do { \
        ::android::binder::Status status; \
        double rate; \
        UT_LOG_INFO("Calling getPlaybackRate(OUT:rate[])"); \
        status = avClockController->getPlaybackRate(&rate); \
        UT_LOG_INFO("Result getPlaybackRate(OUT:rate[%.2f]), status:[isOK:%s, exception:%s]", \
                    rate, UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
        UT_ASSERT_DOUBLE_EQUAL(rate, expectedRate,0); \
    } while (0)

#define AVCLOCK_GET_PLAYBACK_RATE_FAILURE_STATE_OR_MODE(avClock, avClockController) \
    do { \
        ::android::binder::Status status; \
        double rate; \
        UT_LOG_INFO("Calling getPlaybackRate(OUT:rate[])"); \
        status = avClockController->getPlaybackRate(&rate); \
        UT_LOG_INFO("Result getPlaybackRate(), status:[isOK:%s, exception:%s]", \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_FALSE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_ILLEGAL_STATE); \
    } while (0)

#define AVCLOCK_REGISTER_EVENT_SUCCESS(avClock, eventListener) \
    do { \
        ::android::binder::Status status; \
        bool _aidl_return = false; \
        UT_LOG_INFO("Calling registerEventListener(IN:eventListener[0x%p], OUT:_aidl_return[])", eventListener.get()); \
        status = avClock->registerEventListener(eventListener, &_aidl_return); \
        UT_LOG_INFO("Result registerEventListener(IN:eventListener[0x%p], OUT:_aidl_return[%s]), status:[isOK:%s, exception:%s]", \
                    eventListener.get(), UT_Control_GetMapString(bool_mapTable, _aidl_return), \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_TRUE(_aidl_return); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
    } while (0)

#define AVCLOCK_UNREGISTER_EVENT_SUCCESS(avClock, eventListener) \
    do { \
        ::android::binder::Status status; \
        bool _aidl_return = false; \
        UT_LOG_INFO("Calling unregisterEventListener(IN:eventListener[0x%p], OUT:_aidl_return[])", eventListener.get()); \
        status = avClock->unregisterEventListener(eventListener, &_aidl_return); \
        UT_LOG_INFO("Result unregisterEventListener(IN:eventListener[0x%p], OUT:_aidl_return[%s]), status:[isOK:%s, exception:%s]", \
                    eventListener.get(), UT_Control_GetMapString(bool_mapTable, _aidl_return), \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_TRUE(_aidl_return); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
    } while (0)

class AVClockPrimeTest {
public:
    AVClockPrimeTest(IAVClock::Id clockId,
                     sp<IAVClock> clockHandle,
                     avclock::ClockMode clockMode,
                     sp<AVClockControllerListener> clockController,
                     sp<IAVBuffer> bufferManager,
                     IAVBufferHelper* bufferHelper,
                     double playbackRate)
        : mClockId(clockId), mClockHandle(clockHandle), mClockMode(clockMode),
          mClockController(clockController), mBufferManager(bufferManager),
          mBufferHelper(bufferHelper), mPlaybackRate(playbackRate) {}

    int32_t getClockId() const { return mClockId.value; }
    sp<IAVClock> getClockHandle() const { return mClockHandle; }
    double getPlaybackRate() const { return mPlaybackRate; }

    sp<AVClockControllerListener> getControlListener() const {
        return mClockController;
    }

    void setAudioProperties(
    const sp<AudioDecoderControllerListener>& listener,
    bool tunnelMode)
    {
        mDecCtrlListener = listener;
        mAudioTunnelMode = tunnelMode;
    }

    sp<AudioDecoderControllerListener>& getDecCtrlListener()
    {
        return mDecCtrlListener;
    }

    bool getTunnelMode() const
    {
        return mAudioTunnelMode;
    }

    void validateClockTime(std::shared_ptr<AVClockPrimeTest>& clockTest, sp<IAVClock> clockHandle, sp<IAVClockController>& avClockController, double playbackrate, contentprimetype primetype, sp<AVClockControllerListener>& clockControlListener, threadtype threadType);
    void runAudioTest(const AudioConfig& audioCfg, sp<IAVClockController> &avClockController);
    void runVideoTest(const VideoConfig& videoCfg, sp<IAVClockController> &avClockController);
    void runPcrTest(const PcrConfig& pcrCfg, sp<IAVClockController> &avClockController);
    android::sp<com::rdk::hal::avclock::IAVClockController> setupAndStartClock();
    void shutdownClock(android::sp<com::rdk::hal::avclock::IAVClockController> clockcontroller);
    int64_t getMonotonicRawTimestamp();
    void setDecCtrlListener(const sp<AudioDecoderControllerListener>& listener);

private:
    void validateUnprimedClock(sp<IAVClockController>& avClockController,
                               threadtype streamType);
     sp<AudioDecoderControllerListener> mDecCtrlListener;
     bool mAudioTunnelMode = false;

    // Common members
    IAVClock::Id mClockId;
    sp<IAVClock> mClockHandle;
    avclock::ClockMode mClockMode;
    sp<AVClockControllerListener> mClockController;
    sp<IAVBuffer> mBufferManager;
    IAVBufferHelper* mBufferHelper;
    double mPlaybackRate;
    mutable std::mutex mMutex; // Protect shared state
    mutable std::mutex mwaitEventMutex; // Protect shared state
    std::condition_variable mWaitEventNotify;
};

#endif // __VTS_AVCLOCK_H__

 /** @} */ // End of AV_CLOCK_HALTEST_HEADER
 /** @} */ // End of AV_CLOCK_HALTEST
 /** @} */ // End of AV_CLOCK
 /** @} */ // End of HPK

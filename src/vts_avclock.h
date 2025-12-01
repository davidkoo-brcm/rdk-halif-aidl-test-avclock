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

#include "vts_avclock_init.h"

#include "com/rdk/hal/avclock/IAVClockManager.h"
#include "com/rdk/hal/avclock/BnAVClockControllerListener.h"
#include "com/rdk/hal/avclock/BnAVClockEventListener.h"

using android::sp;
using android::String16;
using android::defaultServiceManager;
using android::binder::Status;

using namespace com::rdk::hal;
using namespace com::rdk::hal::avclock;
using namespace com::rdk::hal::audiosink;
using namespace com::rdk::hal::videosink;

extern sp<IAVClockManager> gAVClockManagerService;

/* Status::Exception */
const static ut_control_keyStringMapping_t statusException_mapTable[] =
{
    {(char*)"EX_NONE",                  (int32_t) Status::EX_NONE},
    {(char*)"EX_SECURITY",              (int32_t) Status::EX_SECURITY},
    {(char*)"EX_BAD_PARCELABLE",        (int32_t) Status::EX_BAD_PARCELABLE},
    {(char*)"EX_ILLEGAL_ARGUMENT",      (int32_t) Status::EX_ILLEGAL_ARGUMENT},
    {(char*)"EX_NULL_POINTER",          (int32_t) Status::EX_NULL_POINTER},
    {(char*)"EX_ILLEGAL_STATE",         (int32_t) Status::EX_ILLEGAL_STATE},
    {(char*)"EX_NETWORK_MAIN_THREAD",   (int32_t) Status::EX_NETWORK_MAIN_THREAD},
    {(char*)"EX_UNSUPPORTED_OPERATION", (int32_t) Status::EX_UNSUPPORTED_OPERATION},
    {(char*)"EX_SERVICE_SPECIFIC",      (int32_t) Status::EX_SERVICE_SPECIFIC},
    {(char*)"EX_PARCELABLE",            (int32_t) Status::EX_PARCELABLE},
    {(char*)"EX_HAS_REPLY_HEADER",      (int32_t) Status::EX_HAS_REPLY_HEADER},
    {(char*)"EX_TRANSACTION_FAILED",    (int32_t) Status::EX_TRANSACTION_FAILED},
    {NULL, -1}
};

/* bool */
const static ut_control_keyStringMapping_t bool_mapTable[] =
{
    {(char*)"false", (int32_t) false},
    {(char*)"true",  (int32_t) true},
    {NULL, -1}
};
int getAVClockService();
void vts_avclock_helper_validateCapabilities(int avClockId, const avclock::Capabilities& capabilities);
bool vts_avclock_helper_validatePropertyValue(int avClockId, const std::optional<PropertyValue>& propertyValue, avclock::Property property);
bool vts_avclock_helper_areElementsUnique(const std::vector<int32_t>& ids);

#define MAX_TRANSITORY_STATE_WAIT_MS 10
#define MAX_FINAL_STATE_WAIT_MS 500

class AVClockControllerListener : public BnAVClockControllerListener {
    public:
        AVClockControllerListener() = default;
        virtual ~AVClockControllerListener() = default;

        Status onStateChanged(::com::rdk::hal::State oldState, ::com::rdk::hal::State newState) override {
            UT_LOG_INFO("AVClockControllerListener: State changed from %s to %s",
                        ::com::rdk::hal::toString(oldState).c_str(),
                        ::com::rdk::hal::toString(newState).c_str());
            return Status::ok();
        }
    };
class AVClockEventListener : public BnAVClockEventListener {
    public:
        AVClockEventListener() : mCurrentState(::com::rdk::hal::State::CLOSED) {}
        virtual ~AVClockEventListener() = default;

        /**
         * Wait for the decoder to reach a particular State within a given timeout.
         *
         * @param desiredState The state we want to observe.
         * @param timeout      How long to wait, e.g. std::chrono::milliseconds(500).
         * @return true if the state was reached; false if we timed out waiting.
         */
        bool waitForState(State desiredState, std::chrono::milliseconds timeout)
        {
            std::unique_lock<std::mutex> lock(mMutex);
            // Wait until mCurrentState == desiredState or until timeout
            return mCV.wait_for(lock, timeout, [this, desiredState]() {
                return (mCurrentState == desiredState);
            });
        }
        // Accessors for test verifications if desired
        std::vector<std::pair<State, State>> getStateChanges() const
        {
            std::lock_guard<std::mutex> lock(mMutex);
            return mStateChanges;
        }

        void resetStateChanges()
        {
            std::lock_guard<std::mutex> lock(mMutex);
            mStateChanges.clear();
        }


        Status onStateChanged(::com::rdk::hal::State oldState, ::com::rdk::hal::State newState) override {
            UT_LOG_INFO("AVClockEventListener: State changed from %s to %s",
                        ::com::rdk::hal::toString(oldState).c_str(),
                        ::com::rdk::hal::toString(newState).c_str());
            {
                std::lock_guard<std::mutex> lock(mMutex);
                mStateChanges.emplace_back(oldState, newState);
                mCurrentState = newState;
            }
            mCV.notify_all();
            return Status::ok();
        }

        bool checkStateTransition(::com::rdk::hal::State expOldState, ::com::rdk::hal::State expNewState) {
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
        mutable std::mutex mMutex;
        std::condition_variable mCV;
        ::com::rdk::hal::State mCurrentState;
        std::vector<std::pair< ::com::rdk::hal::State, ::com::rdk::hal::State>> mStateChanges;
    };

#define AVCLOCK_MANAGER_GETAVCLOCKIDS(clockIds) \
    do { \
        ::android::binder::Status status; \
        UT_LOG_INFO("Calling getAVClockIds(OUT:clockIds:[])"); \
        status = gAVClockManagerService->getAVClockIds(&clockIds); \
        UT_LOG_INFO("Result getAVClockIds(OUT:clockIds:[size: %d]), status:[isOK:%s, exception:%s]", \
                    clockIds.size(), UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE_FATAL(status.isOk()); \
        UT_ASSERT_FALSE_FATAL(clockIds.empty()); \
        UT_ASSERT_EQUAL_FATAL(status.exceptionCode(), Status::EX_NONE); \
    } while (0)

#define AVCLOCK_MANAGER_GETAVCLOCK_SUCCESS(avClockId, avClock) \
    do { \
        ::android::binder::Status status; \
        UT_LOG_INFO("Calling getAVClock(IN:avClockId:[%d], OUT:avClock[])", avClockId.value); \
        status = gAVClockManagerService->getAVClock(avClockId, &avClock); \
        UT_LOG_INFO("Result getAVClock(IN:avClockId:[%d], OUT:avClock:[0x%X]), status:[isOK:%s, exception:%s]", \
                    avClockId.value, avClock.get(), \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE_FATAL(status.isOk()); \
        UT_ASSERT_NOT_EQUAL_FATAL(avClock, nullptr); \
        UT_ASSERT_EQUAL_FATAL(status.exceptionCode(), Status::EX_NONE); \
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
        UT_LOG_INFO("Result getProperty(IN:property[%s], OUT:propertyValue[has_value:%s]), status:[isOK:%s, exception:%s]", \
                    com::rdk::hal::avclock::toString(property).c_str(), \
                    propertyValue.has_value() ? "true" : "false", \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_FALSE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_ILLEGAL_ARGUMENT); \
        UT_ASSERT_FALSE(propertyValue.has_value()); \
    } while (0)


#define AVCLOCK_SET_PROPERTY_SUCCESS(avClock, property, propertyValue, result) \
    do { \
        ::android::binder::Status status; \
        UT_LOG_INFO("Calling setProperty(IN:property[%s], IN:propertyValue[has_value:%s])", \
                    com::rdk::hal::avclock::toString(property).c_str(), \
                    propertyValue.value.has_value() ? "true" : "false"); \
        status = avClock->setProperty(property, propertyValue, &result); \
        UT_LOG_INFO("Result setProperty(IN:property[%s], IN:propertyValue[has_value:%s], OUT:result=%s), status:[isOK:%s, exception:%s]", \
                    com::rdk::hal::avclock::toString(property).c_str(), \
                    propertyValue.value.has_value() ? "true" : "false", \
                    result ? "true" : "false", \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
    } while (0)

#define AVCLOCK_SET_PROPERTY_FAILURE(avClock, property, propertyValue,result,exception) \
    do { \
        ::android::binder::Status status; \
        UT_LOG_INFO("Calling setProperty(IN:property[%d], IN:propertyValue[has_value:%s])", \
                    property, \
                    propertyValue.value.has_value() ? "true" : "false"); \
        status = avClock->setProperty(property, propertyValue, &result); \
        UT_LOG_INFO("Result setProperty(IN:property[%d], IN:propertyValue[has_value:%s], OUT:result=%s), status:[isOK:%s, exception:%s]", \
                    property, \
                    propertyValue.value.has_value() ? "true" : "false", \
                    result ? "true" : "false", \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_FALSE(result); \
        UT_ASSERT_EQUAL(status.exceptionCode(), exception); \
    } while (0)


#define AVCLOCK_MANAGER_GETAVCLOCK_FAILURE(avClockId, avClock) \
    do { \
        ::android::binder::Status status; \
        UT_LOG_INFO("Calling getAVClock(IN:avClockId:[%d], OUT:avClock[])", avClockId.value); \
        status = gAVClockManagerService->getAVClock(avClockId, &avClock); \
        UT_LOG_INFO("Result getAVClock(IN:avClockId:[%d], OUT:avClock:[0x%X]), status:[isOK:%s, exception:%s]", \
                    avClockId.value, avClock.get(), \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_FALSE(status.isOk()); \
        UT_ASSERT_EQUAL(avClock, nullptr); \
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
                    com::rdk::hal::toString(state).c_str(), \
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
        UT_ASSERT_TRUE_FATAL(status.isOk()); \
        UT_ASSERT_NOT_EQUAL_FATAL(avClockController.get(), nullptr); \
        UT_ASSERT_EQUAL_FATAL(status.exceptionCode(), Status::EX_NONE); \
        if (stateCheck) { \
            ::com::rdk::hal::State lastState; \
            AVCLOCK_WAIT_FOR_STATE(avClock, ::com::rdk::hal::State::READY, lastState); \
            UT_ASSERT_EQUAL(lastState, ::com::rdk::hal::State::READY); \
        } \
    } while (0)

#define AVCLOCK_CLOSE_SUCCESS(avClock, avClockController, stateCheck) \
    do { \
        ::android::binder::Status status; \
        bool _aidl_return; \
        UT_LOG_INFO("Calling close(IN:avClockController[0x%p], OUT:_aidl_return[])", avClockController.get()); \
        status = avClock->close(avClockController, &_aidl_return); \
        UT_LOG_INFO("Result close(IN:avClockController[0x%p], OUT:_aidl_return[%s]), status:[isOK:%s, exception:%s]", \
                    avClockController.get(), UT_Control_GetMapString(bool_mapTable, _aidl_return), \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE_FATAL(status.isOk()); \
        UT_ASSERT_TRUE_FATAL(_aidl_return); \
        UT_ASSERT_EQUAL_FATAL(status.exceptionCode(), Status::EX_NONE); \
        if (stateCheck) { \
            ::com::rdk::hal::State lastState; \
            AVCLOCK_WAIT_FOR_STATE(avClock, ::com::rdk::hal::State::CLOSED, lastState); \
            UT_ASSERT_EQUAL(lastState, ::com::rdk::hal::State::CLOSED); \
        } \
    } while (0)

#define AVCLOCK_OPEN_FAILURE(avClock, controllerListener, avClockController, exception) \
    do { \
        ::android::binder::Status status; \
        UT_LOG_INFO("Calling open(IN:controllerListener[0x%p], OUT:avClockController[])", controllerListener); \
        status = avClock->open(controllerListener, &avClockController); \
        UT_LOG_INFO("Result open(IN:controllerListener[0x%p], OUT:avClockController[0x%X]), status:[isOK:%s, exception:%s]", \
                    controllerListener, avClockController.get(), \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_FALSE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), exception); \
        UT_ASSERT_EQUAL(avClockController.get(), nullptr); \
    } while (0)

#define AVCLOCK_CLOSE_FAILURE(avClock, avClockController, result) \
    do { \
        ::android::binder::Status status; \
        UT_LOG_INFO("Calling close(IN:avClockController:[0x%X], OUT:result)", avClockController.get()); \
        status = avClock->close(avClockController, &result); \
        UT_LOG_INFO("Result close(IN:avClockController:[0x%X], OUT:result=%s), status:[isOK:%s, exception:%s]", \
                    avClockController.get(), result ? "true" : "false", \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_FALSE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_ILLEGAL_STATE); \
        UT_ASSERT_FALSE(result); \
    } while (0)

#define AVCLOCK_CLOSE_FAILURE_NULL(avClock, avClockController, result) \
    do { \
        ::android::binder::Status status; \
        UT_LOG_INFO("Calling close(IN:avClockController:[0x%X], OUT:result)", avClockController.get()); \
        status = avClock->close(avClockController, &result); \
        UT_LOG_INFO("Result close(IN:avClockController:[0x%X], OUT:result=%s), status:[isOK:%s, exception:%s]", \
                    avClockController.get(), result ? "true" : "false", \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_FALSE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_ILLEGAL_ARGUMENT); \
        UT_ASSERT_FALSE(result); \
    } while (0)

#define AVCLOCK_REGISTER_EVENT_LISTENER_SUCCESS(avClock, eventListener) \
    do { \
        ::android::binder::Status status; \
        bool _aidl_return; \
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
        bool _aidl_return; \
        UT_LOG_INFO("Calling registerEventListener(IN:eventListener[0x%p], OUT:_aidl_return[])", eventListener.get()); \
        status = avClock->registerEventListener(eventListener, &_aidl_return); \
        UT_LOG_INFO("Result registerEventListener(IN:eventListener[0x%p], OUT:_aidl_return[%s]), status:[isOK:%s, exception:%s]", \
                    eventListener.get(), UT_Control_GetMapString(bool_mapTable, _aidl_return), \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_FALSE(status.isOk()); \
        UT_ASSERT_FALSE(_aidl_return); \
    } while (0)

#define AVCLOCK_REGISTER_EVENT_LISTENER_NULL_FAILURE(avClock, eventListener) \
    do { \
        ::android::binder::Status status; \
        bool _aidl_return; \
        UT_LOG_INFO("Calling registerEventListener(IN:eventListener[0x%p], OUT:_aidl_return[])", eventListener.get()); \
        status = avClock->registerEventListener(eventListener, &_aidl_return); \
        UT_LOG_INFO("Result registerEventListener(IN:eventListener[0x%p], OUT:_aidl_return[%s]), status:[isOK:%s, exception:%s]", \
                    eventListener.get(), UT_Control_GetMapString(bool_mapTable, _aidl_return), \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_FALSE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_ILLEGAL_ARGUMENT); \
        UT_ASSERT_FALSE(_aidl_return); \
    } while (0)

#define AVCLOCK_UNREGISTER_EVENT_LISTENER_SUCCESS(avClock, eventListener) \
    do { \
        ::android::binder::Status status; \
        bool _aidl_return; \
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
        bool _aidl_return; \
        UT_LOG_INFO("Calling unregisterEventListener(IN:eventListener[0x%p], OUT:_aidl_return[])", eventListener.get()); \
        status = avClock->unregisterEventListener(eventListener, &_aidl_return); \
        UT_LOG_INFO("Result unregisterEventListener(IN:eventListener[0x%p], OUT:_aidl_return[%s]), status:[isOK:%s, exception:%s]", \
                    eventListener.get(), UT_Control_GetMapString(bool_mapTable, _aidl_return), \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_FALSE(status.isOk()); \
        UT_ASSERT_FALSE(_aidl_return); \
    } while (0)

#define AVCLOCK_UNREGISTER_EVENT_LISTENER_NULL_FAILURE(avClock, eventListener) \
    do { \
        ::android::binder::Status status; \
        bool _aidl_return; \
        UT_LOG_INFO("Calling unregisterEventListener(IN:eventListener[0x%p], OUT:_aidl_return[])", eventListener.get()); \
        status = avClock->unregisterEventListener(eventListener, &_aidl_return); \
        UT_LOG_INFO("Result unregisterEventListener(IN:eventListener[0x%p], OUT:_aidl_return[%s]), status:[isOK:%s, exception:%s]", \
                    eventListener.get(), UT_Control_GetMapString(bool_mapTable, _aidl_return), \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_FALSE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_ILLEGAL_ARGUMENT); \
        UT_ASSERT_FALSE(_aidl_return); \
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
            ::com::rdk::hal::State lastState; \
            AVCLOCK_WAIT_FOR_STATE(avClock, ::com::rdk::hal::State::STARTED, lastState); \
            UT_ASSERT_EQUAL(lastState, ::com::rdk::hal::State::STARTED); \
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
            ::com::rdk::hal::State lastState; \
            AVCLOCK_WAIT_FOR_STATE(avClock, ::com::rdk::hal::State::READY, lastState); \
            UT_ASSERT_EQUAL(lastState, ::com::rdk::hal::State::READY); \
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

#define AVCLOCK_SET_AUDIO_SINK_SUCCESS(avClock, avClockController, audioSinkId, expectedResult) \
    do { \
        ::android::binder::Status status; \
        bool _aidl_return; \
        UT_LOG_INFO("Calling setAudioSink(IN:audioSinkId[%d], OUT:_aidl_return[])", audioSinkId.value); \
        status = avClockController->setAudioSink(audioSinkId, &_aidl_return); \
        UT_LOG_INFO("Result setAudioSink(IN:audioSinkId[%d], OUT:_aidl_return[%s]), status:[isOK:%s, exception:%s]", \
                    audioSinkId.value, UT_Control_GetMapString(bool_mapTable, _aidl_return), \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
        UT_ASSERT_EQUAL(_aidl_return, expectedResult); \
    } while (0)

#define AVCLOCK_SET_AUDIO_SINK_FAILURE_INVALID_ID(avClock, avClockController, audioSinkId) \
    do { \
        ::android::binder::Status status; \
        bool _aidl_return; \
        UT_LOG_INFO("Calling setAudioSink(IN:audioSinkId[%d], OUT:_aidl_return[])", audioSinkId.value); \
        status = avClockController->setAudioSink(audioSinkId, &_aidl_return); \
        UT_LOG_INFO("Result setAudioSink(IN:audioSinkId[%d], OUT:_aidl_return[%s]), status:[isOK:%s, exception:%s]", \
                    audioSinkId.value, UT_Control_GetMapString(bool_mapTable, _aidl_return), \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
        UT_ASSERT_FALSE(_aidl_return); \
    } while (0)

#define AVCLOCK_SET_AUDIO_SINK_FAILURE_STATE(avClock, avClockController, audioSinkId) \
    do { \
        ::android::binder::Status status; \
        bool _aidl_return; \
        UT_LOG_INFO("Calling setAudioSink(IN:audioSinkId[%d], OUT:_aidl_return[])", audioSinkId.value); \
        status = avClockController->setAudioSink(audioSinkId, &_aidl_return); \
        UT_LOG_INFO("Result setAudioSink(IN:audioSinkId[%d], OUT:_aidl_return[%s]), status:[isOK:%s, exception:%s]", \
                    audioSinkId.value, UT_Control_GetMapString(bool_mapTable, _aidl_return), \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_FALSE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_ILLEGAL_STATE); \
        UT_ASSERT_FALSE(_aidl_return); \
    } while (0)


#define AVCLOCK_GET_AUDIO_SINK_SUCCESS(avClock, avClockController, expectedAudioSinkId) \
    do { \
        ::android::binder::Status status; \
        com::rdk::hal::audiosink::IAudioSink::Id audioSinkId; \
        UT_LOG_INFO("Calling getAudioSink(OUT:audioSinkId[])"); \
        status = avClockController->getAudioSink(&audioSinkId); \
        UT_LOG_INFO("Result getAudioSink(OUT:audioSinkId[%d]), status:[isOK:%s, exception:%s]", \
                    audioSinkId.value, UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
        UT_ASSERT_EQUAL(audioSinkId.value, expectedAudioSinkId.value); \
    } while (0)

#define AVCLOCK_GET_AUDIO_SINK_FAILURE_STATE(avClock, avClockController) \
    do { \
        ::android::binder::Status status; \
        com::rdk::hal::audiosink::IAudioSink::Id audioSinkId; \
        UT_LOG_INFO("Calling getAudioSink(OUT:audioSinkId[])"); \
        status = avClockController->getAudioSink(&audioSinkId); \
        UT_LOG_INFO("Result getAudioSink(OUT:audioSinkId[%d]), status:[isOK:%s, exception:%s]", \
                    audioSinkId.value, UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_FALSE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_ILLEGAL_STATE); \
    } while (0)

#define AVCLOCK_SETPROPERTY_FAILURE(avClock, property, propertyValue, expectedResult) \
    do { \
        bool result; \
        UT_LOG_INFO("Calling setProperty(IN:property[%s], IN:propertyValue[%d])", \
                    ::com::rdk::hal::toString(property).c_str(), \
                    propertyValue.value.get<PropertyValue::Value::Tag::intValue>()); \
        result = (avClock)->setProperty(property, propertyValue); \
        UT_LOG_INFO("Result setProperty(IN:property[%s], IN:propertyValue[%d]), result:[%s]", \
                    ::com::rdk::hal::toString(property).c_str(), \
                    propertyValue.value.get<PropertyValue::Value::Tag::intValue>(), \
                    UT_Control_GetMapString(bool_mapTable, result)); \
        UT_ASSERT_EQUAL(result, expectedResult); \
    } while (0)


#define AVCLOCK_SET_SUPPLEMENTARY_AUDIO_SINK_SUCCESS(avClock, avClockController, supplementaryAudioSinkId, expectedResult) \
    do { \
        ::android::binder::Status status; \
        bool _aidl_return; \
        UT_LOG_INFO("Calling setSupplementaryAudioSink(IN:supplementaryAudioSinkId[%d], OUT:_aidl_return[])", supplementaryAudioSinkId.value); \
        status = avClockController->setSupplementaryAudioSink(supplementaryAudioSinkId, &_aidl_return); \
        UT_LOG_INFO("Result setSupplementaryAudioSink(IN:supplementaryAudioSinkId[%d], OUT:_aidl_return[%s]), status:[isOK:%s, exception:%s]", \
                    supplementaryAudioSinkId.value, UT_Control_GetMapString(bool_mapTable, _aidl_return), \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
        UT_ASSERT_EQUAL(_aidl_return, expectedResult); \
    } while (0)

#define AVCLOCK_SET_SUPPLEMENTARY_AUDIO_SINK_FAILURE_INVALID_ID(avClock, avClockController, supplementaryAudioSinkId) \
    do { \
        ::android::binder::Status status; \
        bool _aidl_return; \
        UT_LOG_INFO("Calling setSupplementaryAudioSink(IN:supplementaryAudioSinkId[%d], OUT:_aidl_return[])", supplementaryAudioSinkId.value); \
        status = avClockController->setSupplementaryAudioSink(supplementaryAudioSinkId, &_aidl_return); \
        UT_LOG_INFO("Result setSupplementaryAudioSink(IN:supplementaryAudioSinkId[%d], OUT:_aidl_return[%s]), status:[isOK:%s, exception:%s]", \
                    supplementaryAudioSinkId.value, UT_Control_GetMapString(bool_mapTable, _aidl_return), \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
        UT_ASSERT_FALSE(_aidl_return); \
    } while (0)

#define AVCLOCK_SET_SUPPLEMENTARY_AUDIO_SINK_FAILURE_STATE(avClock, avClockController, supplementaryAudioSinkId) \
    do { \
        ::android::binder::Status status; \
        bool _aidl_return; \
        UT_LOG_INFO("Calling setSupplementaryAudioSink(IN:supplementaryAudioSinkId[%d], OUT:_aidl_return[])", supplementaryAudioSinkId.value); \
        status = avClockController->setSupplementaryAudioSink(supplementaryAudioSinkId, &_aidl_return); \
        UT_LOG_INFO("Result setSupplementaryAudioSink(IN:supplementaryAudioSinkId[%d], OUT:_aidl_return[%s]), status:[isOK:%s, exception:%s]", \
                    supplementaryAudioSinkId.value, UT_Control_GetMapString(bool_mapTable, _aidl_return), \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_FALSE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_ILLEGAL_STATE); \
        UT_ASSERT_FALSE(_aidl_return); \
    } while (0)

#define AVCLOCK_GET_SUPPLEMENTARY_AUDIO_SINK_SUCCESS(avClock, avClockController, expectedSupplementaryAudioSinkId) \
    do { \
        ::android::binder::Status status; \
        com::rdk::hal::audiosink::IAudioSink::Id supplementaryAudioSinkId; \
        UT_LOG_INFO("Calling getSupplementaryAudioSink(OUT:supplementaryAudioSinkId[])"); \
        status = avClockController->getSupplementaryAudioSink(&supplementaryAudioSinkId); \
        UT_LOG_INFO("Result getSupplementaryAudioSink(OUT:supplementaryAudioSinkId[%d]), status:[isOK:%s, exception:%s]", \
                    supplementaryAudioSinkId.value, UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
        UT_ASSERT_EQUAL(supplementaryAudioSinkId.value, expectedSupplementaryAudioSinkId.value); \
    } while (0)

#define AVCLOCK_GET_SUPPLEMENTARY_AUDIO_SINK_FAILURE_STATE(avClock, avClockController) \
    do { \
        ::android::binder::Status status; \
        com::rdk::hal::audiosink::IAudioSink::Id supplementaryAudioSinkId; \
        UT_LOG_INFO("Calling getSupplementaryAudioSink(OUT:supplementaryAudioSinkId[])"); \
        status = avClockController->getSupplementaryAudioSink(&supplementaryAudioSinkId); \
        UT_LOG_INFO("Result getSupplementaryAudioSink(OUT:supplementaryAudioSinkId[%d]), status:[isOK:%s, exception:%s]", \
                    supplementaryAudioSinkId.value, UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_FALSE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_ILLEGAL_STATE); \
    } while (0)

#define AVCLOCK_SET_VIDEO_SINK_SUCCESS(avClock, avClockController, videoSinkId, expectedResult) \
    do { \
        ::android::binder::Status status; \
        bool _aidl_return; \
        UT_LOG_INFO("Calling setVideoSink(IN:videoSinkId[%d], OUT:_aidl_return[])", videoSinkId.value); \
        status = avClockController->setVideoSink(videoSinkId, &_aidl_return); \
        UT_LOG_INFO("Result setVideoSink(IN:videoSinkId[%d], OUT:_aidl_return[%s]), status:[isOK:%s, exception:%s]", \
                    videoSinkId.value, UT_Control_GetMapString(bool_mapTable, _aidl_return), \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
        UT_ASSERT_EQUAL(_aidl_return, expectedResult); \
    } while (0)

#define AVCLOCK_SET_VIDEO_SINK_FAILURE_INVALID_ID(avClock, avClockController, videoSinkId) \
    do { \
        ::android::binder::Status status; \
        bool _aidl_return; \
        UT_LOG_INFO("Calling setVideoSink(IN:videoSinkId[%d], OUT:_aidl_return[])", videoSinkId.value); \
        status = avClockController->setVideoSink(videoSinkId, &_aidl_return); \
        UT_LOG_INFO("Result setVideoSink(IN:videoSinkId[%d], OUT:_aidl_return[%s]), status:[isOK:%s, exception:%s]", \
                    videoSinkId.value, UT_Control_GetMapString(bool_mapTable, _aidl_return), \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
        UT_ASSERT_FALSE(_aidl_return); \
    } while (0)

#define AVCLOCK_SET_VIDEO_SINK_FAILURE_STATE(avClock, avClockController, videoSinkId) \
    do { \
        ::android::binder::Status status; \
        bool _aidl_return; \
        UT_LOG_INFO("Calling setVideoSink(IN:videoSinkId[%d], OUT:_aidl_return[])", videoSinkId.value); \
        status = avClockController->setVideoSink(videoSinkId, &_aidl_return); \
        UT_LOG_INFO("Result setVideoSink(IN:videoSinkId[%d], OUT:_aidl_return[%s]), status:[isOK:%s, exception:%s]", \
                    videoSinkId.value, UT_Control_GetMapString(bool_mapTable, _aidl_return), \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_FALSE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_ILLEGAL_STATE); \
        UT_ASSERT_FALSE(_aidl_return); \
    } while (0)

#define AVCLOCK_GET_VIDEO_SINK_SUCCESS(avClock, avClockController, expectedVideoSinkId) \
    do { \
        ::android::binder::Status status; \
        com::rdk::hal::videosink::IVideoSink::Id videoSinkId; \
        UT_LOG_INFO("Calling getVideoSink(OUT:videoSinkId[])"); \
        status = avClockController->getVideoSink(&videoSinkId); \
        UT_LOG_INFO("Result getVideoSink(OUT:videoSinkId[%d]), status:[isOK:%s, exception:%s]", \
                    videoSinkId.value, UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
        UT_ASSERT_EQUAL(videoSinkId.value, expectedVideoSinkId.value); \
    } while (0)

#define AVCLOCK_GET_VIDEO_SINK_FAILURE_STATE(avClock, avClockController) \
    do { \
        ::android::binder::Status status; \
        com::rdk::hal::videosink::IVideoSink::Id videoSinkId; \
        UT_LOG_INFO("Calling getVideoSink(OUT:videoSinkId[])"); \
        status = avClockController->getVideoSink(&videoSinkId); \
        UT_LOG_INFO("Result getVideoSink(OUT:videoSinkId[%d]), status:[isOK:%s, exception:%s]", \
                    videoSinkId.value, UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_FALSE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_ILLEGAL_STATE); \
    } while (0)


#define AVCLOCK_SET_CLOCK_MODE_SUCCESS(avClock, avClockController, clockMode, expectedResult) \
    do { \
        ::android::binder::Status status; \
        bool _aidl_return; \
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
        bool _aidl_return; \
        UT_LOG_INFO("Calling setClockMode(IN:clockMode[%d], OUT:_aidl_return[])", static_cast<int>(clockMode)); \
        status = avClockController->setClockMode(clockMode, &_aidl_return); \
        UT_LOG_INFO("Result setClockMode(IN:clockMode[%d], OUT:_aidl_return[%s]), status:[isOK:%s, exception:%s]", \
                    static_cast<int>(clockMode), UT_Control_GetMapString(bool_mapTable, _aidl_return), \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
        UT_ASSERT_FALSE(_aidl_return); \
    } while (0)

#define AVCLOCK_SET_CLOCK_MODE_FAILURE_STATE(avClock, avClockController, clockMode) \
    do { \
        ::android::binder::Status status; \
        bool _aidl_return; \
        UT_LOG_INFO("Calling setClockMode(IN:clockMode[%d], OUT:_aidl_return[])", static_cast<int>(clockMode)); \
        status = avClockController->setClockMode(clockMode, &_aidl_return); \
        UT_LOG_INFO("Result setClockMode(IN:clockMode[%d], OUT:_aidl_return[%s]), status:[isOK:%s, exception:%s]", \
                    static_cast<int>(clockMode), UT_Control_GetMapString(bool_mapTable, _aidl_return), \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_FALSE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_ILLEGAL_STATE); \
        UT_ASSERT_FALSE(_aidl_return); \
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
        UT_LOG_INFO("Result getClockMode(OUT:clockMode[%d]), status:[isOK:%s, exception:%s]", \
                    static_cast<int>(clockMode), UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_FALSE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_ILLEGAL_STATE); \
    } while (0)

#define AVCLOCK_NOTIFY_PCR_SAMPLE_SUCCESS(avClock, avClockController, pcrTimeNs, sampleTimestampNs, expectedResult) \
    do { \
        ::android::binder::Status status; \
        bool _aidl_return; \
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
        bool _aidl_return; \
        UT_LOG_INFO("Calling notifyPCRSample(IN:pcrTimeNs[%lld], IN:sampleTimestampNs[%lld], OUT:_aidl_return[])", \
                    static_cast<long long>(pcrTimeNs), static_cast<long long>(sampleTimestampNs)); \
        status = avClockController->notifyPCRSample(pcrTimeNs, sampleTimestampNs, &_aidl_return); \
        UT_LOG_INFO("Result notifyPCRSample(IN:pcrTimeNs[%lld], IN:sampleTimestampNs[%lld], OUT:_aidl_return[%s]), status:[isOK:%s, exception:%s]", \
                    static_cast<long long>(pcrTimeNs), static_cast<long long>(sampleTimestampNs), \
                    UT_Control_GetMapString(bool_mapTable, _aidl_return), \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
        UT_ASSERT_FALSE(_aidl_return); \
    } while (0)

#define AVCLOCK_NOTIFY_PCR_SAMPLE_FAILURE_STATE_OR_MODE(avClock, avClockController, pcrTimeNs, sampleTimestampNs) \
    do { \
        ::android::binder::Status status; \
        bool _aidl_return; \
        UT_LOG_INFO("Calling notifyPCRSample(IN:pcrTimeNs[%lld], IN:sampleTimestampNs[%lld], OUT:_aidl_return[])", \
                    static_cast<long long>(pcrTimeNs), static_cast<long long>(sampleTimestampNs)); \
        status = avClockController->notifyPCRSample(pcrTimeNs, sampleTimestampNs, &_aidl_return); \
        UT_LOG_INFO("Result notifyPCRSample(IN:pcrTimeNs[%lld], IN:sampleTimestampNs[%lld], OUT:_aidl_return[%s]), status:[isOK:%s, exception:%s]", \
                    static_cast<long long>(pcrTimeNs), static_cast<long long>(sampleTimestampNs), \
                    UT_Control_GetMapString(bool_mapTable, _aidl_return), \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_ILLEGAL_STATE); \
        UT_ASSERT_FALSE(_aidl_return); \
    } while (0)

#define AVCLOCK_GET_CURRENT_CLOCK_TIME_SUCCESS(avClock, avClockController, expectedClockTimeNs) \
    do { \
        ::android::binder::Status status; \
        com::rdk::hal::avclock::ClockTime clockTime; \
        UT_LOG_INFO("Calling getCurrentClockTime(OUT:clockTime[])"); \
        status = avClockController->getCurrentClockTime(&clockTime); \
        UT_LOG_INFO("Result getCurrentClockTime(OUT:clockTime[clockTimeNs=%lld, sampleTimestampNs=%lld]), status:[isOK:%s, exception:%s]", \
                    static_cast<long long>(clockTime.clockTimeNs), static_cast<long long>(clockTime.sampleTimestampNs), \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
        UT_ASSERT_TRUE(clockTime.clockTimeNs >= 0); \
        UT_ASSERT_TRUE(clockTime.sampleTimestampNs >= 0); \
    } while (0)

#define AVCLOCK_GET_CURRENT_CLOCK_TIME_FAILURE_STATE(avClock, avClockController) \
    do { \
        ::android::binder::Status status; \
        com::rdk::hal::avclock::ClockTime clockTime; \
        UT_LOG_INFO("Calling getCurrentClockTime(OUT:clockTime[])"); \
        status = avClockController->getCurrentClockTime(&clockTime); \
        UT_LOG_INFO("Result getCurrentClockTime(OUT:clockTime[clockTimeNs=%lld, sampleTimestampNs=%lld]), status:[isOK:%s, exception:%s]", \
                    static_cast<long long>(clockTime.clockTimeNs), static_cast<long long>(clockTime.sampleTimestampNs), \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_FALSE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_ILLEGAL_STATE); \
    } while (0)

#define AVCLOCK_SET_PLAYBACK_RATE_SUCCESS(avClock, avClockController, rate, expectedResult) \
    do { \
        ::android::binder::Status status; \
        bool _aidl_return; \
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
        bool _aidl_return; \
        UT_LOG_INFO("Calling setPlaybackRate(IN:rate[%.2f], OUT:_aidl_return[])", rate); \
        status = avClockController->setPlaybackRate(rate, &_aidl_return); \
        UT_LOG_INFO("Result setPlaybackRate(IN:rate[%.2f], OUT:_aidl_return[%s]), status:[isOK:%s, exception:%s]", \
                    rate, UT_Control_GetMapString(bool_mapTable, _aidl_return), \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
        UT_ASSERT_FALSE(_aidl_return); \
    } while (0)

#define AVCLOCK_SET_PLAYBACK_RATE_FAILURE_STATE_OR_MODE(avClock, avClockController, rate) \
    do { \
        ::android::binder::Status status; \
        bool _aidl_return; \
        UT_LOG_INFO("Calling setPlaybackRate(IN:rate[%.2f], OUT:_aidl_return[])", rate); \
        status = avClockController->setPlaybackRate(rate, &_aidl_return); \
        UT_LOG_INFO("Result setPlaybackRate(IN:rate[%.2f], OUT:_aidl_return[%s]), status:[isOK:%s, exception:%s]", \
                    rate, UT_Control_GetMapString(bool_mapTable, _aidl_return), \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_ILLEGAL_STATE); \
        UT_ASSERT_FALSE(_aidl_return); \
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
        UT_LOG_INFO("Result getPlaybackRate(OUT:rate[%.2f]), status:[isOK:%s, exception:%s]", \
                    rate, UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_ILLEGAL_STATE); \
    } while (0)

#define AVCLOCK_REGISTER_EVENT_SUCCESS(avClock, eventListener) \
    do { \
        ::android::binder::Status status; \
        bool _aidl_return; \
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
        bool _aidl_return; \
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


#endif // __VTS_AVCLOCK_H__

 /** @} */ // End of AV_CLOCK_HALTEST_HEADER
 /** @} */ // End of AV_CLOCK_HALTEST
 /** @} */ // End of AV_CLOCK
 /** @} */ // End of HPK

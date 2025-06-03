
/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2024 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
 
#include <ut.h>
#include <ut_kvp_profile.h>
#include <ut_log.h>
#include <ut_control_plane.h>
#include <deque>
#include <binder/IServiceManager.h>
#include <binder/ProcessState.h>
#include <binder/IPCThreadState.h>
#include <binder/Status.h>
#include <utils/String16.h>
#include <stdio.h>
#include <vector>
#include <optional>
#include <algorithm>
#include <unordered_set>
#include <thread>

#include "com/rdk/hal/avclock/IAVClockManager.h"
#include "com/rdk/hal/avclock/BnAVClockControllerListener.h"
#include "com/rdk/hal/avclock/BnAVClockEventListener.h"


using android::sp;
using android::String16;
using android::defaultServiceManager;
using android::binder::Status;

using namespace com::rdk::hal;



using android::defaultServiceManager;
using android::binder::Status;

using namespace com::rdk::hal;
using namespace com::rdk::hal::avclock;

/* ───────────────────────────── Globals ──────────────────────────────── */
static std::vector<IAVClock::Id>                  mClockIds;
static std::vector<sp<IAVClock>>                  mAvClocks;
static std::vector<sp<IAVClockController>>        mControllers;
static sp<IAVClockManager>                        gAvClockManagerService = nullptr;

static constexpr int MAX_TRANSITORY_STATE_WAIT_MS = 10;
static constexpr int MAX_FINAL_STATE_WAIT_MS      = 500;


/* Status::Exception */
const static ut_control_keyStringMapping_t statusException_mapTable[] =
{
    {"EX_NONE",                  (int32_t) Status::EX_NONE},
    {"EX_SECURITY",              (int32_t) Status::EX_SECURITY},
    {"EX_BAD_PARCELABLE",        (int32_t) Status::EX_BAD_PARCELABLE},
    {"EX_ILLEGAL_ARGUMENT",      (int32_t) Status::EX_ILLEGAL_ARGUMENT},
    {"EX_NULL_POINTER",          (int32_t) Status::EX_NULL_POINTER},
    {"EX_ILLEGAL_STATE",         (int32_t) Status::EX_ILLEGAL_STATE},
    {"EX_NETWORK_MAIN_THREAD",   (int32_t) Status::EX_NETWORK_MAIN_THREAD},
    {"EX_UNSUPPORTED_OPERATION", (int32_t) Status::EX_UNSUPPORTED_OPERATION},
    {"EX_SERVICE_SPECIFIC",      (int32_t) Status::EX_SERVICE_SPECIFIC},
    {"EX_PARCELABLE",            (int32_t) Status::EX_PARCELABLE},
    {"EX_HAS_REPLY_HEADER",      (int32_t) Status::EX_HAS_REPLY_HEADER},
    {"EX_TRANSACTION_FAILED",    (int32_t) Status::EX_TRANSACTION_FAILED},
    {NULL, -1}
};

/* bool */
const static ut_control_keyStringMapping_t bool_mapTable[] =
{
    {"false", (int32_t) false},
    {"true",  (int32_t) true},
    {NULL, -1}
};


/* ──────────────────────────── Service ──────────────────────────────── */
int getService()
{
    gAvClockManagerService =
        android::interface_cast<IAVClockManager>(
            defaultServiceManager()->getService(
                String16(IAVClockManager::serviceName().c_str())));
    if (gAvClockManagerService == nullptr)
    {
        printf("Service not available\n");
        return -1;
    }
    return 0;
}

/* ─────────────────────── Listener implementations ───────────────────── */
class AVClockEventListenerImpl : public BnAVClockEventListener {
public:
    
    AVClockEventListenerImpl() {};
    virtual ~AVClockEventListenerImpl() = default;

    std::mutex                          mtx;
    std::condition_variable             cv;
    std::deque<std::pair<State, State>> stateChanges;

    Status onStateChanged(State oldState, State newState) override {
        {
            std::lock_guard<std::mutex> lg(mtx);
            stateChanges.emplace_back(oldState, newState);
        }
        cv.notify_all();
        return Status::ok();
    }

    
    bool waitForState(State oldState, State newState,
                      int timeoutMs = MAX_FINAL_STATE_WAIT_MS) {
        std::unique_lock<std::mutex> lock(mtx);
	return cv.wait_for(lock, std::chrono::milliseconds(timeoutMs), [&] {
            
	for (const auto& p : stateChanges) {
                if (p.first == oldState && p.second == newState) return true;
            }
            return false;
        });
    }
    
};

class AVClockControllerListenerImpl : public BnAVClockControllerListener {
public:
    AVClockControllerListenerImpl() {};
    virtual ~AVClockControllerListenerImpl() = default;

    ::android::binder::Status onStateChanged(::com::rdk::hal::State oldState,
                                             ::com::rdk::hal::State newState) override
    {
        return ::android::binder::Status::ok();
    }
};

/* ─────────────────────────── Base tests ─────────────────────────────── */
class AVClockManagerBaseTest : public virtual UTCore
{
protected:
    void SetUp() override
    {
        UTCore::SetUp();
        UT_ASSERT_EQUAL(getService(), 0);
    }

    void TearDown() override
    {
        UTCore::TearDown();
        gAvClockManagerService.clear();
    }
};


class AVClockBaseTest : public AVClockManagerBaseTest
{
public:
    sp<AVClockEventListenerImpl>                   listener;
    std::vector<sp<AVClockEventListenerImpl>>      mEvtListeners;
    std::vector<sp<IAVClock>>                      mAvClocks;
    std::vector<IAVClock::Id>                      mClockIds;
    std::vector<sp<AVClockControllerListenerImpl>> mCtrlListeners;
    std::vector<sp<IAVClockController>>            mControllers;
    State state;

    void SetUp() override
    {
        AVClockManagerBaseTest::SetUp();

        // AVClock setup
        if (mClockIds.empty())
        {
            std::vector<IAVClock::Id> ids;
            UT_LOG_INFO("Calling getAVClockIds(OUT:ids:[])");
            Status status = gAvClockManagerService->getAVClockIds(&ids);
            UT_ASSERT_TRUE(status.isOk());
            mClockIds = ids;
            UT_ASSERT_FALSE(mClockIds.empty());
        }

        for (auto& clkId : mClockIds)
        {
            listener = new AVClockEventListenerImpl();
            sp<IAVClock> clk;
            Status status = gAvClockManagerService->getAVClock(clkId, &clk);
            UT_ASSERT_TRUE(status.isOk());
            UT_ASSERT_NOT_EQUAL(clk, nullptr);

            bool registered = false;
            Status regStatus = clk->registerEventListener(listener, &registered);
            UT_ASSERT_TRUE(regStatus.isOk());
            UT_ASSERT_TRUE(registered);

            mAvClocks.push_back(clk);
            mEvtListeners.push_back(listener);
        }

        int i=0;
        // Controller setup
        for (auto& clk : mAvClocks)
        {
            sp<AVClockEventListenerImpl> listener = mEvtListeners[i];
            i++;
            sp<AVClockControllerListenerImpl> ctrlListener = new AVClockControllerListenerImpl();
            sp<IAVClockController> controller;
            Status status = clk->open(ctrlListener, &controller);
            UT_ASSERT_TRUE(status.isOk());
            UT_ASSERT_NOT_EQUAL(controller, nullptr);

            /* waitForState will block until the state change (from onStateChanged) or timeout */
            UT_ASSERT_TRUE(listener->waitForState(State::CLOSED, State::OPENING, MAX_TRANSITORY_STATE_WAIT_MS));
            UT_ASSERT_TRUE(listener->waitForState(State::OPENING, State::READY, MAX_FINAL_STATE_WAIT_MS));


            status = clk->getState(&state);
            UT_ASSERT_TRUE(status.isOk());
            UT_ASSERT_EQUAL(state,State::READY);

            mControllers.push_back(controller);
            mCtrlListeners.push_back(ctrlListener);
        }
    }

    void TearDown() override
    {
        UT_ASSERT_EQUAL(mControllers.size(), mAvClocks.size());
        for (size_t i = 0; i < mAvClocks.size(); ++i)
        {
            sp<AVClockEventListenerImpl> listener = mEvtListeners[i];
            bool closed = false;
            Status status = mAvClocks[i]->close(mControllers[i], &closed);
            UT_ASSERT_TRUE(status.isOk());
            UT_ASSERT_TRUE(closed);

            /* waitForState will block until the state change (from onStateChanged) or timeout */
            UT_ASSERT_TRUE(listener->waitForState(State::READY, State::CLOSING, MAX_TRANSITORY_STATE_WAIT_MS));
            UT_ASSERT_TRUE(listener->waitForState(State::CLOSING, State::CLOSED, MAX_FINAL_STATE_WAIT_MS));

            status = mAvClocks[i]->getState(&state);
            UT_ASSERT_TRUE(status.isOk());
            UT_ASSERT_EQUAL(state,State::CLOSED);
        }

        for (size_t i = 0; i < mAvClocks.size(); ++i)
        {
            bool unregistered = false;
            Status status = mAvClocks[i]->unregisterEventListener(mEvtListeners[i], &unregistered);
            UT_ASSERT_TRUE(unregistered);
        }

        mCtrlListeners.clear();
        mEvtListeners.clear();
        mControllers.clear();
        mAvClocks.clear();
        mClockIds.clear();

        AVClockManagerBaseTest::TearDown();
    }
};

/**
 * @brief Test for retrieving read-only properties from AVClock instances.
 *
 * This test verifies that the getProperty function correctly retrieves the RESOURCE_ID property
 * from each AVClock instance and ensures that the property value is present and the operation
 * completes successfully without exceptions.
 *
 * **Test Group ID:** 02@n
 * **Test Case ID:** 001@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/avClock_High-Level-2_TestSpecification.md)
 */
UT_ADD_TEST(AVClockBaseTest, GetReadOnlyProperties) 
{
    for (const auto& clk : mAvClocks) 
    {
        std::optional<PropertyValue> propertyValue;
        UT_LOG_INFO("Calling getProperty(IN:property:[RESOURCE_ID], OUT:propertyValue[])");
        Status status = clk->getProperty(Property::RESOURCE_ID, &propertyValue);
        UT_LOG_INFO("Result getProperty(IN:property:[RESOURCE_ID], OUT:propertyValue[%s]), status:[isOK:%s, exception:%s]",
                    propertyValue.has_value() ? "value present" : "no value",
                    UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_TRUE(status.isOk());
        UT_ASSERT_TRUE(propertyValue.has_value());


        UT_LOG_INFO("Calling getProperty(IN:property:[METRIC_AV_SYNC_ACCURACY_MS], OUT:propertyValue[])");
        status = clk->getProperty(Property::METRIC_AV_SYNC_ACCURACY_MS, &propertyValue);
        UT_LOG_INFO("Result getProperty(IN:property:[METRIC_AV_SYNC_ACCURACY_MS], OUT:propertyValue[%s]), status:[isOK:%s, exception:%s]",
                    propertyValue.has_value() ? "value present" : "no value",
                    UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_TRUE(status.isOk());
        UT_ASSERT_TRUE(propertyValue.has_value());

        UT_LOG_INFO("Calling getProperty(IN:property:[METRIC_AV_RESYNC_COUNT], OUT:propertyValue[])");
        status = clk->getProperty(Property::METRIC_AV_RESYNC_COUNT, &propertyValue);
        UT_LOG_INFO("Result getProperty(IN:property:[METRIC_AV_RESYNC_COUNT], OUT:propertyValue[%s]), status:[isOK:%s, exception:%s]",
                    propertyValue.has_value() ? "value present" : "no value",
                    UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_TRUE(status.isOk());
        UT_ASSERT_TRUE(propertyValue.has_value());

    }
}

/**
 * @brief Test to verify setting a read-only property in AVClock
 *
 * This test checks the behavior of the AVClock when attempting to set a read-only property.
 * It ensures that the operation does not succeed and the status returned is OK, indicating
 * that the operation was handled gracefully without exceptions.
 *
 * **Test Group ID:** 02@n
 * **Test Case ID:** 002@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/avClock_High-Level-2_TestSpecification.md)
 */
UT_ADD_TEST(AVClockBaseTest, SetReadOnlyPropertyTest) 
{
    for (auto& clk : mAvClocks) {
        PropertyValue propertyValue;
        propertyValue.value = PropertyValue::Value::make<PropertyValue::Value::Tag::intValue>(1);
        bool result = true;

        UT_LOG_INFO("Calling setProperty(IN:property:[RESOURCE_ID], IN:propertyValue:[intValue:123], OUT:result[])");
        Status status = clk->setProperty(Property::RESOURCE_ID, propertyValue, &result);
        UT_LOG_INFO("Result setProperty(IN:property:[RESOURCE_ID], IN:propertyValue:[intValue:123], OUT:result[%s]), status:[isOK:%s, exception:%s]",
                    UT_Control_GetMapString(bool_mapTable, result),
                    UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        EXPECT_FALSE(result);
        EXPECT_TRUE(status.isOk());

        UT_LOG_INFO("Calling setProperty(IN:property:[METRIC_AV_SYNC_ACCURACY_MS], IN:propertyValue:[intValue:123], OUT:result[])");
        status = clk->setProperty(Property::METRIC_AV_SYNC_ACCURACY_MS, propertyValue, &result);
        UT_LOG_INFO("Result setProperty(IN:property:[METRIC_AV_SYNC_ACCURACY_MS], IN:propertyValue:[intValue:123], OUT:result[%s]), status:[isOK:%s, exception:%s]",
                    UT_Control_GetMapString(bool_mapTable, result),
                    UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        EXPECT_FALSE(result);
        EXPECT_TRUE(status.isOk());

        UT_LOG_INFO("Calling setProperty(IN:property:[METRIC_AV_RESYNC_COUNT], IN:propertyValue:[intValue:123], OUT:result[])");
        status = clk->setProperty(Property::METRIC_AV_RESYNC_COUNT, propertyValue, &result);
        UT_LOG_INFO("Result setProperty(IN:property:[METRIC_AV_RESYNC_COUNT], IN:propertyValue:[intValue:123], OUT:result[%s]), status:[isOK:%s, exception:%s]",
                    UT_Control_GetMapString(bool_mapTable, result),
                    UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        EXPECT_FALSE(result);
        EXPECT_TRUE(status.isOk());
    }
}

/**
 * @brief Test the state transitions of the AV Clock
 *
 * This test verifies the state transitions of the AV Clock from OPENING to READY, 
 * STARTING to STARTED, STOPPING to READY, and ensures that the AV Clock can be 
 * opened, started, stopped, and closed successfully without errors.
 *
 * **Test Group ID:** 02@n
 * **Test Case ID:** 003@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/avClock_High-Level-2_TestSpecification.md)
 */

 UT_ADD_TEST(AVClockBaseTest, TestStateTransitions) {
    State state;
    for (size_t i = 0; i < mControllers.size(); ++i) {
        auto& controller = mControllers[i];
        auto& listener = mEvtListeners[i];

        // Start the AV Clock
        UT_LOG_INFO("Calling start()");
        Status startStatus = controller->start();
        UT_LOG_INFO("Result start(), status:[isOK:%s, exception:%s]",
                    UT_Control_GetMapString(bool_mapTable, startStatus.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, startStatus.exceptionCode()));
        UT_ASSERT_TRUE(startStatus.isOk());

        /* waitForState will block until the state change (from onStateChanged) or timeout */
        UT_ASSERT_TRUE(listener->waitForState(State::STARTING, State::STARTED, MAX_FINAL_STATE_WAIT_MS));

        Status status = mAvClocks[i]->getState(&state);
        UT_ASSERT_TRUE(status.isOk());
        UT_ASSERT_EQUAL(state,State::STARTED);

        // Stop the AV Clock
        UT_LOG_INFO("Calling stop()");
        Status stopStatus = controller->stop();
        UT_LOG_INFO("Result stop(), status:[isOK:%s, exception:%s]",
                    UT_Control_GetMapString(bool_mapTable, stopStatus.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, stopStatus.exceptionCode()));
        UT_ASSERT_TRUE(stopStatus.isOk());

        /* waitForState will block until the state change (from onStateChanged) or timeout */
        UT_ASSERT_TRUE(listener->waitForState(State::STOPPING, State::READY, MAX_FINAL_STATE_WAIT_MS));
        
        status = mAvClocks[i]->getState(&state);
        UT_ASSERT_TRUE(status.isOk());
        UT_ASSERT_EQUAL(state,State::READY);
    }
}

/**
 * @brief Verify the clock mode is set to AUTO on open and can be set and retrieved correctly.
 *
 * This test checks the initial clock mode after opening the controller and verifies that it is set to AUTO. 
 * It then sets the clock mode to AUDIO_MASTER and checks if the mode is correctly updated and retrieved.
 *
 * **Test Group ID:** 02@n
 * **Test Case ID:** 004@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/avClock_High-Level-2_TestSpecification.md)
 */
UT_ADD_TEST(AVClockBaseTest, VerifyClockModeSetToAutoOnOpenAndSetGetClockMode) {
    for (size_t i = 0; i < mControllers.size(); ++i) 
    {
        // Verify initial clock mode is AUTO after open
        ClockMode initialMode;
        UT_LOG_INFO("Calling getClockMode(OUT:initialMode[])");
        Status status = mControllers[i]->getClockMode(&initialMode);
        UT_LOG_INFO("Result getClockMode(OUT:initialMode[%d]), status:[isOK:%s, exception:%s]",
                    static_cast<int32_t>(initialMode),
                    UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_TRUE(status.isOk());
        UT_ASSERT_EQUAL(initialMode, ClockMode::AUTO);

        std::vector<ClockMode> clockModes = {
            ClockMode::AUTO,
            ClockMode::PCR,
            ClockMode::AUDIO_MASTER,
            ClockMode::VIDEO_MASTER
        };

        for (ClockMode mode : clockModes)
        {
            bool setResult = false;
            UT_LOG_INFO("Calling setClockMode(IN:clockMode:[%d], OUT:setResult[])", static_cast<int32_t>(mode));
            status = mControllers[i]->setClockMode(mode, &setResult);
            UT_LOG_INFO("Result setClockMode(IN:clockMode:[%d], OUT:setResult[%s]), status:[isOK:%s, exception:%s]",
                        static_cast<int32_t>(mode),
                        UT_Control_GetMapString(bool_mapTable, setResult),
                        UT_Control_GetMapString(bool_mapTable, status.isOk()),
                        UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
            UT_ASSERT_TRUE(status.isOk());
            UT_ASSERT_TRUE(setResult);

            // Verify clock mode was set correctly
            ClockMode setMode;
            UT_LOG_INFO("Calling getClockMode(OUT:setMode[])");
            status = mControllers[i]->getClockMode(&setMode);
            UT_LOG_INFO("Result getClockMode(OUT:setMode[%d]), status:[isOK:%s, exception:%s]",
                        static_cast<int32_t>(setMode),
                        UT_Control_GetMapString(bool_mapTable, status.isOk()),
                        UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
            UT_ASSERT_TRUE(status.isOk());
            UT_ASSERT_EQUAL(setMode, mode);
        }
    }
}
/**
* @brief Verify the playback rate and state transitions of AV Clock controllers
*
* This test checks the functionality of setting and getting playback rates, as well as verifying state transitions when starting and stopping the AV Clock. It ensures that the playback rate can be set to specific values and that the AV Clock transitions through expected states.
*
* **Test Group ID:** 02@n
* **Test Case ID:** 005@n[i]
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/avClock_High-Level-2_TestSpecification.md)
*/
UT_ADD_TEST(AVClockBaseTest, VerifyPlaybackRateAndStateTransitions) {
    State state;
    UT_ASSERT_FALSE(mControllers.empty());

    for (size_t i = 0; i < mControllers.size(); ++i)
    {
        double playbackRate = 0.0;
        bool success = false;
        
        //check initial playback rate
        UT_LOG_INFO("Calling getPlaybackRate(OUT:playbackRate[])");
        Status status = mControllers[i]->getPlaybackRate(&playbackRate);
        UT_LOG_INFO("Result getPlaybackRate(OUT:playbackRate[%f]), status:[isOK:%s, exception:%s]",
                    playbackRate,
                    UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_EQUAL(playbackRate, 1.0);

        // Set playback rate to 0.0, 1.0, 2.0 and verify
        std::vector<double> playbackRates = {0.0, 1.0, 2.0};

        for (double rate : playbackRates)
        {
            bool success = false;
            UT_LOG_INFO("Calling setPlaybackRate(IN:rate:[%f], OUT:success[])", rate);
            Status status = mControllers[i]->setPlaybackRate(rate, &success);
            UT_LOG_INFO("Result setPlaybackRate(IN:rate:[%f], OUT:success[%s]), status:[isOK:%s, exception:%s]",
                        rate,
                        UT_Control_GetMapString(bool_mapTable, success),
                        UT_Control_GetMapString(bool_mapTable, status.isOk()),
                        UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
            UT_ASSERT_TRUE(success);

            double playbackRate = -1.0;
            UT_LOG_INFO("Calling getPlaybackRate(OUT:playbackRate[])");
            status = mControllers[i]->getPlaybackRate(&playbackRate);
            UT_LOG_INFO("Result getPlaybackRate(OUT:playbackRate[%f]), status:[isOK:%s, exception:%s]",
                        playbackRate,
                        UT_Control_GetMapString(bool_mapTable, status.isOk()),
                        UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
            UT_ASSERT_EQUAL(playbackRate, rate);
        }

        // Start the AV Clock and verify state transition
        UT_LOG_INFO("Calling start()");
        status = mControllers[i]->start();
        UT_LOG_INFO("Result start(), status:[isOK:%s, exception:%s]",
                    UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_TRUE(status.isOk());

        status = mAvClocks[i]->getState(&state);
        UT_ASSERT_TRUE(status.isOk());
        UT_ASSERT_EQUAL(state,State::STARTED);

        for (double rate : playbackRates)
        {
            bool success = false;
            UT_LOG_INFO("Calling setPlaybackRate(IN:rate:[%f], OUT:success[])", rate);
            Status status = mControllers[i]->setPlaybackRate(rate, &success);
            UT_LOG_INFO("Result setPlaybackRate(IN:rate:[%f], OUT:success[%s]), status:[isOK:%s, exception:%s]",
                        rate,
                        UT_Control_GetMapString(bool_mapTable, success),
                        UT_Control_GetMapString(bool_mapTable, status.isOk()),
                        UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
            UT_ASSERT_TRUE(success);

            double playbackRate = -1.0;
            UT_LOG_INFO("Calling getPlaybackRate(OUT:playbackRate[])");
            status = mControllers[i]->getPlaybackRate(&playbackRate);
            UT_LOG_INFO("Result getPlaybackRate(OUT:playbackRate[%f]), status:[isOK:%s, exception:%s]",
                        playbackRate,
                        UT_Control_GetMapString(bool_mapTable, status.isOk()),
                        UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
            UT_ASSERT_EQUAL(playbackRate, rate);
        }

        // Stop the AV Clock and verify state transition
        UT_LOG_INFO("Calling stop()");
        status = mControllers[i]->stop();
        UT_LOG_INFO("Result stop(), status:[isOK:%s, exception:%s]",
                    UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_TRUE(status.isOk()); 

        status = mAvClocks[i]->getState(&state);
        UT_ASSERT_TRUE(status.isOk());
        UT_ASSERT_EQUAL(state,State::READY);
    }
}

int main(int argc, char **argv)
{
    UT_status_t result;
    UT_init(argc, argv);
    android::ProcessState::self()->startThreadPool();
    result = UT_run_tests();
    return result;
}



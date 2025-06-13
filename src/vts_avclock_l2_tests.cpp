
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
#include "vts_avclock.h"


using android::sp;
using android::String16;
using android::defaultServiceManager;
using android::binder::Status;

using namespace com::rdk::hal;



using android::defaultServiceManager;
using android::binder::Status;

using namespace com::rdk::hal;
using namespace com::rdk::hal::avclock;


class VTS_L2_AVCLOCK : public UTCore
{
public:
    std::vector<IAVClock::Id>                      mClockIds;
    std::vector<sp<IAVClock>>                      mAvClocks;
    std::vector<sp<AVClockEventListener>>          mEvtListeners;
    std::vector<sp<AVClockControllerListener>>     mCtrlListeners;
    std::vector<sp<IAVClockController>>            mControllers;

    void SetUp() override
    {
        UTCore::SetUp();
        // AVClock setup
        if (mClockIds.empty())
        {
            std::vector<IAVClock::Id> ids;
            UT_LOG_INFO("Calling getAVClockIds(OUT:ids:[])");
            AVCLOCK_MANAGER_GETAVCLOCKIDS(ids);
            mClockIds = ids;
        }

        for (auto& clkId : mClockIds)
        {
            sp<AVClockEventListener> listener = new AVClockEventListener();
            sp<AVClockControllerListener> ctrlListener = new AVClockControllerListener();
            sp<IAVClock> clk;
            sp<IAVClockController> controller;
            AVCLOCK_MANAGER_GETAVCLOCK_SUCCESS(clkId, clk);
            mAvClocks.push_back(clk);
            AVCLOCK_REGISTER_EVENT_LISTENER_SUCCESS(clk,listener);
            mEvtListeners.push_back(listener);
            AVCLOCK_OPEN_SUCCESS(clk,ctrlListener,controller,true);
            mControllers.push_back(controller);
            mCtrlListeners.push_back(ctrlListener);
        }
    }

    void TearDown() override
    {
        UTCore::TearDown();
        for (size_t i = 0; i < mAvClocks.size(); ++i)
        {
            AVCLOCK_UNREGISTER_EVENT_LISTENER_SUCCESS(mAvClocks[i],mEvtListeners[i]);
            AVCLOCK_CLOSE_SUCCESS(mAvClocks[i],mControllers[i], true);
        }
        mCtrlListeners.clear();
        mControllers.clear();
        mEvtListeners.clear();
        mAvClocks.clear();
        mClockIds.clear();
    }
    void checkCompleteStateTransition(sp<IAVClock> clk, sp<AVClockEventListener> eventListener, State oldState, State intermediateState, State newState)
    {
        ::com::rdk::hal::State currentState;
        eventListener->waitForState(intermediateState, std::chrono::milliseconds(MAX_TRANSITORY_STATE_WAIT_MS));
        UT_ASSERT_TRUE(eventListener->checkStateTransition(oldState, intermediateState));
        eventListener->waitForState(newState, std::chrono::milliseconds(MAX_FINAL_STATE_WAIT_MS));
        AVCLOCK_GET_STATE(clk, currentState);
        UT_ASSERT_EQUAL(currentState, newState);
        UT_ASSERT_TRUE(eventListener->checkStateTransition(intermediateState, newState));
        eventListener->resetStateChanges();
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
UT_ADD_TEST(VTS_L2_AVCLOCK, GetReadOnlyProperties)
{
    for (const auto& clk : mAvClocks)
    {
        std::optional<PropertyValue> propertyValue;
        AVCLOCK_GET_PROPERTY_SUCCESS(clk,com::rdk::hal::avclock::Property::RESOURCE_ID,propertyValue);
        AVCLOCK_GET_PROPERTY_SUCCESS(clk,com::rdk::hal::avclock::Property::METRIC_AV_SYNC_ACCURACY_MS,propertyValue);
        AVCLOCK_GET_PROPERTY_SUCCESS(clk,com::rdk::hal::avclock::Property::METRIC_AV_RESYNC_COUNT,propertyValue);
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
UT_ADD_TEST(VTS_L2_AVCLOCK, SetReadOnlyPropertyTest)
{
    for (auto& clk : mAvClocks) {
        PropertyValue propertyValue;
        propertyValue.value = PropertyValue::Value::make<PropertyValue::Value::Tag::intValue>(1);
        bool result = true;
        AVCLOCK_SET_PROPERTY_FAILURE(clk,com::rdk::hal::avclock::Property::RESOURCE_ID,propertyValue,result,Status::EX_NONE);
        EXPECT_FALSE(result);
        AVCLOCK_SET_PROPERTY_FAILURE(clk,com::rdk::hal::avclock::Property::METRIC_AV_SYNC_ACCURACY_MS,propertyValue,result , Status::EX_NONE);
        EXPECT_FALSE(result);
        AVCLOCK_SET_PROPERTY_FAILURE(clk,com::rdk::hal::avclock::Property::METRIC_AV_RESYNC_COUNT,propertyValue,result, Status::EX_NONE);
        EXPECT_FALSE(result);
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

 UT_ADD_TEST(VTS_L2_AVCLOCK, TestStateTransitions) {
    State state;
    for (size_t i = 0; i < mAvClocks.size(); ++i) {
        sp<IAVClockController> controller = mControllers[i];
        sp<AVClockEventListener> listener = mEvtListeners[i];

        // Start the AV Clock
        AVCLOCK_START_SUCCESS(mAvClocks[i],controller,false);
        checkCompleteStateTransition(mAvClocks[i],listener,State::READY,State::STARTING,State::STARTED);

        // Stop the AV Clock
        AVCLOCK_STOP_SUCCESS(mAvClocks[i],controller,false);
        checkCompleteStateTransition(mAvClocks[i],listener,State::STARTED,State::STOPPING,State::READY);
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
UT_ADD_TEST(VTS_L2_AVCLOCK, VerifyClockModeSetToAutoOnOpenAndSetGetClockMode) {
    for (size_t i = 0; i < mAvClocks.size(); ++i)
    {
        // Verify initial clock mode is AUTO after open
        AVCLOCK_GET_CLOCK_MODE_SUCCESS(mAvClock[i],mControllers[i],ClockMode::AUTO);

        std::vector<ClockMode> clockModes = {
            ClockMode::AUTO,
            ClockMode::PCR,
            ClockMode::AUDIO_MASTER,
            ClockMode::VIDEO_MASTER
        };

        for (ClockMode mode : clockModes)
        {
            AVCLOCK_SET_CLOCK_MODE_SUCCESS(mAvClocks[i],mControllers[i],mode,true);
            AVCLOCK_GET_CLOCK_MODE_SUCCESS(mAvClocks[i],mControllers[i],mode);
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
UT_ADD_TEST(VTS_L2_AVCLOCK, VerifyPlaybackRateAndStateTransitions) {
    State state;

    for (size_t i = 0; i < mAvClocks.size(); ++i)
    {
        AVCLOCK_GET_PLAYBACK_RATE_SUCCESS(mAvClocks[i],mControllers[i],1.0);
        // Set playback rate to 0.0, 1.0, 2.0 and verify
        std::vector<double> playbackRates = {0.0, 1.0, 2.0};

        for (double rate : playbackRates)
        {
            AVCLOCK_SET_PLAYBACK_RATE_SUCCESS(mAvClocks[i],mControllers[i],rate,true);
            AVCLOCK_GET_PLAYBACK_RATE_SUCCESS(mAvClocks[i],mControllers[i],rate);
        }

        // Start the AV Clock and verify state transition
        AVCLOCK_START_SUCCESS(mAvClocks[i],mControllers[i],true);

        for (double rate : playbackRates)
        {
            AVCLOCK_SET_PLAYBACK_RATE_SUCCESS(mAvClocks[i],mControllers[i],rate,true);
            AVCLOCK_GET_PLAYBACK_RATE_SUCCESS(mAvClocks[i],mControllers[i],rate);
        }

        // Stop the AV Clock and verify state transition
        AVCLOCK_STOP_SUCCESS(mAvClocks[i],mControllers[i],true);
    }
}



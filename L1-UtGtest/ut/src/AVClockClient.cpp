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

/**
* @file AVClockClient.cpp
* @page AVClock hal Level 1 Tests
*
* ## Module's Role
* This module includes Level 1 functional tests (success and failure scenarios).
* This is to ensure that the AVClock hal APIs meet the requirements across all vendors.
*
*/

#include <ut_kvp_profile.h>
#include <ut.h>
#include <ut_log.h>
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
#include "yaml_utils.h"

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

//declaring global variables
sp<IAVClockManager> service;
sp<IAVClock> _avClock;
IAVClock::Id g_avclockId;

::std::vector<int>_avclock_ids_from_profile;
::std::vector<int>_audiosink_ids_from_profile;
::std::vector<int>_videosink_ids_from_profile;

class AVClockControllerListener : public BnAVClockControllerListener {
public:
    AVClockControllerListener() {};
    virtual ~AVClockControllerListener() = default;

    ::android::binder::Status onStateChanged(::com::rdk::hal::State oldState, ::com::rdk::hal::State newState) override{
        return ::android::binder::Status::ok();
    }
};

class AVClockEventListener : public BnAVClockEventListener {
public:
    AVClockEventListener() {};
    virtual ~AVClockEventListener() = default;

    ::android::binder::Status onStateChanged(::com::rdk::hal::State oldState, ::com::rdk::hal::State newState) override{
        return ::android::binder::Status::ok();
    }
};

class L1AVClockOpenTest : public UTCore
{
protected:
    void SetUp() override
    {
        //prerequisite - checking state
        ::com::rdk::hal::State state;
        ::android::binder::Status status = _avClock->getState(&state);
        EXPECT_TRUE(status.isOk());
        ASSERT_EQ(state, ::com::rdk::hal::State::CLOSED);

        sp<IAVClockControllerListener> listener = new AVClockControllerListener();
        status = _avClock->open(listener, &avClockController);
        ASSERT_TRUE(status.isOk());
        ASSERT_NE(avClockController, nullptr) << "avClockController is null after open()\n";

        status = _avClock->getState(&state);
        EXPECT_TRUE(status.isOk());
        ASSERT_EQ(state, ::com::rdk::hal::State::READY);
    }
    void TearDown() override
    {
        ::com::rdk::hal::State state;
        Status status = _avClock->getState(&state);
        ASSERT_TRUE(status.isOk());
        ASSERT_EQ(state, ::com::rdk::hal::State::READY);
        bool _aidl_return;
        status = _avClock->close(avClockController, &_aidl_return);
        ASSERT_TRUE(status.isOk());
    }
    android::sp<IAVClockController> avClockController;
};

class L1AVClockCloseTest : public UTCore
{
protected:
    void SetUp() override
    {
        //prerequisite - checking state
        ::com::rdk::hal::State state;
        ::android::binder::Status status = _avClock->getState(&state);
        ASSERT_TRUE(status.isOk());
        ASSERT_EQ(state, ::com::rdk::hal::State::CLOSED);

        sp<IAVClockControllerListener> listener = new AVClockControllerListener();
        status = _avClock->open(listener, &avClockController);
        ASSERT_TRUE(status.isOk());
        ASSERT_NE(avClockController, nullptr) << "avClockController is null after open()\n";

        status = _avClock->getState(&state);
        EXPECT_TRUE(status.isOk());
        ASSERT_EQ(state, ::com::rdk::hal::State::READY);
    }
    android::sp<IAVClockController> avClockController;
};

class L1AVClockEventRegisterTest : public UTCore
{
protected:
    void SetUp() override
    {
        avClockEventListener = new AVClockEventListener();
        bool _aidl_return;
        ::android::binder::Status status = _avClock->registerEventListener(avClockEventListener, &_aidl_return);
        ASSERT_TRUE(status.isOk());
        ASSERT_TRUE(_aidl_return);
    }
    android::sp<::com::rdk::hal::avclock::IAVClockEventListener> avClockEventListener;
};

class L1AVClockStartTest : public L1AVClockOpenTest
{
protected:
    void SetUp() override
    {
        L1AVClockOpenTest::SetUp();
        Status status = avClockController->start();
        ASSERT_TRUE(status.isOk());

        ::com::rdk::hal::State state;
        status = _avClock->getState(&state);
        ASSERT_TRUE(status.isOk());
        ASSERT_EQ(state, ::com::rdk::hal::State::STARTED);
    }
    void TearDown() override
    {
        ::com::rdk::hal::State state;
        Status status = _avClock->getState(&state);
        ASSERT_TRUE(status.isOk());
        ASSERT_EQ(state, ::com::rdk::hal::State::STARTED);

        status = avClockController->stop();
        ASSERT_TRUE(status.isOk());
        L1AVClockOpenTest::TearDown();
    }
};

class L1AVClockStopTest : public L1AVClockOpenTest
{
protected:
    void SetUp() override
    {
        L1AVClockOpenTest::SetUp();
        Status status = avClockController->start();
        ASSERT_TRUE(status.isOk());

        ::com::rdk::hal::State state;
        status = _avClock->getState(&state);
        ASSERT_TRUE(status.isOk());
        ASSERT_EQ(state, ::com::rdk::hal::State::STARTED);
    }
    void TearDown() override
    {
        L1AVClockOpenTest::TearDown();
    }
};

class L1AVClockPlayBackRateStartTest : public L1AVClockOpenTest
{
protected:
    void SetUp() override
    {
        L1AVClockOpenTest::SetUp();
        //prerequisite
        bool _aidl_return;

        ::android::binder::Status status = avClockController->getClockMode(&_clockMode);
        EXPECT_TRUE(status.isOk());

        if(_clockMode == ClockMode::PCR)
        {
            status = avClockController->setClockMode(ClockMode::AUTO, &_aidl_return);
            ASSERT_TRUE(status.isOk());
            ASSERT_TRUE(_aidl_return);
        }

        status = avClockController->start();
        ASSERT_TRUE(status.isOk());

        ::com::rdk::hal::State state;
        status = _avClock->getState(&state);
        ASSERT_TRUE(status.isOk());
        ASSERT_EQ(state, ::com::rdk::hal::State::STARTED);
    }
    void TearDown() override
    {
        bool _aidl_return;
        Status status = avClockController->stop();

        ::com::rdk::hal::State state;
        status = _avClock->getState(&state);
        ASSERT_TRUE(status.isOk());
        ASSERT_EQ(state, ::com::rdk::hal::State::READY);

        if(_clockMode == ClockMode::PCR)
        {
            status = avClockController->setClockMode(_clockMode, &_aidl_return);
            ASSERT_TRUE(status.isOk());
        }
        L1AVClockOpenTest::TearDown();
    }
    ::com::rdk::hal::avclock::ClockMode _clockMode;
};

int getService()
{
    service = android::interface_cast<IAVClockManager>(
            defaultServiceManager()->getService(String16(IAVClockManager::serviceName().c_str())));
    if (service == nullptr)
    {
        printf("Service not available\n");
        return -1;
    }
    return 0;
}


bool helper_validatePropertyValue( const std::optional<::com::rdk::hal::PropertyValue>& propertyValue, const ::com::rdk::hal::avclock::Property property)
{
    if (!propertyValue.has_value() || !propertyValue->value.has_value())
    {
        return false;
    }
    std::variant<int32_t, std::string> val;

    const auto& propVal = propertyValue->value.value();
    switch (propVal.getTag())
    {
        case PropertyValue::Value::Tag::intValue:
            val = propVal.get<PropertyValue::Value::Tag::intValue>();
            break;
        case PropertyValue::Value::Tag::byteValue:
            val = propVal.get<PropertyValue::Value::Tag::byteValue>();
            break;
        case PropertyValue::Value::Tag::stringValue:
        {
            android::String16 string16Val = propVal.get<PropertyValue::Value::Tag::stringValue>();
            android::String8 string8Val(string16Val);  // Convert String16 to String8
            val = std::string(string8Val.string());    // Convert String8 to std::string
        }
            break;
        case PropertyValue::Value::Tag::booleanValue:
            val = propVal.get<PropertyValue::Value::Tag::booleanValue>() ? 1 : 0;  // Store as int32_t (1 for true, 0 for false)
            break;

        default:
            std::cerr << "Unsupported type in PropertyValue::Value" << std::endl;
            break;
    }
    
    switch (property)
    {
        case ::com::rdk::hal::avclock::Property::RESOURCE_ID:
            return std::holds_alternative<int32_t>(val) && std::get<int32_t>(val) == g_avclockId.value;

        case ::com::rdk::hal::avclock::Property::METRIC_AV_SYNC_ACCURACY_MS:
            if (static_cast<int32_t>(::com::rdk::hal::avclock::Property::METRIC_AV_SYNC_ACCURACY_MS) != -1)
            {
                return std::holds_alternative<int32_t>(val);
            }
            else
            {
                printf("metric is not yet implemented by the vendor");
                return false;
            }

        case ::com::rdk::hal::avclock::Property::METRIC_AV_RESYNC_COUNT:
            if (static_cast<int32_t>(::com::rdk::hal::avclock::Property::METRIC_AV_RESYNC_COUNT) != -1)
            {
                return std::holds_alternative<int32_t>(val);
            }
            else
            {
                printf("metric is not yet implemented by the vendor");
                return false;
            }
            /*
            * TODO: below parameters dont have specific value or range of values in the AIDL file Property.aidl
            * METRIC_AV_SYNC_ACCURACY_MS
            * METRIC_AV_RESYNC_COUNT
            */
        default:
            return true;
    }
}

bool helper_areElementsUnique(const std::vector<::com::rdk::hal::avclock::IAVClock::Id>& avclockIds) {
    std::unordered_set<int> uniqueElements;
    
    for (const auto& _avclockId : avclockIds) 
    {
        if(_avclockId.value < 0)
        {
            std::cout <<"find Invalid avclock ID : " <<_avclockId.value <<std::endl;
            return false;
        } 
        if (uniqueElements.find(_avclockId.value) != uniqueElements.end()) 
        {
            std::cout<<"avclock ID is not unique : " <<_avclockId.value <<std::endl;
            return false;
        }
        uniqueElements.insert(_avclockId.value);
    }
    return true;
}

/**
* @brief Test for validating the retrieval of AV Clock IDs using getAVClockIds API
*
* This test verifies that the getAVClockIds API retrieves the AV Clock IDs correctly and ensures that the IDs are unique and valid as per the HAL FEATURE PROFILE.
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 001@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data |Expected Result |Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01 | Initialize avclockids vector and Status object status. Call the getAVClockIds API. | avclockids (uninitialized), status (uninitialized) | avclockids should contain ids, status.isOk() should be true | Should Pass |
* | 02 | Verify that avclockids vector is not empty. | avclockids (value returned from API call) | avclockids should not be empty | Should Pass |
* | 03 | Validate that avclockids vector contains unique elements by calling helper function helper_areElementsUnique. | avclockids (value returned from API call) | helper_areElementsUnique should return true | Should Pass |
* | 04 | Loop through each ID in avclockids and ensure it exists in _avclock_ids_from_profile. | avclockids (value returned from API call), _avclock_ids_from_profile (predefined profile) | Each _Id from avclockids should be found in _avclock_ids_from_profile | Should Pass |
*/
TEST(L1AVClockManagerTest, test_l1_avclock_positive_1_getAVClockIds)
{
    std::vector<::com::rdk::hal::avclock::IAVClock::Id> avclockids;
    ::android::binder::Status status = service->getAVClockIds(&avclockids);
    EXPECT_TRUE(status.isOk());
    EXPECT_FALSE(avclockids.empty());

    ASSERT_TRUE(helper_areElementsUnique(avclockids));
    /* checking whether retrieved avclock ids from the api getAVClockIds() is listed in the HAL FEATURE PROFILE */
    for(const auto& _Id: avclockids)
    {   
        auto result = std::find(_avclock_ids_from_profile.begin(), _avclock_ids_from_profile.end(), _Id.value);
        EXPECT_NE(result, _avclock_ids_from_profile.end());
    }
}

/**
* @brief This test verifies the getAVClock function with a positive scenario.
*
* This test checks if the getAVClock function correctly returns an AVClock instance for each AVClock ID from the profile. It iterates through the list of AVClock IDs, retrieves the corresponding AVClock instance, and asserts that the status is OK and the instance is not null.
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 002@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data |Expected Result |Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01 | Initialize AVClock ID from profile  | AVClock ID from profile list | Status should be OK | Should be successful |
* | 02 | Retrieve AVClock instance using getAVClock API | avClockId.value = iterate over each value from profile | avClock should not be null, status.isOk() should be true | Should Pass |
*/
TEST(L1AVClockManagerTest, test_l1_avclock_positive_1_getAVClock)
{
    android::sp<::com::rdk::hal::avclock::IAVClock> avClock;
    IAVClock::Id avClockId;

    for(const auto& _avclockid: _avclock_ids_from_profile)
    {
        avClockId.value = _avclockid;
        android::binder::Status status = service->getAVClock(avClockId, &avClock);
        EXPECT_TRUE(status.isOk());
        EXPECT_NE(avClock, nullptr);
    }

}

/**
 * @brief Test to validate that multiple invocations of getAVClock return the same AVClock instance.
 *
 * This test checks whether the service can successfully retrieve the AVClock instance multiple times and ensure 
 * that the retrieved instances are the same by comparing their RESOURCE_ID properties.@n
 *
 * **Test Group ID:** Basic: 01@n
 * **Test Case ID:** 003@n
 * **Priority:** High@n
 * @n
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 * @n
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Retrieve the AVClock instance using getAVClock for the first time | avClockId = valid id from profile | status.isOk() should be true, retrieved avClock instance is not null | Should be successful |
 * | 02 | Get PROPERTY from the AVClock instance for the RESOURCE_ID | property = RESOURCE_ID | status.isOk() should be true | Should be successful |
 * | 03 | Retrieve the AVClock instance using getAVClock for the second time | avClockId = valid id from profile | status.isOk() should be true, retrieved avClock instance is not null | Should be successful |
 * | 04 | Get PROPERTY from the second AVClock instance for the RESOURCE_ID | property = RESOURCE_ID | status.isOk() should be true | Should be successful |
 * | 05 | Validate RESOURCE_ID from both invocations to check for instance consistency | _resource_id_1,  _resource_id_2 | _resource_id_1 equals _resource_id_2 | Should Pass |
 */
TEST(L1AVClockManagerTest, test_l1_avclock_positive_2_getAVClock)
{
    android::sp<::com::rdk::hal::avclock::IAVClock> avClock_1;
    android::sp<::com::rdk::hal::avclock::IAVClock> avClock_2;
    IAVClock::Id avClockId;
    ::com::rdk::hal::avclock::Property property = ::com::rdk::hal::avclock::Property::RESOURCE_ID;
    std::optional<::com::rdk::hal::PropertyValue> resource_id_1;
    std::optional<::com::rdk::hal::PropertyValue> resource_id_2;

    for(const auto& _avclockid: _avclock_ids_from_profile)
    {
        avClockId.value = _avclockid;
        android::binder::Status status = service->getAVClock(avClockId, &avClock_1);
        EXPECT_TRUE(status.isOk());
        EXPECT_NE(avClock_1, nullptr);

        status = avClock_1->getProperty(property, &resource_id_1);
        EXPECT_TRUE(status.isOk());

        status = service->getAVClock(avClockId, &avClock_2);
        EXPECT_TRUE(status.isOk());
        EXPECT_NE(avClock_2, nullptr);

        status = avClock_2->getProperty(property, &resource_id_2);
        EXPECT_TRUE(status.isOk());

        int32_t _resource_id_1 = resource_id_1->value->get<::com::rdk::hal::PropertyValue::Value::Tag::intValue>();
        int32_t _resource_id_2 = resource_id_2->value->get<::com::rdk::hal::PropertyValue::Value::Tag::intValue>();

        /* validating whether multiple invocation gives same avclock interface instance by checking RESOURCE_ID */
        EXPECT_EQ(_resource_id_1, _resource_id_2)<<"GOT DIFFERENT INSTANCE OF AVCLOCK DURING MULTIPLE INVOCATION. ID for first invocation : "<<_resource_id_1 <<" ID for second invocation : "<<_resource_id_2 <<std::endl;
    }

}

/**
* @brief Test to verify successful retrieval of AVClock instance
*
* This test case aims to validate that the `getAVClock` function successfully retrieves an instance of AVClock when provided with a valid, though undefined, AVClock ID. This ensures that the function can handle and return expected values for normal operational scenarios.
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 004@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description                | Test Data                                    | Expected Result                         | Notes     |
* | :--------------: | -------------------------- | -------------------------------------------- | ---------------------------------------- | --------- |
* | 01               | Initialize avClockId       | avClockId.value = UNDEFINED                  | Should be successful                     |           |
* | 02               | Call getAVClock API        | avClockId = UNDEFINED, avClock initialized    | status.isOk() returns true               | Should Pass|
* | 03               | Verify avClock is not NULL | avClock instance                     | avClock != nullptr                       | Should Pass|
*/
TEST(L1AVClockManagerTest, test_l1_avclock_positive_3_getAVClock)
{
    ::com::rdk::hal::avclock::IAVClock::Id avClockId;
    avClockId.value = ::com::rdk::hal::avclock::IAVClock::Id::UNDEFINED;
    android::sp<::com::rdk::hal::avclock::IAVClock> avClock;
    android::binder::Status status = service->getAVClock(avClockId, &avClock);
    EXPECT_TRUE(status.isOk());
    EXPECT_NE(avClock, nullptr);
}

/**
* @brief Test the negative scenario of getting the AV Clock with an invalid ID
*
* This test verifies the behavior of the `getAVClock` function when called with an invalid AV Clock ID. It ensures that the function correctly handles invalid input and does not return a valid AV Clock instance.
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 005@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data | Expected Result | Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01 | Set AV Clock ID to an invalid value | avClockId.value = -2 | AV Clock ID set to -2 | Should be successful |
* | 02 | Invoke `getAVClock` with the invalid ID and check the status | avClockId.value = -2 | status.isOk() returns false | Should Fail |
* | 03 | Validate that the returned AV Clock instance is null | EXPECT_EQ(avClock, nullptr) | avClock is nullptr | Should Fail |
*/
TEST(L1AVClockManagerTest, test_l1_avclock_negative_1_getAVClock)
{
    ::com::rdk::hal::avclock::IAVClock::Id avClockId;
    avClockId.value = -2;
    android::sp<::com::rdk::hal::avclock::IAVClock> avClock;
    android::binder::Status status = service->getAVClock(avClockId, &avClock);
    EXPECT_FALSE(status.isOk());
    EXPECT_EQ(avClock, nullptr);
}

/**
* @brief Test the positive scenario of getting AV clock capabilities
*
* This test validates the functionality of retrieving the AV clock capabilities in a positive scenario. It checks whether the capabilities are correctly returned without any issues.
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 006@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data | Expected Result | Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01 | Call the getCapabilities function | capabilities object | status.isOk() should be true| Should Pass |
*/
TEST(L1AVClockTest, test_l1_avclock_positive_1_getCapabilities)
{
    ::com::rdk::hal::avclock::Capabilities capabilities;
    Status status = _avClock->getCapabilities(&capabilities);
    EXPECT_TRUE(status.isOk());
}

/**
* @brief Test to validate the proper functionality of the getState method in AVClock.
*
* This test checks the `getState` method of the AVClock to ensure it returns a valid state.
* The state returned by `getState` should match one of the predefined states enumerated by com::rdk::hal::State.
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 007@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data |Expected Result |Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01 | Initialize State variable  | ::com::rdk::hal::State state; | State variable initialized successfully | Should be successful |
* | 02 | Invoke getState function | state = valid variable | status.isOk() should be true | Should Pass |
* | 03 | Validate returned state | state | State should be one of the enumerated states | Should Pass |
*/
TEST(L1AVClockTest, test_l1_avclock_positive_1_getState)
{
    ::com::rdk::hal::State state;
    ::android::binder::Status status = _avClock->getState(&state);
    ASSERT_TRUE(status.isOk());
    bool validEnumValue = (
            state == ::com::rdk::hal::State::UNKNOWN ||
            state == ::com::rdk::hal::State::CLOSED ||
            state == ::com::rdk::hal::State::OPENING ||
            state == ::com::rdk::hal::State::READY ||
            state == ::com::rdk::hal::State::STARTING ||
            state == ::com::rdk::hal::State::STARTED ||
            state == ::com::rdk::hal::State::FLUSHING ||
            state == ::com::rdk::hal::State::STOPPING ||
            state == ::com::rdk::hal::State::CLOSING
    );
    ASSERT_TRUE(validEnumValue);
}

/**
* @brief Tests avClock's getProperty method to ensure properties are fetched correctly
*
* This test checks the retrieval of AVClock properties and verifies their values using a helper function.
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 008@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data |Expected Result |Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01| Retrieve properties list| None | None | Should be successful |
* | 02| Call getProperty for RESOURCE_ID property and validate| Property: RESOURCE_ID | Status.isOk() = true, value of propertyvalue should be valid | Should Pass |
* | 03| Call getProperty for METRIC_AV_SYNC_ACCURACY_MS and validate| Property: METRIC_AV_SYNC_ACCURACY_MS | Status.isOk() = true, value of propertyvalue should be valid | Should Pass |
* | 04| Call getProperty for METRIC_AV_RESYNC_COUNT and validate| Property: METRIC_AV_RESYNC_COUNT | Status.isOk() = true, value of propertyvalue should be valid | Should Pass |
*/
TEST(L1AVClockTest, test_l1_avclock_positive_1_getProperty)
{
    std::vector<::com::rdk::hal::avclock::Property> properties = {
            ::com::rdk::hal::avclock::Property::RESOURCE_ID,
            ::com::rdk::hal::avclock::Property::METRIC_AV_SYNC_ACCURACY_MS,
            ::com::rdk::hal::avclock::Property::METRIC_AV_RESYNC_COUNT
    };
    for (const auto& prop : properties)
    {
        std::optional<::com::rdk::hal::PropertyValue> propertyValue;
        ::android::binder::Status status = _avClock->getProperty(prop, &propertyValue);

        ASSERT_TRUE(status.isOk());
        ASSERT_TRUE(propertyValue.has_value());  // Ensure that the property value was set
        EXPECT_TRUE(helper_validatePropertyValue(propertyValue, prop));
    }
}

/**
* @brief Test negative scenario for getProperty function in L1 AVClock
*
* This test checks the getProperty function for a negative scenario. It attempts to get a property with an invalid value to ensure proper handling of erroneous inputs.
*
* **Test Group ID:** Basic: 01 @n
* **Test Case ID:** 009 @n
* **Priority:** High @n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data | Expected Result | Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01 | Convert the integer -1 to Property enum type | property = -1 | property is an invalid enum value | Should be successful |
* | 02 | Invoke getProperty API with invalid property value | property = -1, result = null | status.isOk() returns false, result.has_value() returns false | Should Pass |
*/
TEST(L1AVClockTest, test_l1_avclock_negative_1_getProperty)
{
    ::std::optional<::com::rdk::hal::PropertyValue> result;
    ::com::rdk::hal::avclock::Property property = static_cast<::com::rdk::hal::avclock::Property>(-1);
    ::android::binder::Status status = _avClock->getProperty(property, &result);
    EXPECT_FALSE(status.isOk());
    EXPECT_FALSE(result.has_value());
}

/**
* @brief To test the negative scenario for setting an AVClock property.
*
* This test is designed to validate the error handling of the setProperty method in L1 AVClock functionality. It specifically checks whether the system properly handles an invalid property setting operation and ensures that it does not mistakenly confirm the success of the operation.
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 010@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data |Expected Result |Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01| Set the Property to RESOURCE_ID and initialize PropertyValue with intValue = 1 | property = RESOURCE_ID, propertyValue.value = 1 | Should be successful | Should be successful |
* | 02| Call the setProperty API with the above data and a boolean return value placeholder | property = RESOURCE_ID, propertyValue.value = 1, _aidl_return | status.isOk() should be true, _aidl_return should be false | Should fail |
*/
TEST(L1AVClockTest, test_l1_avclock_negative_1_setProperty)
{
    ::com::rdk::hal::avclock::Property property = ::com::rdk::hal::avclock::Property::RESOURCE_ID;
    PropertyValue propertyValue;
    propertyValue.value = PropertyValue::Value::make<PropertyValue::Value::Tag::intValue>(1);
    bool _aidl_return;
    android::binder::Status status = _avClock->setProperty(property, propertyValue, &_aidl_return);
    EXPECT_TRUE(status.isOk());
    EXPECT_FALSE(_aidl_return);
}

/**
* @brief Test the positive scenario of opening AVClock.
*
* This test validates the positive scenario where the AVClock is opened and verified for successful opening. Additionally, it ensures that the necessary steps are taken to close the AVClock and clean up after the test.@n
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 011@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data |Expected Result |Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01| Call `getState` API to get initial state | &state | state = ::com::rdk::hal::State::CLOSED | Should be successful |
* | 02| Instantiate AVClockControllerListener | new AVClockControllerListener() | listener should not be nullptr | Should be successful |
* | 03| Open AVClock using the listener | listener, &avClockController = valid pointer | returned  value avClockController != nullptr, status.isOk() should be true | Should Pass |
* | 04| Close AVClock instance | avClockController, &_aidl_return | _aidl_return and status.isOk() should be true | Should Pass |
*/
TEST(L1AVClockTest, test_l1_avclock_positive_1_open)
{
    //prerequisite - checking state
    ::com::rdk::hal::State state;
    ::android::binder::Status status = _avClock->getState(&state);
    ASSERT_TRUE(status.isOk());
    ASSERT_EQ(state, ::com::rdk::hal::State::CLOSED);

    android::sp<IAVClockController> avClockController;
    sp<IAVClockControllerListener> listener = new AVClockControllerListener();
    status = _avClock->open(listener, &avClockController);
    ASSERT_TRUE(status.isOk());
    ASSERT_NE(avClockController, nullptr);

    //cleaning up
    bool _aidl_return;
    status = _avClock->close(avClockController, &_aidl_return);
    ASSERT_TRUE(status.isOk());
    ASSERT_TRUE(_aidl_return);
}

/**
* @brief Test case to validate the behavior of the open method when it is called in an invalid state.
*
* This test case aims to verify that calling the open method returns an illegal state
* exception when  already opened. This helps to ensure that the avClock properly handles and
* rejects the invalid state transitions.
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 012@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data |Expected Result |Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01 | Setup for L1AVClockOpenTest: Get initial clock state | state | Check if state is CLOSED, , status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 02 | Open AV Clock | listener = valid listener | avClockController should not be null, , status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 03 | Ensure clock state is READY | state | state should be READY , status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 04 | Attempt to call the open method on an already opened avClock and check the returned status | listener = AVClockControllerListener, aidl_return | status.exceptionCode() = EX_ILLEGAL_STATE state exception code | Should fail |
* | 05 | Teardown for L1AVClockOpenTest: Close AVClock | avClockController, &_aidl_return | State should be CLOSED, status.isOk() should be true | Done by Pre-requisite TearDown function |
*/
TEST_F(L1AVClockOpenTest, test_l1_avclock_negative_1_open)
{
    android::sp<IAVClockController> aidl_return;
    android::sp<IAVClockControllerListener> listener = new AVClockControllerListener();
    Status status = _avClock->open(listener, &aidl_return);
    ASSERT_EQ(status.exceptionCode(), android::binder::Status::EX_ILLEGAL_STATE);
}

/**
* @brief This test validates the successful closing of the AV Clock.
*
* This test ensures that the AV Clock transitions to a closed state correctly, verifying the functionality of the close operation.@n
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 013@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data |Expected Result |Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01 | Verify initial state is CLOSED and then open AV Clock and verify state is READY | None | The initial state should be CLOSED. The state after open should be READY | Done by Pre-requisite SetUp function |
* | 02 | Invoke close method on AV Clock | avClockController = valid controller instance | status.isOk() should be true and _aidl_return should be true | Should Pass |
*/
TEST_F(L1AVClockCloseTest, test_l1_avclock_positive_1_close)
{
    bool _aidl_return;
    ::android::binder::Status status = _avClock->close(avClockController, &_aidl_return);
    ASSERT_TRUE(status.isOk());
    ASSERT_TRUE(_aidl_return);
}

/**
* @brief Test the close function of the AVClock in an invalid state
*
* This test checks the behavior of the AVClock close function when called from the STARTED state, expecting an illegal state exception. It also verifies the proper transition of states and cleanup process.
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 014@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data | Expected Result | Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01 | Setup: Getting the initial state of AVClock and opening it | None | State should be CLOSED, then transition to READY state after open() | Done by Pre-requisite SetUp function |
* | 02 | Changing state to STARTED by invoking start() on avClockController | None | The call to start() should be successful | Should be successful |
* | 03 | Attempting to close() AVClock from STARTED state expects an illegal state exception | avClockController = valid instance, aidl_return | Exception code should be EX_ILLEGAL_STATE, aidl_return should be false | Should Fail |
* | 04 | Cleaning up by stopping avClockController using stop() | None | The call to stop() should be successful | Should be successful |
* | 05 | Closing AVClock after stopping the controller | avClockController, aidl_return | The call to close() should be successful | Should Pass | 
*/
TEST_F(L1AVClockCloseTest, test_l1_avclock_negative_1_close)
{
    bool aidl_return;
    //changing state to STARTED
    Status status = avClockController->start();
    ASSERT_TRUE(status.isOk());
    //calling close() from STARTED state (EX_ILLEGAL_STATE)
    status = _avClock->close(avClockController, &aidl_return);
    ASSERT_EQ(status.exceptionCode(), android::binder::Status::EX_ILLEGAL_STATE);
    EXPECT_FALSE(aidl_return);
    //clean up
    status = avClockController->stop();
    ASSERT_TRUE(status.isOk());
    status = _avClock->close(avClockController, &aidl_return);
    ASSERT_TRUE(status.isOk());
}

/**
* @brief This test verifies the positive scenario of registering an AVClock event listener.
*
* This test checks the registration of an AVClock event listener and asserts the success of the registration operation by ensuring the status is OK and the return value is true. It also cleans up by unregistering the event listener and verifying the success of the unregistration.
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 015@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data |Expected Result |Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01| Create an instance of AVClockEventListener | listener = new AVClockEventListener() | Success | Should be successful |
* | 02| Register the event listener to AVClock | listener = valid listener | Status.isOK(), _aidl_return - true | Should Pass |
* | 03| Unregister the event listener | listener = valid listener | Status.isOK(), _aidl_return - true | Should Pass |
*/
TEST(L1AVClockTest, test_l1_avclock_positive_1_registerEventListener)
{
    android::sp<com::rdk::hal::avclock::IAVClockEventListener> listener = new AVClockEventListener();
    bool _aidl_return;
    android::binder::Status status = _avClock->registerEventListener(listener, &_aidl_return);
    ASSERT_TRUE(status.isOk());
    ASSERT_TRUE(_aidl_return);
    //cleaning up
    status = _avClock->unregisterEventListener(listener, &_aidl_return);
    ASSERT_TRUE(status.isOk());
    ASSERT_TRUE(_aidl_return);
}

/**
* @brief Test to verify the behavior when attempting to register an already registered Event Listener
*
* This test is intended to verify the behavior of the API when attempting to register an event listener that has already been registered. This is a negative test case to ensure the system handles duplicate registrations appropriately.
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 016@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data |Expected Result |Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01 | Setup the environment by creating an instance of AVClockEventListener and registering it | avClockEventListener = valid listener, _aidl_return = true | Event listener should be successfully registered | Done by Pre-requisite SetUp function |
* | 02 | Attempt to register the event listener for the second time to test negative scenario | avClockEventListener = allready registered listener, _aidl_return = false | status.isOk() should be true and _aidl_return should be false | Should Fail |
* | 03 | Cleanup by unregistering the event listener | avClockEventListener = valid , _aidl_return | status.isOk() should be true and _aidl_return should be true | Done by Pre-requisite TearDown function |
*/
TEST_F(L1AVClockEventRegisterTest, test_l1_avclock_negative_1_registerEventListener)
{
    bool _aidl_return;
    // registering listener for the second time
    Status status = _avClock->registerEventListener(avClockEventListener, &_aidl_return);
    EXPECT_TRUE(status.isOk());
    EXPECT_FALSE(_aidl_return);
    //cleaning up
    status = _avClock->unregisterEventListener(avClockEventListener, &_aidl_return);
    ASSERT_TRUE(status.isOk());
    ASSERT_TRUE(_aidl_return);
}

/**
* @brief Tests the unregistration of the AVClock event listener
*
* This test verifies that the AVClock event listener can be unregistered successfully. It ensures 
* that the unregisterEventListener function behaves correctly by checking the status and return value.@n
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 017@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data | Expected Result | Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01| Set up the AVClock event listener by creating a new AVClockEventListener object. | None | Should be successful | Done by Pre-requisite SetUp function | 
* | 02| Register the AVClockEventListener with the AVClock. Check for successful status and return value. | avClockEventListener = valid listener | status.isOk() should be true | Done by Pre-requisite SetUp function | 
* | 03| Call unregisterEventListener with the registered AVClockEventListener. Check for status and return value. | avClockEventListener = <valid_listener> | status.isOk() = true, _aidl_return = true | Should Pass |
*/
TEST_F(L1AVClockEventRegisterTest, test_l1_avclock_positive_1_unregisterEventListener)
{
    bool _aidl_return;
    Status status = _avClock->unregisterEventListener(avClockEventListener, &_aidl_return);
    ASSERT_TRUE(status.isOk());
    ASSERT_TRUE(_aidl_return);
}

/**
* @brief Test the unregisterEventListener function without registering the listener
*
* This test aims to validate the behavior of the unregisterEventListener function when an event listener has not been registered first.
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 018@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data |Expected Result |Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01| Create an instance of AVClockEventListener | listener = new AVClockEventListener() | Should be successful |       |
* | 02| Call unregisterEventListener without registering the listener | listener = AVClockEventListener | Expected status.isOk() to be true, _aidl_return should be false  | Should fail |
*/
TEST(L1AVClockTest, test_l1_avclock_negative_1_unregisterEventListener)
{
    bool _aidl_return;
    android::sp<::com::rdk::hal::avclock::IAVClockEventListener> listener = new AVClockEventListener();
    //calling unregisterEventListener() without registering listener
    Status status = _avClock->unregisterEventListener(listener, &_aidl_return);
    ASSERT_TRUE(status.isOk());
    ASSERT_FALSE(_aidl_return);
}

/**
* @brief This test is designed to verify the behavior of the unregisterEventListener method, specifically in a scenario where the method is called twice in succession.
*
* The test ensures that the unregisterEventListener method functions correctly:
* - First, it verifies that the method can successfully unregister an event listener that had been previously registered.
* - Second, it checks that calling the method again for the same listener will fail, as the listener should no longer be registered.
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 019@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data | Expected Result | Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01 | Setup the test environment by registering an event listener | avClockEventListener = valid listener | Listener registered successfully | Done by Pre-requisite SetUp function |
* | 02 | Invoke unregisterEventListener for the first time | avClockEventListener = valid listener , _aidl_return | status.isOk() = true, _aidl_return = true | Should Pass |
* | 03 | Invoke unregisterEventListener for the second time | avClockEventListener = unregistered listener, _aidl_return | status.isOk() = true, _aidl_return = false | Should Fail |
*/
TEST_F(L1AVClockEventRegisterTest, test_l1_avclock_negative_2_unregisterEventListener)
{
    bool _aidl_return;
    Status status = _avClock->unregisterEventListener(avClockEventListener, &_aidl_return);
    ASSERT_TRUE(status.isOk());
    ASSERT_TRUE(_aidl_return);
    //calling unregisterEventListener() again
    status = _avClock->unregisterEventListener(avClockEventListener, &_aidl_return);
    ASSERT_TRUE(status.isOk());
    ASSERT_FALSE(_aidl_return);
}

/**
* @brief Test to verify the positive scenario for start() api the AVClock.
*
* This test checks the capability of starting the AVClock when it is in the correct state, ensuring that the start() work as intended.@n
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 020@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
*
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data | Expected Result | Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01 | Setup for L1AVClockOpenTest: Get initial clock state | state | Check if state is CLOSED, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 02 | Open AV Clock | listener = valid listener | avClockController should not be null, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 03 | Ensure clock state is READY | state | state should be READY, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 04 | Call start() to start the AVClock. | None |status.isOk() should be true | Should Pass |
* | 05 | Call stop() to stop the AVClock. | None | status.isOk() should be true | Should be successful |
* | 06 | Teardown for L1AVClockOpenTest: Close AVClock | avClockController, &_aidl_return | State should be CLOSED, status.isOk() should be true | Done by Pre-requisite TearDown function |
*/
TEST_F(L1AVClockOpenTest, test_l1_avclock_positive_1_start)
{
    Status status = avClockController->start();
    ASSERT_TRUE(status.isOk());
    //cleaning up
    status = avClockController->stop();
    ASSERT_TRUE(status.isOk());
}

/**
* @brief This test case aims to test the negative scenario of starting the AV clock without proper prerequisites.
*
* The primary objective of this test is to ensure that the AV clock controller does not transition to the "started" state if the state machine is not in the correct state beforehand.
*
* **Test Group ID:** Basic: 01 @n
* **Test Case ID:** 021 @n
* **Priority:** High @n
* @n
* **Pre-Conditions:** None @n
* **Dependencies:** None @n
* **User Interaction:** None @n
* @n
* **Test Procedure:** @n
* | Variation / Step | Description | Test Data | Expected Result | Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01 | Setup for L1AVClockOpenTest: Get initial clock state | state | Check if state is CLOSED, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 02 | Open AV Clock | listener = valid listener | avClockController should not be null, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 03 | Ensure clock state is READY | state | state should be READY, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 04 | Setup for L1AVClockStartTest: Start AV Clock | None | State should be STARTED | Done by Pre-requisite SetUp function |
* | 05 | Verify initial state after start | state | State should be STARTED | Done by Pre-requisite SetUp function |
* | 06 | Invoke start function from the  STARTED state | None | status.exceptionCode() = Status::EX_ILLEGAL_STATE | Should Fail |
* | 07 | TearDown for L1AVClockStartTest: Stop AV Clock | None | State should be READY | Done by Pre-requisite TearDown function |
* | 08 | Teardown for L1AVClockOpenTest: Close AVClock | avClockController, &_aidl_return | State should be CLOSED, status.isOk() should be true | Done by Pre-requisite TearDown function |
*/
TEST_F(L1AVClockStartTest, test_l1_avclock_negative_1_start)
{
    Status status = avClockController->start();
    ASSERT_EQ(status.exceptionCode(), Status::EX_ILLEGAL_STATE);
}

/**
* @brief Test the positive scenario of stopping the AV clock
*
* This test validates the stop functionality of the AV clock controller in a positive scenario. It ensures that the clock is stopped successfully after being started.@n
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 022@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data | Expected Result | Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01 | Setup for L1AVClockOpenTest: Get initial clock state | state | Check if state is CLOSED, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 02 | Open AV Clock | listener = valid listener | avClockController should not be null, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 03 | Ensure clock state is READY | state | state should be READY, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 04 | Setup for L1AVClockStartTest: Start AV Clock | None | State should be STARTED | Done by Pre-requisite SetUp function |
* | 05 | Verify initial state after start | state | State should be STARTED | Done by Pre-requisite SetUp function |
* | 06 | Invoke stop method of AV clock controller | None | status.isOk() should be true | Should Pass |
* | 07 | Teardown for L1AVClockOpenTest: Close AVClock | avClockController, &_aidl_return | State should be CLOSED, status.isOk() should be true | Done by Pre-requisite TearDown function |
*/
TEST_F(L1AVClockStopTest, test_l1_avclock_positive_1_stop)
{
    Status status = avClockController->stop();
    ASSERT_TRUE(status.isOk());
}

/**
* @brief This test is designed to validate the behavior of the stop() function under incorrect conditions.
*
* This test specifically checks if the stop() function returns an error when the avClockController is in a state that does not allow stopping. This is to ensure the method handles illegal states correctly.
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 023@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data | Expected Result | Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01 | Setup for L1AVClockOpenTest: Get initial clock state | state | Check if state is CLOSED, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 02 | Open AV Clock | listener = valid listener | avClockController should not be null, , status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 03 | Ensure clock state is READY | state | state should be READY, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 04 | Call the stop() function on avClockController | None | Status::EX_ILLEGAL_STATE | Should Fail |
* | 05 | Teardown for L1AVClockOpenTest: Close AVClock | avClockController, &_aidl_return | State should be CLOSED, status.isOk() should be true | Done by Pre-requisite TearDown function |
*/
TEST_F(L1AVClockOpenTest, test_l1_avclock_negative_1_stop)
{
    Status status = avClockController->stop();
    ASSERT_EQ(status.exceptionCode(), Status::EX_ILLEGAL_STATE);
}

/**
* @brief Test to verify the functionality of getAudioSink method from the READY state.
*
* Verifies if the getAudioSink method can be invoked successfully from the READY state and 
* checks if the returned audio sink ID is valid by comparing it against predefined IDs.@n
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 024@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data |Expected Result |Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01 | Setup for L1AVClockOpenTest: Get initial clock state | state | Check if state is CLOSED, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 02 | Open AV Clock | listener = valid listener | avClockController should not be null, , status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 03 | Ensure clock state is READY | state | state should be READY, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 04 | Call getAudioSink() from READY state | audioSinkId = valid parameter | status.isOk() == true | Should Pass |
* | 05 | Validating whether the audioSinkId value is UNDEFINED or included in  _audiosink_ids_from_profile | audioSinkId.value | audioSinkIdflag should be true | Should be successful |
* | 06 | Teardown for L1AVClockOpenTest: Close AVClock | avClockController, &_aidl_return | State should be CLOSED, status.isOk() should be true | Done by Pre-requisite TearDown function |
*/
TEST_F(L1AVClockOpenTest, test_l1_avclock_positive_1_getAudioSink)
{
    bool audioSinkIdflag = false;
    ::com::rdk::hal::audiosink::IAudioSink::Id audioSinkId;
    //calling getAudioSink() from READY state
    ::android::binder::Status status = avClockController->getAudioSink(&audioSinkId);
    EXPECT_TRUE(status.isOk());
    if(audioSinkId.value == IAudioSink::Id::UNDEFINED)
    {
        audioSinkIdflag = true;
    }
    auto result = std::find(_audiosink_ids_from_profile.begin(), _audiosink_ids_from_profile.end(), audioSinkId.value);
    if(result != _audiosink_ids_from_profile.end())
    {
        audioSinkIdflag = true;        
    }
    EXPECT_TRUE(audioSinkIdflag);
}

/**
* @brief Test the retrieval of the audio sink when the AVClock is in STARTED state
*
* This test checks the functionality of the getAudioSink() method to ensure it returns an appropriate audio sink ID when the AVClock is in the STARTED state. 
* 
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 025@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description                                                                 | Test Data                  | Expected Result                                               | Notes                                     |
* | :--------------: | --------------------------------------------------------------------------- | -------------------------- | ------------------------------------------------------------- | ----------------------------------------- |
* | 01 | Setup for L1AVClockOpenTest: Get initial clock state | state | Check if state is CLOSED, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 02 | Open AV Clock | listener = valid listener | avClockController should not be null, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 03 | Ensure clock state is READY | state | state should be READY, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 04 | Setup for L1AVClockStartTest: Start AV Clock | None | State should be STARTED | Done by Pre-requisite SetUp function |
* | 05 | Verify initial state after start | state | State should be STARTED | Done by Pre-requisite SetUp function |
* | 06 | Call getAudioSink() from STARTED state| audioSinkId = valid pointer| status.isOk() should be true | Should be successful |
* | 07 | Check if the returned audioSinkId is either UNDEFINED or present in profile | audioSinkId.value | boolean flag set if valid audioSinkId | Should Pass|
* | 08 | TearDown for L1AVClockStartTest: Stop AV Clock | None | State should be READY | Done by Pre-requisite TearDown function |
* | 09 | Teardown for L1AVClockOpenTest: Close AVClock | avClockController, &_aidl_return | State should be CLOSED, status.isOk() should be true | Done by Pre-requisite TearDown function |
*/
TEST_F(L1AVClockStartTest, test_l1_avclock_positive_2_getAudioSink)
{
    bool audioSinkIdflag = false;
    ::com::rdk::hal::audiosink::IAudioSink::Id audioSinkId;
    //calling getAudioSink() from STARTED state
    ::android::binder::Status status = avClockController->getAudioSink(&audioSinkId);
    EXPECT_TRUE(status.isOk());
    if(audioSinkId.value == IAudioSink::Id::UNDEFINED)
    {
        audioSinkIdflag = true;
    }
    auto result = std::find(_audiosink_ids_from_profile.begin(), _audiosink_ids_from_profile.end(), audioSinkId.value);
    if(result != _audiosink_ids_from_profile.end())
    {
        audioSinkIdflag = true;        
    }
    EXPECT_TRUE(audioSinkIdflag);
}

/**
* @brief Test for setting the audio sink in AV Clock
*
* This test validates the functionality of setting various audio sinks through the avClockController@n
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 026@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data | Expected Result | Notes |
* | :-------------: | :----------: | --------- | --------------- | ----- |
* | 01 | Setup for L1AVClockOpenTest: Get initial clock state | state | Check if state is CLOSED, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 02 | Open AV Clock | listener = valid listener | avClockController should not be null, , status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 03 | Ensure clock state is READY | state | state should be READY, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 04 | Get initial audio sink ID | initial_audioSinkId = valid pointer | status.isOk() should be true | Should be successful |
* | 05 | Iterate over _audiosink_ids_from_profile and set audio sink ID using setAudioSink() | audioSinkId from profile | status.isOk() should be true, aidl_return is true | Should Pass |
* | 06 | Revert to initial audio sink ID | initial_audioSinkId | status.isOk() should be true, aidl_return is true | Should Pass |
* | 07 | Teardown for L1AVClockOpenTest: Close AVClock | avClockController, &_aidl_return | State should be CLOSED, status.isOk() should be true | Done by Pre-requisite TearDown function |
*/
TEST_F(L1AVClockOpenTest, test_l1_avclock_positive_1_setAudioSink)
{
    bool aidl_return;
    IAudioSink::Id audioSinkId;
    IAudioSink::Id initial_audioSinkId;

    ::android::binder::Status status = avClockController->getAudioSink(&initial_audioSinkId);
    EXPECT_TRUE(status.isOk());

    for(const auto& id: _audiosink_ids_from_profile)
    {
        audioSinkId.value = id;
        //calling setAudioSink() from state READY
        status = avClockController->setAudioSink(audioSinkId, &aidl_return);
        ASSERT_TRUE(status.isOk());
        ASSERT_TRUE(aidl_return);
    }

    //cleaning up, reverting values
    status = avClockController->setAudioSink(initial_audioSinkId, &aidl_return);
    EXPECT_TRUE(status.isOk());
    EXPECT_TRUE(aidl_return);
}

/**
* @brief Tests the functionality of setAudioSink from the state STARTED to verify correct behavior for valid audio sink IDs.
*
* This test validates that setAudioSink can correctly set different audio sink IDs and then revert 
* to the initial audio sink ID without errors. 
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 027@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data | Expected Result | Notes |
* | :-------------: | :----------: | --------- | --------------- | ----- |
* | 01 | Setup for L1AVClockOpenTest: Get initial clock state | state | Check if state is CLOSED, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 02 | Open AV Clock | listener = valid listener | avClockController should not be null, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 03 | Ensure clock state is READY | state | state should be READY, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 04 | Setup for L1AVClockStartTest: Start AV Clock | None | State should be STARTED | Done by Pre-requisite SetUp function |
* | 05 | Verify initial state after start | state | State should be STARTED | Done by Pre-requisite SetUp function |
* | 06 | Get initial audio sink | &initial_audioSinkId = valid pointer | status.isOk() should be true | Should be successful |
* | 07 | Try each audio sink ID and verify it sets correctly | &audioSinkId from _audiosink_ids_from_profile | status.isOk() should be true, aidl_return is true | Should Pass |
* | 08 | Reset to initial audio sink at the end | initial_audioSinkId | status.isOk() should be true, aidl_return is true | Should Pass |
* | 09 | TearDown for L1AVClockStartTest: Stop AV Clock | None | State should be READY | Done by Pre-requisite TearDown function |
* | 10 | Teardown for L1AVClockOpenTest: Close AVClock | avClockController, &_aidl_return | State should be CLOSED, status.isOk() should be true | Done by Pre-requisite TearDown function |
*/
TEST_F(L1AVClockStartTest, test_l1_avclock_positive_2_setAudioSink)
{
    bool aidl_return;
    IAudioSink::Id audioSinkId;
    IAudioSink::Id initial_audioSinkId;

    ::android::binder::Status status = avClockController->getAudioSink(&initial_audioSinkId);
    EXPECT_TRUE(status.isOk());

    for(const auto& id: _audiosink_ids_from_profile)
    {
        audioSinkId.value = id;
        //calling setAudioSink() from state READY
        status = avClockController->setAudioSink(audioSinkId, &aidl_return);
        ASSERT_TRUE(status.isOk());
        ASSERT_TRUE(aidl_return);
    }

    //cleaning up, reverting values
    status = avClockController->setAudioSink(initial_audioSinkId, &aidl_return);
    EXPECT_TRUE(status.isOk());
    EXPECT_TRUE(aidl_return);
}

/**
* @brief This test case verifies the functionality of the setAudioSink method in the avClockController.
*
* This test ensures that the setAudioSink method can correctly set audio sink IDs, including setting it to UNDEFINED and reverting it back.@n
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 028 @n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data | Expected Result | Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01 | Setup for L1AVClockOpenTest: Get initial clock state | state | Check if state is CLOSED, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 02 | Open AV Clock | listener = valid listener | avClockController should not be null, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 03 | Ensure clock state is READY | state | state should be READY, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 04 | Invoke getAudioSink to get initial audio sink ID | initial_audioSinkId = valid pointer | status.isOk() should be true | Should Pass |
* | 05 | Call setAudioSink with Id::UNDEFINED | audioSinkId.value = IAudioSink::Id::UNDEFINED | aidl_return = true, status.isOk() should be true | Should Pass |
* | 06 | Revert back the audio sink ID to its initial value using setAudioSink | initial_audioSinkId = initial audiosink id before setting | aidl_return = true, status.isOk() should be true | Should Pass |
* | 07 | Teardown for L1AVClockOpenTest: Close AVClock | avClockController, &_aidl_return | State should be CLOSED, status.isOk() should be true | Done by Pre-requisite TearDown function |
*/
TEST_F(L1AVClockOpenTest, test_l1_avclock_positive_3_setAudioSink)
{
    IAudioSink::Id initial_audioSinkId;
    IAudioSink::Id audioSinkId;
    audioSinkId.value = IAudioSink::Id::UNDEFINED;
    bool aidl_return;

    ::android::binder::Status status = avClockController->getAudioSink(&initial_audioSinkId);
    EXPECT_TRUE(status.isOk());

    //calling setAudioSink() from state READY and Id::UNDEFINED
    status = avClockController->setAudioSink(audioSinkId, &aidl_return);
    ASSERT_TRUE(status.isOk());
    ASSERT_TRUE(aidl_return);
    //cleaning up, reverting values
    status = avClockController->setAudioSink(initial_audioSinkId, &aidl_return);
    EXPECT_TRUE(status.isOk());
    EXPECT_TRUE(aidl_return);
}

/**
* @brief Test the functionality of setAudioSink() from state STARTED and Id::UNDEFINED
*
* This test verifies if the setAudioSink() function can successfully set an audio sink ID to UNDEFINED and then revert back to its initial value. This ensures that the function works correctly even with the ID set to an undefined state.
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 029@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data |Expected Result |Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01 | Setup for L1AVClockOpenTest: Get initial clock state | state | Check if state is CLOSED, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 02 | Open AV Clock | listener = valid listener | avClockController should not be null, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 03 | Ensure clock state is READY | state | state should be READY, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 04 | Setup for L1AVClockStartTest: Start AV Clock | None | State should be STARTED | Done by Pre-requisite SetUp function |
* | 05 | Verify initial state after start | state | State should be STARTED | Done by Pre-requisite SetUp function |
* | 06 | Retrieve the initial audio sink ID | initial_audioSinkId = valid pointer | status.isOk() should be true | Should be successful |
* | 07 | Set the audio sink ID to UNDEFINED and verify | audioSinkId = UNDEFINED | status.isOk() should be true, aidl_return = true | Should Pass |
* | 08 | Revert the audio sink ID to its initial value and verify | audioSinkId = initial_audioSinkId | status.isOk() should be true, aidl_return = true | Should Pass |
* | 09 | TearDown for L1AVClockStartTest: Stop AV Clock | None | State should be READY | Done by Pre-requisite TearDown function |
* | 10 | Teardown for L1AVClockOpenTest: Close AVClock | avClockController, &_aidl_return | State should be CLOSED, status.isOk() should be true | Done by Pre-requisite TearDown function |
*/
TEST_F(L1AVClockStartTest, test_l1_avclock_positive_4_setAudioSink)
{
    IAudioSink::Id initial_audioSinkId;
    IAudioSink::Id audioSinkId;
    audioSinkId.value = IAudioSink::Id::UNDEFINED;
    bool aidl_return;

    ::android::binder::Status status = avClockController->getAudioSink(&initial_audioSinkId);
    EXPECT_TRUE(status.isOk());

    //calling setAudioSink() from state READY and Id::UNDEFINED
    status = avClockController->setAudioSink(audioSinkId, &aidl_return);
    ASSERT_TRUE(status.isOk());
    ASSERT_TRUE(aidl_return);
    //cleaning up, reverting values
    status = avClockController->setAudioSink(initial_audioSinkId, &aidl_return);
    EXPECT_TRUE(status.isOk());
    EXPECT_TRUE(aidl_return);
}

/**
* @brief Testing negative scenario for setting Audio Sink with invalid ID.
*
* This test aims to validate the proper handling and response of the `setAudioSink`
* method when provided with an invalid Audio Sink ID in the READY state.
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 030@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data | Expected Result | Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01 | Setup for L1AVClockOpenTest: Get initial clock state | state | Check if state is CLOSED, , status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 02 | Open AV Clock | listener = valid listener | avClockController should not be null, , status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 03 | Ensure clock state is READY | state | state should be READY, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 04 | Perform the `setAudioSink` operation with invalid ID (-2) | audioSinkId = -2, aidl_return | status.isOk() should be true, aidl_return should be false | Should Fail |
* | 05 | Teardown for L1AVClockOpenTest: Close AVClock | avClockController, &_aidl_return | State should be CLOSED, status.isOk() should be true | Done by Pre-requisite TearDown function |
*/
TEST_F(L1AVClockOpenTest, test_l1_avclock_negative_1_setAudioSink)
{
    IAudioSink::Id audioSinkId;
    audioSinkId.value = -2;
    bool aidl_return;
    //calling setAudioSink() from state READY
    ::android::binder::Status status = avClockController->setAudioSink(audioSinkId, &aidl_return);
    EXPECT_TRUE(status.isOk());
    EXPECT_FALSE(aidl_return);
}

/**
* @brief Tests negative scenario for setAudioSink with invalid ID in STARTED state.
*
* This test evaluates the response of the function setAudioSink when an invalid audio sink ID is provided in the STARTED state of the AV clock. It ensures that the function correctly identifies the invalid input and returns false, ensuring robustness in input handling.
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 031@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data |Expected Result |Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01 | Setup for L1AVClockOpenTest: Get initial clock state | state | Check if state is CLOSED, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 02 | Open AV Clock | listener = valid listener | avClockController should not be null, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 03 | Ensure clock state is READY | state | state should be READY, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 04 | Setup for L1AVClockStartTest: Start AV Clock | None | State should be STARTED | Done by Pre-requisite SetUp function |
* | 05 | Verify initial state after start | state | State should be STARTED | Done by Pre-requisite SetUp function |
* | 06 | Invoke setAudioSink() with invalid ID -2 | audioSinkId.value = -2, aidl_return | status.isOk() should be true and aidl_return Should  false | Should Fail |
* | 07 | TearDown for L1AVClockStartTest: Stop AV Clock | None | State should be READY | Done by Pre-requisite TearDown function |
* | 08 | Teardown for L1AVClockOpenTest: Close AVClock | avClockController, &_aidl_return | State should be CLOSED, status.isOk() should be true | Done by Pre-requisite TearDown function |
*/
TEST_F(L1AVClockStartTest, test_l1_avclock_negative_2_setAudioSink)
{
    IAudioSink::Id audioSinkId;
    audioSinkId.value = -2;
    bool aidl_return;
    //calling setAudioSink() from state STARTED
    ::android::binder::Status status = avClockController->setAudioSink(audioSinkId, &aidl_return);
    EXPECT_TRUE(status.isOk());
    EXPECT_FALSE(aidl_return);
}

/**
* @brief Test for setting the supplementary audio sink in AVClock from state READY
*
* This test checks the functionality of setting a supplementary audio sink in an AVClock controller when the controller is in the READY state. 
* It ensures that the function setSupplementaryAudioSink() correctly sets the supplementary audio sink and can revert the value back to the initial state.@n
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 032@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data | Expected Result | Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01 | Setup for L1AVClockOpenTest: Get initial clock state | state | Check if state is CLOSED, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 02 | Open AV Clock | listener = valid listener | avClockController should not be null, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 03 | Ensure clock state is READY | state | state should be READY, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 04 | Retrieve initial supplementary audio sink ID | initial_supplementaryAudioSinkId = valid pointer | Initial supplementary audio sink ID retrieved, status.isOk() should be true | Should be successful |
* | 05 | Set new supplementary audio sink ID | supplementaryAudioSinkId.value = second audio sink id from profile  | status.isOk() and _aidl_return should be true  | Should Pass |
* | 06 | Revert supplementary audio sink ID to original value | initial_supplementaryAudioSinkId = the initial value of supplementaryAudioSinkId | status.isOk() and _aidl_return should be true | Should Pass |
* | 07 | Teardown for L1AVClockOpenTest: Close AVClock | avClockController, &_aidl_return | State should be CLOSED, status.isOk() should be true | Done by Pre-requisite TearDown function |
*/
TEST_F(L1AVClockOpenTest, test_l1_avclock_positive_1_setSupplementaryAudioSink)
{
    IAudioSink::Id initial_supplementaryAudioSinkId;
    IAudioSink::Id supplementaryAudioSinkId;
    ASSERT_GE(_audiosink_ids_from_profile.size(),2) <<"supplementary audiosink not available in the profile";

    Status status = avClockController->getSupplementaryAudioSink(&initial_supplementaryAudioSinkId);
    ASSERT_TRUE(status.isOk());

    supplementaryAudioSinkId.value = _audiosink_ids_from_profile[1];
    bool _aidl_return;
    //calling setSupplementaryAudioSink() from state READY
    status = avClockController->setSupplementaryAudioSink(supplementaryAudioSinkId, &_aidl_return);
    ASSERT_TRUE(status.isOk());
    ASSERT_TRUE(_aidl_return);
    //cleaning up, reverting values
    status = avClockController->setSupplementaryAudioSink(initial_supplementaryAudioSinkId, &_aidl_return);
    EXPECT_TRUE(status.isOk());
    EXPECT_TRUE(_aidl_return);
}

/**
* @brief Testing the setSupplementaryAudioSink functionality in L1AVClockStartTest
*
* The test checks whether the setSupplementaryAudioSink method correctly sets and reverts the supplementary audio sink in the state STARTED.@n
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 033@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data | Expected Result | Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01 | Setup for L1AVClockOpenTest: Get initial clock state | state | Check if state is CLOSED, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 02 | Open AV Clock | listener = valid listener | avClockController should not be null, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 03 | Ensure clock state is READY | state | state should be READY, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 04 | Setup for L1AVClockStartTest: Start AV Clock | None | State should be STARTED | Done by Pre-requisite SetUp function |
* | 05 | Verify initial state after start | state | State should be STARTED | Done by Pre-requisite SetUp function |
* | 06 | Get initial supplementary audio sink | initial_supplementaryAudioSinkId = valid pointer | status.isOk() should be true | Should be successful |
* | 07 | Set supplementary audio sink | supplementaryAudioSinkId = valid id from profile | status.isOk() should be true, _aidl_return == true | Should Pass |
* | 08 | Revert supplementary audio sink to initial value | initial_supplementaryAudioSinkId | status.isOk() should be true, _aidl_return == true | Should Pass |
* | 09 | TearDown for L1AVClockStartTest: Stop AV Clock | None | State should be READY | Done by Pre-requisite TearDown function |
* | 10 | Teardown for L1AVClockOpenTest: Close AVClock | avClockController, &_aidl_return | State should be CLOSED, status.isOk() should be true | Done by Pre-requisite TearDown function |
*/
TEST_F(L1AVClockStartTest, test_l1_avclock_positive_2_setSupplementaryAudioSink)
{
    IAudioSink::Id initial_supplementaryAudioSinkId;
    IAudioSink::Id supplementaryAudioSinkId;
    ASSERT_GE(_audiosink_ids_from_profile.size(),2) <<"supplementary audiosink not available in the profile";

    Status status = avClockController->getSupplementaryAudioSink(&initial_supplementaryAudioSinkId);
    ASSERT_TRUE(status.isOk());

    supplementaryAudioSinkId.value = _audiosink_ids_from_profile[1];
    bool _aidl_return;
    //calling setSupplementaryAudioSink() from state READY
    status = avClockController->setSupplementaryAudioSink(supplementaryAudioSinkId, &_aidl_return);
    ASSERT_TRUE(status.isOk());
    ASSERT_TRUE(_aidl_return);
    //cleaning up, reverting values
    status = avClockController->setSupplementaryAudioSink(initial_supplementaryAudioSinkId, &_aidl_return);
    EXPECT_TRUE(status.isOk());
    EXPECT_TRUE(_aidl_return);
}

/**
* @brief Test for setting supplemental audio sink with negative value.
*
* This test checks the behavior of the setSupplementaryAudioSink function when provided a negative value as the supplementary audio sink ID. This is to ensure that the function can handle erroneous inputs appropriately and return the expected failure result. This helps in verifying the robustness of the API in handling invalid inputs.
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 034@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data |Expected Result |Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01 | Setup for L1AVClockOpenTest: Get initial clock state | state | Check if state is CLOSED, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 02 | Open AV Clock | listener = valid listener | avClockController should not be null, , status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 03 | Ensure clock state is READY | state | state should be READY, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 04 | Set supplementary audio sink ID with negative value | supplementaryAudioSinkId.value = -2 | status.isOk() should be true, _aidl_return  should return false | Should fail |
* | 05 | Teardown for L1AVClockOpenTest: Close AVClock | avClockController, &_aidl_return | State should be CLOSED, status.isOk() should be true | Done by Pre-requisite TearDown function |
*/
TEST_F(L1AVClockOpenTest, test_l1_avclock_negative_1_setSupplementaryAudioSink)
{
    IAudioSink::Id supplementaryAudioSinkId;
    supplementaryAudioSinkId.value = -2;
    bool _aidl_return;
    //calling setSupplementaryAudioSink() from state READY
    ::android::binder::Status status = avClockController->setSupplementaryAudioSink(supplementaryAudioSinkId, &_aidl_return);
    EXPECT_TRUE(status.isOk());
    EXPECT_FALSE(_aidl_return);
}

/**
* @brief Test for setting a supplementary audio sink with a negative ID in the STARTED state
*
* This test checks the behavior of the setSupplementaryAudioSink() method when a negative ID is provided while the AV clock is in the STARTED state. This helps validate if the function properly handles invalid input.
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 035@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data | Expected Result | Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01 | Setup for L1AVClockOpenTest: Get initial clock state | state | Check if state is CLOSED, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 02 | Open AV Clock | listener = valid listener | avClockController should not be null, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 03 | Ensure clock state is READY | state | state should be READY, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 04 | Setup for L1AVClockStartTest: Start AV Clock | None | State should be STARTED | Done by Pre-requisite SetUp function |
* | 05 | Verify initial state after start | state | State should be STARTED | Done by Pre-requisite SetUp function |
* | 06 | Set a negative supplementary audio sink ID and call setSupplementaryAudioSink() method from state STARTED | supplementaryAudioSinkId.value = -2 | status.isOk() should be true and _aidl_return should be false | Should fail |
* | 07 | TearDown for L1AVClockStartTest: Stop AV Clock | None | State should be READY | Done by Pre-requisite TearDown function |
* | 08 | Teardown for L1AVClockOpenTest: Close AVClock | avClockController, &_aidl_return | State should be CLOSED, status.isOk() should be true | Done by Pre-requisite TearDown function |
*/
TEST_F(L1AVClockStartTest, test_l1_avclock_negative_2_setSupplementaryAudioSink)
{
    IAudioSink::Id supplementaryAudioSinkId;
    supplementaryAudioSinkId.value = -2;
    bool _aidl_return;
    //calling setSupplementaryAudioSink() from state STARTED
    ::android::binder::Status status = avClockController->setSupplementaryAudioSink(supplementaryAudioSinkId, &_aidl_return);
    EXPECT_TRUE(status.isOk());
    EXPECT_FALSE(_aidl_return);
}

/**
* @brief Test to get supplementary audio sink from READY state
*
* This test verifies if the `getSupplementaryAudioSink()` function correctly retrieves the supplementary audio sink when the system is in the READY state.
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 036@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data | Expected Result | Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01 | Setup for L1AVClockOpenTest: Get initial clock state | state | Check if state is CLOSED, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 02 | Open AV Clock | listener = valid listener | avClockController should not be null, , status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 03 | Ensure clock state is READY | state | state should be READY, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 04 | Call `getSupplementaryAudioSink()` in READY state | SupplementaryAudioSink = valid reference | status.isOk() should be true  | Should Pass |
* | 05 | check SupplementaryAudioSink is valid  | SupplementaryAudioSink = output from getSupplementaryAudioSink() | SupplementaryAudioSink should be UNDEFINED or listed in the _audiosink_ids_from_profile  | Should Pass |
* | 06 | Teardown for L1AVClockOpenTest: Close AVClock | avClockController, &_aidl_return | State should be CLOSED, status.isOk() should be true | Done by Pre-requisite TearDown function |
*/
TEST_F(L1AVClockOpenTest, test_l1_avclock_positive_1_getSupplementaryAudioSink)
{
    IAudioSink::Id SupplementaryAudioSink;
    bool SupplementaryAudioSink_flag = false;
    //calling getSupplementaryAudioSink() from READY state
    Status status = avClockController->getSupplementaryAudioSink(&SupplementaryAudioSink);
    ASSERT_TRUE(status.isOk());
    
    if(SupplementaryAudioSink.value == IAudioSink::Id::UNDEFINED)
    {
        SupplementaryAudioSink_flag = true;
    }
    auto result = std::find(_audiosink_ids_from_profile.begin(), _audiosink_ids_from_profile.end(), SupplementaryAudioSink.value);
    if(result != _audiosink_ids_from_profile.end())
    {
        SupplementaryAudioSink_flag = true;        
    }
    EXPECT_TRUE(SupplementaryAudioSink_flag);
}

/**
* @brief This test verifies the functionality of the getSupplementaryAudioSink method in the STARTED state.
*
* The test confirms that the getSupplementaryAudioSink method correctly fetches the supplementary audio sink when the AV clock is in the STARTED state.
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 037@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data |Expected Result |Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01 | Setup for L1AVClockOpenTest: Get initial clock state | state | Check if state is CLOSED, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 02 | Open AV Clock | listener = valid listener | avClockController should not be null, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 03 | Ensure clock state is READY | state | state should be READY, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 04 | Setup for L1AVClockStartTest: Start AV Clock | None | State should be STARTED | Done by Pre-requisite SetUp function |
* | 05 | Verify initial state after start | state | State should be STARTED | Done by Pre-requisite SetUp function |
* | 06 | Fetch the supplementary audio sink from STARTED state | SupplementaryAudioSink = valid pointer | status.isOk() should be true | Should Pass |
* | 07 | Determine the value of SupplementaryAudioSink | SupplementaryAudioSink.value | SupplementaryAudioSink.value == IAudioSink::Id::UNDEFINED or found in profile | Should be successful |
* | 08 | TearDown for L1AVClockStartTest: Stop AV Clock | None | State should be READY | Done by Pre-requisite TearDown function |
* | 09 | Teardown for L1AVClockOpenTest: Close AVClock | avClockController, &_aidl_return | State should be CLOSED, status.isOk() should be true | Done by Pre-requisite TearDown function |
*/
TEST_F(L1AVClockStartTest, test_l1_avclock_positive_2_getSupplementaryAudioSink)
{
    IAudioSink::Id SupplementaryAudioSink;
    bool SupplementaryAudioSink_flag = false;
    //calling getSupplementaryAudioSink() from STARTED state
    Status status = avClockController->getSupplementaryAudioSink(&SupplementaryAudioSink);
    ASSERT_TRUE(status.isOk());

    if(SupplementaryAudioSink.value == IAudioSink::Id::UNDEFINED)
    {
        SupplementaryAudioSink_flag = true;
    }
    auto result = std::find(_audiosink_ids_from_profile.begin(), _audiosink_ids_from_profile.end(), SupplementaryAudioSink.value);
    if(result != _audiosink_ids_from_profile.end())
    {
        SupplementaryAudioSink_flag = true;        
    }
    EXPECT_TRUE(SupplementaryAudioSink_flag);
}

/**
* @brief Test to verify setting of VideoSink from READY state in AVClock
*
* This test validates the setting of VideoSink using setVideoSink() API and ensures the transition state is handled correctly.@n
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 038@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data |Expected Result |Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01 | Setup for L1AVClockOpenTest: Get initial clock state | state | Check if state is CLOSED, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 02 | Open AV Clock | listener = valid listener | avClockController should not be null, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 03 | Ensure clock state is READY | state | state should be READY, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 04 | Call getVideoSink API to get the initial VideoSink ID | initial_videoSinkId = valid pointer | status.isOk() should be true, should retrieve the initial VideoSink ID | Should be successful |
* | 05 | Iterate over video sink IDs from profile and set each as current sink ID using setVideoSink API | videoSinkId = ID from profile | status.isOk() should be true, aidl_return should be true | Should Pass |
* | 06 | Reset the VideoSink back to initial state using setVideoSink API | initial_videoSinkId | status.isOk() should be true, aidl_return should be true | Should Pass |
* | 07 | Teardown for L1AVClockOpenTest: Close AVClock | avClockController, &_aidl_return | State should be CLOSED, status.isOk() should be true | Done by Pre-requisite TearDown function |
*/
TEST_F(L1AVClockOpenTest, test_l1_avclock_positive_1_setVideoSink)
{
    IVideoSink::Id videoSinkId;
    bool aidl_return;
    IVideoSink::Id initial_videoSinkId;

    ::android::binder::Status status = avClockController->getVideoSink(&initial_videoSinkId);
    ASSERT_TRUE(status.isOk());

    for(const auto& id: _videosink_ids_from_profile)
    {
        videoSinkId.value = id;
        //calling setVideoSink() from READY state
        status = avClockController->setVideoSink(videoSinkId, &aidl_return);
        ASSERT_TRUE(status.isOk());
        ASSERT_TRUE(aidl_return);
    }

    //cleaning up, reverting values
    status = avClockController->setVideoSink(initial_videoSinkId, &aidl_return);
    EXPECT_TRUE(status.isOk());
    EXPECT_TRUE(aidl_return);
}

/**
* @brief This test verifies the functionality of setting the video sink in AV Clock.
*
* This test first obtains the initial video sink ID, then iterates through a list of video sink IDs to set the video sink using the `setVideoSink()` function from the READY state and finally reverts to the initial video sink ID.
*
* **Test Group ID:** Basic: 01 @n
* **Test Case ID:** 039 @n
* **Priority:** High @n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data | Expected Result | Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01| Setup for L1AVClockOpenTest: Get initial clock state | state | Check if state is CLOSED, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 02| Open AV Clock | listener = valid listener | avClockController should not be null, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 03| Ensure clock state is READY | state | state should be READY, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 04| Setup for L1AVClockStartTest: Start AV Clock | None | State should be STARTED | Done by Pre-requisite SetUp function |
* | 05| Verify initial state after start | state | State should be STARTED | Done by Pre-requisite SetUp function |
* | 06| Get initial video sink ID using `getVideoSink()` | &initial_videoSinkId = valid pointer| status.isOk() should be true | Should be successful |
* | 07| Iterate `_videosink_ids_from_profile` and call `setVideoSink()` with each ID | videoSinkId = id, &aidl_return | status.isOk() should be true, aidl_return should be true | Should Pass |
* | 08| Clean up by setting the video sink back to the initial ID | initial_videoSinkId, &aidl_return | status.isOk() should be true, aidl_return should be true | Should be successful |
* | 09| TearDown for L1AVClockStartTest: Stop AV Clock | None | State should be READY | Done by Pre-requisite TearDown function |
* | 10| Teardown for L1AVClockOpenTest: Close AVClock | avClockController, &_aidl_return | State should be CLOSED, status.isOk() should be true | Done by Pre-requisite TearDown function |
*/
TEST_F(L1AVClockStartTest, test_l1_avclock_positive_2_setVideoSink)
{
    IVideoSink::Id videoSinkId;
    bool aidl_return;
    IVideoSink::Id initial_videoSinkId;

    ::android::binder::Status status = avClockController->getVideoSink(&initial_videoSinkId);
    ASSERT_TRUE(status.isOk());

    for(const auto& id: _videosink_ids_from_profile)
    {
        videoSinkId.value = id;
        //calling setVideoSink() from READY state
        status = avClockController->setVideoSink(videoSinkId, &aidl_return);
        ASSERT_TRUE(status.isOk());
        ASSERT_TRUE(aidl_return);
    }

    //cleaning up, reverting values
    status = avClockController->setVideoSink(initial_videoSinkId, &aidl_return);
    EXPECT_TRUE(status.isOk());
    EXPECT_TRUE(aidl_return);
}

/**
* @brief Test to verify setting the video sink in AVClock
*
* This test verifies that the video sink can be set to a specific value and then reverted back to its initial value in the READY state.
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 040@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data |Expected Result |Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01 | Setup for L1AVClockOpenTest: Get initial clock state | state | Check if state is CLOSED, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 02 | Open AV Clock | listener = valid listener | avClockController should not be null, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 03 | Ensure clock state is READY | state | state should be READY, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 04 | Fetch the initial video sink by calling getVideoSink | initial_videoSinkId = valid pointer | status.isOk() should be true | Should be successful |
* | 05 | Set the video sink to UNDEFINED by calling setVideoSink | videoSinkId.value = IVideoSink::Id::UNDEFINED | status.isOk() should be true, aidl_return = true | Should Pass |
* | 06 | Revert to initial video sink by calling setVideoSink again | initial_videoSinkId = initial value  | status.isOk() should be true, aidl_return = true | Should Pass |
* | 07 | Teardown for L1AVClockOpenTest: Close AVClock | avClockController, &_aidl_return | State should be CLOSED, status.isOk() should be true | Done by Pre-requisite TearDown function |
*/
TEST_F(L1AVClockOpenTest, test_l1_avclock_positive_3_setVideoSink)
{
    IVideoSink::Id videoSinkId;
    videoSinkId.value = IVideoSink::Id::UNDEFINED;
    bool aidl_return;
    IVideoSink::Id initial_videoSinkId;

    ::android::binder::Status status = avClockController->getVideoSink(&initial_videoSinkId);
    ASSERT_TRUE(status.isOk());

    //calling setVideoSink() from READY state and Id::UNDEFINED
    status = avClockController->setVideoSink(videoSinkId, &aidl_return);
    ASSERT_TRUE(status.isOk());
    ASSERT_TRUE(aidl_return);
    //cleaning up, reverting values
    status = avClockController->setVideoSink(initial_videoSinkId, &aidl_return);
    EXPECT_TRUE(status.isOk());
    EXPECT_TRUE(aidl_return);
}

/**
* @brief This test verifies the functionality of setting the video sink using setVideoSink() method while in the STARTED state.
*
* The test will first bring the AV clock to the READY state by running the SetUp() method. Then, it will verify that setVideoSink can transition from READY state using an undefined video sink ID and check if the value reverts successfully.
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 041@n
* **Priority:** High @n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data | Expected Result | Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01 | Setup for L1AVClockOpenTest: Get initial clock state | state | Check if state is CLOSED, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 02 | Open AV Clock | listener = valid listener | avClockController should not be null, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 03 | Ensure clock state is READY | state | state should be READY, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 04 | Setup for L1AVClockStartTest: Start AV Clock | None | State should be STARTED | Done by Pre-requisite SetUp function |
* | 05 | Verify initial state after start | state | State should be STARTED | Done by Pre-requisite SetUp function |
* | 06 | Retrieve initial video sink | &initial_videoSinkId = valid pointer | status.isOk() should be true | Should be successful |
* | 07 | Set video sink to UNDEFINED ID from STARTED state | videoSinkId = UNDEFINED, &aidl_return | status.isOk() and aidl_return = true | Should Pass |
* | 08 | Revert video sink to initial value | initial_videoSinkId, &aidl_return | status.isOk() and aidl_return = true | Should Pass |
* | 09 | TearDown for L1AVClockStartTest: Stop AV Clock | None | State should be READY | Done by Pre-requisite TearDown function |
* | 10 | Teardown for L1AVClockOpenTest: Close AVClock | avClockController, &_aidl_return | State should be CLOSED, status.isOk() should be true | Done by Pre-requisite TearDown function |
*/
TEST_F(L1AVClockStartTest, test_l1_avclock_positive_4_setVideoSink)
{
    IVideoSink::Id videoSinkId;
    videoSinkId.value = IVideoSink::Id::UNDEFINED;
    bool aidl_return;
    IVideoSink::Id initial_videoSinkId;

    ::android::binder::Status status = avClockController->getVideoSink(&initial_videoSinkId);
    ASSERT_TRUE(status.isOk());

    //calling setVideoSink() from READY state and Id::UNDEFINED
    status = avClockController->setVideoSink(videoSinkId, &aidl_return);
    ASSERT_TRUE(status.isOk());
    ASSERT_TRUE(aidl_return);
    //cleaning up, reverting values
    status = avClockController->setVideoSink(initial_videoSinkId, &aidl_return);
    EXPECT_TRUE(status.isOk());
    EXPECT_TRUE(aidl_return);
}

/**
* @brief Tests the setVideoSink function with an invalid video sink ID
*
* This test verifies that the setVideoSink function correctly handles the 
* scenario where it is provided with an invalid video sink ID (-2) in the READY state.
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 042@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data | Expected Result | Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01 | Setup for L1AVClockOpenTest: Get initial clock state | state | Check if state is CLOSED, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 02 | Open AV Clock | listener = valid listener | avClockController should not be null, , status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 03 | Ensure clock state is READY | state | state should be READY, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 02 | Define a video sink ID with an invalid value (-2) | videoSinkId.value = -2 | Should be successful | |
* | 03 | Call setVideoSink function with the invalid video sink ID | videoSinkId.value = -2 | status.isOk() should be true and aidl_return should be FALSE | Should Fail |
* | 08 | Teardown for L1AVClockOpenTest: Close AVClock | avClockController, &_aidl_return | State should be CLOSED, status.isOk() should be true | Done by Pre-requisite TearDown function |
*/
TEST_F(L1AVClockOpenTest, test_l1_avclock_negative_1_setVideoSink)
{
    IVideoSink::Id videoSinkId;
    videoSinkId.value = -2;
    bool aidl_return;
    //calling setVideoSink() from READY state
    ::android::binder::Status status = avClockController->setVideoSink(videoSinkId, &aidl_return);
    EXPECT_TRUE(status.isOk());
    EXPECT_FALSE(aidl_return);
}

/**
* @brief Test setting VideoSink with invalid Id (-2) in STARTED state.
*
* This test verifies that the setVideoSink() method fails when invoked with an invalid VideoSink ID (-2) while the AVClock is in the STARTED state.
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 043@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data |Expected Result |Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01 | Setup for L1AVClockOpenTest: Get initial clock state | state | Check if state is CLOSED, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 02 | Open AV Clock | listener = valid listener | avClockController should not be null, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 03 | Ensure clock state is READY | state | state should be READY, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 04 | Setup for L1AVClockStartTest: Start AV Clock | None | State should be STARTED | Done by Pre-requisite SetUp function |
* | 05 | Verify initial state after start | state | State should be STARTED | Done by Pre-requisite SetUp function |
* | 06 | Call setVideoSink() with invalid VideoSink ID (-2) | videoSinkId = -2, aidl_return | aidl_return should return false, status.isOk() should be true | Should Fail |
* | 07 | TearDown for L1AVClockStartTest: Stop AV Clock | None | State should be READY | Done by Pre-requisite TearDown function |
* | 08 | Teardown for L1AVClockOpenTest: Close AVClock | avClockController, &_aidl_return | State should be CLOSED, status.isOk() should be true | Done by Pre-requisite TearDown function |
*/
TEST_F(L1AVClockStartTest, test_l1_avclock_negative_2_setVideoSink)
{
    IVideoSink::Id videoSinkId;
    videoSinkId.value = -2;
    bool aidl_return;
    //calling setVideoSink() from READY state
    ::android::binder::Status status = avClockController->setVideoSink(videoSinkId, &aidl_return);
    EXPECT_TRUE(status.isOk());
    EXPECT_FALSE(aidl_return);
}

/**
* @brief Test the functionality of getVideoSink method in a positive scenario
*
* This test is designed to verify the correct functioning of the getVideoSink()
* method when the AV Clock Controller is in a READY state. It confirms that the
* method returns a valid video sink ID, which should either be UNDEFINED or one 
* of the IDs present in a predefined list of valid video sink IDs.
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 044@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data | Expected Result | Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01 | Setup for L1AVClockOpenTest: Get initial clock state | state | Check if state is CLOSED, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 02 | Open AV Clock | listener = valid listener | avClockController should not be null, , status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 03 | Ensure clock state is READY | state | state should be READY, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 04 | Call getVideoSink() to get the current video sink ID | videoSinkId = valid reference | status.isOk() should be true | Should Pass |
* | 05 | Verify if the returned video sink ID is either UNDEFINED or a valid video sink ID from the profile | videoSinkId = returned value | videoSinkId should match either UNDEFINED or an element from _videosink_ids_from_profile | Should be successful |
* | 06 | Teardown for L1AVClockOpenTest: Close AVClock | avClockController, &_aidl_return | State should be CLOSED, status.isOk() should be true | Done by Pre-requisite TearDown function |
*/
TEST_F(L1AVClockOpenTest,test_l1_avclock_positive_1_getVideoSink)
{
    bool videoSinkIdflag = false;
    ::com::rdk::hal::videosink::IVideoSink::Id videoSinkId;
    //calling getVideoSink() from READY state
    ::android::binder::Status status = avClockController->getVideoSink(&videoSinkId);
    ASSERT_TRUE(status.isOk());

    if(videoSinkId.value == IVideoSink::Id::UNDEFINED)
    {
        videoSinkIdflag = true;
    }
    auto result = std::find(_videosink_ids_from_profile.begin(), _videosink_ids_from_profile.end(), videoSinkId.value);
    if(result != _videosink_ids_from_profile.end())
    {
        videoSinkIdflag = true;        
    }
    EXPECT_TRUE(videoSinkIdflag);
}

/**
* @brief Test for verifying the retrieval of video sink from AV Clock in STARTED state
*
* This test checks the functionality of getVideoSink() API when the AV Clock is in STARTED state, 
* verifying that the video sink ID returned is either undefined or matches an expected profile ID.
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 045@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data |Expected Result |Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01 | Setup for L1AVClockOpenTest: Get initial clock state | state | Check if state is CLOSED, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 02 | Open AV Clock | listener = valid listener | avClockController should not be null, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 03 | Ensure clock state is READY | state | state should be READY, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 04 | Setup for L1AVClockStartTest: Start AV Clock | None | State should be STARTED | Done by Pre-requisite SetUp function |
* | 05 | Verify initial state after start | state | State should be STARTED | Done by Pre-requisite SetUp function |
* | 06 | Get the video sink id | &videoSinkId = valid pointer |status.isOk() should be true | Should Pass |
* | 07 | Validate the video sink ID against UNDEFINED or a profile ID | videoSinkId.value | video sink ID should be UNDEFINED or a value in the profile  | Should Pass |
* | 08 | TearDown for L1AVClockStartTest: Stop AV Clock | None | State should be READY | Done by Pre-requisite TearDown function |
* | 09 | Teardown for L1AVClockOpenTest: Close AVClock | avClockController, &_aidl_return | State should be CLOSED, status.isOk() should be true | Done by Pre-requisite TearDown function |
*/
TEST_F(L1AVClockStartTest,test_l1_avclock_positive_2_getVideoSink)
{
    bool videoSinkIdflag = false;
    ::com::rdk::hal::videosink::IVideoSink::Id videoSinkId;
    //calling getVideoSink() from READY state
    ::android::binder::Status status = avClockController->getVideoSink(&videoSinkId);
    ASSERT_TRUE(status.isOk());

    if(videoSinkId.value == IVideoSink::Id::UNDEFINED)
    {
        videoSinkIdflag = true;
    }
    auto result = std::find(_videosink_ids_from_profile.begin(), _videosink_ids_from_profile.end(), videoSinkId.value);
    if(result != _videosink_ids_from_profile.end())
    {
        videoSinkIdflag = true;        
    }
    EXPECT_TRUE(videoSinkIdflag);
}

/**
* @brief Test the setClockMode API in a positive scenario
*
* This test validates that the setClockMode API can successfully transition through various clock modes and revert back to the initial clock mode.
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 046@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data| Expected Result | Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01 | Setup for L1AVClockOpenTest: Get initial clock state | state | Check if state is CLOSED, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 02 | Open AV Clock | listener = valid listener | avClockController should not be null, , status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 03 | Ensure clock state is READY | state | state should be READY, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 04 | Retrieve the initial ClockMode | initial_clockMode = value | status.isOk() should be true | Should be successful |
* | 05 | Set ClockMode to AUTO | mode = ClockMode::AUTO, _aidl_return | status.isOk() should be true, _aidl_return is true | Should Pass |
* | 06 | Set ClockMode to PCR | mode = ClockMode::PCR, _aidl_return | status.isOk() should be true, _aidl_return is true | Should Pass |
* | 07 | Set ClockMode to AUDIO_MASTER | mode = ClockMode::AUDIO_MASTER, _aidl_return | status.isOk() should be true, _aidl_return is true | Should Pass |
* | 08 | Set ClockMode to VIDEO_MASTER | mode= ClockMode::VIDEO_MASTER, _aidl_return | status.isOk() should be true, _aidl_return is true | Should Pass |
* | 09 | Revert ClockMode to initial state | mode = initial_clockMode, _aidl_return | status.isOk() should be true, _aidl_return is true | Should be successful |
* | 10 | Teardown for L1AVClockOpenTest: Close AVClock | avClockController, &_aidl_return | State should be CLOSED, status.isOk() should be true | Done by Pre-requisite TearDown function |
*/
TEST_F(L1AVClockOpenTest, test_l1_avclock_positive_1_setClockMode)
{
    bool _aidl_return;
    ::com::rdk::hal::avclock::ClockMode initial_clockMode;
    Status status = avClockController->getClockMode(&initial_clockMode);
    ASSERT_TRUE(status.isOk());

    for (auto mode : {
        ClockMode::AUTO,
        ClockMode::PCR,
        ClockMode::AUDIO_MASTER,
        ClockMode::VIDEO_MASTER
    })
    {
        status = avClockController->setClockMode(mode, &_aidl_return);
        ASSERT_TRUE(_aidl_return);
        ASSERT_TRUE(status.isOk());
    }
    //cleaning up, reverting values
    status = avClockController->setClockMode(initial_clockMode, &_aidl_return);
    ASSERT_TRUE(status.isOk());
    ASSERT_TRUE(_aidl_return);
}

/**
* @brief Test the setting of an invalid clock mode in AVClock
*
* This test verifies that setting an invalid clock mode (i.e., a mode not defined in the enum) is handled correctly by the system. 
* The expectation is that the operation will fail and the clock mode will not be set.
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 047@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data | Expected Result | Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01 | Setup for L1AVClockOpenTest: Get initial clock state | state | Check if state is CLOSED, , status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 02 | Open AV Clock | listener = valid listener | avClockController should not be null, , status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 03 | Ensure clock state is READY | state | state should be READY, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 04 | Set clock mode to invalid value and check return value | mode = static_cast<ClockMode>(5), _aidl_return | _aidl_return is false, status.isOk() should be true | Should fail |
* | 05 | Teardown for L1AVClockOpenTest: Close AVClock | avClockController, &_aidl_return | State should be CLOSED, status.isOk() should be true | Done by Pre-requisite TearDown function |
*/
TEST_F(L1AVClockOpenTest, test_l1_avclock_negative_1_setClockMode)
{
    bool _aidl_return;
    Status status = avClockController->setClockMode(static_cast<ClockMode>(5), &_aidl_return);
    EXPECT_FALSE(_aidl_return);
    EXPECT_TRUE(status.isOk());
}

/**
* @brief Test the negative scenario for setting clock mode when the AV clock is not started.
*
* This test checks the behavior of the setClockMode API when the AV clock is in an illegal state (i.e., when it is not started).
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 048@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data | Expected Result | Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01 | Setup for L1AVClockOpenTest: Get initial clock state | state | Check if state is CLOSED, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 02 | Open AV Clock | listener = valid listener | avClockController should not be null, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 03 | Ensure clock state is READY | state | state should be READY, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 04 | Setup for L1AVClockStartTest: Start AV Clock | None | State should be STARTED | Done by Pre-requisite SetUp function |
* | 05 | Verify initial state after start | state | State should be STARTED | Done by Pre-requisite SetUp function |
* | 06 | Invoke setClockMode to AUTO in illegal state and check exception | input: ClockMode::AUTO, output: _aidl_return | status.exceptionCode() == Status::EX_ILLEGAL_STATE | Should Fail |
* | 07 | TearDown for L1AVClockStartTest: Stop AV Clock | None | State should be READY | Done by Pre-requisite TearDown function |
* | 08 | Teardown for L1AVClockOpenTest: Close AVClock | avClockController, &_aidl_return | State should be CLOSED, status.isOk() should be true | Done by Pre-requisite TearDown function |
*/
TEST_F(L1AVClockStartTest, test_l1_avclock_negative_2_setClockMode)
{
    bool _aidl_return;
    Status status = avClockController->setClockMode(ClockMode::AUTO, &_aidl_return);
    EXPECT_EQ(status.exceptionCode(), Status::EX_ILLEGAL_STATE);
}

/**
* @brief To validate the getClockMode method functionality of L1AVClock in READY state
*
* This test verifies the correct functionality of the getClockMode method in the IAVClockController. 
* It ensures that the method returns a valid ClockMode enum value when the state is READY.
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 049@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data |Expected Result |Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01 | Setup for L1AVClockOpenTest: Get initial clock state | state | Check if state is CLOSED, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 02 | Open AV Clock | listener = valid listener | avClockController should not be null, , status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 03 | Ensure clock state is READY | state | state should be READY, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 04 | Call getClockMode() in READY state to retrieve the current clock mode | None | status.isOk() should be true, valid enum value must be returned | Should Pass |
* | 05 | Verify that the returned ClockMode is either AUTO, PCR, AUDIO_MASTER, or VIDEO_MASTER | clockMode | clockMode == ClockMode::AUTO or clockMode == ClockMode::PCR or clockMode == ClockMode::AUDIO_MASTER or clockMode == ClockMode::VIDEO_MASTER | Should be successful |
* | 06 | Teardown for L1AVClockOpenTest: Close AVClock | avClockController, &_aidl_return | State should be CLOSED, status.isOk() should be true | Done by Pre-requisite TearDown function |
*/
TEST_F(L1AVClockOpenTest, test_l1_avclock_positive_1_getClockMode)
{
    ::com::rdk::hal::avclock::ClockMode clockMode;
    //calling getClockMode() from state READY
    Status status = avClockController->getClockMode(&clockMode);
    ASSERT_TRUE(status.isOk());
    bool validEnumValue = (
            clockMode == ClockMode::AUTO ||
            clockMode == ClockMode::PCR ||
            clockMode == ClockMode::AUDIO_MASTER ||
            clockMode == ClockMode::VIDEO_MASTER);
    ASSERT_TRUE(validEnumValue);
}

/**
* @brief Test to verify the retrieval of ClockMode in STARTED state.
*
* This test verifies the getClockMode function when the avClockController is in STARTED state.
* The function call should return a valid clock mode indicating the operation is successful.
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 050@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data | Expected Result | Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01 | Setup for L1AVClockOpenTest: Get initial clock state | state | Check if state is CLOSED, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 02 | Open AV Clock | listener = valid listener | avClockController should not be null, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 03 | Ensure clock state is READY | state | state should be READY, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 04 | Setup for L1AVClockStartTest: Start AV Clock | None | State should be STARTED | Done by Pre-requisite SetUp function |
* | 05 | Verify initial state after start | state | State should be STARTED | Done by Pre-requisite SetUp function |
* | 06 | Invoke getClockMode to retrieve current clock mode | clockMode = valid pointer | Valid clock mode should be returned and status.isOk() should be true | Should Pass |
* | 07 | TearDown for L1AVClockStartTest: Stop AV Clock | None | State should be READY | Done by Pre-requisite TearDown function |
* | 08 | Teardown for L1AVClockOpenTest: Close AVClock | avClockController, &_aidl_return | State should be CLOSED, status.isOk() should be true | Done by Pre-requisite TearDown function |
*/
TEST_F(L1AVClockStartTest, test_l1_avclock_positive_2_getClockMode)
{
    ::com::rdk::hal::avclock::ClockMode clockMode;
    //calling getClockMode() from state STARTED
    Status status = avClockController->getClockMode(&clockMode);
    ASSERT_TRUE(status.isOk());
    bool validEnumValue = (
            clockMode == ClockMode::AUTO ||
            clockMode == ClockMode::PCR ||
            clockMode == ClockMode::AUDIO_MASTER ||
            clockMode == ClockMode::VIDEO_MASTER);
    ASSERT_TRUE(validEnumValue);
}

/**
* @brief Test the notifyPCRSample function of the avClockController to ensure it operates correctly when provided with valid inputs.
*
* This test verifies that the notifyPCRSample function of the avClockController successfully notifies a PCR sample given valid pcrTimeNs and sampleTimestampNs inputs. It confirms that the function returns true and the status is OK.
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 051@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data |Expected Result |Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01 | Setup for L1AVClockOpenTest: Get initial clock state | state | Check if state is CLOSED, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 02 | Open AV Clock | listener = valid listener | avClockController should not be null, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 03 | Ensure clock state is READY | state | state should be READY, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 04 | Setup for L1AVClockStartTest: Start AV Clock | None | State should be STARTED | Done by Pre-requisite SetUp function |
* | 05 | Verify initial state after start | state | State should be STARTED | Done by Pre-requisite SetUp function |
* | 06 | Notify a PCR sample to avClockController | pcrTimeNs = 10000000, sampleTimestampNs = 12000000 | _aidl_return should be true and status.isOk() should be true | Should Pass |
* | 07 | TearDown for L1AVClockStartTest: Stop AV Clock | None | State should be READY | Done by Pre-requisite TearDown function |
* | 08 | Teardown for L1AVClockOpenTest: Close AVClock | avClockController, &_aidl_return | State should be CLOSED, status.isOk() should be true | Done by Pre-requisite TearDown function |
*/
TEST_F(L1AVClockStartTest, test_l1_avclock_positive_1_notifyPCRSample)
{
    /* Hardcoding of pcrTimeNs, sampleTimestampNs will be replaced with profile driven values soon*/
    int64_t pcrTimeNs = 10000000;
    int64_t sampleTimestampNs = 12000000;
    bool _aidl_return;
    Status status = avClockController->notifyPCRSample(pcrTimeNs, sampleTimestampNs, &_aidl_return);
    EXPECT_TRUE(status.isOk());
    EXPECT_TRUE(_aidl_return);
}

/**
* @brief Test the notifyPCRSample() function in a scenario where it should fail due to improper state.
*
* This test verifies if the notifyPCRSample() function correctly handles the case when called in an illegal state, 
* specifically when the AVClock is in the READY state but notifyPCRSample() is expected to be called in another state.
* The function is expected to return an ILLEGAL_STATE exception@n
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 052@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data | Expected Result | Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01 | Setup for L1AVClockOpenTest: Get initial clock state | state | Check if state is CLOSED, , status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 02 | Open AV Clock | listener = valid listener | avClockController should not be null, , status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 03 | Ensure clock state is READY | state | state should be READY, , status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 04 | Call notifyPCRSample() from READY state | pcrTimeNs = 10000000, sampleTimestampNs = 12000000 | Status should return ILLEGAL_STATE exception | Should Fail |
* | 05 | Teardown for L1AVClockOpenTest: Close AVClock | avClockController, &_aidl_return | State should be CLOSED, status.isOk() should be true | Done by Pre-requisite TearDown function |
*/
TEST_F(L1AVClockOpenTest, test_l1_avclock_negative_1_notifyPCRSample)
{
    /* Hardcoding of pcrTimeNs, sampleTimestampNs will be replaced with profile driven values soon*/
    int64_t pcrTimeNs = 10000000;
    int64_t sampleTimestampNs = 12000000;
    bool _aidl_return;
    //calling notifyPCRSample() from state READY
    Status status = avClockController->notifyPCRSample(pcrTimeNs, sampleTimestampNs, &_aidl_return);
    EXPECT_EQ(status.exceptionCode(), Status::EX_ILLEGAL_STATE);
}

/**
* @brief Test the AVClock notifyPCRSample() with an invalid PCR time
*
* This test validates the behavior of the notifyPCRSample function in the AV Clock controller when provided with an invalid PCR time (negative). It is designed to ensure that the function correctly handles erroneous inputs without causing unexpected failures.
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 053@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data |Expected Result |Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01 | Setup for L1AVClockOpenTest: Get initial clock state | state | Check if state is CLOSED, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 02 | Open AV Clock | listener = valid listener | avClockController should not be null, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 03 | Ensure clock state is READY | state | state should be READY, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 04 | Setup for L1AVClockStartTest: Start AV Clock | None | State should be STARTED | Done by Pre-requisite SetUp function |
* | 05 | Verify initial state after start | state | State should be STARTED | Done by Pre-requisite SetUp function |
* | 06 | Call notifyPCRSample with invalid pcrTimeNs | pcrTimeNs = -1, sampleTimestampNs = 12000000, _aidl_return | status.isOk() should be true and _aidl_return should be  false | Should fail |
* | 07 | TearDown for L1AVClockStartTest: Stop AV Clock | None | State should be READY | Done by Pre-requisite TearDown function |
* | 08 | Teardown for L1AVClockOpenTest: Close AVClock | avClockController, &_aidl_return | State should be CLOSED, status.isOk() should be true | Done by Pre-requisite TearDown function |
*/
TEST_F(L1AVClockStartTest, test_l1_avclock_negative_2_notifyPCRSample)
{
    /* Hardcoding of pcrTimeNs, sampleTimestampNs will be replaced with profile driven values soon*/
    int64_t pcrTimeNs = -1; //invalid value
    int64_t sampleTimestampNs = 12000000;
    bool _aidl_return;
    Status status = avClockController->notifyPCRSample(pcrTimeNs, sampleTimestampNs, &_aidl_return);
    EXPECT_TRUE(status.isOk());
    EXPECT_FALSE(_aidl_return);
}

/**
* @brief Negative test for notifyPCRSample with invalid sampleTimestampNs
*
* This test validates the behavior of the notifyPCRSample method with an invalid sampleTimestampNs value.
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 054@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data | Expected Result | Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01 | Setup for L1AVClockOpenTest: Get initial clock state | state | Check if state is CLOSED, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 02 | Open AV Clock | listener = valid listener | avClockController should not be null, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 03 | Ensure clock state is READY | state | state should be READY, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 04 | Setup for L1AVClockStartTest: Start AV Clock | None | State should be STARTED | Done by Pre-requisite SetUp function |
* | 05 | Verify initial state after start | state | State should be STARTED | Done by Pre-requisite SetUp function |
* | 06 | Call notifyPCRSample | pcrTimeNs = 1000000, sampleTimestampNs = -1 | status.isOk() should be true, _aidl_return should be FALSE | Should Fail |
* | 07 | TearDown for L1AVClockStartTest: Stop AV Clock | None | State should be READY | Done by Pre-requisite TearDown function |
* | 08 | Teardown for L1AVClockOpenTest: Close AVClock | avClockController, &_aidl_return | State should be CLOSED, status.isOk() should be true | Done by Pre-requisite TearDown function |
*/
TEST_F(L1AVClockStartTest, test_l1_avclock_negative_3_notifyPCRSample)
{
    /* Hardcoding of pcrTimeNs, sampleTimestampNs will be replaced with profile driven values soon*/
    int64_t pcrTimeNs = 1000000;
    int64_t sampleTimestampNs = -1; //invalid value
    bool _aidl_return;
    Status status = avClockController->notifyPCRSample(pcrTimeNs, sampleTimestampNs, &_aidl_return);
    EXPECT_TRUE(status.isOk());
    EXPECT_FALSE(_aidl_return);
}

/**
* @brief Test to get current clock time in the started state of AV clock
*
* This test verifies if the current clock time can be retrieved successfully when the AV clock is in the started state. It ensures that the API call to get current clock time functions correctly, providing valid time values.
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 055@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data |Expected Result |Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01 | Setup for L1AVClockOpenTest: Get initial clock state | state | Check if state is CLOSED, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 02 | Open AV Clock | listener = valid listener | avClockController should not be null, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 03 | Ensure clock state is READY | state | state should be READY, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 04 | Setup for L1AVClockStartTest: Start AV Clock | None | State should be STARTED | Done by Pre-requisite SetUp function |
* | 05 | Verify initial state after start | state | State should be STARTED | Done by Pre-requisite SetUp function |
* | 06 | Invoke getCurrentClockTime | clockTime = valid pointer| status.isOk() should be true, clockTime should be valid  | Should Pass |
* | 07 | TearDown for L1AVClockStartTest: Stop AV Clock | None | State should be READY | Done by Pre-requisite TearDown function |
* | 08 | Teardown for L1AVClockOpenTest: Close AVClock | avClockController, &_aidl_return | State should be CLOSED, status.isOk() should be true | Done by Pre-requisite TearDown function |
*/
TEST_F(L1AVClockStartTest, test_l1_avclock_positive_1_getCurrentClockTime)
{
    ::com::rdk::hal::avclock::ClockTime clockTime;
    Status status = avClockController->getCurrentClockTime(&clockTime);
    ASSERT_TRUE(status.isOk());
    ASSERT_TRUE(clockTime.clockTimeNs > 0);
    ASSERT_TRUE(clockTime.sampleTimestampNs > 0);
}

/**
* @brief Test function for checking illegal state exception in getCurrentClockTime
*
* This test verifies that calling getCurrentClockTime from an incorrect state (READY) results in an EX_ILLEGAL_STATE exception.
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 056@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data | Expected Result | Notes |
* | :---------------:| ----------- | --------- | --------------- | ----- |
* | 01 | Check initial state | state | State should be CLOSED | Done by Pre-requisite SetUp function |
* | 02 | Open avClockController and set to READY state | listener = valid listener | State should be READY | Done by Pre-requisite SetUp function |
* | 03 | Call getCurrentClockTime from READY state | ClockTime = valid parameter | EX_ILLEGAL_STATE exception code | Should Fail |
* | 04 | Close avClockController and reset state to CLOSED | None | None | Done by Pre-requisite TearDown function |
*/
TEST_F(L1AVClockOpenTest, test_l1_avclock_negative_1_getCurrentClockTime)
{
    ::com::rdk::hal::avclock::ClockTime clockTime;
    //calling getCurrentClockTime() from state READY (EX_ILLEGAL_STATE)
    Status status = avClockController->getCurrentClockTime(&clockTime);
    EXPECT_EQ(status.exceptionCode(), Status::EX_ILLEGAL_STATE);
}

/**
* @brief This test verifies the setPlaybackRate function from the avClockController interface when the clock is in the READY state.
*
* This test will first check the current clock mode. Depending on the mode, it will ensure that the clock is in the appropriate mode for setting the playback rate. It verifies that setPlaybackRate operates correctly by setting the rate and then reverting the values.
*
* **Test Group ID:** Basic: 01 @n
* **Test Case ID:** 057 @n
* **Priority:** High @n
* @n
* **Pre-Conditions:** None @n
* **Dependencies:** None @n
* **User Interaction:** None @n
* @n
* **Test Procedure:**
* | Variation / Step | Description | Test Data | Expected Result | Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01 | Setup for L1AVClockOpenTest: Get initial clock state | state | Check if state is CLOSED, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 02 | Open AV Clock | listener = valid listener | avClockController should not be null, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 03 | Ensure clock state is READY | state | state should be READY, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 04 | Get initial clock mode | _clockMode = valid pointer | status.isOk() should be true | Should be successful |
* | 05 | If clock mode is PCR, change to AUTO | _clockMode | _aidl_return = true, status.isOk() should be true | Should be successful |
* | 06 | Get initial playback rate | initial_playbackRate = valid pointer |status.isOk() should be true | Should be successful |
* | 07 | Set playback rate to 1.0 in READY state | playbackRate = 1.0 | _aidl_return = true, status.isOk() should be true | Should Pass |
* | 08 | Revert playback rate to initial value | initial_playbackRate | _aidl_return = true, status.isOk() should be true | Should be successful |
* | 09 | Revert clock mode to initial value if changed | _clockMode | status.isOk() should be true | Should be successful |
* | 10 | Teardown for L1AVClockOpenTest: Close AVClock | avClockController, &_aidl_return | State should be CLOSED, status.isOk() should be true | Done by Pre-requisite TearDown function |
*/
TEST_F(L1AVClockOpenTest, test_l1_avclock_positive_1_setPlaybackRate)
{
    double playbackRate = 1.0;
    bool _aidl_return;
    ::com::rdk::hal::avclock::ClockMode _clockMode;
    double initial_playbackRate;

    ::android::binder::Status status = avClockController->getClockMode(&_clockMode);
    EXPECT_TRUE(status.isOk());
    if(_clockMode == ClockMode::PCR)
    {
        //prerequisite
        status = avClockController->setClockMode(ClockMode::AUTO, &_aidl_return);
        ASSERT_TRUE(status.isOk());
        ASSERT_TRUE(_aidl_return);

        status = avClockController->getPlaybackRate(&initial_playbackRate);
        ASSERT_TRUE(status.isOk());

        //calling setPlaybackRate() from state READY
        status = avClockController->setPlaybackRate(playbackRate, &_aidl_return);
        EXPECT_TRUE(_aidl_return);
        EXPECT_TRUE(status.isOk());

        //cleaning up, reverting values
        status = avClockController->setPlaybackRate(initial_playbackRate, &_aidl_return);
        ASSERT_TRUE(status.isOk());
        status = avClockController->setClockMode(_clockMode, &_aidl_return);
        ASSERT_TRUE(status.isOk());
    }
    else
    {
        status = avClockController->getPlaybackRate(&initial_playbackRate);
        ASSERT_TRUE(status.isOk());

        //calling setPlaybackRate() from state READY
        status = avClockController->setPlaybackRate(playbackRate, &_aidl_return);
        EXPECT_TRUE(_aidl_return);
        EXPECT_TRUE(status.isOk());

        //cleaning up, reverting values
        status = avClockController->setPlaybackRate(initial_playbackRate, &_aidl_return);
        ASSERT_TRUE(status.isOk());        
    }
}

/**
* @brief Test to verify setting the playback rate
*
* This test verifies if the playback rate can be correctly set from state STARTED and if it maintains the expected state transitions.
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 058@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data | Expected Result | Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01 | Get the initial state of the AV clock and ensure it is CLOSED using Set Up method. | None | AV clock state should be CLOSED | Done by Pre-requisite SetUp function |
* | 02 | Open the AV clock with the listener and retrieve the AV clock controller. | listener = new AVClockControllerListener(), &avClockController | AV clock controller should be non-null | Done by Pre-requisite SetUp function |
* | 03 | Get the state of the AV clock and ensure it is READY after opening. | None | AV clock state should be READY | Done by Pre-requisite SetUp function |
* | 04 | Get the clock mode and set it to AUTO if it is currently PCR mode, then start the AV clock. | ClockMode = current clock mode | Clock mode set to AUTO and state should be STARTED | Should be successful |
* | 05 | Get the state of the AV clock to ensure it is STARTED. | N/A | AV clock state should be STARTED | Done by Pre-requisite SetUp function |
* | 06 | Get the initial playback rate from avClockController | initial_playBackRate = valid pointer | status.isOk() should be true | Should be successful |
* | 07 | Set the new playback rate on started state | playBackRate = 1.0 | _aidl_return = true and status.isOk() = true | Should Pass |
* | 08 | Revert the playback rate to initial value | initial_playBackRate value | status.isOk() = true | Should be successful |
* | 09 | Stop the AV clock. | None | AV clock state should be READY | Done by Pre-requisite TearDown function |
* | 10 | Reset the clock mode to its original value if it was changed. | _clockMode = initial value | status.isOk() should be true | Done by Pre-requisite TearDown function |
* | 11 | Close the AV clock and ensure it is CLOSED after closing. | avClockController = valid instance | AV clock state should be CLOSED, status.isOk() should be true | Done by Pre-requisite TearDown function |
*/
TEST_F(L1AVClockPlayBackRateStartTest, test_l1_avclock_positive_2_setPlaybackRate)
{
    double playBackRate = 1.0;
    double initial_playBackRate;
    bool _aidl_return;

    Status status = avClockController->getPlaybackRate(&initial_playBackRate);
    EXPECT_TRUE(status.isOk());
    //calling setPlaybackRate() from state STARTED
    status = avClockController->setPlaybackRate(playBackRate, &_aidl_return);
    EXPECT_TRUE(_aidl_return);
    EXPECT_TRUE(status.isOk());

    //cleaning up, reverting values
    status = avClockController->setPlaybackRate(initial_playBackRate, &_aidl_return);
    ASSERT_TRUE(status.isOk());
}

/**
 * @brief Test to verify the correct setting of playback rate in AVClock controller.
 *
 * This test verifies that the playback rate can be set correctly through the AVClock controller in different 
 * states and modes. It ensures that the appropriate status and return values are obtained.
 *
 * **Test Group ID:** Basic: 01@n
 * **Test Case ID:** 059@n
 * **Priority:** High@n
 * @n
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 * @n
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
* | 01 | Setup for L1AVClockOpenTest: Get initial clock state | state | Check if state is CLOSED, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 02 | Open AV Clock | listener = valid listener | avClockController should not be null, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 03 | Ensure clock state is READY | state | state should be READY, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 04| Get the current clock mode | _clockMode = valid pointer | status.isOk() should be true | Should pass |
* | 05| If clock mode is PCR, set it to AUTO | ClockMode = AUTO | status.isOk() should be true, _aidl_return=true | Should pass |
* | 06| Get initial playback rate | initial_playbackRate = valid pointer | status.isOk() should be true | Should pass |
* | 07| Set playback rate to 0.5 | playbackRate = 0.5 | status.isOk() should be true, _aidl_return=true | Should pass |
* | 08| Revert playback rate to initial value | initial_playbackRate = initial value | status.isOk() should be true, _aidl_return=true  | Should pass |
* | 09| Revert clock mode to initial value if changed | ClockMode = initial_clockMode | status.isOk() should be true | Should pass |
* | 10| Teardown for L1AVClockOpenTest: Close AVClock | avClockController, &_aidl_return | State should be CLOSED, status.isOk() should be true | Done by Pre-requisite TearDown function |
*/
TEST_F(L1AVClockOpenTest, test_l1_avclock_positive_3_setPlaybackRate)
{
    double playbackRate = 0.5;
    bool _aidl_return;
    ::com::rdk::hal::avclock::ClockMode _clockMode;
    double initial_playbackRate;

    ::android::binder::Status status = avClockController->getClockMode(&_clockMode);
    EXPECT_TRUE(status.isOk());
    if(_clockMode == ClockMode::PCR)
    {
        //prerequisite
        status = avClockController->setClockMode(ClockMode::AUTO, &_aidl_return);
        ASSERT_TRUE(status.isOk());
        ASSERT_TRUE(_aidl_return);

        status = avClockController->getPlaybackRate(&initial_playbackRate);
        ASSERT_TRUE(status.isOk());

        //calling setPlaybackRate() from state READY
        status = avClockController->setPlaybackRate(playbackRate, &_aidl_return);
        EXPECT_TRUE(_aidl_return);
        EXPECT_TRUE(status.isOk());

        //cleaning up, reverting values
        status = avClockController->setPlaybackRate(initial_playbackRate, &_aidl_return);
        ASSERT_TRUE(status.isOk());
        status = avClockController->setClockMode(_clockMode, &_aidl_return);
        ASSERT_TRUE(status.isOk());
    }
    else
    {
        status = avClockController->getPlaybackRate(&initial_playbackRate);
        ASSERT_TRUE(status.isOk());

        //calling setPlaybackRate() from state READY
        status = avClockController->setPlaybackRate(playbackRate, &_aidl_return);
        EXPECT_TRUE(_aidl_return);
        EXPECT_TRUE(status.isOk());

        //cleaning up, reverting values
        status = avClockController->setPlaybackRate(initial_playbackRate, &_aidl_return);
        ASSERT_TRUE(status.isOk());        
    }
}

/**
* @brief Test  the getClockMode api of the AVClock
*
* This test verifies that the playback rate can be successfully set on the AVClock when it is in the READY state. 
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 060@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data |Expected Result |Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01 | Setup for L1AVClockOpenTest: Get initial clock state | state | Check if state is CLOSED, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 02 | Open AV Clock | listener = valid listener | avClockController should not be null, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 03 | Ensure clock state is READY | state | state should be READY, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 04| Retrieve the current clock mode using getClockMode. | _clockMode = valid pointer | status.isOk() = true| Should be successful |
* | 05| If clock mode is PCR, change it to AUTO mode using setClockMode. | ClockMode = AUTO | status.isOk() = true, _aidl_return = true | Should be successful |
* | 06| Retrieve the initial playback rate using getPlaybackRate. | initial_playbackRate = valid pointer | status.isOk() = true | Should be successful |
* | 07| Set the new playback rate using setPlaybackRate. | playbackRate = 2.0 | _aidl_return = true, status.isOk() = true | Should Pass |
* | 08| Revert the playback rate to its initial value using setPlaybackRate. | playbackRate = initial_playbackRate | status.isOk() = true, _aidl_return = true | Should be successful |
* | 09| Revert the clock mode to its initial value if it was changed  | Clock mode = initial clock mode | status.isOk() = true, _aidl_return = true | Should be successful |
* | 10 | Teardown for L1AVClockOpenTest: Close AVClock | avClockController, &_aidl_return | State should be CLOSED, status.isOk() should be true | Done by Pre-requisite TearDown function |
*/
TEST_F(L1AVClockOpenTest, test_l1_avclock_positive_4_setPlaybackRate)
{
    double playbackRate = 2.0;
    bool _aidl_return;
    ::com::rdk::hal::avclock::ClockMode _clockMode;
    double initial_playbackRate;

    ::android::binder::Status status = avClockController->getClockMode(&_clockMode);
    EXPECT_TRUE(status.isOk());
    if(_clockMode == ClockMode::PCR)
    {
        //prerequisite
        status = avClockController->setClockMode(ClockMode::AUTO, &_aidl_return);
        ASSERT_TRUE(status.isOk());
        ASSERT_TRUE(_aidl_return);

        status = avClockController->getPlaybackRate(&initial_playbackRate);
        ASSERT_TRUE(status.isOk());

        //calling setPlaybackRate() from state READY
        status = avClockController->setPlaybackRate(playbackRate, &_aidl_return);
        EXPECT_TRUE(_aidl_return);
        EXPECT_TRUE(status.isOk());

        //cleaning up, reverting values
        status = avClockController->setPlaybackRate(initial_playbackRate, &_aidl_return);
        ASSERT_TRUE(status.isOk());
        status = avClockController->setClockMode(_clockMode, &_aidl_return);
        ASSERT_TRUE(status.isOk());
    }
    else
    {
        status = avClockController->getPlaybackRate(&initial_playbackRate);
        ASSERT_TRUE(status.isOk());

        //calling setPlaybackRate() from state READY
        status = avClockController->setPlaybackRate(playbackRate, &_aidl_return);
        EXPECT_TRUE(_aidl_return);
        EXPECT_TRUE(status.isOk());

        //cleaning up, reverting values
        status = avClockController->setPlaybackRate(initial_playbackRate, &_aidl_return);
        ASSERT_TRUE(status.isOk());        
    }
}

/**
* @brief Test case for setting playback rate with incorrect clock mode
*
* This test verifies the behavior of the setPlaybackRate API when it is called with ClockMode::PCR mode. It ensures that the API does not set the playback rate successfully when ClockMode::PCR is active.
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 061@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data |Expected Result |Notes |
* |:----:| --------- | ---------- |-------------- | ----- |
* | 01 | Setup for L1AVClockOpenTest: Get initial clock state | state | Check if state is CLOSED , status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 02 | Open AV Clock | listener = valid listener | avClockController should not be null, , status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 03 | Ensure clock state is READY | state | state should be READY, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 04 | Verify the initial clock mode and change to ClockMode::PCR if necessary| _clockMode = [Current Clock Mode] | ClockMode is checked and possibly set to PCR| Should be successful|
* | 05 | Call setPlaybackRate with ClockMode::PCR | playbackRate = 1.0, _aidl_return = [Output Boolean] |_aidl_return = false, status.isOk() should be true| Should Pass |
* | 06 | Restore the original clock mode if it was changed | _clockMode = [Original Clock Mode], _aidl_return = [Output Boolean] |_aidl_return = true, status.isOk() should be true| Should be successful |
* | 07 | Teardown for L1AVClockOpenTest: Close AVClock | avClockController, &_aidl_return | State should be CLOSED, status.isOk() should be true | Done by Pre-requisite TearDown function |
*/
TEST_F(L1AVClockOpenTest, test_l1_avclock_negative_1_setPlaybackRate)
{
    double playbackRate = 1.0;
    bool _aidl_return;
    ::com::rdk::hal::avclock::ClockMode _clockMode;

    ::android::binder::Status status = avClockController->getClockMode(&_clockMode);
    EXPECT_TRUE(status.isOk());
    if(_clockMode != ClockMode::PCR)
    {
        status = avClockController->setClockMode(ClockMode::PCR, &_aidl_return);
        ASSERT_TRUE(status.isOk());
        ASSERT_TRUE(_aidl_return);

        //calling setPlaybackRate() with clockMode ClockMode::PCR
        status = avClockController->setPlaybackRate(playbackRate, &_aidl_return);
        EXPECT_FALSE(_aidl_return);
        EXPECT_TRUE(status.isOk());

        status = avClockController->setClockMode(_clockMode, &_aidl_return);
        ASSERT_TRUE(status.isOk());
    }
    else
    {
        status = avClockController->setPlaybackRate(playbackRate, &_aidl_return);
        EXPECT_FALSE(_aidl_return);
        EXPECT_TRUE(status.isOk());        
    }

}

/**
 * @brief This test verifies the behavior of setPlaybackRate when an invalid rate is provided.
 *
 * This test ensures that the setPlaybackRate API handles invalid playback rates correctly. It specifically 
 * checks if a playback rate of 0.4, which is considered invalid, results in the expected failure.
 *
 * **Test Group ID:** Basic: 01 @n
 * **Test Case ID:** 062 @n
 * **Priority:** High @n
 * @n
 * **Pre-Conditions:** None @n
 * **Dependencies:** None @n
 * **User Interaction:** None @n
 *
 * **Test Procedure:** 
 * | Variation / Step | Description                                                                 | Test Data                                         | Expected Result                  | Notes                               |
 * | :--------------: | --------------------------------------------------------------------------- | ------------------------------------------------- | ------------------------------- | ----------------------------------- |
* | 01 | Setup for L1AVClockOpenTest: Get initial clock state | state | Check if state is CLOSED, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 02 | Open AV Clock | listener = valid listener | avClockController should not be null, , status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 03 | Ensure clock state is READY | state | state should be READY, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 02 | Get the current clock mode using getClockMode method.                       | N/A | status.isOk() should be true    | Should be successful |
* | 03 | If clock mode is PCR, set clock mode to AUTO using setClockMode.            | input: ClockMode=ClockMode::AUTO | status.isOk() should be true    | Should be successful|
* | 04 | Call setPlaybackRate with an invalid rate while in READY state.             | input: playbackRate=0.4 | status.isOk() should be true,  _aidl_return should be fail| Should fail |
* | 05 | If clock mode was changed to AUTO, revert back to original clock mode.      | input: ClockMode=original _clockMode | status.isOk() should be true | Should be successful|
* | 08 | Teardown for L1AVClockOpenTest: Close AVClock | avClockController, &_aidl_return | State should be CLOSED, status.isOk() should be true | Done by Pre-requisite TearDown function |
 */
TEST_F(L1AVClockOpenTest, test_l1_avclock_negative_2_setPlaybackRate)
{
    double playbackRate = 0.4; //invalid value
    bool _aidl_return;
    ::com::rdk::hal::avclock::ClockMode _clockMode;

    ::android::binder::Status status = avClockController->getClockMode(&_clockMode);
    EXPECT_TRUE(status.isOk());
    if(_clockMode == ClockMode::PCR)
    {
        //prerequisite
        status = avClockController->setClockMode(ClockMode::AUTO, &_aidl_return);
        ASSERT_TRUE(status.isOk());
        ASSERT_TRUE(_aidl_return);

        //calling setPlaybackRate() from state READY
        status = avClockController->setPlaybackRate(playbackRate, &_aidl_return);
        EXPECT_FALSE(_aidl_return);
        EXPECT_TRUE(status.isOk());

        status = avClockController->setClockMode(_clockMode, &_aidl_return);
        ASSERT_TRUE(status.isOk());
    }
    else
    {
        //calling setPlaybackRate() from state READY
        status = avClockController->setPlaybackRate(playbackRate, &_aidl_return);
        EXPECT_FALSE(_aidl_return);
        EXPECT_TRUE(status.isOk());   
    }
}

/**
* @brief Test to verify invalid playback rate handling in AVClock
*
* This test checks the behavior of the AVClock when an invalid playback rate is set. The goal is to ensure that the system handles the invalid input correctly and doesn't set the playback rate.
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 063@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data | Expected Result | Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01 | Setup for L1AVClockOpenTest: Get initial clock state | state | Check if state is CLOSED, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 02 | Open AV Clock | listener = valid listener | avClockController should not be null, , status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 03 | Ensure clock state is READY | state | state should be READY, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 02 | Get the current clock mode using getClockMode method.                       | N/A | status.isOk() should be true    | Should be successful |
* | 03 | If clock mode is PCR, set clock mode to AUTO using setClockMode.            | input: ClockMode=ClockMode::AUTO | status.isOk() should be true    | Should be successful|
* | 04 | Call setPlaybackRate with an invalid rate while in READY state.             | input: playbackRate=2.1 | status.isOk() should be true,  _aidl_return should be fail| Should fail |
* | 05 | If clock mode was changed to AUTO, revert back to original clock mode.      | input: ClockMode=original _clockMode | status.isOk() should be true | Should be successful|
* | 08 | Teardown for L1AVClockOpenTest: Close AVClock | avClockController, &_aidl_return | State should be CLOSED, status.isOk() should be true | Done by Pre-requisite TearDown function |
*/
TEST_F(L1AVClockOpenTest, test_l1_avclock_negative_3_setPlaybackRate)
{
    double playbackRate = 2.1; //invalid value
    bool _aidl_return;
    ::com::rdk::hal::avclock::ClockMode _clockMode;

    ::android::binder::Status status = avClockController->getClockMode(&_clockMode);
    EXPECT_TRUE(status.isOk());
    if(_clockMode == ClockMode::PCR)
    {
        //prerequisite
        status = avClockController->setClockMode(ClockMode::AUTO, &_aidl_return);
        ASSERT_TRUE(status.isOk());
        ASSERT_TRUE(_aidl_return);

        //calling setPlaybackRate() from state READY
        status = avClockController->setPlaybackRate(playbackRate, &_aidl_return);
        EXPECT_FALSE(_aidl_return);
        EXPECT_TRUE(status.isOk());

        status = avClockController->setClockMode(_clockMode, &_aidl_return);
        ASSERT_TRUE(status.isOk());
    }
    else
    {
        //calling setPlaybackRate() from state READY
        status = avClockController->setPlaybackRate(playbackRate, &_aidl_return);
        EXPECT_FALSE(_aidl_return);
        EXPECT_TRUE(status.isOk());   
    }
}

/**
* @brief Test function for validating the playback rate functionality under different clock modes.
*
* This test checks that the playback rate is accurately retrieved and falls within the expected range of 0.5 to 2.0. 
* The test is performed by checking the initial clock mode, switching to ClockMode::AUTO if the initial mode is ClockMode::PCR, 
* and validating the playback rate returned after the mode switch.
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 064@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description                                           | Test Data                                          | Expected Result                     | Notes                           |
* | :--------------: | ----------------------------------------------------- | -------------------------------------------------- | ---------------------------------- | ------------------------------- |
* | 01 | Setup for L1AVClockOpenTest: Get initial clock state | state | Check if state is CLOSED, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 02 | Open AV Clock | listener = valid listener | avClockController should not be null, , status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 03 | Ensure clock state is READY | state | state should be READY, status.isOk() should be true | Done by Pre-requisite SetUp function |
* | 04 | Call `getClockMode` to get mode    |  `_clockMode` (initial clock mode) | status.isOk() should be true | Should be successful |
* | 05 | If ClockMode is PCR, switch to AUTO |  `_clockMode=ClockMode::AUTO, &_aidl_return`  | status.isOk() and _aidl_return should be true | Should Pass |
* | 06 | Get Playback rate using avClockController |  `playbackRate` | playbackRate in range [0.5, 2.0]   | Should be successful |
* | 07 | Revert clock mode to initial |  `_clockMode` (reverted back to initialmode) | status.isOk() and _aidl_return should be true | Should Pass |
* | 08 | Teardown for L1AVClockOpenTest: Close AVClock | avClockController, &_aidl_return | State should be CLOSED, status.isOk() should be true | Done by Pre-requisite TearDown function |
*/
TEST_F(L1AVClockOpenTest, test_l1_avclock_positive_1_getPlaybackRate)
{
    ::com::rdk::hal::avclock::ClockMode _clockMode;
    double playbackRate;
    bool _aidl_return;
    //prerequisite

    ::android::binder::Status status = avClockController->getClockMode(&_clockMode);
    EXPECT_TRUE(status.isOk());
    if(_clockMode == ClockMode::PCR)
    {
        status = avClockController->setClockMode(ClockMode::AUTO, &_aidl_return);
        ASSERT_TRUE(status.isOk());
        ASSERT_TRUE(_aidl_return);
        status = avClockController->getPlaybackRate(&playbackRate);
        ASSERT_TRUE(status.isOk());
        ASSERT_TRUE(playbackRate >= 0.5 && playbackRate <= 2.0);

        //cleaning up, reverting values
        status = avClockController->setClockMode(_clockMode, &_aidl_return);
        ASSERT_TRUE(status.isOk());
    }
    else
    {
        status = avClockController->getPlaybackRate(&playbackRate);
        ASSERT_TRUE(status.isOk());
        ASSERT_TRUE(playbackRate >= 0.5 && playbackRate <= 2.0);
    }
}

/**
* @brief Test for verifying the playback rate of the AV clock in the positive scenario.
*
* This test verifies that the playback rate of the AV clock falls within the expected range of 0.5 to 2.0 under normal circumstances.
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 065@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data | Expected Result | Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01 | Get the initial state of the AV clock and ensure it is CLOSED using Set Up method. | None | AV clock state should be CLOSED | Done by Pre-requisite SetUp function |
* | 02 | Open the AV clock with the listener and retrieve the AV clock controller. | listener = new AVClockControllerListener(), &avClockController | AV clock controller should be non-null | Done by Pre-requisite SetUp function |
* | 03 | Get the state of the AV clock and ensure it is READY after opening. | None | AV clock state should be READY | Done by Pre-requisite SetUp function |
* | 04 | Get the clock mode and set it to AUTO if it is currently PCR mode, then start the AV clock. | ClockMode = current clock mode | Clock mode set to AUTO and state should be STARTED | Should be successful |
* | 05 | Get the state of the AV clock to ensure it is STARTED. | N/A | AV clock state should be STARTED | Done by Pre-requisite SetUp function |
* | 06 | Get the playback rate from the AV clock. | playbackRate = valid pointer | Playback rate should be between 0.5 and 2.0, status.isOk() should be true | Should Pass |
* | 07 | Stop the AV clock. | None | AV clock state should be READY | Done by Pre-requisite TearDown function |
* | 08 | Reset the clock mode to its original value if it was changed. | _clockMode = initial value | status.isOk() should be true | Done by Pre-requisite TearDown function |
* | 09 | Close the AV clock and ensure it is CLOSED after closing. | avClockController = valid instance | AV clock state should be CLOSED, status.isOk() should be true | Done by Pre-requisite TearDown function |
*/
TEST_F(L1AVClockPlayBackRateStartTest, test_l1_avclock_positive_2_getPlaybackRate)
{
    ::com::rdk::hal::avclock::ClockMode _clockMode;
    double playbackRate;
    bool _aidl_return;
    //prerequisite

    ::android::binder::Status status = avClockController->getPlaybackRate(&playbackRate);
    ASSERT_TRUE(status.isOk());
    ASSERT_TRUE(playbackRate >= 0.5 && playbackRate <= 2.0);
}

/**
* @brief Test for validating avClock getPlaybackRate functionality in a negative scenario
*
* This test verifies that getPlaybackRate API returns an error when ClockMode::PCR is set.
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 066@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data | Expected Result |Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01 | Verification of getState() in SetUp | State | state == ::com::rdk::hal::State::CLOSED | Done by Pre-requisite SetUp function |
* | 02 | Opening avClock with listener | listener = valid listener | avClockController is assigned | Done by Pre-requisite SetUp function |
* | 03 | Verification of getState() after open | State is fetched from _avClock | state == ::com::rdk::hal::State::READY | Done by Pre-requisite SetUp function |
* | 04 | Call getClockMode() | ClockMode = valid parameter | _clockMode is retrieved, status.isOk() should be true | Should be successful |
* | 05 | If _clockMode is not ClockMode::PCR, set it to ClockMode::PCR and fetch playbackRate | ClockMode |ClockMode should be PCR | n/a |
* | 06 | Set ClockMode to PCR if not already | setClockMode called with ClockMode::PCR | _clockMode = ClockMode::PCR, _aidl_return = true | Should be successful |
* | 07 | Attempt getPlaybackRate() with ClockMode::PCR | getPlaybackRate called | playbackRate is not retrieved, status.isOk() should be false | Should Fail |
* | 08 | Revert the ClockMode to original | setClockMode called with original _clockMode | _clockMode is restored, _aidl_return = true | Should be successful |
* | 09 | (Alternate) Attempt getPlaybackRate() directly if ClockMode is PCR | getPlaybackRate called | playbackRate is not retrieved, status.isOk() should be false | Should Fail |
* | 10 | Verification of getState() in TearDown | State is fetched from _avClock | state == ::com::rdk::hal::State::READY | Done by Pre-requisite TearDown function |
* | 11 | Closing avClockController | close() called with avClockController | _aidl_return is true | Done by Pre-requisite TearDown function |
*/
TEST_F(L1AVClockOpenTest, test_l1_avclock_negative_1_getPlaybackRate) 
{
    ::com::rdk::hal::avclock::ClockMode _clockMode;
    double playbackRate;
    bool _aidl_return;

    ::android::binder::Status status = avClockController->getClockMode(&_clockMode);
    EXPECT_TRUE(status.isOk());

    if(_clockMode != ClockMode::PCR)
    {
        status = avClockController->setClockMode(ClockMode::PCR, &_aidl_return);
        ASSERT_TRUE(status.isOk());
        ASSERT_TRUE(_aidl_return);
        //calling getPlaybackRate() with clockMode ClockMode::PCR
        status = avClockController->getPlaybackRate(&playbackRate);
        EXPECT_FALSE(status.isOk());

        //cleaning up, reverting values
        status = avClockController->setClockMode(_clockMode, &_aidl_return);
        ASSERT_TRUE(status.isOk());
    }
    else
    {
        status = avClockController->getPlaybackRate(&playbackRate);
        EXPECT_FALSE(status.isOk());        
    }
}

int main(int argc, char **argv) 
{
    std::string yaml_file;
    int opt;
    int result1;
    int result2;

    int number_of_avclock;
    int number_of_audiosink;
    int number_of_videosink;

    std::string avclockpath = "components.IAVClockManager.IAVClock";
    std::string audiosinkpath = "components.IAudioSinkManager.IAudioSink";
    std::string videosinkpath = "components.IVideoSinkManager.IVideoSink";

    UT_init(argc, argv);
    if(getService() == -1)
    {
        return -1;
    }

    optind = 1;
    while ((opt = getopt(argc, argv, "p:")) != -1) 
    {
        switch (opt) 
        {
            case 'p':
                yaml_file = optarg;
                break;
            default:
                std::cerr << "Usage: " << argv[0] << " -p <yaml_file>" << std::endl;
                return 1;
        }
    }

    if (!yaml_file.empty()) 
    {
        try 
        {
            YAML::Node config = YAML::LoadFile(yaml_file.c_str());

            number_of_avclock = get_list_Count_from_profile(config, avclockpath);
            if(number_of_avclock <= 0)
            {
                std::cout<< " No avclock find in the profile" <<std::endl;
                return -1;
            }
            std::cout <<"avclock : " <<number_of_avclock <<std::endl;

            for(int i=0; i<number_of_avclock; ++i)
            {
                std::string avclockidpath = avclockpath+"."+std::to_string(i) + ".id";
                int id = get_integer_value_from_profile(config,avclockidpath);
                _avclock_ids_from_profile.push_back(id);
            }

            number_of_audiosink = get_list_Count_from_profile(config, audiosinkpath);
            if(number_of_audiosink <= 0)
            {
                std::cout<< " No audiosink find in the profile" <<std::endl;
                return -1;
            }
            std::cout <<"audiosink count : " <<number_of_audiosink <<std::endl;

            for(int i=0; i<number_of_audiosink; ++i)
            {
                std::string audiosinkidpath = audiosinkpath+"."+std::to_string(i) + ".id";
                int id = get_integer_value_from_profile(config,audiosinkidpath);
                _audiosink_ids_from_profile.push_back(id);
            }

            number_of_videosink = get_list_Count_from_profile(config, videosinkpath);
            if(number_of_videosink <= 0)
            {
                std::cout<< " No videosink find in the profile" <<std::endl;
                return -1;
            }
            std::cout <<"videosink count : " <<number_of_videosink <<std::endl;

            for(int i=0; i<number_of_videosink; ++i)
            {
                std::string videosinkidpath = videosinkpath+"."+std::to_string(i) + ".id";
                int id = get_integer_value_from_profile(config,videosinkidpath);
                _videosink_ids_from_profile.push_back(id);
            }

            for(const auto& id: _avclock_ids_from_profile)
            {
                g_avclockId.value = id;
                Status status = service->getAVClock(g_avclockId, &_avClock);
                ::testing::GTEST_FLAG(filter) = "L1AVClockTest.*:L1AVClockOpenTest.*:L1AVClockCloseTest.*:L1AVClockEventRegisterTest.*:L1AVClockStartTest.*:L1AVClockStopTest.*:L1AVClockPlayBackRateStartTest.*";
                result1 = UT_run_tests();
            }
            ::testing::GTEST_FLAG(filter) = "L1AVClockManagerTest.*";
            result2 = UT_run_tests();
            return (result1 || result2);

        }
    
        catch (const YAML::BadFile& e) 
        {
            std::cerr << "Error: Could not open YAML file. " << e.what() << std::endl;
            return -1;
        } 
        catch (const YAML::BadConversion& e) 
        {
            std::cerr << "Error: Bad type conversion in YAML. " << e.what() << std::endl;
            return -1;
        }
        catch (const std::runtime_error& e) 
        {
            std::cerr << "Runtime Error: " << e.what() << std::endl;
            return -1;
        } 
        catch (const std::exception& e) 
        {
            std::cerr << "Error: " << e.what() << std::endl;
            return -1;
        }
    }
    else
    {
        std::cout << "USAGE: " << argv[0] << " -p <config_file>" <<std::endl;
    }
}

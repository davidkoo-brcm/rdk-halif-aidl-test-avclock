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

#include "vts_avclock.h"


class VTS_L1_AVClock : public UTCore
{
public:
    VTS_L1_AVClock() : UTCore() {}

    ~VTS_L1_AVClock() override = default;

    void SetUp() override
    {
    }

    void TearDown() override
    {
    }
};
#if 0
class AVClockControllerListenerImpl : public BnAVClockControllerListener {
    public:
        AVClockControllerListenerImpl() {};
        virtual ~AVClockControllerListenerImpl() = default;

        ::android::binder::Status onStateChanged(::com::rdk::hal::State oldState, ::com::rdk::hal::State newState) override{
            return ::android::binder::Status::ok();
        }
    };

class AVClockEventListenerImpl : public BnAVClockEventListener {
public:
    AVClockEventListenerImpl() {};
    virtual ~AVClockEventListenerImpl() = default;

    ::android::binder::Status onStateChanged(::com::rdk::hal::State oldState, ::com::rdk::hal::State newState) override{
        return ::android::binder::Status::ok();
    }
};

class AVClockManagerBaseTest : public virtual UTCore
{
protected:
    static void SetUpTestSuite()
    {
        UT_ASSERT_EQUAL(getAVclockService(), 0);
    }

    static void TearDownTestSuite()
    {
        gAVClockManagerService.clear();
    }
};

class AVClockBaseTest : public AVClockManagerBaseTest
{
protected:
    static std::vector<IAVClock::Id> mAVClockIds;
    static std::vector<sp<IAVClock>> mAVClocks;

    void SetUp() override
    {
        Status status;
        AVClockManagerBaseTest::SetUp();

        if (mAVClockIds.empty())
        {
            std::vector<IAVClock::Id> ids;
            status = gAVClockManagerService->getAVClockIds(&ids);
            UT_ASSERT_TRUE(status.isOk());
            mAVClockIds = ids;
            UT_ASSERT_FALSE(mAVClockIds.empty());
        }

        if (mAVClocks.empty())
        {
            for (auto& clockId : mAVClockIds)
            {
                sp<IAVClock> clock;
                status = gAVClockManagerService->getAVClock(clockId, &clock);
                UT_ASSERT_TRUE(status.isOk());
                UT_ASSERT_NOT_EQUAL(clock, nullptr);
                mAVClocks.push_back(clock);
            }
        }

    }

    void TearDown() override
    {
        AVClockManagerBaseTest::TearDown();
    }
};

std::vector<IAVClock::Id> AVClockBaseTest::mAVClockIds;
std::vector<sp<IAVClock>> AVClockBaseTest::mAVClocks;

class AVClockControllerBaseTest : public AVClockBaseTest
{
protected:
    static std::vector<sp<IAVClockController>> mControllers;
    static sp<IAVClockControllerListener> mControllerListener;

    void SetUp() override
    {
        Status status;
        AVClockBaseTest::SetUp();

        if (mControllers.empty() && !mAVClocks.empty())
        {
            mControllerListener = new AVClockControllerListenerImpl;
            for (auto& clock : mAVClocks)
            {
                sp<IAVClockController> controller;
                status = clock->open(mControllerListener, &controller);
                UT_ASSERT_TRUE(status.isOk());
                UT_ASSERT_NOT_EQUAL(controller, nullptr);
                mControllers.push_back(controller);
            }
        }
    }

    void TearDown() override
    {
        Status status;
        UT_ASSERT_EQUAL(mControllers.size(), mAVClocks.size());
        for (size_t i = 0; i < mAVClocks.size(); ++i)
        {
            bool closed = false;
            status = mAVClocks[i]->close(mControllers[i], &closed);
            UT_ASSERT_TRUE(status.isOk());
            UT_ASSERT_TRUE(closed);
        }
        mControllers.clear();
        AVClockBaseTest::TearDown();
    }
};

std::vector<sp<IAVClockController>> AVClockControllerBaseTest::mControllers;
sp<IAVClockControllerListener> AVClockControllerBaseTest::mControllerListener = nullptr;
#endif
/**
 * @brief Test the positive scenario of retrieving AV Clock IDs from the AVClockManager service.
 *
 * This test verifies that the `getAVClockIds` function correctly retrieves a list of AV Clock IDs from the AVClockManager service. It checks that the function returns a successful status, the list is not empty, and the IDs match the expected count and values.
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
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Initialize the test environment and log the start of the test case. | None | None | Should be successful |
 * | 02 | Call the `getAVClockIds` function to retrieve AV Clock IDs. | OUT:_aidl_return[] | Status should be OK, avClockIds should not be empty | Should Pass |
 * | 03 | Log the result of the `getAVClockIds` function call, including the size of the returned list and status details. | avClockIds.size(), status.isOk(), status.exceptionCode() | None | Should be successful |
 * | 04 | Assert that the status is OK and the list of AV Clock IDs is not empty. | status.isOk(), avClockIds.empty() | Assertions should pass | Should Pass |
 * | 05 | Retrieve the expected count of AV Clock IDs from the profile and assert that the size matches the expected count. | expectedCount | Assertion should pass | Should Pass |
 * | 06 | Iterate over each AV Clock ID and verify that it matches the expected values from the profile. | id.value, expectedCount | Assertion should pass for each ID | Should Pass |
 */
UT_ADD_TEST(VTS_L1_AVClock, getAVClockIds_pos)
{

    std::vector<IAVClock::Id> initialClockIds;
    std::vector<IAVClock::Id> secondClockIds;
    std::vector<int32_t> clockIdValues;
    std::unordered_set<int> clockIds_from_profile;

    // Step 01: Get the AV Clock IDs first time
    AVCLOCK_MANAGER_GETAVCLOCKIDS(initialClockIds);

    // Extract ID values
    for (const auto& obj : initialClockIds)
    {
        clockIdValues.push_back(obj.value);
    }

    // Step 02: Verify uniqueness of IDs
    UT_ASSERT_TRUE(vts_avclock_helper_areElementsUnique(clockIdValues));

    // Step 03: Get the AV Clock IDs second time
    AVCLOCK_MANAGER_GETAVCLOCKIDS(secondClockIds);

    // Step 04: Size should match between first and second calls
    UT_ASSERT_EQUAL(initialClockIds.size(), secondClockIds.size());

    // Step 05: Each ID should match
    for (size_t i = 0; i < initialClockIds.size(); ++i)
    {
        UT_ASSERT_EQUAL(initialClockIds[i].value, secondClockIds[i].value);
    }

    // Step 06: Expected clock IDs from HAL Feature Profile
    const char* profile_path = "IAVClockManager.IAVClock";
    size_t expectedCount = UT_KVP_PROFILE_GET_LIST_COUNT(profile_path);
    UT_ASSERT_EQUAL(initialClockIds.size(), expectedCount);

    // Insert expected IDs into set
    for (size_t i = 0; i < expectedCount; ++i)
    {
        std::string avclockid_path = std::string(profile_path) + "." + std::to_string(i);
        clockIds_from_profile.insert(UT_KVP_PROFILE_GET_UINT32(avclockid_path.c_str()));
    }

    // Step 07: Each retrieved ID must exist in profile
    for (int id : clockIdValues)
    {
        UT_ASSERT_NOT_EQUAL(clockIds_from_profile.find(id), clockIds_from_profile.end());
    }
}

#if 0
/**
 * @brief Test the positive scenario of retrieving AVClock using AVClockManager
 *
 * This test verifies that the AVClockManager service can successfully retrieve a list of AVClock IDs and subsequently obtain each AVClock instance using these IDs. It ensures that the service returns valid AVClock objects and that the status of each operation is successful.
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
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Retrieve AVClock IDs from AVClockManager service | avClockIds = [] | Status is OK, avClockIds is not empty | Should be successful |
 * | 02 | Iterate over each AVClock ID and retrieve the corresponding AVClock | avClockId = value, avClock = nullptr | Status is OK, avClock is not nullptr | Should Pass |
 */
UT_ADD_TEST(VTS_L1_AVClock, getAVClock_pos)
{
    android::sp<IAVClock> avclock_1;
    android::sp<IAVClock> avclock_2;
    std::vector<IAVClock::Id> clockIds;
    std::optional<PropertyValue> resource_id_1;
    std::optional<PropertyValue> resource_id_2;
    com::rdk::hal::avclock::Property property = com::rdk::hal::avclock::Property::RESOURCE_ID;

    /* Get the AV clock IDs */
    AVCLOCK_MANAGER_GETAVCLOCKIDS(clockIds);

    UT_ASSERT_TRUE(!clockIds.empty());

    for (const auto& _clockId : clockIds)
    {
        /* Get AV clock handle (first invocation) */
        AVCLOCK_MANAGER_GETAVCLOCK_SUCCESS(_clockId, avclock_1);
        /* Get the resource ID */
        AVCLOCK_GET_PROPERTY_SUCCESS(avclock_1, property, resource_id_1);

        /* Get AV clock handle (second invocation) */
        AVCLOCK_MANAGER_GETAVCLOCK_SUCCESS(_clockId, avclock_2);
        /* Get the resource ID */
        AVCLOCK_GET_PROPERTY_SUCCESS(avclock_2, property, resource_id_2);

        UT_ASSERT_TRUE(resource_id_1->value != nullptr);
        UT_ASSERT_TRUE(resource_id_2->value != nullptr);
        int32_t _resource_id_1 = resource_id_1->value.intValue;
        int32_t _resource_id_2 = resource_id_2->value.intValue;

        UT_ASSERT_EQUAL(_resource_id_1, _resource_id_2);
    }
}
#endif
/**
 * @brief Test the behavior of getAVClock with invalid AVClock IDs
 *
 * This test verifies that the getAVClock function correctly handles invalid AVClock IDs by returning an error status and a null pointer. It ensures that the function does not succeed when provided with IDs that are outside the valid range.
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
 * | 01 | Retrieve all AVClock IDs from the service | None | Status is OK, avClockIds is not empty | Should be successful |
 * | 02 | Sort the retrieved AVClock IDs to determine the lowest and highest values | avClockIds | Sorted avClockIds | Should be successful |
 * | 03 | Calculate an invalid low ID by subtracting one from the lowest ID | lowest ID = avClockIds.front().value | invalidLowId = lowest ID - 1 | Should be successful |
 * | 04 | Calculate an invalid high ID by adding one to the highest ID | highest ID = avClockIds.back().value | invalidHighId = highest ID + 1 | Should be successful |
 * | 05 | Call getAVClock with the invalid low ID and verify the result | avClockId = invalidLowId, avClock = nullptr | Status is not OK, avClock is nullptr | Should Fail |
 * | 06 | Call getAVClock with the invalid high ID and verify the result | avClockId = invalidHighId, avClock = nullptr | Status is not OK, avClock is nullptr | Should Fail |
 */
/**
 * **Test Procedure:**@n
 * |Variation / Step|Description|Test Data|Expected Result|Notes|
 * |----------------|-----------|---------|---------------|-----|
 * |01|Retrieve the initial AVClock IDs|None|Should retrieve a non-empty list of IDs|Should Pass|
 * |02|Get AVClock with invalid id (min id value - 1)|(min id value - 1), avClock|nullptr|status.isOk() should be false and avClock should be nullptr|Should Pass|
 * |03|Get AVClock with invalid id (max id value + 1)|(max id value + 1), avClock|nullptr|status.isOk() should be false and avClock should be nullptr|Should Pass|
 * |04|Get AVClock with UNDEFINED ID|UNDEFINED, avClock|nullptr|status.isOk() should be false and avClock should be nullptr|Should Pass|
 */
UT_ADD_TEST(VTS_L1_AVClock, getAVClock_neg)
{
    std::vector<IAVClock::Id> avClockIds;
    sp<IAVClock> avClock;
    IAVClock::Id invalidID;

    /* Get the AVClock IDs */
    UT_LOG_INFO("Starting getAVClock_neg test");
    AVCLOCK_MANAGER_GETAVCLOCKIDS(avClockIds);

    /* Ensure we have at least one valid ID */
    UT_LOG_INFO("Retrieved %zu AVClock IDs", avClockIds.size());
    UT_ASSERT_TRUE(!avClockIds.empty());

    /* Sort IDs for consistent min/max selection */
    if (avClockIds.size() > 1) {
        std::sort(avClockIds.begin(), avClockIds.end(),
                  [](const IAVClock::Id& a, const IAVClock::Id& b) { return a.value < b.value; });
        UT_LOG_INFO("Sorted IDs: min=%d, max=%d", avClockIds.front().value, avClockIds.back().value);
    }

    /* Test with invalid ID: negative ID */
    invalidID.value = -1;
    UT_LOG_INFO("Testing invalid ID: negative ID = %d", invalidID.value);
    AVCLOCK_MANAGER_GETAVCLOCK_FAILURE(invalidID, avClock);

    /* Test with invalid ID: maximum ID + 1 */
    invalidID = avClockIds.back();
    invalidID.value += 1;
    UT_LOG_INFO("Testing invalid ID: max ID + 1 = %d", invalidID.value);
    AVCLOCK_MANAGER_GETAVCLOCK_FAILURE(invalidID, avClock);

}

#if 1
/**
 * @brief Test the positive scenario for retrieving AVClock capabilities
 *
 * This test verifies the successful retrieval of AVClock capabilities using the getCapabilities function. It checks if the function returns the expected capabilities without any errors.
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
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Initialize the test environment and prepare for capability retrieval | None | Environment should be ready for testing | Should be successful |
 * | 02 | Call the getCapabilities function to retrieve AVClock capabilities | capabilities = nullptr | status.isOk() = true, capabilities != nullptr | Should Pass |
 * | 03 | Validate the retrieved capabilities against expected audio sink IDs | expectedAudioSinkIds = {0, 1} | All expected audio sink IDs should be present in capabilities | Should Pass |
 * | 04 | Validate the retrieved capabilities against expected video sink IDs | expectedVideoSinkIds = {0, 1} | All expected video sink IDs should be present in capabilities | Should Pass |
 */
UT_ADD_TEST(VTS_L1_AVClock, getCapabilities_pos)
{
    avclock::Capabilities capabilities;
    std::vector<IAVClock::Id> Ids;
    sp<IAVClock> avClock;

    /* Get the AVClock IDs */
    UT_LOG_INFO("Starting avclock_getCapabilities_pos test");
    AVCLOCK_MANAGER_GETAVCLOCKIDS(Ids);

    /* Ensure we have at least one AVClock ID */
    UT_LOG_INFO("Retrieved %zu AVClock IDs", Ids.size());
    UT_ASSERT_TRUE(!Ids.empty());

    for (const auto& id : Ids)
    {
        /* Get AVClock handle */
        IAVClock::Id avClockId = id; // id is already IAVClock::Id
        UT_LOG_INFO("Getting AVClock for ID=%d", avClockId.value);
        AVCLOCK_MANAGER_GETAVCLOCK_SUCCESS(avClockId, avClock);

        /* Get Capabilities of AVClock */
        UT_LOG_INFO("Calling getCapabilities for AVClock ID=%d", avClockId.value);
        AVCLOCK_GETCAPABILITIES(avClock, &capabilities);

        /* Validate capabilities */
        UT_LOG_INFO("Validating capabilities for AVClock ID=%d", avClockId.value);
        vts_avclock_helper_validateCapabilities(avClockId.value, capabilities);
    }
}
#endif

/**
 * @brief Test the AV Clock's state retrieval function.
 *
 * This test verifies if the getState function of the AV Clock retrieves the state correctly and ensures the state is a valid enum value.
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
 * |Variation / Step|Description|Test Data|Expected Result|Notes|
 * |----------------|-----------|---------|---------------|-----|
 * |01|Retrieve available AV Clock IDs|None|clockIds should not be empty, status.isOk() should be true|Should Pass|
 * |02|Get AV Clock handle and capabilities for each ID|_clockId|avClock should be valid, status.isOk() should be true|Should Pass|
 * |03|Invoke getState to retrieve the initial state|None|status.isOk() should be true, state should be CLOSED or READY|Should Pass|
 * |04|Open the AV Clock and verify the state|defaultConfig, controllerListener|status.isOk() should be true, state should be READY|Should Pass|
 * |05|Close the AV Clock and verify the state|avClockController|status.isOk() should be true, state should be CLOSED|Should Pass|
 */
/**
 * @brief Test the AV Clock's state retrieval function.
 *
 * This test verifies if the getState function of the AV Clock retrieves the state correctly and ensures the state is a valid enum value.
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
 * |Variation / Step|Description|Test Data|Expected Result|Notes|
 * |----------------|-----------|---------|---------------|-----|
 * |01|Retrieve available AV Clock IDs|None|clockIds should not be empty, status.isOk() should be true|Should Pass|
 * |02|Get AV Clock handle for each ID|_clockId|avClock should be valid, status.isOk() should be true|Should Pass|
 * |03|Invoke getState to retrieve the initial state|None|status.isOk() should be true, state should be CLOSED|Should Pass|
 * |04|Open the AV Clock and verify the state|controllerListener|status.isOk() should be true, state should be READY|Should Pass|
 * |05|Close the AV Clock and verify the state|avClockController|status.isOk() should be true, state should be CLOSED|Should Pass|
 */
UT_ADD_TEST(VTS_L1_AVClock, getState_pos)
{
    std::vector<com::rdk::hal::avclock::IAVClock::Id> clockIds;
    android::sp<com::rdk::hal::avclock::IAVClock> avClock;
    com::rdk::hal::avclock::Capabilities capabilities;
    ::android::binder::Status status;

    /* Get the AV Clock IDs */
    AVCLOCK_MANAGER_GETAVCLOCKIDS(clockIds);

    for (const auto& _clockId : clockIds)
    {
        /* Get AV Clock handle */
        AVCLOCK_MANAGER_GETAVCLOCK_SUCCESS(_clockId, avClock);

        /* Retrieve and validate the initial state */
        ::com::rdk::hal::State state;
        AVCLOCK_GET_STATE(avClock, state);
        UT_LOG_INFO("Initial state for clock ID %d: %s", _clockId.value, com::rdk::hal::toString(state).c_str());
        UT_ASSERT_EQUAL(state, ::com::rdk::hal::State::CLOSED); // Expect CLOSED initially

        /* Open the AV Clock to test state transition */
        android::sp<com::rdk::hal::avclock::IAVClockController> avClockController = nullptr;
        android::sp<com::rdk::hal::avclock::IAVClockControllerListener> controllerListener = new AVClockControllerListener();

        AVCLOCK_OPEN_SUCCESS(avClock, controllerListener, avClockController, true);

        /* Verify state after opening */
        AVCLOCK_GET_STATE(avClock, state);
        UT_LOG_INFO("State after open for clock ID %d: %s", _clockId.value, com::rdk::hal::toString(state).c_str());
        UT_ASSERT_EQUAL(state, ::com::rdk::hal::State::READY); // Expect stable READY state
        // Note: Transitory state OPENING may occur briefly but is not tested in L1.

        /* Close the AV Clock */
        AVCLOCK_CLOSE_SUCCESS(avClock, avClockController, true);

        /* Verify state after closing */
        AVCLOCK_GET_STATE(avClock, state);
        UT_LOG_INFO("State after close for clock ID %d: %s", _clockId.value, com::rdk::hal::toString(state).c_str());
        UT_ASSERT_EQUAL(state, ::com::rdk::hal::State::CLOSED); // Expect stable CLOSED state
        // Note: Transitory state CLOSING may occur briefly but is not tested in L1.
    }
}

/**
 * @brief Test the AV Clock's getProperty function for valid property keys.
 *
 * This test verifies if the getProperty function retrieves valid property values correctly for all supported properties.
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
 * |Variation / Step|Description|Test Data|Expected Result|Notes|
 * |----------------|-----------|---------|---------------|-----|
 * |01|Retrieve available AV Clock IDs|None|clockIds should not be empty, status.isOk() should be true|Should Pass|
 * |02|Get AV Clock handle for each ID|_clockId|avClock should be valid, status.isOk() should be true|Should Pass|
 * |03|Invoke getProperty for RESOURCE_ID|Property::RESOURCE_ID|status.isOk() should be true, PropertyValue should have value, intValue >= 0|Should Pass|
 * |04|Invoke getProperty for METRIC_AV_SYNC_ACCURACY_MS|Property::METRIC_AV_SYNC_ACCURACY_MS|status.isOk() should be true, PropertyValue should have value, intValue >= -1|Should Pass|
 * |05|Invoke getProperty for METRIC_AV_RESYNC_COUNT|Property::METRIC_AV_RESYNC_COUNT|status.isOk() should be true, PropertyValue should have value, intValue >= -1|Should Pass|
 */
UT_ADD_TEST(VTS_L1_AVClock, getProperty_pos)
{
    std::vector<com::rdk::hal::avclock::IAVClock::Id> clockIds;
    android::sp<com::rdk::hal::avclock::IAVClock> avClock;
    ::android::binder::Status status;

    /* List of valid properties to test */
    std::vector<com::rdk::hal::avclock::Property> validProperties = {
        com::rdk::hal::avclock::Property::RESOURCE_ID,
        com::rdk::hal::avclock::Property::METRIC_AV_SYNC_ACCURACY_MS,
        com::rdk::hal::avclock::Property::METRIC_AV_RESYNC_COUNT
    };

    /* Get the AV Clock IDs */
    AVCLOCK_MANAGER_GETAVCLOCKIDS(clockIds);

    for (const auto& _clockId : clockIds)
    {
        /* Get AV Clock handle */
        AVCLOCK_MANAGER_GETAVCLOCK_SUCCESS(_clockId, avClock);

        /* Test getProperty for each valid property */
        for (const auto& _property : validProperties)
        {
            std::optional<com::rdk::hal::PropertyValue> propertyValue;
            AVCLOCK_GET_PROPERTY_SUCCESS(avClock, _property, propertyValue);

            /* Validate the returned PropertyValue */
            UT_ASSERT_EQUAL(propertyValue->value->getTag(), com::rdk::hal::PropertyValue::Value::intValue);
            int32_t intValue = propertyValue->value->get<com::rdk::hal::PropertyValue::Value::intValue>();
            UT_LOG_INFO("Property %s for clock ID %d: intValue=%d",
                        com::rdk::hal::avclock::toString(_property).c_str(),
                        _clockId.value,
                        intValue);
            if (_property == com::rdk::hal::avclock::Property::RESOURCE_ID)
            {
                UT_ASSERT_TRUE(intValue >= 0); // RESOURCE_ID must be non-negative
            }
            else
            {
                UT_ASSERT_TRUE(intValue >= -1); // Metrics can be -1 if not implemented
            }
        }
    }
}

/**
 * @brief Test the AV Clock's getProperty function for an invalid property key.
 *
 * This test verifies if the getProperty function handles an invalid property key correctly by returning null.
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
 * |Variation / Step|Description|Test Data|Expected Result|Notes|
 * |----------------|-----------|---------|---------------|-----|
 * |01|Retrieve available AV Clock IDs|None|clockIds should not be empty, status.isOk() should be true|Should Pass|
 * |02|Get AV Clock handle for each ID|_clockId|avClock should be valid, status.isOk() should be true|Should Pass|
 * |03|Invoke getProperty with an invalid property key|Invalid Property key (e.g., -1)|status.isOk() should be true, PropertyValue should be null|Should Pass|
 */
UT_ADD_TEST(VTS_L1_AVClock, getProperty_neg)
{
    std::vector<com::rdk::hal::avclock::IAVClock::Id> clockIds;
    android::sp<com::rdk::hal::avclock::IAVClock> avClock;
    ::android::binder::Status status;

    /* Invalid property key */
    com::rdk::hal::avclock::Property invalidProperty = static_cast<com::rdk::hal::avclock::Property>(-1);

    /* Get the AV Clock IDs */
    AVCLOCK_MANAGER_GETAVCLOCKIDS(clockIds);

    for (const auto& _clockId : clockIds)
    {
        /* Get AV Clock handle */
        AVCLOCK_MANAGER_GETAVCLOCK_SUCCESS(_clockId, avClock);

        /* Test getProperty with invalid property */
        std::optional<com::rdk::hal::PropertyValue> propertyValue;
        AVCLOCK_GET_PROPERTY_FAILURE(avClock, invalidProperty, propertyValue);

        /* Validate that PropertyValue is null */
        UT_LOG_INFO("Invalid property %d for clock ID %d: PropertyValue is null", invalidProperty, _clockId.value);
    }
}

/**
 * @brief Test the AV Clock's setProperty function for valid property keys.
 *
 * This test verifies if the setProperty function handles valid property keys correctly, returning false for read-only properties.
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
 * |Variation / Step|Description|Test Data|Expected Result|Notes|
 * |----------------|-----------|---------|---------------|-----|
 * |01|Retrieve available AV Clock IDs|None|clockIds should not be empty, status.isOk() should be true|Should Pass|
 * |02|Get AV Clock handle for each ID|_clockId|avClock should be valid, status.isOk() should be true|Should Pass|
 * |03|Invoke setProperty for RESOURCE_ID|Property::RESOURCE_ID, PropertyValue with intValue=1|status.isOk() should be true, return false (read-only)|Should Pass|
 * |04|Invoke setProperty for METRIC_AV_SYNC_ACCURACY_MS|Property::METRIC_AV_SYNC_ACCURACY_MS, PropertyValue with intValue=0|status.isOk() should be true, return false (read-only)|Should Pass|
 * |05|Invoke setProperty for METRIC_AV_RESYNC_COUNT|Property::METRIC_AV_RESYNC_COUNT, PropertyValue with intValue=0|status.isOk() should be true, return false (read-only)|Should Pass|
 */
UT_ADD_TEST(VTS_L1_AVClock, setProperty_pos)
{
    std::vector<com::rdk::hal::avclock::IAVClock::Id> clockIds;
    android::sp<com::rdk::hal::avclock::IAVClock> avClock;
    ::android::binder::Status status;

    /* List of valid properties to test (all read-only) */
    std::vector<com::rdk::hal::avclock::Property> validProperties = {
        com::rdk::hal::avclock::Property::RESOURCE_ID,
        com::rdk::hal::avclock::Property::METRIC_AV_SYNC_ACCURACY_MS,
        com::rdk::hal::avclock::Property::METRIC_AV_RESYNC_COUNT
    };

    /* Get the AV Clock IDs */
    AVCLOCK_MANAGER_GETAVCLOCKIDS(clockIds);

    for (const auto& _clockId : clockIds)
    {
        /* Get AV Clock handle */
        AVCLOCK_MANAGER_GETAVCLOCK_SUCCESS(_clockId, avClock);

        /* Test setProperty for each valid property */
        for (const auto& _property : validProperties)
        {
            com::rdk::hal::PropertyValue propertyValue;
            propertyValue.value = com::rdk::hal::PropertyValue::Value::make<com::rdk::hal::PropertyValue::Value::intValue>(1); // Example value
            bool result = false;
            AVCLOCK_SET_PROPERTY_SUCCESS(avClock, _property, propertyValue, result);

            /* Validate the result (false for read-only properties) */
            UT_LOG_INFO("setProperty for property %s, clock ID %d: result=%s",
                        com::rdk::hal::avclock::toString(_property).c_str(),
                        _clockId.value,
                        result ? "true" : "false");
            UT_ASSERT_FALSE(result); // Expect false for read-only properties
        }
    }
}

/**
 * @brief Test the AV Clock's setProperty function for invalid inputs.
 *
 * This test verifies if the setProperty function handles invalid property keys or values correctly by returning false.
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
 * |Variation / Step|Description|Test Data|Expected Result|Notes|
 * |----------------|-----------|---------|---------------|-----|
 * |01|Retrieve available AV Clock IDs|None|clockIds should not be empty, status.isOk() should be true|Should Pass|
 * |02|Get AV Clock handle for each ID|_clockId|avClock should be valid, status.isOk() should be true|Should Pass|
 * |03|Invoke setProperty with an invalid property key|Property=-1, PropertyValue with intValue=1|status.isOk() should be true, return false|Should Pass|
 * |04|Invoke setProperty with a null PropertyValue|Property::RESOURCE_ID, PropertyValue with null value|status.isOk() should be true, return false|Should Pass|
 */
UT_ADD_TEST(VTS_L1_AVClock, setProperty_neg)
{
    std::vector<com::rdk::hal::avclock::IAVClock::Id> clockIds;
    android::sp<com::rdk::hal::avclock::IAVClock> avClock;
    ::android::binder::Status status;

    /* Get the AV Clock IDs */
    AVCLOCK_MANAGER_GETAVCLOCKIDS(clockIds);

    for (const auto& _clockId : clockIds)
    {
        /* Get AV Clock handle */
        AVCLOCK_MANAGER_GETAVCLOCK_SUCCESS(_clockId, avClock);

        /* Test 1: Invalid property key */
        {
            com::rdk::hal::PropertyValue propertyValue;
            propertyValue.value = com::rdk::hal::PropertyValue::Value::make<com::rdk::hal::PropertyValue::Value::intValue>(1);
            com::rdk::hal::avclock::Property invalidProperty = static_cast<com::rdk::hal::avclock::Property>(-1);
            bool result = false;
            AVCLOCK_SET_PROPERTY_FAILURE(avClock, invalidProperty, propertyValue, result);

            UT_LOG_INFO("setProperty for invalid property %d, clock ID %d: result=%s",
                        invalidProperty, _clockId.value, result ? "true" : "false");
            UT_ASSERT_FALSE(result);
        }

        /* Test 2: Null PropertyValue */
        {
            com::rdk::hal::PropertyValue propertyValue;
            propertyValue.value = std::nullopt; // Null value
            com::rdk::hal::avclock::Property property = com::rdk::hal::avclock::Property::RESOURCE_ID;
            bool result = false;
            AVCLOCK_SET_PROPERTY_FAILURE(avClock, property, propertyValue, result);

            UT_LOG_INFO("setProperty for property %s with null value, clock ID %d: result=%s",
                        com::rdk::hal::avclock::toString(property).c_str(),
                        _clockId.value, result ? "true" : "false");
            UT_ASSERT_FALSE(result);
        }
    }
}
/**
 * @brief Positive test for successful open of AVClock
 *
 * This test verifies the successful opening of AVClock using the open API. It ensures that the controller is not null and the status is OK after the open call.
 *
 * **Test Group ID:** Basic: 01@n
 * **Test Case ID:** 009@n
 * **Priority:** High@n
 * @n
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 * @n
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Verify mControllerListener is not null | mControllerListener = [0x%X] | Should be successful |  |
 * | 02 | Verify mAVClocks is not empty | mAVClocks.size() > 0 | Should be successful |  |
 * | 03 | Iterate over each clock in mAVClocks | clock = current clock | Should be successful |  |
 * | 04 | Log before calling open API | mControllerListener = [0x%X] | Should be successful |  |
 * | 05 | Call open API with mControllerListener | mControllerListener = [0x%X], controller = nullptr | status.isOk() = true, controller != nullptr | Should Pass |
 * | 06 | Log the result of open API call | mControllerListener = [0x%X], controller = [0x%X] | Should be successful |  |
 * | 07 | Call close API to teardown | controller = [0x%X], closed = false | status.isOk() = true, closed = true | Done by Pre-requisite TearDown function |
 */
UT_ADD_TEST(VTS_L1_AVClock, open_pos)
{
    std::vector<com::rdk::hal::avclock::IAVClock::Id> clockIds;
    android::sp<com::rdk::hal::avclock::IAVClock> avClock;

    AVCLOCK_MANAGER_GETAVCLOCKIDS(clockIds);

    for (const auto& _clockId : clockIds)
    {
        AVCLOCK_MANAGER_GETAVCLOCK_SUCCESS(_clockId, avClock);

        android::sp<com::rdk::hal::avclock::IAVClockController> avClockController;
        android::sp<com::rdk::hal::avclock::IAVClockControllerListener> controllerListener = new AVClockControllerListener();

        AVCLOCK_OPEN_SUCCESS(avClock, controllerListener, avClockController, true);
        AVCLOCK_CLOSE_SUCCESS(avClock, avClockController, true);
    }
}

/**
 * @brief Negative test for opening AVClock with a null listener
 *
 * This test checks the behavior of the AVClock open function when provided with a null listener. The purpose is to ensure that the function correctly handles invalid input by returning an appropriate error status.
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
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Initialize AVClock controller | controller = nullptr | controller should be nullptr | Should be successful |
 * | 02 | Call open function with null listener | avClockControllerListener = nullptr, _aidl_return = nullptr | status.isOk() should be false, status.exceptionCode() should be EX_NULL_POINTER | Should Fail |
 * | 03 | Log the result of open function | status.isOk(), status.exceptionCode() | Log should show status.isOk as false and exceptionCode as EX_NULL_POINTER | Should be successful |
 * | 04 | Assert that controller is nullptr | controller = nullptr | Assertion should pass | Should Pass |
 * | 05 | Assert that exception code is EX_NULL_POINTER | status.exceptionCode() = EX_NULL_POINTER | Assertion should pass | Should Pass |
 */
UT_ADD_TEST(VTS_L1_AVClock, open_neg)
{
    std::vector<com::rdk::hal::avclock::IAVClock::Id> clockIds;
    android::sp<com::rdk::hal::avclock::IAVClock> avClock;
    android::sp<com::rdk::hal::avclock::IAVClockController> avClockController_nullptr = nullptr;
    android::sp<com::rdk::hal::avclock::IAVClockControllerListener> controllerListener = new AVClockControllerListener();

    AVCLOCK_MANAGER_GETAVCLOCKIDS(clockIds);

    for (const auto& _clockId : clockIds)
    {
        AVCLOCK_MANAGER_GETAVCLOCK_SUCCESS(_clockId, avClock);

        /* Null listener */
        AVCLOCK_OPEN_FAILURE(avClock, nullptr, avClockController_nullptr, Status::EX_NULL_POINTER);

        /* Open successfully */
        android::sp<com::rdk::hal::avclock::IAVClockController> avClockController;
        AVCLOCK_OPEN_SUCCESS(avClock, controllerListener, avClockController, true);

        /* Open again (already open) */
        AVCLOCK_OPEN_FAILURE(avClock, controllerListener, avClockController_nullptr, Status::EX_ILLEGAL_STATE);

        AVCLOCK_CLOSE_SUCCESS(avClock, avClockController, true);
    }
}

/**
 * @brief Test the AV Clock's close function in the READY state.
 *
 * This test verifies if the close function successfully transitions the AV Clock from READY to CLOSING and then CLOSED.
 *
 * **Test Group ID:** Basic: 01@n
 * **Test Case ID:** 009@n
 * **Priority:** High@n
 * @n
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 * @n
 * **Test Procedure:**@n
 * |Variation / Step|Description|Test Data|Expected Result|Notes|
 * |----------------|-----------|---------|---------------|-----|
 * |01|Retrieve available AV Clock IDs|None|clockIds should not be empty, status.isOk() should be true|Should Pass|
 * |02|Get AV Clock handle for each ID|_clockId|avClock should be valid, status.isOk() should be true|Should Pass|
 * |03|Open the AV Clock|IAVClockControllerListener|avClockController should be valid, status.isOk() should be true, state transitions to READY|Should Pass|
 * |04|Close the AV Clock|avClockController|status.isOk() should be true, return true, state transitions to CLOSING then CLOSED|Should Pass|
 */
UT_ADD_TEST(VTS_L1_AVClock, close_pos)
{
    std::vector<com::rdk::hal::avclock::IAVClock::Id> clockIds;
    android::sp<com::rdk::hal::avclock::IAVClock> avClock;
    ::android::binder::Status status;

    /* Get the AV Clock IDs */
    AVCLOCK_MANAGER_GETAVCLOCKIDS(clockIds);

    for (const auto& _clockId : clockIds)
    {
        /* Get AV Clock handle */
        AVCLOCK_MANAGER_GETAVCLOCK_SUCCESS(_clockId, avClock);

        android::sp<com::rdk::hal::avclock::IAVClockController> avClockController;
        android::sp<com::rdk::hal::avclock::IAVClockControllerListener> controllerListener = new AVClockControllerListener();

        AVCLOCK_OPEN_SUCCESS(avClock, controllerListener, avClockController, true);

        /* Close the AV Clock */
        AVCLOCK_CLOSE_SUCCESS(avClock, avClockController, true);

        /* Validate state transition (handled by AVCLOCK_CLOSE_SUCCESS) */
        UT_LOG_INFO("close for clock ID %d: success", _clockId.value);
    }
}

/**
 * @brief Test the AV Clock's close function in an invalid state or with invalid parameters.
 *
 * This test verifies if the close function handles invalid states or parameters correctly by throwing EX_ILLEGAL_STATE or EX_ILLEGAL_ARGUMENT.
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
 * |Variation / Step|Description|Test Data|Expected Result|Notes|
 * |----------------|-----------|---------|---------------|-----|
 * |01|Retrieve available AV Clock IDs|None|clockIds should not be empty, status.isOk() should be true|Should Pass|
 * |02|Get AV Clock handle for each ID|_clockId|avClock should be valid, status.isOk() should be true|Should Pass|
 * |03|Open the AV Clock|IAVClockControllerListener|avClockController should be valid, status.isOk() should be true, state transitions to READY|Should Pass|
 * |04|Close the AV Clock to reach CLOSED|avClockController|status.isOk() should be true, return true, state transitions to CLOSED|Should Pass|
 * |05|Invoke close in CLOSED state|Valid IAVClockController|status.isOk() should be false, exception EX_ILLEGAL_STATE, result false|Should Pass|
 * |06|Invoke close with null IAVClockController|Null IAVClockController|status.isOk() should be false, exception EX_ILLEGAL_ARGUMENT, result false|Should Pass|
 */
UT_ADD_TEST(VTS_L1_AVClock, close_neg)
{
    std::vector<com::rdk::hal::avclock::IAVClock::Id> clockIds;
    android::sp<com::rdk::hal::avclock::IAVClock> avClock;
    ::android::binder::Status status;

    /* Get the AV Clock IDs */
    AVCLOCK_MANAGER_GETAVCLOCKIDS(clockIds);

    for (const auto& _clockId : clockIds)
    {
        /* Get AV Clock handle */
        AVCLOCK_MANAGER_GETAVCLOCK_SUCCESS(_clockId, avClock);

        /* Test 1: Close in CLOSED state */
        {
            android::sp<com::rdk::hal::avclock::IAVClockController> avClockController;
            android::sp<com::rdk::hal::avclock::IAVClockControllerListener> controllerListener = new AVClockControllerListener();

            AVCLOCK_OPEN_SUCCESS(avClock, controllerListener, avClockController, true);

            /* Close the AV Clock to reach CLOSED */
            AVCLOCK_CLOSE_SUCCESS(avClock, avClockController, true);

            bool result = false;
            AVCLOCK_CLOSE_FAILURE(avClock, avClockController, result);

            UT_LOG_INFO("close in CLOSED state for clock ID %d: result=%s", _clockId.value, result ? "true" : "false");
            UT_ASSERT_FALSE(result);
        }

        /*TODO this */
        /* Test 2: Close with null IAVClockController
        {
            android::sp<com::rdk::hal::avclock::IAVClockController> nullController = nullptr;
            bool result = false;
            AVCLOCK_CLOSE_FAILURE_NULL(avClock, nullController, result);

            UT_LOG_INFO("close with null controller for clock ID %d: result=%s", _clockId.value, result ? "true" : "false");
            UT_ASSERT_FALSE(result);
        }*/
    }
}

/**
 * @brief Test the AV Clock's registerEventListener function in valid scenarios.
 *
 * This test verifies if the registerEventListener function successfully registers an event listener.
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
 * |Variation / Step|Description|Test Data|Expected Result|Notes|
 * |----------------|-----------|---------|---------------|-----|
 * |01|Retrieve available AV Clock IDs|None|clockIds should not be empty, status.isOk() should be true|Should Pass|
 * |02|Get AV Clock handle for each ID|_clockId|avClock should be valid, status.isOk() should be true|Should Pass|
 * |03|Register an event listener|IAVClockEventListener|status.isOk() should be true, return true|Should Pass|
 */
UT_ADD_TEST(VTS_L1_AVClock, registerEventListener_pos)
{
    std::vector<com::rdk::hal::avclock::IAVClock::Id> clockIds;
    android::sp<com::rdk::hal::avclock::IAVClock> avClock;
    ::android::binder::Status status;

    /* Get the AV Clock IDs */
    AVCLOCK_MANAGER_GETAVCLOCKIDS(clockIds);

    for (const auto& _clockId : clockIds)
    {
        /* Get AV Clock handle */
        AVCLOCK_MANAGER_GETAVCLOCK_SUCCESS(_clockId, avClock);

        /* Register an event listener */
        android::sp<com::rdk::hal::avclock::IAVClockEventListener> eventListener = new AVClockEventListener();
        AVCLOCK_REGISTER_EVENT_LISTENER_SUCCESS(avClock, eventListener);

        UT_LOG_INFO("registerEventListener for clock ID %d: success", _clockId.value);
    }
}

/**
 * @brief Test the AV Clock's registerEventListener function in invalid scenarios.
 *
 * This test verifies if the registerEventListener function handles invalid cases correctly by returning false or throwing EX_ILLEGAL_ARGUMENT.
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
 * |Variation / Step|Description|Test Data|Expected Result|Notes|
 * |----------------|-----------|---------|---------------|-----|
 * |01|Retrieve available AV Clock IDs|None|clockIds should not be empty, status.isOk() should be true|Should Pass|
 * |02|Get AV Clock handle for each ID|_clockId|avClock should be valid, status.isOk() should be true|Should Pass|
 * |03|Register an event listener|IAVClockEventListener|status.isOk() should be true, return true|Should Pass|
 * |04|Register the same event listener again|Same IAVClockEventListener|status.isOk() should be true, return false|Should Pass|
 * |05|Register a null event listener|Null IAVClockEventListener|status.isOk() should be false, exception EX_ILLEGAL_ARGUMENT, return false|Should Pass|
 */
UT_ADD_TEST(VTS_L1_AVClock, registerEventListener_neg)
{
    std::vector<com::rdk::hal::avclock::IAVClock::Id> clockIds;
    android::sp<com::rdk::hal::avclock::IAVClock> avClock;
    ::android::binder::Status status;

    /* Get the AV Clock IDs */
    AVCLOCK_MANAGER_GETAVCLOCKIDS(clockIds);

    for (const auto& _clockId : clockIds)
    {
        /* Get AV Clock handle */
        AVCLOCK_MANAGER_GETAVCLOCK_SUCCESS(_clockId, avClock);

        /* Test 1: Register an event listener and attempt to register it again */
        {
            android::sp<com::rdk::hal::avclock::IAVClockEventListener> eventListener = new AVClockEventListener();
            AVCLOCK_REGISTER_EVENT_LISTENER_SUCCESS(avClock, eventListener);

            /* Attempt to register the same listener again */
            AVCLOCK_REGISTER_EVENT_LISTENER_FAILURE(avClock, eventListener);

            UT_LOG_INFO("registerEventListener (already registered) for clock ID %d: failed as expected", _clockId.value);
        }
        /*TODO this testcase*/
        /* Test 2: Register a null event listener
        {
            android::sp<com::rdk::hal::avclock::IAVClockEventListener> nullListener = nullptr;
            AVCLOCK_REGISTER_EVENT_LISTENER_NULL_FAILURE(avClock, nullListener);

            UT_LOG_INFO("registerEventListener (null listener) for clock ID %d: failed as expected", _clockId.value);
        }*/
    }
}

/**
 * @brief Test the AV Clock's unregisterEventListener function in valid scenarios.
 *
 * This test verifies if the unregisterEventListener function successfully unregisters a registered event listener.
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
 * |Variation / Step|Description|Test Data|Expected Result|Notes|
 * |----------------|-----------|---------|---------------|-----|
 * |01|Retrieve available AV Clock IDs|None|clockIds should not be empty, status.isOk() should be true|Should Pass|
 * |02|Get AV Clock handle for each ID|_clockId|avClock should be valid, status.isOk() should be true|Should Pass|
 * |03|Register an event listener|IAVClockEventListener|status.isOk() should be true, return true|Should Pass|
 * |04|Unregister the event listener|IAVClockEventListener|status.isOk() should be true, return true|Should Pass|
 */
UT_ADD_TEST(VTS_L1_AVClock, unregisterEventListener_pos)
{
    std::vector<com::rdk::hal::avclock::IAVClock::Id> clockIds;
    android::sp<com::rdk::hal::avclock::IAVClock> avClock;
    ::android::binder::Status status;

    /* Get the AV Clock IDs */
    AVCLOCK_MANAGER_GETAVCLOCKIDS(clockIds);

    for (const auto& _clockId : clockIds)
    {
        /* Get AV Clock handle */
        AVCLOCK_MANAGER_GETAVCLOCK_SUCCESS(_clockId, avClock);

        /* Register and unregister an event listener */
        android::sp<com::rdk::hal::avclock::IAVClockEventListener> eventListener = new AVClockEventListener();
        AVCLOCK_REGISTER_EVENT_LISTENER_SUCCESS(avClock, eventListener);

        AVCLOCK_UNREGISTER_EVENT_LISTENER_SUCCESS(avClock, eventListener);

        UT_LOG_INFO("unregisterEventListener for clock ID %d: success", _clockId.value);
    }
}

/**
 * @brief Test the AV Clock's unregisterEventListener function in invalid scenarios.
 *
 * This test verifies if the unregisterEventListener function handles invalid cases correctly by returning false or throwing EX_ILLEGAL_ARGUMENT.
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
 * |Variation / Step|Description|Test Data|Expected Result|Notes|
 * |----------------|-----------|---------|---------------|-----|
 * |01|Retrieve available AV Clock IDs|None|clockIds should not be empty, status.isOk() should be true|Should Pass|
 * |02|Get AV Clock handle for each ID|_clockId|avClock should be valid, status.isOk() should be true|Should Pass|
 * |03|Unregister an event listener that was not registered|IAVClockEventListener|status.isOk() should be true, return false|Should Pass|
 * |04|Unregister a null event listener|Null IAVClockEventListener|status.isOk() should be false, exception EX_ILLEGAL_ARGUMENT, return false|Should Pass|
 */
UT_ADD_TEST(VTS_L1_AVClock, unregisterEventListener_neg)
{
    std::vector<com::rdk::hal::avclock::IAVClock::Id> clockIds;
    android::sp<com::rdk::hal::avclock::IAVClock> avClock;
    ::android::binder::Status status;

    /* Get the AV Clock IDs */
    AVCLOCK_MANAGER_GETAVCLOCKIDS(clockIds);

    for (const auto& _clockId : clockIds)
    {
        /* Get AV Clock handle */
        AVCLOCK_MANAGER_GETAVCLOCK_SUCCESS(_clockId, avClock);

        /* Test 1: Unregister an event listener that was not registered */
        {
            android::sp<com::rdk::hal::avclock::IAVClockEventListener> eventListener = new AVClockEventListener();
            AVCLOCK_UNREGISTER_EVENT_LISTENER_FAILURE(avClock, eventListener);

            UT_LOG_INFO("unregisterEventListener (not registered) for clock ID %d: failed as expected", _clockId.value);
        }

        /*TODO this testcase */
        /* Test 2: Unregister a null event listener
        {
            android::sp<com::rdk::hal::avclock::IAVClockEventListener> nullListener = nullptr;
            AVCLOCK_UNREGISTER_EVENT_LISTENER_NULL_FAILURE(avClock, nullListener);

            UT_LOG_INFO("unregisterEventListener (null listener) for clock ID %d: failed as expected", _clockId.value);
        }*/
    }
}

/**
 * @brief Test the AV Clock's start function in valid scenarios.
 *
 * This test verifies if the start function successfully transitions the AV Clock from READY to STARTING and then STARTED.
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
 * |Variation / Step|Description|Test Data|Expected Result|Notes|
 * |----------------|-----------|---------|---------------|-----|
 * |01|Retrieve available AV Clock IDs|None|clockIds should not be empty, status.isOk() should be true|Should Pass|
 * |02|Get AV Clock handle for each ID|_clockId|avClock should be valid, status.isOk() should be true|Should Pass|
 * |03|Open the AV Clock|IAVClockControllerListener|avClockController should be valid, status.isOk() should be true, state transitions to READY|Should Pass|
 * |04|Start the AV Clock|None|status.isOk() should be true, state transitions to STARTING then STARTED|Should Pass|
 */
UT_ADD_TEST(VTS_L1_AVClock, start_pos)
{
    std::vector<com::rdk::hal::avclock::IAVClock::Id> clockIds;
    android::sp<com::rdk::hal::avclock::IAVClock> avClock;
    ::android::binder::Status status;

    /* Get the AV Clock IDs */
    AVCLOCK_MANAGER_GETAVCLOCKIDS(clockIds);

    for (const auto& _clockId : clockIds)
    {
        /* Get AV Clock handle */
        AVCLOCK_MANAGER_GETAVCLOCK_SUCCESS(_clockId, avClock);

        /* Open the AV Clock to reach READY */
        android::sp<com::rdk::hal::avclock::IAVClockController> avClockController;
        android::sp<com::rdk::hal::avclock::IAVClockControllerListener> controllerListener = new AVClockControllerListener();
        AVCLOCK_OPEN_SUCCESS(avClock, controllerListener, avClockController, true);

        /* Start the AV Clock */
        AVCLOCK_START_SUCCESS(avClock, avClockController);

        UT_LOG_INFO("start for clock ID %d: success", _clockId.value);
    }
}

/**
 * @brief Test the AV Clock's start function in invalid scenarios.
 *
 * This test verifies if the start function handles invalid states correctly by throwing EX_ILLEGAL_STATE.
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
 * |Variation / Step|Description|Test Data|Expected Result|Notes|
 * |----------------|-----------|---------|---------------|-----|
 * |01|Retrieve available AV Clock IDs|None|clockIds should not be empty, status.isOk() should be true|Should Pass|
 * |02|Get AV Clock handle for each ID|_clockId|avClock should be valid, status.isOk() should be true|Should Pass|
 * |03|Start the AV Clock in CLOSED state|None|status.isOk() should be false, exception EX_ILLEGAL_STATE|Should Pass|
 * |04|Open the AV Clock|IAVClockControllerListener|avClockController should be valid, status.isOk() should be true, state transitions to READY|Should Pass|
 * |05|Start the AV Clock|None|status.isOk() should be true, state transitions to STARTING then STARTED|Should Pass|
 * |06|Start the AV Clock again in STARTED state|None|status.isOk() should be false, exception EX_ILLEGAL_STATE|Should Pass|
 */
UT_ADD_TEST(VTS_L1_AVClock, start_neg)
{
    std::vector<com::rdk::hal::avclock::IAVClock::Id> clockIds;
    android::sp<com::rdk::hal::avclock::IAVClock> avClock;
    ::android::binder::Status status;

    /* Get the AV Clock IDs */
    AVCLOCK_MANAGER_GETAVCLOCKIDS(clockIds);

    for (const auto& _clockId : clockIds)
    {
        /* Get AV Clock handle */
        AVCLOCK_MANAGER_GETAVCLOCK_SUCCESS(_clockId, avClock);

        /* Test 1: Start in CLOSED state */
        {
            android::sp<com::rdk::hal::avclock::IAVClockController> avClockController;
            android::sp<com::rdk::hal::avclock::IAVClockControllerListener> controllerListener = new AVClockControllerListener();
            AVCLOCK_OPEN_SUCCESS(avClock, controllerListener, avClockController, true);

            /* Close the AV Clock to ensure CLOSED state */
            AVCLOCK_CLOSE_SUCCESS(avClock, avClockController, true);

            AVCLOCK_START_FAILURE(avClock, avClockController);

            UT_LOG_INFO("start in CLOSED state for clock ID %d: failed as expected", _clockId.value);
        }

        #if 0 /*TODO*/
        /* Test 2: Start in STARTED state */
        {
            android::sp<com::rdk::hal::avclock::IAVClockController> avClockController;
            android::sp<com::rdk::hal::avclock::IAVClockControllerListener> controllerListener = new AVClockControllerListener();
            AVCLOCK_OPEN_SUCCESS(avClock, controllerListener, avClockController, true);

            /* Start the AV Clock to reach STARTED */
            AVCLOCK_START_SUCCESS(avClock, avClockController);

            /* Attempt to start again in STARTED state */
            AVCLOCK_START_FAILURE(avClock, avClockController);

            UT_LOG_INFO("start in STARTED state for clock ID %d: failed as expected", _clockId.value);
        }
        #endif
    }
}

/**
 * @brief Test the AV Clock's stop function in valid scenarios.
 *
 * This test verifies if the stop function successfully transitions the AV Clock from STARTED to STOPPING and then READY.
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
 * |Variation / Step|Description|Test Data|Expected Result|Notes|
 * |----------------|-----------|---------|---------------|-----|
 * |01|Retrieve available AV Clock IDs|None|clockIds should not be empty, status.isOk() should be true|Should Pass|
 * |02|Get AV Clock handle for each ID|_clockId|avClock should be valid, status.isOk() should be true|Should Pass|
 * |03|Open the AV Clock|IAVClockControllerListener|avClockController should be valid, status.isOk() should be true, state transitions to READY|Should Pass|
 * |04|Start the AV Clock|None|status.isOk() should be true, state transitions to STARTING then STARTED|Should Pass|
 * |05|Stop the AV Clock|None|status.isOk() should be true, state transitions to STOPPING then READY|Should Pass|
 */
UT_ADD_TEST(VTS_L1_AVClock, stop_pos)
{
    std::vector<com::rdk::hal::avclock::IAVClock::Id> clockIds;
    android::sp<com::rdk::hal::avclock::IAVClock> avClock;
    ::android::binder::Status status;

    /* Get the AV Clock IDs */
    AVCLOCK_MANAGER_GETAVCLOCKIDS(clockIds);

    for (const auto& _clockId : clockIds)
    {
        /* Get AV Clock handle */
        AVCLOCK_MANAGER_GETAVCLOCK_SUCCESS(_clockId, avClock);

        /* Open the AV Clock to reach READY */
        android::sp<com::rdk::hal::avclock::IAVClockController> avClockController;
        android::sp<com::rdk::hal::avclock::IAVClockControllerListener> controllerListener = new AVClockControllerListener();
        AVCLOCK_OPEN_SUCCESS(avClock, controllerListener, avClockController, true);

        /* Start the AV Clock to reach STARTED */
        AVCLOCK_START_SUCCESS(avClock, avClockController);

        /* Stop the AV Clock */
        AVCLOCK_STOP_SUCCESS(avClock, avClockController);

        UT_LOG_INFO("stop for clock ID %d: success", _clockId.value);
    }
}

/**
 * @brief Test the AV Clock's stop function in invalid scenarios.
 *
 * This test verifies if the stop function handles invalid states correctly by throwing EX_ILLEGAL_STATE.
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
 * |Variation / Step|Description|Test Data|Expected Result|Notes|
 * |----------------|-----------|---------|---------------|-----|
 * |01|Retrieve available AV Clock IDs|None|clockIds should not be empty, status.isOk() should be true|Should Pass|
 * |02|Get AV Clock handle for each ID|_clockId|avClock should be valid, status.isOk() should be true|Should Pass|
 * |03|Stop the AV Clock in CLOSED state|None|status.isOk() should be false, exception EX_ILLEGAL_STATE|Should Pass|
 * |04|Open the AV Clock|IAVClockControllerListener|avClockController should be valid, status.isOk() should be true, state transitions to READY|Should Pass|
 * |05|Stop the AV Clock in READY state|None|status.isOk() should be false, exception EX_ILLEGAL_STATE|Should Pass|
 */
UT_ADD_TEST(VTS_L1_AVClock, stop_neg)
{
    std::vector<com::rdk::hal::avclock::IAVClock::Id> clockIds;
    android::sp<com::rdk::hal::avclock::IAVClock> avClock;
    ::android::binder::Status status;

    /* Get the AV Clock IDs */
    AVCLOCK_MANAGER_GETAVCLOCKIDS(clockIds);

    for (const auto& _clockId : clockIds)
    {
        /* Get AV Clock handle */
        AVCLOCK_MANAGER_GETAVCLOCK_SUCCESS(_clockId, avClock);

        /* Test 1: Stop in CLOSED state */
        {
            android::sp<com::rdk::hal::avclock::IAVClockController> avClockController;
            android::sp<com::rdk::hal::avclock::IAVClockControllerListener> controllerListener = new AVClockControllerListener();
            AVCLOCK_OPEN_SUCCESS(avClock, controllerListener, avClockController, true);

            /* Close the AV Clock to ensure CLOSED state */
            AVCLOCK_CLOSE_SUCCESS(avClock, avClockController, true);

            /* Create a new controller for the stop call */
            avClockController = nullptr;
            controllerListener = new AVClockControllerListener();
            AVCLOCK_OPEN_SUCCESS(avClock, controllerListener, avClockController, false); // No state check, as state is CLOSED

            AVCLOCK_STOP_FAILURE(avClock, avClockController);

            UT_LOG_INFO("stop in CLOSED state for clock ID %d: failed as expected", _clockId.value);
        }

        /*TODO */
        /* Test 2: Stop in READY state
        {
            android::sp<com::rdk::hal::avclock::IAVClockController> avClockController;
            android::sp<com::rdk::hal::avclock::IAVClockControllerListener> controllerListener = new AVClockControllerListener();
            AVCLOCK_OPEN_SUCCESS(avClock, controllerListener, avClockController, true);
           
            AVCLOCK_STOP_FAILURE(avClock, avClockController);

            UT_LOG_INFO("stop in READY state for clock ID %d: failed as expected", _clockId.value);
        }*/
    }
}

/**
 * @brief Test the AV Clock's setAudioSink function in valid scenarios.
 *
 * This test verifies if the setAudioSink function successfully sets valid audio sink IDs in READY and STARTED states.
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
 * |Variation / Step|Description|Test Data|Expected Result|Notes|
 * |----------------|-----------|---------|---------------|-----|
 * |01|Retrieve available AV Clock IDs|None|clockIds should not be empty, status.isOk() should be true|Should Pass|
 * |02|Get AV Clock handle for each ID|_clockId|avClock should be valid, status.isOk() should be true|Should Pass|
 * |03|Open the AV Clock|IAVClockControllerListener|avClockController should be valid, status.isOk() should be true, state transitions to READY|Should Pass|
 * |04|Set audio sink to UNDEFINED in READY state|IAudioSink.Id.UNDEFINED (-1)|status.isOk() should be true, return true|Should Pass|
 * |05|Set audio sink to valid ID in READY state|IAudioSink.Id (0)|status.isOk() should be true, return true|Should Pass|
 * |06|Start the AV Clock|None|status.isOk() should be true, state transitions to STARTING then STARTED|Should Pass|
 * |07|Set audio sink to valid ID in STARTED state|IAudioSink.Id (1)|status.isOk() should be true, return true|Should Pass|
 */
UT_ADD_TEST(VTS_L1_AVClock, setAudioSink_pos)
{
    std::vector<com::rdk::hal::avclock::IAVClock::Id> clockIds;
    android::sp<com::rdk::hal::avclock::IAVClock> avClock;
    ::android::binder::Status status;

    /* Get the AV Clock IDs */
    AVCLOCK_MANAGER_GETAVCLOCKIDS(clockIds);

    for (const auto& _clockId : clockIds)
    {
        /* Get AV Clock handle */
        AVCLOCK_MANAGER_GETAVCLOCK_SUCCESS(_clockId, avClock);

        /* Open the AV Clock to reach READY */
        android::sp<com::rdk::hal::avclock::IAVClockController> avClockController;
        android::sp<com::rdk::hal::avclock::IAVClockControllerListener> controllerListener = new AVClockControllerListener();
        AVCLOCK_OPEN_SUCCESS(avClock, controllerListener, avClockController, true);

        /* Test 1: Set audio sink to UNDEFINED in READY state */
        {
            com::rdk::hal::audiosink::IAudioSink::Id audioSinkId;
            audioSinkId.value = -1; // IAudioSink.Id.UNDEFINED
            AVCLOCK_SET_AUDIO_SINK_SUCCESS(avClock, avClockController, audioSinkId, true);
            UT_LOG_INFO("setAudioSink to UNDEFINED for clock ID %d in READY: success", _clockId.value);
        }

        /* Test 2: Set audio sink to valid ID in READY state */
        {
            com::rdk::hal::audiosink::IAudioSink::Id audioSinkId;
            audioSinkId.value = 0; // Valid ID
            AVCLOCK_SET_AUDIO_SINK_SUCCESS(avClock, avClockController, audioSinkId, true);
            UT_LOG_INFO("setAudioSink to ID 0 for clock ID %d in READY: success", _clockId.value);
        }

        /* Test 3: Set audio sink to valid ID in STARTED state */
        {
            AVCLOCK_START_SUCCESS(avClock, avClockController);
            com::rdk::hal::audiosink::IAudioSink::Id audioSinkId;
            audioSinkId.value = 1; // Valid ID
            AVCLOCK_SET_AUDIO_SINK_SUCCESS(avClock, avClockController, audioSinkId, true);
            UT_LOG_INFO("setAudioSink to ID 1 for clock ID %d in STARTED: success", _clockId.value);
        }
    }
}

/**
 * @brief Test the AV Clock's setAudioSink function in invalid scenarios.
 *
 * This test verifies if the setAudioSink function handles invalid IDs and states correctly.
 *
 * **Test Group ID:** Basic: 01@n
 * **Test Case ID:** 020@n
 * **Priority:** High@n
 * @n
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 * @n
 * **Test Procedure:**@n
 * |Variation / Step|Description|Test Data|Expected Result|Notes|
 * |----------------|-----------|---------|---------------|-----|
 * |01|Retrieve available AV Clock IDs|None|clockIds should not be empty, status.isOk() should be true|Should Pass|
 * |02|Get AV Clock handle for each ID|_clockId|avClock should be valid, status.isOk() should be true|Should Pass|
 * |03|Open the AV Clock|IAVClockControllerListener|avClockController should be valid, status.isOk() should be true, state transitions to READY|Should Pass|
 * |04|Close the AV Clock|avClockController|status.isOk() should be true, state transitions to CLOSED|Should Pass|
 * |05|Open a new controller|IAVClockControllerListener|avClockController should be valid, status.isOk() should be true|Should Pass|
 * |06|Set audio sink in CLOSED state|IAudioSink.Id (0)|status.isOk() should be false, exception EX_ILLEGAL_STATE, return false|Should Pass|
 * |07|Open the AV Clock again|IAVClockControllerListener|avClockController should be valid, status.isOk() should be true, state transitions to READY|Should Pass|
 * |08|Set audio sink to invalid ID in READY state|IAudioSink.Id (NUM_AUDIO_SINK + 1)|status.isOk() should be true, return false|Should Pass|
 */
UT_ADD_TEST(VTS_L1_AVClock, setAudioSink_neg)
{
    std::vector<com::rdk::hal::avclock::IAVClock::Id> clockIds;
    android::sp<com::rdk::hal::avclock::IAVClock> avClock;
    ::android::binder::Status status;

    /* Get the AV Clock IDs */
    AVCLOCK_MANAGER_GETAVCLOCKIDS(clockIds);

    for (const auto& _clockId : clockIds)
    {
        /* Get AV Clock handle */
        AVCLOCK_MANAGER_GETAVCLOCK_SUCCESS(_clockId, avClock);

        /* Test 1: Set audio sink in CLOSED state */
        {
            android::sp<com::rdk::hal::avclock::IAVClockController> avClockController;
            android::sp<com::rdk::hal::avclock::IAVClockControllerListener> controllerListener = new AVClockControllerListener();
            AVCLOCK_OPEN_SUCCESS(avClock, controllerListener, avClockController, true);

            /* Close the AV Clock to reach CLOSED */
            AVCLOCK_CLOSE_SUCCESS(avClock, avClockController, true);

            com::rdk::hal::audiosink::IAudioSink::Id audioSinkId;
            audioSinkId.value = 0; // Valid ID
            AVCLOCK_SET_AUDIO_SINK_FAILURE_STATE(avClock, avClockController, audioSinkId);

            UT_LOG_INFO("setAudioSink in CLOSED state for clock ID %d: failed as expected", _clockId.value);
        }

        /*TODO*/
        /* Test 2: Set audio sink to invalid ID in READY state
        {
            android::sp<com::rdk::hal::avclock::IAVClockController> avClockController;
            android::sp<com::rdk::hal::avclock::IAVClockControllerListener> controllerListener = new AVClockControllerListener();
            AVCLOCK_OPEN_SUCCESS(avClock, controllerListener, avClockController, true);

            com::rdk::hal::audiosink::IAudioSink::Id audioSinkId;
            audioSinkId.value = NUM_AUDIO_SINK + 1; // Invalid ID
            AVCLOCK_SET_AUDIO_SINK_FAILURE_INVALID_ID(avClock, avClockController, audioSinkId);

            UT_LOG_INFO("setAudioSink to invalid ID %d for clock ID %d in READY: failed as expected", audioSinkId.value, _clockId.value);
        }*/
    }
}

/**
 * @brief Test the AV Clock's getAudioSink function in valid scenarios.
 *
 * This test verifies if the getAudioSink function correctly returns the set audio sink ID in READY and STARTED states.
 *
 * **Test Group ID:** Basic: 01@n
 * **Test Case ID:** 021@n
 * **Priority:** High@n
 * @n
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 * @n
 * **Test Procedure:**@n
 * |Variation / Step|Description|Test Data|Expected Result|Notes|
 * |----------------|-----------|---------|---------------|-----|
 * |01|Retrieve available AV Clock IDs|None|clockIds should not be empty, status.isOk() should be true|Should Pass|
 * |02|Get AV Clock handle for each ID|_clockId|avClock should be valid, status.isOk() should be true|Should Pass|
 * |03|Open the AV Clock|IAVClockControllerListener|avClockController should be valid, status.isOk() should be true, state transitions to READY|Should Pass|
 * |04|Get audio sink in READY state|None|status.isOk() should be true, return IAudioSink.Id.UNDEFINED (-1)|Should Pass|
 * |05|Set audio sink to valid ID in READY state|IAudioSink.Id (0)|status.isOk() should be true, return true|Should Pass|
 * |06|Get audio sink in READY state|None|status.isOk() should be true, return IAudioSink.Id (0)|Should Pass|
 * |07|Start the AV Clock|None|status.isOk() should be true, state transitions to STARTING then STARTED|Should Pass|
 * |08|Get audio sink in STARTED state|None|status.isOk() should be true, return IAudioSink.Id (0)|Should Pass|
 */
UT_ADD_TEST(VTS_L1_AVClock, getAudioSink_pos)
{
    std::vector<com::rdk::hal::avclock::IAVClock::Id> clockIds;
    android::sp<com::rdk::hal::avclock::IAVClock> avClock;
    ::android::binder::Status status;

    /* Get the AV Clock IDs */
    AVCLOCK_MANAGER_GETAVCLOCKIDS(clockIds);

    for (const auto& _clockId : clockIds)
    {
        /* Get AV Clock handle */
        AVCLOCK_MANAGER_GETAVCLOCK_SUCCESS(_clockId, avClock);

        /* Open the AV Clock to reach READY */
        android::sp<com::rdk::hal::avclock::IAVClockController> avClockController;
        android::sp<com::rdk::hal::avclock::IAVClockControllerListener> controllerListener = new AVClockControllerListener();
        AVCLOCK_OPEN_SUCCESS(avClock, controllerListener, avClockController, true);

        /* Test 1: Get audio sink in READY state (default UNDEFINED) */
        {
            com::rdk::hal::audiosink::IAudioSink::Id expectedAudioSinkId;
            expectedAudioSinkId.value = -1; // IAudioSink.Id.UNDEFINED
            AVCLOCK_GET_AUDIO_SINK_SUCCESS(avClock, avClockController, expectedAudioSinkId);
            UT_LOG_INFO("getAudioSink in READY state for clock ID %d: returned UNDEFINED", _clockId.value);
        }

        /* Test 2: Set and get audio sink in READY state */
        {
            com::rdk::hal::audiosink::IAudioSink::Id audioSinkId;
            audioSinkId.value = 0; // Valid ID
            AVCLOCK_SET_AUDIO_SINK_SUCCESS(avClock, avClockController, audioSinkId, true);

            com::rdk::hal::audiosink::IAudioSink::Id expectedAudioSinkId;
            expectedAudioSinkId.value = 0;
            AVCLOCK_GET_AUDIO_SINK_SUCCESS(avClock, avClockController, expectedAudioSinkId);
            UT_LOG_INFO("getAudioSink in READY state for clock ID %d: returned ID 0", _clockId.value);
        }

        /* Test 3: Get audio sink in STARTED state */
        {
            AVCLOCK_START_SUCCESS(avClock, avClockController);
            com::rdk::hal::audiosink::IAudioSink::Id expectedAudioSinkId;
            expectedAudioSinkId.value = 0;
            AVCLOCK_GET_AUDIO_SINK_SUCCESS(avClock, avClockController, expectedAudioSinkId);
            UT_LOG_INFO("getAudioSink in STARTED state for clock ID %d: returned ID 0", _clockId.value);
        }
    }
}

/**
 * @brief Test the AV Clock's getAudioSink function in invalid scenarios.
 *
 * This test verifies if the getAudioSink function handles invalid states correctly by throwing EX_ILLEGAL_STATE.
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
 * |Variation / Step|Description|Test Data|Expected Result|Notes|
 * |----------------|-----------|---------|---------------|-----|
 * |01|Retrieve available AV Clock IDs|None|clockIds should not be empty, status.isOk() should be true|Should Pass|
 * |02|Get AV Clock handle for each ID|_clockId|avClock should be valid, status.isOk() should be true|Should Pass|
 * |03|Open the AV Clock|IAVClockControllerListener|avClockController should be valid, status.isOk() should be true, state transitions to READY|Should Pass|
 * |04|Close the AV Clock|avClockController|status.isOk() should be true, state transitions to CLOSED|Should Pass|
 * |05|Open a new controller|IAVClockControllerListener|avClockController should be valid, status.isOk() should be true|Should Pass|
 * |06|Get audio sink in CLOSED state|None|status.isOk() should be false, exception EX_ILLEGAL_STATE|Should Pass|
 */
UT_ADD_TEST(VTS_L1_AVClock, getAudioSink_neg)
{
    std::vector<com::rdk::hal::avclock::IAVClock::Id> clockIds;
    android::sp<com::rdk::hal::avclock::IAVClock> avClock;
    ::android::binder::Status status;

    /* Get the AV Clock IDs */
    AVCLOCK_MANAGER_GETAVCLOCKIDS(clockIds);

    for (const auto& _clockId : clockIds)
    {
        /* Get AV Clock handle */
        AVCLOCK_MANAGER_GETAVCLOCK_SUCCESS(_clockId, avClock);

        /* Test 1: Get audio sink in CLOSED state */
        {
            android::sp<com::rdk::hal::avclock::IAVClockController> avClockController;
            android::sp<com::rdk::hal::avclock::IAVClockControllerListener> controllerListener = new AVClockControllerListener();
            AVCLOCK_OPEN_SUCCESS(avClock, controllerListener, avClockController, true);

            /* Close the AV Clock to reach CLOSED */
            AVCLOCK_CLOSE_SUCCESS(avClock, avClockController, true);

            AVCLOCK_GET_AUDIO_SINK_FAILURE_STATE(avClock, avClockController);

            UT_LOG_INFO("getAudioSink in CLOSED state for clock ID %d: failed as expected", _clockId.value);
        }
    }
}

/**
 * @brief Test the AV Clock's setSupplementaryAudioSink function in valid scenarios.
 *
 * This test verifies if the setSupplementaryAudioSink function successfully sets valid supplementary audio sink IDs in READY and STARTED states.
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
 * |Variation / Step|Description|Test Data|Expected Result|Notes|
 * |----------------|-----------|---------|---------------|-----|
 * |01|Retrieve available AV Clock IDs|None|clockIds should not be empty, status.isOk() should be true|Should Pass|
 * |02|Get AV Clock handle for each ID|_clockId|avClock should be valid, status.isOk() should be true|Should Pass|
 * |03|Open the AV Clock|IAVClockControllerListener|avClockController should be valid, status.isOk() should be true, state transitions to READY|Should Pass|
 * |04|Set supplementary audio sink to UNDEFINED in READY state|IAudioSink.Id.UNDEFINED (-1)|status.isOk() should be true, return true|Should Pass|
 * |05|Set supplementary audio sink to valid ID in READY state|IAudioSink.Id (0)|status.isOk() should be true, return true|Should Pass|
 * |06|Start the AV Clock|None|status.isOk() should be true, state transitions to STARTING then STARTED|Should Pass|
 * |07|Set supplementary audio sink to valid ID in STARTED state|IAudioSink.Id (1)|status.isOk() should be true, return true|Should Pass|
 */
UT_ADD_TEST(VTS_L1_AVClock, setSupplementaryAudioSink_pos)
{
    std::vector<com::rdk::hal::avclock::IAVClock::Id> clockIds;
    android::sp<com::rdk::hal::avclock::IAVClock> avClock;
    ::android::binder::Status status;

    /* Get the AV Clock IDs */
    AVCLOCK_MANAGER_GETAVCLOCKIDS(clockIds);

    for (const auto& _clockId : clockIds)
    {
        /* Get AV Clock handle */
        AVCLOCK_MANAGER_GETAVCLOCK_SUCCESS(_clockId, avClock);

        /* Open the AV Clock to reach READY */
        android::sp<com::rdk::hal::avclock::IAVClockController> avClockController;
        android::sp<com::rdk::hal::avclock::IAVClockControllerListener> controllerListener = new AVClockControllerListener();
        AVCLOCK_OPEN_SUCCESS(avClock, controllerListener, avClockController, true);

        /* Test 1: Set supplementary audio sink to UNDEFINED in READY state */
        {
            com::rdk::hal::audiosink::IAudioSink::Id supplementaryAudioSinkId;
            supplementaryAudioSinkId.value = -1; // IAudioSink.Id.UNDEFINED
            AVCLOCK_SET_SUPPLEMENTARY_AUDIO_SINK_SUCCESS(avClock, avClockController, supplementaryAudioSinkId, true);
            UT_LOG_INFO("setSupplementaryAudioSink to UNDEFINED for clock ID %d in READY: success", _clockId.value);
        }

        /* Test 2: Set supplementary audio sink to valid ID in READY state */
        {
            com::rdk::hal::audiosink::IAudioSink::Id supplementaryAudioSinkId;
            supplementaryAudioSinkId.value = 0; // Valid ID
            AVCLOCK_SET_SUPPLEMENTARY_AUDIO_SINK_SUCCESS(avClock, avClockController, supplementaryAudioSinkId, true);
            UT_LOG_INFO("setSupplementaryAudioSink to ID 0 for clock ID %d in READY: success", _clockId.value);
        }

        /* Test 3: Set supplementary audio sink to valid ID in STARTED state */
        {
            AVCLOCK_START_SUCCESS(avClock, avClockController);
            com::rdk::hal::audiosink::IAudioSink::Id supplementaryAudioSinkId;
            supplementaryAudioSinkId.value = 1; // Valid ID
            AVCLOCK_SET_SUPPLEMENTARY_AUDIO_SINK_SUCCESS(avClock, avClockController, supplementaryAudioSinkId, true);
            UT_LOG_INFO("setSupplementaryAudioSink to ID 1 for clock ID %d in STARTED: success", _clockId.value);
        }
    }
}

/**
 * @brief Test the AV Clock's setSupplementaryAudioSink function in invalid scenarios.
 *
 * This test verifies if the setSupplementaryAudioSink function handles invalid IDs and states correctly.
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
 * |Variation / Step|Description|Test Data|Expected Result|Notes|
 * |----------------|-----------|---------|---------------|-----|
 * |01|Retrieve available AV Clock IDs|None|clockIds should not be empty, status.isOk() should be true|Should Pass|
 * |02|Get AV Clock handle for each ID|_clockId|avClock should be valid, status.isOk() should be true|Should Pass|
 * |03|Open the AV Clock|IAVClockControllerListener|avClockController should be valid, status.isOk() should be true, state transitions to READY|Should Pass|
 * |04|Close the AV Clock|avClockController|status.isOk() should be true, state transitions to CLOSED|Should Pass|
 * |05|Open a new controller|IAVClockControllerListener|avClockController should be valid, status.isOk() should be true|Should Pass|
 * |06|Set supplementary audio sink in CLOSED state|IAudioSink.Id (0)|status.isOk() should be false, exception EX_ILLEGAL_STATE, return false|Should Pass|
 * |07|Open the AV Clock again|IAVClockControllerListener|avClockController should be valid, status.isOk() should be true, state transitions to READY|Should Pass|
 * |08|Set supplementary audio sink to invalid ID in READY state|IAudioSink.Id (NUM_AUDIO_SINK_SUPPLEMENTARY + 1)|status.isOk() should be true, return false|Should Pass|
 */
UT_ADD_TEST(VTS_L1_AVClock, setSupplementaryAudioSink_neg)
{
    std::vector<com::rdk::hal::avclock::IAVClock::Id> clockIds;
    android::sp<com::rdk::hal::avclock::IAVClock> avClock;
    ::android::binder::Status status;

    /* Get the AV Clock IDs */
    AVCLOCK_MANAGER_GETAVCLOCKIDS(clockIds);

    for (const auto& _clockId : clockIds)
    {
        /* Get AV Clock handle */
        AVCLOCK_MANAGER_GETAVCLOCK_SUCCESS(_clockId, avClock);

        /* Test 1: Set supplementary audio sink in CLOSED state */
        {
            android::sp<com::rdk::hal::avclock::IAVClockController> avClockController;
            android::sp<com::rdk::hal::avclock::IAVClockControllerListener> controllerListener = new AVClockControllerListener();
            AVCLOCK_OPEN_SUCCESS(avClock, controllerListener, avClockController, true);

            /* Close the AV Clock to reach CLOSED */
            AVCLOCK_CLOSE_SUCCESS(avClock, avClockController, true);

            com::rdk::hal::audiosink::IAudioSink::Id supplementaryAudioSinkId;
            supplementaryAudioSinkId.value = 0; // Valid ID
            AVCLOCK_SET_SUPPLEMENTARY_AUDIO_SINK_FAILURE_STATE(avClock, avClockController, supplementaryAudioSinkId);

            UT_LOG_INFO("setSupplementaryAudioSink in CLOSED state for clock ID %d: failed as expected", _clockId.value);
        }

        #if 0 /*TODO*/
        /* Test 2: Set supplementary audio sink to invalid ID in READY state */
        {
            android::sp<com::rdk::hal::avclock::IAVClockController> avClockController;
            android::sp<com::rdk::hal::avclock::IAVClockControllerListener> controllerListener = new AVClockControllerListener();
            AVCLOCK_OPEN_SUCCESS(avClock, controllerListener, avClockController, true);

            com::rdk::hal::audiosink::IAudioSink::Id supplementaryAudioSinkId;
            supplementaryAudioSinkId.value = NUM_AUDIO_SINK_SUPPLEMENTARY + 1; // Invalid ID
            AVCLOCK_SET_SUPPLEMENTARY_AUDIO_SINK_FAILURE_INVALID_ID(avClock, avClockController, supplementaryAudioSinkId);

            UT_LOG_INFO("setSupplementaryAudioSink to invalid ID %d for clock ID %d in READY: failed as expected", supplementaryAudioSinkId.value, _clockId.value);
        }
        #endif
    }
}

/**
 * @brief Test the AV Clock's getSupplementaryAudioSink function in valid scenarios.
 *
 * This test verifies if the getSupplementaryAudioSink function correctly returns the set supplementary audio sink ID in READY and STARTED states.
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
 * |Variation / Step|Description|Test Data|Expected Result|Notes|
 * |----------------|-----------|---------|---------------|-----|
 * |01|Retrieve available AV Clock IDs|None|clockIds should not be empty, status.isOk() should be true|Should Pass|
 * |02|Get AV Clock handle for each ID|_clockId|avClock should be valid, status.isOk() should be true|Should Pass|
 * |03|Open the AV Clock|IAVClockControllerListener|avClockController should be valid, status.isOk() should be true, state transitions to READY|Should Pass|
 * |04|Get supplementary audio sink in READY state|None|status.isOk() should be true, return IAudioSink.Id.UNDEFINED (-1)|Should Pass|
 * |05|Set supplementary audio sink to valid ID in READY state|IAudioSink.Id (0)|status.isOk() should be true, return true|Should Pass|
 * |06|Get supplementary audio sink in READY state|None|status.isOk() should be true, return IAudioSink.Id (0)|Should Pass|
 * |07|Start the AV Clock|None|status.isOk() should be true, state transitions to STARTING then STARTED|Should Pass|
 * |08|Get supplementary audio sink in STARTED state|None|status.isOk() should be true, return IAudioSink.Id (0)|Should Pass|
 */
UT_ADD_TEST(VTS_L1_AVClock, getSupplementaryAudioSink_pos)
{
    std::vector<com::rdk::hal::avclock::IAVClock::Id> clockIds;
    android::sp<com::rdk::hal::avclock::IAVClock> avClock;
    ::android::binder::Status status;

    /* Get the AV Clock IDs */
    AVCLOCK_MANAGER_GETAVCLOCKIDS(clockIds);

    for (const auto& _clockId : clockIds)
    {
        /* Get AV Clock handle */
        AVCLOCK_MANAGER_GETAVCLOCK_SUCCESS(_clockId, avClock);

        /* Open the AV Clock to reach READY */
        android::sp<com::rdk::hal::avclock::IAVClockController> avClockController;
        android::sp<com::rdk::hal::avclock::IAVClockControllerListener> controllerListener = new AVClockControllerListener();
        AVCLOCK_OPEN_SUCCESS(avClock, controllerListener, avClockController, true);

        /* Test 1: Get supplementary audio sink in READY state (default UNDEFINED) */
        {
            com::rdk::hal::audiosink::IAudioSink::Id expectedSupplementaryAudioSinkId;
            expectedSupplementaryAudioSinkId.value = -1; // IAudioSink.Id.UNDEFINED
            AVCLOCK_GET_SUPPLEMENTARY_AUDIO_SINK_SUCCESS(avClock, avClockController, expectedSupplementaryAudioSinkId);
            UT_LOG_INFO("getSupplementaryAudioSink in READY state for clock ID %d: returned UNDEFINED", _clockId.value);
        }

        /* Test 2: Set and get supplementary audio sink in READY state */
        {
            com::rdk::hal::audiosink::IAudioSink::Id supplementaryAudioSinkId;
            supplementaryAudioSinkId.value = 0; // Valid ID
            AVCLOCK_SET_SUPPLEMENTARY_AUDIO_SINK_SUCCESS(avClock, avClockController, supplementaryAudioSinkId, true);

            com::rdk::hal::audiosink::IAudioSink::Id expectedSupplementaryAudioSinkId;
            expectedSupplementaryAudioSinkId.value = 0;
            AVCLOCK_GET_SUPPLEMENTARY_AUDIO_SINK_SUCCESS(avClock, avClockController, expectedSupplementaryAudioSinkId);
            UT_LOG_INFO("getSupplementaryAudioSink in READY state for clock ID %d: returned ID 0", _clockId.value);
        }

        /* Test 3: Get supplementary audio sink in STARTED state */
        {
            AVCLOCK_START_SUCCESS(avClock, avClockController);
            com::rdk::hal::audiosink::IAudioSink::Id expectedSupplementaryAudioSinkId;
            expectedSupplementaryAudioSinkId.value = 0;
            AVCLOCK_GET_SUPPLEMENTARY_AUDIO_SINK_SUCCESS(avClock, avClockController, expectedSupplementaryAudioSinkId);
            UT_LOG_INFO("getSupplementaryAudioSink in STARTED state for clock ID %d: returned ID 0", _clockId.value);
        }
    }
}

/**
 * @brief Test the AV Clock's getSupplementaryAudioSink function in invalid scenarios.
 *
 * This test verifies if the getSupplementaryAudioSink function handles invalid states correctly by throwing EX_ILLEGAL_STATE.
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
 * |Variation / Step|Description|Test Data|Expected Result|Notes|
 * |----------------|-----------|---------|---------------|-----|
 * |01|Retrieve available AV Clock IDs|None|clockIds should not be empty, status.isOk() should be true|Should Pass|
 * |02|Get AV Clock handle for each ID|_clockId|avClock should be valid, status.isOk() should be true|Should Pass|
 * |03|Open the AV Clock|IAVClockControllerListener|avClockController should be valid, status.isOk() should be true, state transitions to READY|Should Pass|
 * |04|Close the AV Clock|avClockController|status.isOk() should be true, state transitions to CLOSED|Should Pass|
 * |05|Open a new controller|IAVClockControllerListener|avClockController should be valid, status.isOk() should be true|Should Pass|
 * |06|Get supplementary audio sink in CLOSED state|None|status.isOk() should be false, exception EX_ILLEGAL_STATE|Should Pass|
 */
UT_ADD_TEST(VTS_L1_AVClock, getSupplementaryAudioSink_neg)
{
    std::vector<com::rdk::hal::avclock::IAVClock::Id> clockIds;
    android::sp<com::rdk::hal::avclock::IAVClock> avClock;
    ::android::binder::Status status;

    /* Get the AV Clock IDs */
    AVCLOCK_MANAGER_GETAVCLOCKIDS(clockIds);

    for (const auto& _clockId : clockIds)
    {
        /* Get AV Clock handle */
        AVCLOCK_MANAGER_GETAVCLOCK_SUCCESS(_clockId, avClock);

        /* Test 1: Get supplementary audio sink in CLOSED state */
        {
            android::sp<com::rdk::hal::avclock::IAVClockController> avClockController;
            android::sp<com::rdk::hal::avclock::IAVClockControllerListener> controllerListener = new AVClockControllerListener();
            AVCLOCK_OPEN_SUCCESS(avClock, controllerListener, avClockController, true);

            /* Close the AV Clock to reach CLOSED */
            AVCLOCK_CLOSE_SUCCESS(avClock, avClockController, true);

            AVCLOCK_GET_SUPPLEMENTARY_AUDIO_SINK_FAILURE_STATE(avClock, avClockController);

            UT_LOG_INFO("getSupplementaryAudioSink in CLOSED state for clock ID %d: failed as expected", _clockId.value);
        }
    }
}

#if 0
/**
 * @brief Test the functionality of getting video sink in different AV Clock states.
 *
 * This test verifies the behavior of the `getVideoSink` method when the AV Clock is in READY and STARTED states. It ensures that the video sink ID is valid and not undefined in these states.
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
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Check if controllers list is not empty | mControllers | Should be successful | Should be successful |
 * | 02 | Set AV Clock to READY state | State = READY, isReady = false | status.isOk() = true, isReady = true | Should be successful |
 * | 03 | Call getVideoSink in READY state | videoSinkId | status.isOk() = true, videoSinkId.value != UNDEFINED | Should Pass |
 * | 04 | Set AV Clock to STARTED state | State = STARTED, isStarted = false | status.isOk() = true, isStarted = true | Should be successful |
 * | 05 | Call getVideoSink in STARTED state | videoSinkId | status.isOk() = true, videoSinkId.value != UNDEFINED | Should Pass |
 * | 06 | Validate videoSinkId against YAML data | videoSinkId, videoSinkCount | isValidId = true or videoSinkId.value = UNDEFINED | Should Pass |
 */
UT_ADD_TEST(VTS_L1_AVClock, getVideoSink_pos)
{
    UT_ASSERT_FALSE(mControllers.empty());

    Status status;
    std::vector<int32_t> videoSinkIds;
    std::string videosinkcount_path = "IVideoSinkManager.IVideoSink";
    size_t expectedCount = UT_KVP_PROFILE_GET_LIST_COUNT(videosinkcount_path.c_str());

    for (size_t i = 0; i < expectedCount; ++i)
    {
        std::string videosinkid_path = "IVideoSinkManager.IVideoSink."+std::to_string(i);
        videoSinkIds.push_back(UT_KVP_PROFILE_GET_UINT32(videosinkid_path.c_str()));
    }
    UT_ASSERT_GREATER(videoSinkIds.size(), 0);

    for (auto& controller : mControllers)
    {
        for(auto& id : videoSinkIds)
        {
            // Test getVideoSink in READY state
            IVideoSink::Id videoSinkId;
            videoSinkId.value = id;
            UT_LOG_INFO("Calling getVideoSink(OUT:_aidl_return)");
            status = controller->getVideoSink(&videoSinkId);
            UT_LOG_INFO("Result getVideoSink(OUT:_aidl_return:[%d]), status:[isOK:%s, exception:%s]",
                        videoSinkId.value, UT_Control_GetMapString(bool_mapTable, status.isOk()),
                        UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
            UT_ASSERT_TRUE(status.isOk());
            UT_ASSERT_TRUE(videoSinkId.value == IAudioSink::Id::UNDEFINED || std::find(videoSinkIds.begin(), videoSinkIds.end(), videoSinkId.value) != videoSinkIds.end());

            // Test getVideoSink in STARTED state

            // Test from STARTED state
            status = controller->start();
            UT_ASSERT_TRUE(status.isOk());

            status = controller->getVideoSink(&videoSinkId);
            UT_LOG_INFO("Result getVideoSink(OUT:_aidl_return:[%d]), status:[isOK:%s, exception:%s]",
                        videoSinkId.value, UT_Control_GetMapString(bool_mapTable, status.isOk()),
                        UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
            UT_ASSERT_TRUE(status.isOk());
            UT_ASSERT_TRUE(videoSinkId.value == IAudioSink::Id::UNDEFINED || std::find(videoSinkIds.begin(), videoSinkIds.end(), videoSinkId.value) != videoSinkIds.end());

            status = controller->stop();
            UT_ASSERT_TRUE(status.isOk());
        }
    }
}

/**
 * @brief Test the positive scenario of setting clock mode in AVClockController.
 *
 * This test verifies that the AVClockController can successfully set various clock modes without errors. It ensures that the controller is not empty and iterates through available clock modes, checking the status and return values for correctness.
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
 * | 01 | Verify that mControllers is not empty | mControllers | Should be successful | Should be successful |
 * | 02 | Retrieve list of clock modes using UT_KVP_PROFILE_GET_LIST_COUNT | clockModes | Should be successful | Should be successful |
 * | 03 | Iterate through each clock mode and call setClockMode | mode = value, aidlReturn = false | status.isOk() = true, aidlReturn = true | Should Pass |
 */
UT_ADD_TEST(VTS_L1_AVClock, setClockMode_pos)
{
    UT_ASSERT_FALSE(mControllers.empty());

    std::vector<::com::rdk::hal::avclock::ClockMode> clockModes = {
        ::com::rdk::hal::avclock::ClockMode::AUTO,
        ::com::rdk::hal::avclock::ClockMode::PCR,
        ::com::rdk::hal::avclock::ClockMode::AUDIO_MASTER,
        ::com::rdk::hal::avclock::ClockMode::VIDEO_MASTER
    };

    for (auto& controller : mControllers)
    {
        for (const auto& mode : clockModes)
        {
            bool aidlReturn = false;
            UT_LOG_INFO("Calling setClockMode(IN:clockMode:[%d], OUT:_aidl_return)", mode);

            Status status = controller->setClockMode(mode, &aidlReturn);

            UT_LOG_INFO("Result setClockMode(IN:clockMode:[%d], OUT:_aidl_return:[%s]), status:[isOK:%s, exception:%s]",
                        mode, UT_Control_GetMapString(bool_mapTable, aidlReturn),
                        UT_Control_GetMapString(bool_mapTable, status.isOk()),
                        UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));

            UT_ASSERT_TRUE(status.isOk());
            UT_ASSERT_TRUE(aidlReturn);
        }
    }
}

/**
 * @brief Test the negative scenario for setting the clock mode in AVClockController.
 *
 * This test verifies that the `setClockMode` function fails when the AV Clock is not in the READY state. 
 * It checks the return value and the status to ensure the function behaves as expected in this scenario.
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
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Verify that the controller list is not empty | mControllers | Should be successful | |
 * | 02 | Retrieve clockMode from YAML using macros | clockMode = UT_KVP_PROFILE_GET_UINT32(IAVClockManager.IAVClock.<index>) | Should be successful | |
 * | 03 | Ensure the AV Clock is not in the READY state | Initial state assumed as STARTED | Should be successful | |
 * | 04 | Call setClockMode with the retrieved clockMode | clockMode = [value], result = false | Should Fail | |
 * | 05 | Verify the return value and output parameter | result = false, status.isOk() = false, status.exceptionCode() = android::binder::Status::EX_ILLEGAL_STATE | Should Fail | |
 */
UT_ADD_TEST(VTS_L1_AVClock, setClockMode_neg) {
    UT_ASSERT_FALSE(mControllers.empty());

    std::vector<::com::rdk::hal::avclock::ClockMode> clockModes = {
        ::com::rdk::hal::avclock::ClockMode::AUTO,
        ::com::rdk::hal::avclock::ClockMode::PCR,
        ::com::rdk::hal::avclock::ClockMode::AUDIO_MASTER,
        ::com::rdk::hal::avclock::ClockMode::VIDEO_MASTER
    };
    //test 1 : with invalid state

    for (auto& controller : mControllers)
    {
        bool result = false;
        Status status;
        status = controller->start();
        UT_ASSERT_TRUE(status.isOk());

        for (const auto& mode : clockModes)
        {
            UT_LOG_INFO("Calling setClockMode(IN:clockMode:[%d], OUT:_aidl_return[])", mode);
            status = controller->setClockMode(mode, &result);
            UT_ASSERT_FALSE(result);
            UT_ASSERT_FALSE(status.isOk());
            UT_ASSERT_EQUAL(status.exceptionCode(), android::binder::Status::EX_ILLEGAL_STATE);

            UT_LOG_INFO("Result setClockMode(IN:clockMode:[%d], OUT:_aidl_return[%d]), status:[isOK:%s, exception:%s]",
                mode, result, UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        }
        status = controller->stop();
        UT_ASSERT_TRUE(status.isOk());
    }

    //test 2 : with invalid clockmode
    for (auto& controller : mControllers)
    {
        bool result = false;
        Status status;

        UT_LOG_INFO("Calling setClockMode(IN:clockMode:[-1], OUT:_aidl_return[])");
        status = controller->setClockMode(static_cast<::com::rdk::hal::avclock::ClockMode>(-1), &result);
        UT_ASSERT_FALSE(result);
        UT_ASSERT_FALSE(status.isOk());
        UT_LOG_INFO("Result setClockMode(IN:clockMode:[-1], OUT:_aidl_return[%d]), status:[isOK:%s, exception:%s]",
                result, UT_Control_GetMapString(bool_mapTable, status.isOk()),
                UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));

    }
}

/**
 * @brief Test the positive scenario for retrieving the clock mode from AVClockController.
 *
 * This test verifies the functionality of the getClockMode method in the AVClockControllerBaseTest class. It checks whether the method correctly retrieves the clock mode and ensures that the returned mode is valid and the operation is successful.
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
 * | 01 | Initialize the test environment and setup necessary components for AVClockControllerBaseTest. | None | Environment should be initialized successfully. | Should be successful |
 * | 02 | Iterate over each controller in mControllers and call getClockMode method. | mControllers[i], clockMode | Status should be OK, clockMode should not be nullptr. | Should Pass |
 * | 03 | Log the result of getClockMode and check the status and exception code. | status.isOk(), status.exceptionCode() | Status should be OK, exception code should be valid. | Should Pass |
 * | 04 | Assert that the status is OK and clockMode is not nullptr. | status.isOk(), clockMode | Assertions should pass. | Should Pass |
 * | 05 | Verify that the clockMode is one of the valid modes: AUTO, PCR, AUDIO_MASTER, VIDEO_MASTER. | clockMode | clockMode should be in validModes set. | Should Pass |
 * | 06 | Clean up the test environment and perform any necessary teardown operations. | None | Environment should be cleaned up successfully. | Done by Pre-requisite TearDown function |
 */
UT_ADD_TEST(VTS_L1_AVClock, getClockMode_pos) {
    UT_ASSERT_FALSE(mControllers.empty());

    Status status;
    for (size_t i = 0; i < mControllers.size(); ++i) {
        ClockMode clockMode;
        UT_LOG_INFO("Calling getClockMode(OUT:clockMode)");
        status = mControllers[i]->getClockMode(&clockMode);
        UT_LOG_INFO("Result getClockMode(OUT:clockMode:[%d]), status:[isOK:%s, exception:%s]",
                    clockMode,
                    UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));

        UT_ASSERT_TRUE(status.isOk());

        std::unordered_set<ClockMode> validModes = {
            ClockMode::AUTO,
            ClockMode::PCR,
            ClockMode::AUDIO_MASTER,
            ClockMode::VIDEO_MASTER
        };
        UT_ASSERT_NOT_EQUAL(validModes.find(clockMode), validModes.end());
    }

    //test from STARTED state
    for (size_t i = 0; i < mControllers.size(); ++i) {
        status =  mControllers[i]->start();
        UT_ASSERT_TRUE(status.isOk());

        ClockMode clockMode;
        UT_LOG_INFO("Calling getClockMode(OUT:clockMode)");
        status = mControllers[i]->getClockMode(&clockMode);
        UT_LOG_INFO("Result getClockMode(OUT:clockMode:[%d]), status:[isOK:%s, exception:%s]",
                    clockMode,
                    UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));

        UT_ASSERT_TRUE(status.isOk());

        std::unordered_set<ClockMode> validModes = {
            ClockMode::AUTO,
            ClockMode::PCR,
            ClockMode::AUDIO_MASTER,
            ClockMode::VIDEO_MASTER
        };
        UT_ASSERT_NOT_EQUAL(validModes.find(clockMode), validModes.end());

        status = mControllers[i]->stop();
        UT_ASSERT_TRUE(status.isOk());
    }
}

/**
 * @brief Test the positive scenario of notifying a PCR sample in AVClockController.
 *
 * This test verifies that the AVClockController correctly handles a valid PCR sample notification when the clock is in the STARTED state and in PCR mode. It ensures that the notification is successful and returns the expected results.
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
 * | 01 | Verify that the controller list is not empty | mControllers | Should be successful | |
 * | 02 | Check if the AV Clock is in STARTED state | controller->isStarted(&isStarted) | status.isOk() = true, isStarted = true | Should be successful |
 * | 03 | Verify the clock mode is PCR | controller->getClockMode(&currentMode) | status.isOk() = true, currentMode = ClockMode::PCR | Should be successful |
 * | 04 | Notify a valid PCR sample | controller->notifyPCRSample(pcrTimeNs, sampleTimestampNs, &aidl_return) | status.isOk() = true, aidl_return = true | Should Pass |
 */
UT_ADD_TEST(VTS_L1_AVClock, notifyPCRSample_pos) {
    UT_ASSERT_FALSE(mControllers.empty());
    Status status;

    for (auto& controller : mControllers) {

        //test from STARTED state
        status = controller->start();
        UT_ASSERT_TRUE(status.isOk());

        bool result = false;
        status = controller->setClockMode(::com::rdk::hal::avclock::ClockMode::PCR, &result);
        UT_ASSERT_TRUE(result);
        UT_ASSERT_TRUE(status.isOk());

        // Test Case 1: Valid PCR Sample Notification
        int64_t pcrTimeNs = 10000000; // Valid PCR time in nanoseconds
        int64_t sampleTimestampNs = 20000000; // Valid CLOCK_MONOTONIC_RAW timestamp in nanoseconds
        bool aidl_return = false;

        UT_LOG_INFO("Calling notifyPCRSample(IN:pcrTimeNs:[%lld], IN:sampleTimestampNs:[%lld], OUT:_aidl_return)", pcrTimeNs, sampleTimestampNs);
        status = controller->notifyPCRSample(pcrTimeNs, sampleTimestampNs, &aidl_return);
        UT_LOG_INFO("Result notifyPCRSample(IN:pcrTimeNs:[%lld], IN:sampleTimestampNs:[%lld], OUT:_aidl_return:[%d]), status:[isOK:%s, exception:%s]", pcrTimeNs, sampleTimestampNs, aidl_return, UT_Control_GetMapString(bool_mapTable, status.isOk()), UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));

        UT_ASSERT_TRUE(status.isOk());
        UT_ASSERT_TRUE(aidl_return);

        status = controller->stop();
        UT_ASSERT_TRUE(status.isOk());
    }

}

/**
 * @brief Test the negative scenario for notifyPCRSample in AVClockController
 *
 * This test verifies the behavior of the notifyPCRSample function when the AVClockController is in an illegal state. It ensures that the function returns the expected result and status code when invoked with valid PCR and sample timestamps.
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
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Verify that mControllers is not empty | mControllers | Should not be empty | Should be successful |
 * | 02 | Iterate over each controller in mControllers | mControllers | Each controller should be processed | Should be successful |
 * | 03 | Initialize result to false and set valid PCR and sample timestamps | result = false, pcrTimeNs = 1234567890, sampleTimestampNs = 9876543210 | Variables initialized | Should be successful |
 * | 04 | Call notifyPCRSample with valid timestamps and check result | pcrTimeNs = 1234567890, sampleTimestampNs = 9876543210, result | result should be false, status.exceptionCode() should be Status::EX_ILLEGAL_STATE | Should Fail |
 */
UT_ADD_TEST(VTS_L1_AVClock, notifyPCRSample_neg) {
    UT_ASSERT_FALSE(mControllers.empty());
    //invoke api from READY state (illegal state)
    for (auto& controller : mControllers)
    {
        int64_t pcrTimeNs = 10000000; // Valid PCR time in nanoseconds
        int64_t sampleTimestampNs = 20000000; // Valid CLOCK_MONOTONIC_RAW timestamp in nanoseconds
        bool result = false;

        UT_LOG_INFO("Calling notifyPCRSample(IN:pcrTimeNs:[%lld], IN:sampleTimestampNs:[%lld], OUT:result)", pcrTimeNs, sampleTimestampNs);

        Status status = controller->notifyPCRSample(pcrTimeNs, sampleTimestampNs, &result);

        UT_LOG_INFO("Result notifyPCRSample(IN:pcrTimeNs:[%lld], IN:sampleTimestampNs:[%lld], OUT:result[%d]), status:[isOK:%s, exception:%s]",
                    pcrTimeNs, sampleTimestampNs, result, UT_Control_GetMapString(bool_mapTable, status.isOk()), UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));

        UT_ASSERT_FALSE(result);
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_ILLEGAL_STATE);
    }
}

/**
 * @brief Test the positive scenario of retrieving the current clock time from AVClockController.
 *
 * This test verifies that the AVClockController can successfully retrieve the current clock time when the clock is in the STARTED state. It checks the correctness of the returned clock time and ensures that the operation completes without errors.
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
 * | 01 | Ensure the AV Clock is in the STARTED state by invoking the start method on the controller. | started = false | status.isOk() = true, started = true | Should be successful |
 * | 02 | Prepare to retrieve the current clock time by initializing ClockTime object. | clockTime initialized | N/A | Should be successful |
 * | 03 | Call the getCurrentClockTime API to retrieve the current clock time. | clockTime = ClockTime object | status.isOk() = true, clockTime.clockTimeNs != 0, clockTime.sampleTimestampNs != 0 | Should Pass |
 * | 04 | Log the result of the getCurrentClockTime API call. | clockTime.clockTimeNs, clockTime.sampleTimestampNs | N/A | Should be successful |
 * | 05 | Verify the results by asserting the correctness of the returned clock time values. | clockTime.clockTimeNs, clockTime.sampleTimestampNs | status.isOk() = true, clockTime.clockTimeNs != 0, clockTime.sampleTimestampNs != 0 | Should Pass |
 * | 06 | Stop the AV Clock by invoking the stop method on the controller. | stopped = false | status.isOk() = true, stopped = true | Should be successful |
 */
UT_ADD_TEST(VTS_L1_AVClock, getCurrentClockTime_pos)
{
    for (auto& controller : mControllers)
    {
        Status status = controller->start();
        UT_ASSERT_TRUE(status.isOk());

        ClockTime clockTime;
        UT_LOG_INFO("Calling getCurrentClockTime(OUT:clockTime)");
        status = controller->getCurrentClockTime(&clockTime);
        UT_LOG_INFO("Result getCurrentClockTime(OUT:clockTime[clockTimeNs:%lld, sampleTimestampNs:%lld]), status:[isOK:%s, exception:%s]",
                    clockTime.clockTimeNs, clockTime.sampleTimestampNs,
                    UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_TRUE(status.isOk());

        // Stop the AV Clock
        status = controller->stop();
        UT_ASSERT_TRUE(status.isOk());
    }
}

/**
 * @brief Test the negative scenario for getting the current clock time from AVClockController
 *
 * This test verifies the behavior of the AVClockController when attempting to retrieve the current clock time in an illegal state. It ensures that the function returns the expected exception code and clock time values when the controller is not in the READY state.
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
 * | 01 | Initialize status and clockTime variables | status = undefined, clockTime = undefined | Variables initialized successfully | Should be successful |
 * | 02 | Iterate over mControllers to check if any controller is in READY state | controller = mControllers[i], currentState = undefined | currentState retrieved successfully | Should be successful |
 * | 03 | Check if currentState is READY and set isReadyState accordingly | currentState = State::READY | isReadyState set to true if READY state found | Should be successful |
 * | 04 | Assert that isReadyState is true | isReadyState = true | Assertion passes if any controller is in READY state | Should Pass |
 * | 05 | Call getCurrentClockTime on the first AVClock in mAVClocks | mAVClocks[0], clockTime = undefined | status and clockTime retrieved | Should Fail |
 * | 06 | Log the result of getCurrentClockTime call | status = retrieved, clockTime = retrieved | Log output generated | Should be successful |
 * | 07 | Assert that status.exceptionCode is EX_ILLEGAL_STATE | status.exceptionCode = Status::EX_ILLEGAL_STATE | Assertion passes if exception code is EX_ILLEGAL_STATE | Should Pass |
 * | 08 | Assert that clockTime.clockTimeNs is 0L | clockTime.clockTimeNs = 0L | Assertion passes if clockTimeNs is 0L | Should Pass |
 * | 09 | Assert that clockTime.sampleTimestampNs is 0L | clockTime.sampleTimestampNs = 0L | Assertion passes if sampleTimestampNs is 0L | Should Pass |
 */
UT_ADD_TEST(VTS_L1_AVClock, getCurrentClockTime_neg)
{
    Status status;
    ClockTime clockTime;
    UT_ASSERT_FALSE(mControllers.empty());

    UT_LOG_INFO("Calling getCurrentClockTime(OUT:_aidl_return)");
    status = mControllers[0]->getCurrentClockTime(&clockTime);
    UT_LOG_INFO("Result getCurrentClockTime(OUT:_aidl_return), status:[isOK:%s, exception:%s]",
                UT_Control_GetMapString(bool_mapTable, status.isOk()),
                UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));

    UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_ILLEGAL_STATE);
}

/**
 * @brief Test the positive scenarios for setting playback rate in AVClockController.
 *
 * This test verifies the functionality of setting different playback rates in various states of the AVClockController. It ensures that the controller correctly handles normal, half, double speed, and pause scenarios.
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
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Check if mControllers is not empty | mControllers | Should not be empty | Should be successful |
 * | 02 | Set playback rate to normal speed (1.0) in READY state | rate = 1.0, _aidl_return = false | status.isOk() = true, _aidl_return = true | Should Pass |
 * | 03 | Set playback rate to half speed (0.5) in STARTED state | rate = 0.5, _aidl_return = false | status.isOk() = true, _aidl_return = true | Should Pass |
 * | 04 | Set playback rate to double speed (2.0) in READY state | rate = 2.0, _aidl_return = false | status.isOk() = true, _aidl_return = true | Should Pass |
 * | 05 | Pause playback by setting rate to 0.0 in STARTED state | rate = 0.0, _aidl_return = false | status.isOk() = true, _aidl_return = true | Should Pass |
 */
UT_ADD_TEST(VTS_L1_AVClock, setPlaybackRate_pos) {
    UT_ASSERT_FALSE(mControllers.empty());
    Status status;

    for (auto& controller : mControllers)
    {
        bool aidl_return = false;

        ::com::rdk::hal::avclock::ClockMode initial_clockmode;
        status = controller->getClockMode(&initial_clockmode);
        UT_ASSERT_TRUE(status.isOk());

        if(initial_clockmode == ::com::rdk::hal::avclock::ClockMode::PCR)
        {
            bool aidlReturn1 = false;
            ::com::rdk::hal::avclock::ClockMode mode = ::com::rdk::hal::avclock::ClockMode::AUTO;
            UT_LOG_INFO("Calling setClockMode(IN:clockMode:[%d], OUT:_aidl_return)", mode);
            status = controller->setClockMode(mode, &aidlReturn1);
            UT_ASSERT_TRUE(status.isOk());
            UT_ASSERT_TRUE(aidlReturn1);
        }
        // Test Case 1: Set playback rate to normal speed (1.0) in READY state
        UT_LOG_INFO("Calling setPlaybackRate(IN:rate:[1.0], OUT:_aidl_return)");
        status = controller->setPlaybackRate(1.0, &aidl_return);
        UT_LOG_INFO("Result setPlaybackRate(IN:rate:[1.0], OUT:_aidl_return:[%s]), status:[isOK:%s, exception:%s]",
                    UT_Control_GetMapString(bool_mapTable, aidl_return),
                    UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_TRUE(status.isOk());
        UT_ASSERT_TRUE(aidl_return);
        // Test Case 3: Set playback rate to double speed (2.0) in READY state
        UT_LOG_INFO("Calling setPlaybackRate(IN:rate:[2.0], OUT:_aidl_return)");
        status = controller->setPlaybackRate(2.0, &aidl_return);
        UT_LOG_INFO("Result setPlaybackRate(IN:rate:[2.0], OUT:_aidl_return:[%s]), status:[isOK:%s, exception:%s]",
                    UT_Control_GetMapString(bool_mapTable, aidl_return),
                    UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_TRUE(status.isOk());
        UT_ASSERT_TRUE(aidl_return);

        status = controller->start();
        UT_ASSERT_TRUE(status.isOk());

        // Test Case 2: Set playback rate to half speed (0.5) in STARTED state
        UT_LOG_INFO("Calling setPlaybackRate(IN:rate:[0.5], OUT:_aidl_return)");
        status = controller->setPlaybackRate(0.5, &aidl_return);
        UT_LOG_INFO("Result setPlaybackRate(IN:rate:[0.5], OUT:_aidl_return:[%s]), status:[isOK:%s, exception:%s]",
                    UT_Control_GetMapString(bool_mapTable, aidl_return),
                    UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_TRUE(status.isOk());
        UT_ASSERT_TRUE(aidl_return);

        // Test Case 4: Pause playback by setting rate to 0.0 in STARTED state
        UT_LOG_INFO("Calling setPlaybackRate(IN:rate:[0.0], OUT:_aidl_return)");
        status = controller->setPlaybackRate(0.0, &aidl_return);
        UT_LOG_INFO("Result setPlaybackRate(IN:rate:[0.0], OUT:_aidl_return:[%s]), status:[isOK:%s, exception:%s]",
                    UT_Control_GetMapString(bool_mapTable, aidl_return),
                    UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_TRUE(status.isOk());
        UT_ASSERT_TRUE(aidl_return);

        status = controller->stop();
        UT_ASSERT_TRUE(status.isOk());


        if(initial_clockmode == ::com::rdk::hal::avclock::ClockMode::PCR)
        {
            bool aidlReturn1 = false;
            UT_LOG_INFO("Calling setClockMode(IN:clockMode:[PCR], OUT:_aidl_return)");
            status = controller->setClockMode(::com::rdk::hal::avclock::ClockMode::PCR, &aidlReturn1);
            UT_ASSERT_TRUE(status.isOk());
            UT_ASSERT_TRUE(aidlReturn1);
        }
    }

}

/**
 * @brief Test the behavior of setPlaybackRate with invalid values and states
 *
 * This test evaluates the setPlaybackRate function of AVClockControllerBaseTest by providing invalid playback rates and checking the response in different states. It ensures that the function correctly handles invalid inputs and returns appropriate status and aidl_return values.
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
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Verify that mControllers is not empty | mControllers | Should be successful | |
 * | 02 | Set playback rate to an invalid negative value (-1.0) in READY state | rate = -1.0, aidl_return = false | aidl_return = false, status.isOk() = false | Should Fail |
 * | 03 | Set playback rate to an invalid high value (3.0) in STARTED state | rate = 3.0, aidl_return = false | aidl_return = false, status.isOk() = false | Should Fail |
 * | 04 | Attempt to set playback rate in PCR mode | rate = 1.0, aidl_return = false | aidl_return = false, status.isOk() = false | Should Fail |
 */
UT_ADD_TEST(VTS_L1_AVClock, setPlaybackRate_neg) {
    UT_ASSERT_FALSE(mControllers.empty());
    Status status;

    for (auto& controller : mControllers) {
        bool aidl_return = false;

        ::com::rdk::hal::avclock::ClockMode initial_clockmode;
        status = controller->getClockMode(&initial_clockmode);
        UT_ASSERT_TRUE(status.isOk());

        if(initial_clockmode == ::com::rdk::hal::avclock::ClockMode::PCR)
        {
            bool aidlReturn1 = false;
            ::com::rdk::hal::avclock::ClockMode mode = ::com::rdk::hal::avclock::ClockMode::AUTO;
            UT_LOG_INFO("Calling setClockMode(IN:clockMode:[%d], OUT:_aidl_return)", mode);
            status = controller->setClockMode(mode, &aidlReturn1);
            UT_ASSERT_TRUE(status.isOk());
            UT_ASSERT_TRUE(aidlReturn1);
        }


        // Test Case 5: Set playback rate to an invalid negative value (-1.0) in READY state
        UT_LOG_INFO("Calling setPlaybackRate(IN:rate:[-1.0], OUT:_aidl_return)");
        status = controller->setPlaybackRate(-1.0, &aidl_return);
        UT_LOG_INFO("Result setPlaybackRate(IN:rate:[-1.0], OUT:_aidl_return[%d]), status:[isOK:%s, exception:%s]",
                    aidl_return, UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_FALSE(aidl_return);
        UT_ASSERT_FALSE(status.isOk());

        // Test Case 6: Set playback rate to an invalid high value (3.0) in STARTED state
        UT_LOG_INFO("Calling setPlaybackRate(IN:rate:[3.0], OUT:_aidl_return)");
        status = controller->setPlaybackRate(3.0, &aidl_return);
        UT_LOG_INFO("Result setPlaybackRate(IN:rate:[3.0], OUT:_aidl_return[%d]), status:[isOK:%s, exception:%s]",
                    aidl_return, UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_FALSE(aidl_return);
        UT_ASSERT_FALSE(status.isOk());

        // Test Case 7: Attempt to set playback rate in PCR mode


        bool aidlReturn3 = false;
        UT_LOG_INFO("Calling setClockMode(IN:clockMode:[PCR], OUT:_aidl_return)");
        status = controller->setClockMode(::com::rdk::hal::avclock::ClockMode::PCR, &aidlReturn3);
        UT_ASSERT_TRUE(status.isOk());
        UT_ASSERT_TRUE(aidlReturn3);


        UT_LOG_INFO("Calling setPlaybackRate(IN:rate:[1.0], OUT:_aidl_return)");
        status = controller->setPlaybackRate(1.0, &aidl_return);
        UT_LOG_INFO("Result setPlaybackRate(IN:rate:[1.0], OUT:_aidl_return[%d]), status:[isOK:%s, exception:%s]",
                    aidl_return, UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_FALSE(aidl_return);
        UT_ASSERT_FALSE(status.isOk());

        bool aidlReturn4 = false;
        UT_LOG_INFO("Calling setClockMode(IN:clockMode:[], OUT:_aidl_return)");
        status = controller->setClockMode(initial_clockmode, &aidlReturn4);
        UT_ASSERT_TRUE(status.isOk());
        UT_ASSERT_TRUE(aidlReturn4);
    }
}

/**
 * @brief Test the retrieval of playback rate in different states of AV Clock Controller
 *
 * This test verifies the correct retrieval of playback rate from the AV Clock Controller in both READY and STARTED states. It ensures that the playback rate is within the expected range and that the operations succeed without errors.
 *
 * **Test Group ID:** Basic: 01@n
 * **Test Case ID:** 039@n
 * **Priority:** High@n
 * @n
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 * @n
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Check if controllers list is not empty | mControllers | Should be successful | Should be successful |
 * | 02 | Set AV Clock to READY state | State = READY, success = false | status.isOk() = true, success = true | Should be successful |
 * | 03 | Retrieve playback rate in READY state | playbackRate = 0.0 | status.isOk() = true, playbackRate >= 0.5, playbackRate <= 2.0 | Should Pass |
 * | 04 | Set AV Clock to STARTED state | State = STARTED, success = false | status.isOk() = true, success = true | Should be successful |
 * | 05 | Retrieve playback rate in STARTED state | playbackRate = 0.0 | status.isOk() = true, playbackRate >= 0.5, playbackRate <= 2.0 | Should Pass |
 */
UT_ADD_TEST(VTS_L1_AVClock, getPlaybackRate_pos) {
    UT_ASSERT_FALSE(mControllers.empty());
    Status status;

    for (auto& controller : mControllers)
    {
        double playbackRate = 0.0;

        ::com::rdk::hal::avclock::ClockMode initial_clockmode;
        status = controller->getClockMode(&initial_clockmode);
        UT_ASSERT_TRUE(status.isOk());

        if(initial_clockmode == ::com::rdk::hal::avclock::ClockMode::PCR)
        {
            bool aidlReturn1 = false;
            ::com::rdk::hal::avclock::ClockMode mode = ::com::rdk::hal::avclock::ClockMode::AUTO;
            UT_LOG_INFO("Calling setClockMode(IN:clockMode:[%d], OUT:_aidl_return)", mode);
            status = controller->setClockMode(mode, &aidlReturn1);
            UT_ASSERT_TRUE(status.isOk());
            UT_ASSERT_TRUE(aidlReturn1);
        }

        // Test Case 1: Valid Playback Rate Retrieval in READY State
        UT_LOG_INFO("Calling getPlaybackRate(OUT:_aidl_return)");
        status = controller->getPlaybackRate(&playbackRate);
        UT_LOG_INFO("Result getPlaybackRate(OUT:_aidl_return:[%f]), status:[isOK:%s, exception:%s]",
                    playbackRate, UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_TRUE(status.isOk());
        UT_ASSERT_GREATER_EQUAL(playbackRate, 0.0);
        UT_ASSERT_LESS_EQUAL(playbackRate, 2.0);

        // Test Case 2: Valid Playback Rate Retrieval in STARTED State
        status = controller->start();
        UT_ASSERT_TRUE(status.isOk());

        UT_LOG_INFO("Calling getPlaybackRate(OUT:_aidl_return)");
        status = controller->getPlaybackRate(&playbackRate);
        UT_LOG_INFO("Result getPlaybackRate(OUT:_aidl_return:[%f]), status:[isOK:%s, exception:%s]",
                    playbackRate, UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_TRUE(status.isOk());
        UT_ASSERT_GREATER_EQUAL(playbackRate, 0.0);
        UT_ASSERT_LESS_EQUAL(playbackRate, 2.0);

        status = controller->stop();
        UT_ASSERT_TRUE(status.isOk());

        if(initial_clockmode == ::com::rdk::hal::avclock::ClockMode::PCR)
        {
            bool aidlReturn1 = false;
            UT_LOG_INFO("Calling setClockMode(IN:clockMode:[PCR], OUT:_aidl_return)");
            status = controller->setClockMode(::com::rdk::hal::avclock::ClockMode::PCR, &aidlReturn1);
            UT_ASSERT_TRUE(status.isOk());
            UT_ASSERT_TRUE(aidlReturn1);
        }
    }
}

/**
 * @brief Test the negative scenario for getting playback rate in AVClockController
 *
 * This test verifies the behavior of the AVClockController when attempting to get the playback rate
 * while the clock mode is set to PCR. It ensures that the function handles the illegal state correctly
 * by returning an appropriate exception code and leaving the playback rate unchanged.
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
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Check if mControllers is not empty | mControllers | Should be successful | |
 * | 02 | Iterate over each controller in mControllers | controller | Should be successful | |
 * | 03 | Set the clock mode to ClockMode::PCR | controller, ClockMode::PCR | Status isOk: true | Should be successful |
 * | 04 | Call getPlaybackRate with playbackRate as output | controller, playbackRate = 0.0 | Status exceptionCode: EX_ILLEGAL_STATE, playbackRate: 0.0 | Should Fail |
 */
UT_ADD_TEST(VTS_L1_AVClock, getPlaybackRate_neg) {
    UT_ASSERT_FALSE(mControllers.empty());
    Status status;

    for (auto& controller : mControllers) {
        double playbackRate = 0.0;

        ::com::rdk::hal::avclock::ClockMode initial_clockmode;
        status = controller->getClockMode(&initial_clockmode);
        UT_ASSERT_TRUE(status.isOk());

        if(initial_clockmode != ::com::rdk::hal::avclock::ClockMode::PCR)
        {
            bool aidlReturn1 = false;
            UT_LOG_INFO("Calling setClockMode(IN:clockMode:[%d], OUT:_aidl_return)", static_cast<int>(::com::rdk::hal::avclock::ClockMode::PCR));
            ::com::rdk::hal::avclock::ClockMode mode = ::com::rdk::hal::avclock::ClockMode::PCR;
            status = controller->setClockMode(mode, &aidlReturn1);
            UT_ASSERT_TRUE(status.isOk());
            UT_ASSERT_TRUE(aidlReturn1);
        }

        UT_LOG_INFO("Calling getPlaybackRate(OUT:_aidl_return:[%p])", &playbackRate);
        status = controller->getPlaybackRate(&playbackRate);

        UT_LOG_INFO("Result getPlaybackRate(OUT:_aidl_return:[%p]), status:[isOK:%s, exception:%s]",
                    &playbackRate, UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_ILLEGAL_STATE);

        bool aidlReturn4 = false;
        UT_LOG_INFO("Calling setClockMode(IN:clockMode:[PCR], OUT:_aidl_return)");
        status = controller->setClockMode(initial_clockmode, &aidlReturn4);
        UT_ASSERT_TRUE(status.isOk());
        UT_ASSERT_TRUE(aidlReturn4);
    }
}

#endif
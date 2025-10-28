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
 * @defgroup AV_CLOCK_HALTEST_L1 AV Clock HAL Tests helper functions Header File
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

#include "vts_avclock.h"


class VTS_L1_AVClock : public UTCore
{
public:
    VTS_L1_AVClock() : UTCore() {}

    ~VTS_L1_AVClock() override = default;

    void SetUp() override
    {
        if (gAVClockManagerService == nullptr)
        {
            UT_FAIL_FATAL("AVClock Service not available");
        }
    }

    void TearDown() override
    {
    }
};
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
            UT_ASSERT_EQUAL(propertyValue->value.value().getTag(), com::rdk::hal::PropertyValue::Value::intValue);
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

    /* List of read only properties to test (all read-only) */
    std::vector<com::rdk::hal::avclock::Property> readOnlyProperties = {
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

        /* Test 1: Invalid property key */
        {
            com::rdk::hal::PropertyValue propertyValue;
            propertyValue.value = com::rdk::hal::PropertyValue::Value::make<com::rdk::hal::PropertyValue::Value::intValue>(1);
            com::rdk::hal::avclock::Property invalidProperty = static_cast<com::rdk::hal::avclock::Property>(-1);
            bool result = false;
            AVCLOCK_SET_PROPERTY_FAILURE(avClock, invalidProperty, propertyValue, result,Status::EX_ILLEGAL_ARGUMENT);

            UT_LOG_INFO("setProperty for invalid property %d, clock ID %d: result=%s",
                        invalidProperty, _clockId.value, result ? "true" : "false");
            UT_ASSERT_FALSE(result);
        }

        /* Test setProperty for each valid property */
        for (const auto& _property : readOnlyProperties)
        {
            com::rdk::hal::PropertyValue propertyValue;
            propertyValue.value = com::rdk::hal::PropertyValue::Value::make<com::rdk::hal::PropertyValue::Value::intValue>(1); // Example value
            bool result = false;
            AVCLOCK_SET_PROPERTY_FAILURE(avClock, _property, propertyValue, result,Status::EX_UNSUPPORTED_OPERATION);

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

    // Store pairs of Clock and Listener to clean up later
    std::vector<std::pair<android::sp<com::rdk::hal::avclock::IAVClock>, android::sp<com::rdk::hal::avclock::IAVClockEventListener>>> activeListeners;

    AVCLOCK_MANAGER_GETAVCLOCKIDS(clockIds);

    // STEP 1: Register on ALL clocks
    for (const auto& _clockId : clockIds)
    {
        AVCLOCK_MANAGER_GETAVCLOCK_SUCCESS(_clockId, avClock);

        android::sp<com::rdk::hal::avclock::IAVClockEventListener> eventListener = new AVClockEventListener();

        // Register and verify success
        AVCLOCK_REGISTER_EVENT_LISTENER_SUCCESS(avClock, eventListener);
        UT_LOG_INFO("registerEventListener for clock ID %d: success", _clockId.value);

        // Store them for later unregistration
        activeListeners.push_back({avClock, eventListener});
    }

    // STEP 2: Unregister ALL clocks (Batch Cleanup)
    for (const auto& pair : activeListeners)
    {
        AVCLOCK_UNREGISTER_EVENT_SUCCESS(pair.first, pair.second);
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

            AVCLOCK_UNREGISTER_EVENT_SUCCESS(avClock,eventListener);
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
        AVCLOCK_START_SUCCESS(avClock, avClockController, true);

        /* Stop the AV Clock */
        AVCLOCK_STOP_SUCCESS(avClock, avClockController, true);

        /* Close the AV Clock */
        AVCLOCK_CLOSE_SUCCESS(avClock, avClockController, true);

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
            AVCLOCK_START_SUCCESS(avClock, avClockController,true);

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
        AVCLOCK_START_SUCCESS(avClock, avClockController, true);

        /* Stop the AV Clock */
        AVCLOCK_STOP_SUCCESS(avClock, avClockController, true);

        /* Close the AV Clock */
        AVCLOCK_CLOSE_SUCCESS(avClock, avClockController, true);

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

        /* Test 1: Stop in READY state */
        {
            android::sp<com::rdk::hal::avclock::IAVClockController> avClockController;
            android::sp<com::rdk::hal::avclock::IAVClockControllerListener> controllerListener = new AVClockControllerListener();
            AVCLOCK_OPEN_SUCCESS(avClock, controllerListener, avClockController, true);

            AVCLOCK_STOP_FAILURE(avClock, avClockController);
            UT_LOG_INFO("stop in READY state for clock ID %d: failed as expected", _clockId.value);

            /* Close the AV Clock to ensure CLOSED state */
            AVCLOCK_CLOSE_SUCCESS(avClock, avClockController, true);

        }
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
            AVCLOCK_START_SUCCESS(avClock, avClockController, true);
            com::rdk::hal::audiosink::IAudioSink::Id audioSinkId;
            audioSinkId.value = 1; // Valid ID
            AVCLOCK_SET_AUDIO_SINK_SUCCESS(avClock, avClockController, audioSinkId, true);
            UT_LOG_INFO("setAudioSink to ID 1 for clock ID %d in STARTED: success", _clockId.value);

            /* Stop the AV Clock */
            AVCLOCK_STOP_SUCCESS(avClock, avClockController, true);
        }

        /* Close the AV Clock */
        AVCLOCK_CLOSE_SUCCESS(avClock, avClockController, true);
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
            AVCLOCK_START_SUCCESS(avClock, avClockController, true);
            com::rdk::hal::audiosink::IAudioSink::Id expectedAudioSinkId;
            expectedAudioSinkId.value = 0;
            AVCLOCK_GET_AUDIO_SINK_SUCCESS(avClock, avClockController, expectedAudioSinkId);
            UT_LOG_INFO("getAudioSink in STARTED state for clock ID %d: returned ID 0", _clockId.value);

            /* Stop the AV Clock */
            AVCLOCK_STOP_SUCCESS(avClock, avClockController, true);
        }

        /* Close the AV Clock */
        AVCLOCK_CLOSE_SUCCESS(avClock, avClockController, true);
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
            AVCLOCK_START_SUCCESS(avClock, avClockController, true);
            com::rdk::hal::audiosink::IAudioSink::Id supplementaryAudioSinkId;
            supplementaryAudioSinkId.value = 1; // Valid ID
            AVCLOCK_SET_SUPPLEMENTARY_AUDIO_SINK_SUCCESS(avClock, avClockController, supplementaryAudioSinkId, true);
            UT_LOG_INFO("setSupplementaryAudioSink to ID 1 for clock ID %d in STARTED: success", _clockId.value);

            /* Stop the AV Clock */
            AVCLOCK_STOP_SUCCESS(avClock, avClockController, true);
        }

        /* Close the AV Clock */
        AVCLOCK_CLOSE_SUCCESS(avClock, avClockController, true);
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
            AVCLOCK_START_SUCCESS(avClock, avClockController, true);
            com::rdk::hal::audiosink::IAudioSink::Id expectedSupplementaryAudioSinkId;
            expectedSupplementaryAudioSinkId.value = 0;
            AVCLOCK_GET_SUPPLEMENTARY_AUDIO_SINK_SUCCESS(avClock, avClockController, expectedSupplementaryAudioSinkId);
            UT_LOG_INFO("getSupplementaryAudioSink in STARTED state for clock ID %d: returned ID 0", _clockId.value);

            /* Stop the AV Clock */
            AVCLOCK_STOP_SUCCESS(avClock, avClockController, true);
        }

        /* Close the AV Clock */
        AVCLOCK_CLOSE_SUCCESS(avClock, avClockController, true);
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

/**
 * @brief Test the AV Clock's setVideoSink function in valid scenarios.
 *
 * This test verifies if the setVideoSink function successfully sets valid video sink IDs in READY and STARTED states.
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
 * |Variation / Step|Description|Test Data|Expected Result|Notes|
 * |----------------|-----------|---------|---------------|-----|
 * |01|Retrieve available AV Clock IDs|None|clockIds should not be empty, status.isOk() should be true|Should Pass|
 * |02|Get AV Clock handle for each ID|_clockId|avClock should be valid, status.isOk() should be true|Should Pass|
 * |03|Open the AV Clock|IAVClockControllerListener|avClockController should be valid, status.isOk() should be true, state transitions to READY|Should Pass|
 * |04|Set video sink to UNDEFINED in READY state|IVideoSink.Id.UNDEFINED (-1)|status.isOk() should be true, return true|Should Pass|
 * |05|Set video sink to valid ID in READY state|IVideoSink.Id (0)|status.isOk() should be true, return true|Should Pass|
 * |06|Start the AV Clock|None|status.isOk() should be true, state transitions to STARTING then STARTED|Should Pass|
 * |07|Set video sink to valid ID in STARTED state|IVideoSink.Id (1)|status.isOk() should be true, return true|Should Pass|
 */
UT_ADD_TEST(VTS_L1_AVClock, setVideoSink_pos)
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

        /* Test 1: Set video sink to UNDEFINED in READY state */
        {
            com::rdk::hal::videosink::IVideoSink::Id videoSinkId;
            videoSinkId.value = -1; // IVideoSink.Id.UNDEFINED
            AVCLOCK_SET_VIDEO_SINK_SUCCESS(avClock, avClockController, videoSinkId, true);
            UT_LOG_INFO("setVideoSink to UNDEFINED for clock ID %d in READY: success", _clockId.value);
        }

        /* Test 2: Set video sink to valid ID in READY state */
        {
            com::rdk::hal::videosink::IVideoSink::Id videoSinkId;
            videoSinkId.value = 0; // Valid ID
            AVCLOCK_SET_VIDEO_SINK_SUCCESS(avClock, avClockController, videoSinkId, true);
            UT_LOG_INFO("setVideoSink to ID 0 for clock ID %d in READY: success", _clockId.value);
        }

        /* Test 3: Set video sink to valid ID in STARTED state */
        {
            AVCLOCK_START_SUCCESS(avClock, avClockController, true);
            com::rdk::hal::videosink::IVideoSink::Id videoSinkId;
            videoSinkId.value = 1; // Valid ID
            AVCLOCK_SET_VIDEO_SINK_SUCCESS(avClock, avClockController, videoSinkId, true);
            UT_LOG_INFO("setVideoSink to ID 1 for clock ID %d in STARTED: success", _clockId.value);

            /* Stop the AV Clock */
            AVCLOCK_STOP_SUCCESS(avClock, avClockController, true);
        }

        /* Close the AV Clock */
        AVCLOCK_CLOSE_SUCCESS(avClock, avClockController, true);
    }
}

/**
 * @brief Test the AV Clock's setVideoSink function in invalid scenarios.
 *
 * This test verifies if the setVideoSink function handles invalid IDs and states correctly.
 *
 * **Test Group ID:** Basic: 01@n
 * **Test Case ID:** 028@n
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
 * |06|Set video sink in CLOSED state|IVideoSink.Id (0)|status.isOk() should be false, exception EX_ILLEGAL_STATE, return false|Should Pass|
 * |07|Open the AV Clock again|IAVClockControllerListener|avClockController should be valid, status.isOk() should be true, state transitions to READY|Should Pass|
 * |08|Set video sink to invalid ID in READY state|IVideoSink.Id (NUM_VIDEO_SINK + 1)|status.isOk() should be true, return false|Should Pass|
 */
UT_ADD_TEST(VTS_L1_AVClock, setVideoSink_neg)
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

        /* Test 1: Set video sink in CLOSED state */
        {
            android::sp<com::rdk::hal::avclock::IAVClockController> avClockController;
            android::sp<com::rdk::hal::avclock::IAVClockControllerListener> controllerListener = new AVClockControllerListener();
            AVCLOCK_OPEN_SUCCESS(avClock, controllerListener, avClockController, true);

            /* Close the AV Clock to reach CLOSED */
            AVCLOCK_CLOSE_SUCCESS(avClock, avClockController, true);

            com::rdk::hal::videosink::IVideoSink::Id videoSinkId;
            videoSinkId.value = 0; // Valid ID
            AVCLOCK_SET_VIDEO_SINK_FAILURE_STATE(avClock, avClockController, videoSinkId);

            UT_LOG_INFO("setVideoSink in CLOSED state for clock ID %d: failed as expected", _clockId.value);
        }

        /*TODO*/
        /* Test 2: Set video sink to invalid ID in READY state
        {
            android::sp<com::rdk::hal::avclock::IAVClockController> avClockController;
            android::sp<com::rdk::hal::avclock::IAVClockControllerListener> controllerListener = new AVClockControllerListener();
            AVCLOCK_OPEN_SUCCESS(avClock, controllerListener, avClockController, true);

            com::rdk::hal::videosink::IVideoSink::Id videoSinkId;
            videoSinkId.value = NUM_VIDEO_SINK + 1; // Invalid ID
            AVCLOCK_SET_VIDEO_SINK_FAILURE_INVALID_ID(avClock, avClockController, videoSinkId);

            UT_LOG_INFO("setVideoSink to invalid ID %d for clock ID %d in READY: failed as expected", videoSinkId.value, _clockId.value);
        }*/
    }
}

/**
 * @brief Test the AV Clock's getVideoSink function in valid scenarios.
 *
 * This test verifies if the getVideoSink function correctly returns the set video sink ID in READY and STARTED states.
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
 * |Variation / Step|Description|Test Data|Expected Result|Notes|
 * |----------------|-----------|---------|---------------|-----|
 * |01|Retrieve available AV Clock IDs|None|clockIds should not be empty, status.isOk() should be true|Should Pass|
 * |02|Get AV Clock handle for each ID|_clockId|avClock should be valid, status.isOk() should be true|Should Pass|
 * |03|Open the AV Clock|IAVClockControllerListener|avClockController should be valid, status.isOk() should be true, state transitions to READY|Should Pass|
 * |04|Get video sink in READY state|None|status.isOk() should be true, return IVideoSink.Id.UNDEFINED (-1)|Should Pass|
 * |05|Set video sink to valid ID in READY state|IVideoSink.Id (0)|status.isOk() should be true, return true|Should Pass|
 * |06|Get video sink in READY state|None|status.isOk() should be true, return IVideoSink.Id (0)|Should Pass|
 * |07|Start the AV Clock|None|status.isOk() should be true, state transitions to STARTING then STARTED|Should Pass|
 * |08|Get video sink in STARTED state|None|status.isOk() should be true, return IVideoSink.Id (0)|Should Pass|
 */
UT_ADD_TEST(VTS_L1_AVClock, getVideoSink_pos)
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

        /* Test 1: Get video sink in READY state (default UNDEFINED) */
        {
            com::rdk::hal::videosink::IVideoSink::Id expectedVideoSinkId;
            expectedVideoSinkId.value = -1; // IVideoSink.Id.UNDEFINED
            AVCLOCK_GET_VIDEO_SINK_SUCCESS(avClock, avClockController, expectedVideoSinkId);
            UT_LOG_INFO("getVideoSink in READY state for clock ID %d: returned UNDEFINED", _clockId.value);
        }

        /* Test 2: Set and get video sink in READY state */
        {
            com::rdk::hal::videosink::IVideoSink::Id videoSinkId;
            videoSinkId.value = 0; // Valid ID
            AVCLOCK_SET_VIDEO_SINK_SUCCESS(avClock, avClockController, videoSinkId, true);

            com::rdk::hal::videosink::IVideoSink::Id expectedVideoSinkId;
            expectedVideoSinkId.value = 0;
            AVCLOCK_GET_VIDEO_SINK_SUCCESS(avClock, avClockController, expectedVideoSinkId);
            UT_LOG_INFO("getVideoSink in READY state for clock ID %d: returned ID 0", _clockId.value);
        }

        /* Test 3: Get video sink in STARTED state */
        {
            AVCLOCK_START_SUCCESS(avClock, avClockController, true);
            com::rdk::hal::videosink::IVideoSink::Id expectedVideoSinkId;
            expectedVideoSinkId.value = 0;
            AVCLOCK_GET_VIDEO_SINK_SUCCESS(avClock, avClockController, expectedVideoSinkId);
            UT_LOG_INFO("getVideoSink in STARTED state for clock ID %d: returned ID 0", _clockId.value);

            /* Stop the AV Clock */
            AVCLOCK_STOP_SUCCESS(avClock, avClockController, true);
        }

        /* Close the AV Clock */
        AVCLOCK_CLOSE_SUCCESS(avClock, avClockController, true);
    }
}
/**
 * @brief Test the AV Clock's getVideoSink function in invalid scenarios.
 *
 * This test verifies if the getVideoSink function handles invalid states correctly by throwing EX_ILLEGAL_STATE.
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
 * |Variation / Step|Description|Test Data|Expected Result|Notes|
 * |----------------|-----------|---------|---------------|-----|
 * |01|Retrieve available AV Clock IDs|None|clockIds should not be empty, status.isOk() should be true|Should Pass|
 * |02|Get AV Clock handle for each ID|_clockId|avClock should be valid, status.isOk() should be true|Should Pass|
 * |03|Open the AV Clock|IAVClockControllerListener|avClockController should be valid, status.isOk() should be true, state transitions to READY|Should Pass|
 * |04|Close the AV Clock|avClockController|status.isOk() should be true, state transitions to CLOSED|Should Pass|
 * |05|Open a new controller|IAVClockControllerListener|avClockController should be valid, status.isOk() should be true|Should Pass|
 * |06|Get video sink in CLOSED state|None|status.isOk() should be false, exception EX_ILLEGAL_STATE|Should Pass|
 */
UT_ADD_TEST(VTS_L1_AVClock, getVideoSink_neg)
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

        /* Test 1: Get video sink in CLOSED state */
        {
            android::sp<com::rdk::hal::avclock::IAVClockController> avClockController;
            android::sp<com::rdk::hal::avclock::IAVClockControllerListener> controllerListener = new AVClockControllerListener();
            AVCLOCK_OPEN_SUCCESS(avClock, controllerListener, avClockController, true);

            /* Close the AV Clock to reach CLOSED */
            AVCLOCK_CLOSE_SUCCESS(avClock, avClockController, true);

            AVCLOCK_GET_VIDEO_SINK_FAILURE_STATE(avClock, avClockController);

            UT_LOG_INFO("getVideoSink in CLOSED state for clock ID %d: failed as expected", _clockId.value);
        }
    }
}

/**
 * @brief Test the AV Clock's setClockMode function in valid scenarios.
 *
 * This test verifies if the setClockMode function successfully sets valid clock modes in READY state.
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
 * |Variation / Step|Description|Test Data|Expected Result|Notes|
 * |----------------|-----------|---------|---------------|-----|
 * |01|Retrieve available AV Clock IDs|None|clockIds should not be empty, status.isOk() should be true|Should Pass|
 * |02|Get AV Clock handle for each ID|_clockId|avClock should be valid, status.isOk() should be true|Should Pass|
 * |03|Open the AV Clock|IAVClockControllerListener|avClockController should be valid, status.isOk() should be true, state transitions to READY|Should Pass|
 * |04|Set clock mode to AUTO in READY state|ClockMode.AUTO (0)|status.isOk() should be true, return true|Should Pass|
 * |05|Set clock mode to PCR in READY state|ClockMode.PCR (1)|status.isOk() should be true, return true|Should Pass|
 * |06|Set clock mode to AUDIO_MASTER in READY state|ClockMode.AUDIO_MASTER (2)|status.isOk() should be true, return true|Should Pass|
 * |07|Set clock mode to VIDEO_MASTER in READY state|ClockMode.VIDEO_MASTER (3)|status.isOk() should be true, return true|Should Pass|
 */
UT_ADD_TEST(VTS_L1_AVClock, setClockMode_pos)
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

        /* Test 1: Set clock mode to AUTO in READY state */
        {
            com::rdk::hal::avclock::ClockMode clockMode = com::rdk::hal::avclock::ClockMode::AUTO;
            AVCLOCK_SET_CLOCK_MODE_SUCCESS(avClock, avClockController, clockMode, true);
            UT_LOG_INFO("setClockMode to AUTO for clock ID %d in READY: success", _clockId.value);
        }

        /* Test 2: Set clock mode to PCR in READY state */
        {
            com::rdk::hal::avclock::ClockMode clockMode = com::rdk::hal::avclock::ClockMode::PCR;
            AVCLOCK_SET_CLOCK_MODE_SUCCESS(avClock, avClockController, clockMode, true);
            UT_LOG_INFO("setClockMode to PCR for clock ID %d in READY: success", _clockId.value);
        }

        /* Test 3: Set clock mode to AUDIO_MASTER in READY state */
        {
            com::rdk::hal::avclock::ClockMode clockMode = com::rdk::hal::avclock::ClockMode::AUDIO_MASTER;
            AVCLOCK_SET_CLOCK_MODE_SUCCESS(avClock, avClockController, clockMode, true);
            UT_LOG_INFO("setClockMode to AUDIO_MASTER for clock ID %d in READY: success", _clockId.value);
        }

        /* Test 4: Set clock mode to VIDEO_MASTER in READY state */
        {
            com::rdk::hal::avclock::ClockMode clockMode = com::rdk::hal::avclock::ClockMode::VIDEO_MASTER;
            AVCLOCK_SET_CLOCK_MODE_SUCCESS(avClock, avClockController, clockMode, true);
            UT_LOG_INFO("setClockMode to VIDEO_MASTER for clock ID %d in READY: success", _clockId.value);
        }

        /* Close the AV Clock */
        AVCLOCK_CLOSE_SUCCESS(avClock, avClockController, true);
    }
}

/**
 * @brief Test the AV Clock's setClockMode function in invalid scenarios.
 *
 * This test verifies if the setClockMode function handles invalid modes and states correctly.
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
 * |Variation / Step|Description|Test Data|Expected Result|Notes|
 * |----------------|-----------|---------|---------------|-----|
 * |01|Retrieve available AV Clock IDs|None|clockIds should not be empty, status.isOk() should be true|Should Pass|
 * |02|Get AV Clock handle for each ID|_clockId|avClock should be valid, status.isOk() should be true|Should Pass|
 * |03|Open the AV Clock|IAVClockControllerListener|avClockController should be valid, status.isOk() should be true, state transitions to READY|Should Pass|
 * |04|Set clock mode to invalid mode in READY state|ClockMode (4)|status.isOk() should be true, return false|Should Pass|
 * |05|Start the AV Clock|None|status.isOk() should be true, state transitions to STARTING then STARTED|Should Pass|
 * |06|Set clock mode in STARTED state|ClockMode.AUTO (0)|status.isOk() should be false, exception EX_ILLEGAL_STATE, return false|Should Pass|
 * |07|Stop the AV Clock|None|status.isOk() should be true, state transitions to STOPPING then READY|Should Pass|
 * |08|Close the AV Clock|avClockController|status.isOk() should be true, state transitions to CLOSED|Should Pass|
 * |09|Open a new controller|IAVClockControllerListener|avClockController should be valid, status.isOk() should be true|Should Pass|
 * |10|Set clock mode in CLOSED state|ClockMode.AUTO (0)|status.isOk() should be false, exception EX_ILLEGAL_STATE, return false|Should Pass|
 */
UT_ADD_TEST(VTS_L1_AVClock, setClockMode_neg)
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

        /* Test 1: Set clock mode to invalid mode in READY state */
        {
            com::rdk::hal::avclock::ClockMode clockMode = static_cast<com::rdk::hal::avclock::ClockMode>(4); // Invalid mode
            AVCLOCK_SET_CLOCK_MODE_FAILURE_INVALID_MODE(avClock, avClockController, clockMode);
            UT_LOG_INFO("setClockMode to invalid mode %d for clock ID %d in READY: failed as expected", static_cast<int>(clockMode), _clockId.value);
        }

        /* Test 2: Set clock mode in STARTED state */
        {
            AVCLOCK_START_SUCCESS(avClock, avClockController,true);
            com::rdk::hal::avclock::ClockMode clockMode = com::rdk::hal::avclock::ClockMode::AUTO;
            AVCLOCK_SET_CLOCK_MODE_FAILURE_STATE(avClock, avClockController, clockMode);
            UT_LOG_INFO("setClockMode to AUTO in STARTED state for clock ID %d: failed as expected", _clockId.value);

            /* Stop the clock to return to READY for cleanup */
            AVCLOCK_STOP_SUCCESS(avClock, avClockController,true);
        }

        /* Test 3: Set clock mode in CLOSED state */
        {
            /* Close the AV Clock to reach CLOSED */
            AVCLOCK_CLOSE_SUCCESS(avClock, avClockController, true);

            com::rdk::hal::avclock::ClockMode clockMode = com::rdk::hal::avclock::ClockMode::AUTO;
            AVCLOCK_SET_CLOCK_MODE_FAILURE_STATE(avClock, avClockController, clockMode);
            UT_LOG_INFO("setClockMode to AUTO in CLOSED state for clock ID %d: failed as expected", _clockId.value);
        }
    }
}

/**
 * @brief Test the AV Clock's getClockMode function in valid scenarios.
 *
 * This test verifies if the getClockMode function correctly returns the set clock mode in READY and STARTED states.
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
 * |Variation / Step|Description|Test Data|Expected Result|Notes|
 * |----------------|-----------|---------|---------------|-----|
 * |01|Retrieve available AV Clock IDs|None|clockIds should not be empty, status.isOk() should be true|Should Pass|
 * |02|Get AV Clock handle for each ID|_clockId|avClock should be valid, status.isOk() should be true|Should Pass|
 * |03|Open the AV Clock|IAVClockControllerListener|avClockController should be valid, status.isOk() should be true, state transitions to READY|Should Pass|
 * |04|Get clock mode in READY state|None|status.isOk() should be true, return ClockMode.AUTO (0)|Should Pass|
 * |05|Set clock mode to PCR in READY state|ClockMode.PCR (1)|status.isOk() should be true, return true|Should Pass|
 * |06|Get clock mode in READY state|None|status.isOk() should be true, return ClockMode.PCR (1)|Should Pass|
 * |07|Start the AV Clock|None|status.isOk() should be true, state transitions to STARTING then STARTED|Should Pass|
 * |08|Get clock mode in STARTED state|None|status.isOk() should be true, return ClockMode.PCR (1)|Should Pass|
 */
UT_ADD_TEST(VTS_L1_AVClock, getClockMode_pos)
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

        /* Test 1: Get clock mode in READY state (default AUTO) */
        {
            com::rdk::hal::avclock::ClockMode expectedClockMode = com::rdk::hal::avclock::ClockMode::AUTO;
            AVCLOCK_GET_CLOCK_MODE_SUCCESS(avClock, avClockController, expectedClockMode);
            UT_LOG_INFO("getClockMode in READY state for clock ID %d: returned AUTO", _clockId.value);
        }

        /* Test 2: Set and get clock mode in READY state */
        {
            com::rdk::hal::avclock::ClockMode clockMode = com::rdk::hal::avclock::ClockMode::PCR;
            AVCLOCK_SET_CLOCK_MODE_SUCCESS(avClock, avClockController, clockMode, true);

            com::rdk::hal::avclock::ClockMode expectedClockMode = com::rdk::hal::avclock::ClockMode::PCR;
            AVCLOCK_GET_CLOCK_MODE_SUCCESS(avClock, avClockController, expectedClockMode);
            UT_LOG_INFO("getClockMode in READY state for clock ID %d: returned PCR", _clockId.value);
        }

        /* Test 3: Get clock mode in STARTED state */
        {
            AVCLOCK_START_SUCCESS(avClock, avClockController, true);
            com::rdk::hal::avclock::ClockMode expectedClockMode = com::rdk::hal::avclock::ClockMode::PCR;
            AVCLOCK_GET_CLOCK_MODE_SUCCESS(avClock, avClockController, expectedClockMode);
            UT_LOG_INFO("getClockMode in STARTED state for clock ID %d: returned PCR", _clockId.value);

            /* Stop the AV Clock */
            AVCLOCK_STOP_SUCCESS(avClock, avClockController, true);
        }

        /* Close the AV Clock */
        AVCLOCK_CLOSE_SUCCESS(avClock, avClockController, true);
    }
}

/**
 * @brief Test the AV Clock's getClockMode function in invalid scenarios.
 *
 * This test verifies if the getClockMode function handles invalid states correctly by throwing EX_ILLEGAL_STATE.
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
 * |Variation / Step|Description|Test Data|Expected Result|Notes|
 * |----------------|-----------|---------|---------------|-----|
 * |01|Retrieve available AV Clock IDs|None|clockIds should not be empty, status.isOk() should be true|Should Pass|
 * |02|Get AV Clock handle for each ID|_clockId|avClock should be valid, status.isOk() should be true|Should Pass|
 * |03|Open the AV Clock|IAVClockControllerListener|avClockController should be valid, status.isOk() should be true, state transitions to READY|Should Pass|
 * |04|Close the AV Clock|avClockController|status.isOk() should be true, state transitions to CLOSED|Should Pass|
 * |05|Open a new controller|IAVClockControllerListener|avClockController should be valid, status.isOk() should be true|Should Pass|
 * |06|Get clock mode in CLOSED state|None|status.isOk() should be false, exception EX_ILLEGAL_STATE|Should Pass|
 */
UT_ADD_TEST(VTS_L1_AVClock, getClockMode_neg)
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

        /* Test 1: Get clock mode in CLOSED state */
        {
            android::sp<com::rdk::hal::avclock::IAVClockController> avClockController;
            android::sp<com::rdk::hal::avclock::IAVClockControllerListener> controllerListener = new AVClockControllerListener();
            AVCLOCK_OPEN_SUCCESS(avClock, controllerListener, avClockController, true);

            /* Close the AV Clock to reach CLOSED */
            AVCLOCK_CLOSE_SUCCESS(avClock, avClockController, true);

            AVCLOCK_GET_CLOCK_MODE_FAILURE_STATE(avClock, avClockController);

            UT_LOG_INFO("getClockMode in CLOSED state for clock ID %d: failed as expected", _clockId.value);
        }
    }
}

/**
 * @brief Test the AV Clock's notifyPCRSample function in valid scenarios.
 *
 * This test verifies if the notifyPCRSample function successfully notifies a PCR sample in STARTED state with PCR clock mode.
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
 * |Variation / Step|Description|Test Data|Expected Result|Notes|
 * |----------------|-----------|---------|---------------|-----|
 * |01|Retrieve available AV Clock IDs|None|clockIds should not be empty, status.isOk() should be true|Should Pass|
 * |02|Get AV Clock handle for each ID|_clockId|avClock should be valid, status.isOk() should be true|Should Pass|
 * |03|Open the AV Clock|IAVClockControllerListener|avClockController should be valid, status.isOk() should be true, state transitions to READY|Should Pass|
 * |04|Set clock mode to PCR in READY state|ClockMode.PCR (1)|status.isOk() should be true, return true|Should Pass|
 * |05|Start the AV Clock|None|status.isOk() should be true, state transitions to STARTING then STARTED|Should Pass|
 * |06|Notify PCR sample in STARTED state with PCR mode|pcrTimeNs=1000000000, sampleTimestampNs=2000000000|status.isOk() should be true, return true|Should Pass|
 */
UT_ADD_TEST(VTS_L1_AVClock, notifyPCRSample_pos)
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

        /* Set clock mode to PCR */
        com::rdk::hal::avclock::ClockMode clockMode = com::rdk::hal::avclock::ClockMode::PCR;
        AVCLOCK_SET_CLOCK_MODE_SUCCESS(avClock, avClockController, clockMode, true);

        /* Start the AV Clock to reach STARTED */
        AVCLOCK_START_SUCCESS(avClock, avClockController, true);

        /* Test 1: Notify PCR sample in STARTED state with PCR mode */
        {
            int64_t pcrTimeNs = 1000000000LL; // 1 second
            int64_t sampleTimestampNs = 2000000000LL; // 2 seconds
            AVCLOCK_NOTIFY_PCR_SAMPLE_SUCCESS(avClock, avClockController, pcrTimeNs, sampleTimestampNs, true);
            UT_LOG_INFO("notifyPCRSample with pcrTimeNs=%lld, sampleTimestampNs=%lld for clock ID %d in STARTED with PCR mode: success",
                        static_cast<long long>(pcrTimeNs), static_cast<long long>(sampleTimestampNs), _clockId.value);
        }

        /* Stop the AV Clock */
        AVCLOCK_STOP_SUCCESS(avClock, avClockController, true);

        /* Close the AV Clock */
        AVCLOCK_CLOSE_SUCCESS(avClock, avClockController, true);
    }
}

/**
 * @brief Test the AV Clock's notifyPCRSample function in invalid scenarios.
 *
 * This test verifies if the notifyPCRSample function handles invalid parameters, states, and modes correctly.
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
 * |Variation / Step|Description|Test Data|Expected Result|Notes|
 * |----------------|-----------|---------|---------------|-----|
 * |01|Retrieve available AV Clock IDs|None|clockIds should not be empty, status.isOk() should be true|Should Pass|
 * |02|Get AV Clock handle for each ID|_clockId|avClock should be valid, status.isOk() should be true|Should Pass|
 * |03|Open the AV Clock|IAVClockControllerListener|avClockController should be valid, status.isOk() should be true, state transitions to READY|Should Pass|
 * |04|Notify PCR sample in READY state with PCR mode|pcrTimeNs=1000000000, sampleTimestampNs=2000000000|status.isOk() should be false, exception EX_ILLEGAL_STATE, return false|Should Pass|
 * |05|Set clock mode to PCR in READY state|ClockMode.PCR (1)|status.isOk() should be true, return true|Should Pass|
 * |06|Start the AV Clock|None|status.isOk() should be true, state transitions to STARTING then STARTED|Should Pass|
 * |07|Notify PCR sample with invalid pcrTimeNs|pcrTimeNs=-1, sampleTimestampNs=2000000000|status.isOk() should be true, return false|Should Pass|
 * |08|Notify PCR sample with invalid sampleTimestampNs|pcrTimeNs=1000000000, sampleTimestampNs=-1|status.isOk() should be true, return false|Should Pass|
 * |09|Stop the AV Clock|None|status.isOk() should be true, state transitions to STOPPING then READY|Should Pass|
 * |10|Set clock mode to AUTO in READY state|ClockMode.AUTO (0)|status.isOk() should be true, return true|Should Pass|
 * |11|Start the AV Clock|None|status.isOk() should be true, state transitions to STARTING then STARTED|Should Pass|
 * |12|Notify PCR sample in STARTED state with AUTO mode|pcrTimeNs=1000000000, sampleTimestampNs=2000000000|status.isOk() should be false, exception EX_ILLEGAL_STATE, return false|Should Pass|
 * |13|Stop the AV Clock|None|status.isOk() should be true, state transitions to STOPPING then READY|Should Pass|
 * |14|Close the AV Clock|avClockController|status.isOk() should be true, state transitions to CLOSED|Should Pass|
 * |15|Open a new controller|IAVClockControllerListener|avClockController should be valid, status.isOk() should be true|Should Pass|
 * |16|Notify PCR sample in CLOSED state|pcrTimeNs=1000000000, sampleTimestampNs=2000000000|status.isOk() should be false, exception EX_ILLEGAL_STATE, return false|Should Pass|
 */
UT_ADD_TEST(VTS_L1_AVClock, notifyPCRSample_neg)
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

        /* Test 1: Notify PCR sample in READY state with PCR mode */
        {
            com::rdk::hal::avclock::ClockMode clockMode = com::rdk::hal::avclock::ClockMode::PCR;
            AVCLOCK_SET_CLOCK_MODE_SUCCESS(avClock, avClockController, clockMode, true);

            int64_t pcrTimeNs = 1000000000LL;
            int64_t sampleTimestampNs = 2000000000LL;
            AVCLOCK_NOTIFY_PCR_SAMPLE_FAILURE_STATE_OR_MODE(avClock, avClockController, pcrTimeNs, sampleTimestampNs);
            UT_LOG_INFO("notifyPCRSample in READY state for clock ID %d: failed as expected", _clockId.value);
        }

        /* Test 2: Notify PCR sample with invalid pcrTimeNs in STARTED state with PCR mode */
        {
            AVCLOCK_START_SUCCESS(avClock, avClockController, true);

            int64_t pcrTimeNs = -1LL;
            int64_t sampleTimestampNs = 2000000000LL;
            AVCLOCK_NOTIFY_PCR_SAMPLE_FAILURE_INVALID_PARAMS(avClock, avClockController, pcrTimeNs, sampleTimestampNs);
            UT_LOG_INFO("notifyPCRSample with invalid pcrTimeNs=%lld for clock ID %d: failed as expected",
                        static_cast<long long>(pcrTimeNs), _clockId.value);

            /* Stop the AV Clock */
            AVCLOCK_STOP_SUCCESS(avClock, avClockController, true);
        }

        /* Test 3: Notify PCR sample with invalid sampleTimestampNs in STARTED state with PCR mode */
        {
            AVCLOCK_START_SUCCESS(avClock, avClockController, true);

            int64_t pcrTimeNs = 1000000000LL;
            int64_t sampleTimestampNs = -1LL;
            AVCLOCK_NOTIFY_PCR_SAMPLE_FAILURE_INVALID_PARAMS(avClock, avClockController, pcrTimeNs, sampleTimestampNs);
            UT_LOG_INFO("notifyPCRSample with invalid sampleTimestampNs=%lld for clock ID %d: failed as expected",
                        static_cast<long long>(sampleTimestampNs), _clockId.value);

            /* Stop the AV Clock */
            AVCLOCK_STOP_SUCCESS(avClock, avClockController, true);
        }

        /* Test 4: Notify PCR sample in STARTED state with AUTO mode */
        {
            com::rdk::hal::avclock::ClockMode clockMode = com::rdk::hal::avclock::ClockMode::AUTO;
            AVCLOCK_SET_CLOCK_MODE_SUCCESS(avClock, avClockController, clockMode, true);

            AVCLOCK_START_SUCCESS(avClock, avClockController, true);

            int64_t pcrTimeNs = 1000000000LL;
            int64_t sampleTimestampNs = 2000000000LL;
            AVCLOCK_NOTIFY_PCR_SAMPLE_FAILURE_STATE_OR_MODE(avClock, avClockController, pcrTimeNs, sampleTimestampNs);
            UT_LOG_INFO("notifyPCRSample in STARTED state with AUTO mode for clock ID %d: failed as expected", _clockId.value);

            /* Stop the AV Clock */
            AVCLOCK_STOP_SUCCESS(avClock, avClockController, true);
        }

        /* Test 5: Notify PCR sample in CLOSED state */
        {
            /* Close the AV Clock to reach CLOSED */
            AVCLOCK_CLOSE_SUCCESS(avClock, avClockController, true);

            /* Create a new controller */
            avClockController = nullptr;
            controllerListener = new AVClockControllerListener();
            AVCLOCK_OPEN_SUCCESS(avClock, controllerListener, avClockController, false); // No state check, as state is CLOSED

            int64_t pcrTimeNs = 1000000000LL;
            int64_t sampleTimestampNs = 2000000000LL;
            AVCLOCK_NOTIFY_PCR_SAMPLE_FAILURE_STATE_OR_MODE(avClock, avClockController, pcrTimeNs, sampleTimestampNs);
            UT_LOG_INFO("notifyPCRSample in CLOSED state for clock ID %d: failed as expected", _clockId.value);

            /* Close the AV Clock */
            AVCLOCK_CLOSE_SUCCESS(avClock, avClockController, true);
        }
    }
}

/**
 * @brief Test the AV Clock's getCurrentClockTime function in valid scenarios.
 *
 * This test verifies if the getCurrentClockTime function correctly returns the clock time in STARTED state for different clock modes.
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
 * |Variation / Step|Description|Test Data|Expected Result|Notes|
 * |----------------|-----------|---------|---------------|-----|
 * |01|Retrieve available AV Clock IDs|None|clockIds should not be empty, status.isOk() should be true|Should Pass|
 * |02|Get AV Clock handle for each ID|_clockId|avClock should be valid, status.isOk() should be true|Should Pass|
 * |03|Open the AV Clock|IAVClockControllerListener|avClockController should be valid, status.isOk() should be true, state transitions to READY|Should Pass|
 * |04|Set clock mode to AUTO in READY state|ClockMode.AUTO (0)|status.isOk() should be true, return true|Should Pass|
 * |05|Start the AV Clock|None|status.isOk() should be true, state transitions to STARTING then STARTED|Should Pass|
 * |06|Get current clock time in STARTED state with AUTO mode|None|status.isOk() should be true, clockTimeNs >= 0, sampleTimestampNs >= 0|Should Pass|
 * |07|Stop the AV Clock|None|status.isOk() should be true, state transitions to STOPPING then READY|Should Pass|
 * |08|Set clock mode to PCR in READY state|ClockMode.PCR (1)|status.isOk() should be true, return true|Should Pass|
 * |09|Start the AV Clock|None|status.isOk() should be true, state transitions to STARTING then STARTED|Should Pass|
 * |10|Notify PCR sample in STARTED state with PCR mode|pcrTimeNs=1000000000, sampleTimestampNs=2000000000|status.isOk() should be true, return true|Should Pass|
 * |11|Get current clock time in STARTED state with PCR mode|None|status.isOk() should be true, clockTimeNs=1000000000, sampleTimestampNs >= 0|Should Pass|
 */
UT_ADD_TEST(VTS_L1_AVClock, getCurrentClockTime_pos)
{
    std::vector<com::rdk::hal::avclock::IAVClock::Id> clockIds;
    android::sp<com::rdk::hal::avclock::IAVClock> avClock;
    com::rdk::hal::avclock::ClockTime getclocktime;
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

        /* Test 1: Get current clock time in STARTED state with AUTO mode */
        {
            com::rdk::hal::avclock::ClockMode clockMode = com::rdk::hal::avclock::ClockMode::AUTO;
            AVCLOCK_SET_CLOCK_MODE_SUCCESS(avClock, avClockController, clockMode, true);

            AVCLOCK_START_SUCCESS(avClock, avClockController, true);

            AVCLOCK_GET_CURRENT_CLOCK_TIME_SUCCESS(avClock, avClockController, getclocktime); // No exact clockTimeNs check
            UT_LOG_INFO("getCurrentClockTime in STARTED state with AUTO mode for clock ID %d: success", _clockId.value);

            AVCLOCK_STOP_SUCCESS(avClock, avClockController, true);
        }

        /* Test 2: Get current clock time in STARTED state with PCR mode */
        {
            com::rdk::hal::avclock::ClockMode clockMode = com::rdk::hal::avclock::ClockMode::PCR;
            AVCLOCK_SET_CLOCK_MODE_SUCCESS(avClock, avClockController, clockMode, true);

            AVCLOCK_START_SUCCESS(avClock, avClockController, true);

            int64_t pcrTimeNs = 1000000000LL;
            int64_t sampleTimestampNs = 2000000000LL;
            AVCLOCK_NOTIFY_PCR_SAMPLE_SUCCESS(avClock, avClockController, pcrTimeNs, sampleTimestampNs, true);

            AVCLOCK_GET_CURRENT_CLOCK_TIME_SUCCESS(avClock, avClockController, getclocktime); // Exact clockTimeNs check
            UT_LOG_INFO("getCurrentClockTime in STARTED state with PCR mode for clock ID %d: success", _clockId.value);

            AVCLOCK_STOP_SUCCESS(avClock, avClockController, true);
        }

        /* Close the AV Clock */
        AVCLOCK_CLOSE_SUCCESS(avClock, avClockController, true);
    }
}

/**
 * @brief Test the AV Clock's getCurrentClockTime function in invalid scenarios.
 *
 * This test verifies if the getCurrentClockTime function handles invalid states correctly by throwing EX_ILLEGAL_STATE.
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
 * |Variation / Step|Description|Test Data|Expected Result|Notes|
 * |----------------|-----------|---------|---------------|-----|
 * |01|Retrieve available AV Clock IDs|None|clockIds should not be empty, status.isOk() should be true|Should Pass|
 * |02|Get AV Clock handle for each ID|_clockId|avClock should be valid, status.isOk() should be true|Should Pass|
 * |03|Open the AV Clock|IAVClockControllerListener|avClockController should be valid, status.isOk() should be true, state transitions to READY|Should Pass|
 * |04|Get current clock time in READY state|None|status.isOk() should be false, exception EX_ILLEGAL_STATE|Should Pass|
 * |05|Close the AV Clock|avClockController|status.isOk() should be true, state transitions to CLOSED|Should Pass|
 * |06|Open a new controller|IAVClockControllerListener|avClockController should be valid, status.isOk() should be true|Should Pass|
 * |07|Get current clock time in CLOSED state|None|status.isOk() should be false, exception EX_ILLEGAL_STATE|Should Pass|
 */
UT_ADD_TEST(VTS_L1_AVClock, getCurrentClockTime_neg)
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

        /* Test 1: Get current clock time in READY state */
        {
            AVCLOCK_GET_CURRENT_CLOCK_TIME_FAILURE_STATE(avClock, avClockController);
            UT_LOG_INFO("getCurrentClockTime in READY state for clock ID %d: failed as expected", _clockId.value);
        }

        /* Test 2: Get current clock time in CLOSED state */
        {
            /* Close the AV Clock to reach CLOSED */
            AVCLOCK_CLOSE_SUCCESS(avClock, avClockController, true);

            /* Create a new controller */
            avClockController = nullptr;
            controllerListener = new AVClockControllerListener();
            AVCLOCK_OPEN_SUCCESS(avClock, controllerListener, avClockController, false); // No state check, as state is CLOSED

            AVCLOCK_GET_CURRENT_CLOCK_TIME_FAILURE_STATE(avClock, avClockController);
            UT_LOG_INFO("getCurrentClockTime in CLOSED state for clock ID %d: failed as expected", _clockId.value);

            /* Close the AV Clock */
            AVCLOCK_CLOSE_SUCCESS(avClock, avClockController, true);
        }
    }
}

/**
 * @brief Test the AV Clock's setPlaybackRate function in valid scenarios.
 *
 * This test verifies if the setPlaybackRate function successfully sets valid playback rates in READY and STARTED states with non-PCR clock modes.
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
 * |Variation / Step|Description|Test Data|Expected Result|Notes|
 * |----------------|-----------|---------|---------------|-----|
 * |01|Retrieve available AV Clock IDs|None|clockIds should not be empty, status.isOk() should be true|Should Pass|
 * |02|Get AV Clock handle for each ID|_clockId|avClock should be valid, status.isOk() should be true|Should Pass|
 * |03|Open the AV Clock|IAVClockControllerListener|avClockController should be valid, status.isOk() should be true, state transitions to READY|Should Pass|
 * |04|Set clock mode to AUTO in READY state|ClockMode.AUTO (0)|status.isOk() should be true, return true|Should Pass|
 * |05|Set playback rate to 0.0 in READY state|rate=0.0|status.isOk() should be true, return true|Should Pass|
 * |06|Set playback rate to 1.0 in READY state|rate=1.0|status.isOk() should be true, return true|Should Pass|
 * |07|Set playback rate to 0.5 in READY state|rate=0.5|status.isOk() should be true, return true|Should Pass|
 * |08|Set playback rate to 2.0 in READY state|rate=2.0|status.isOk() should be true, return true|Should Pass|
 * |09|Start the AV Clock|None|status.isOk() should be true, state transitions to STARTING then STARTED|Should Pass|
 * |10|Set playback rate to 1.5 in STARTED state|rate=1.5|status.isOk() should be true, return true|Should Pass|
 */
UT_ADD_TEST(VTS_L1_AVClock, setPlaybackRate_pos)
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

        /* Set clock mode to AUTO */
        com::rdk::hal::avclock::ClockMode clockMode = com::rdk::hal::avclock::ClockMode::AUTO;
        AVCLOCK_SET_CLOCK_MODE_SUCCESS(avClock, avClockController, clockMode, true);

        /* Test 1: Set playback rate to 0.0 in READY state */
        {
            double rate = 0.0; // Pause
            AVCLOCK_SET_PLAYBACK_RATE_SUCCESS(avClock, avClockController, rate, true);
            UT_LOG_INFO("setPlaybackRate to %.2f for clock ID %d in READY: success", rate, _clockId.value);
        }

        /* Test 2: Set playback rate to 1.0 in READY state */
        {
            double rate = 1.0; // Normal speed
            AVCLOCK_SET_PLAYBACK_RATE_SUCCESS(avClock, avClockController, rate, true);
            UT_LOG_INFO("setPlaybackRate to %.2f for clock ID %d in READY: success", rate, _clockId.value);
        }

        /* Test 3: Set playback rate to 0.5 in READY state */
        {
            double rate = 0.5; // Slow with pitch correction
            AVCLOCK_SET_PLAYBACK_RATE_SUCCESS(avClock, avClockController, rate, true);
            UT_LOG_INFO("setPlaybackRate to %.2f for clock ID %d in READY: success", rate, _clockId.value);
        }

        /* Test 4: Set playback rate to 2.0 in READY state */
        {
            double rate = 2.0; // Fast with pitch correction
            AVCLOCK_SET_PLAYBACK_RATE_SUCCESS(avClock, avClockController, rate, true);
            UT_LOG_INFO("setPlaybackRate to %.2f for clock ID %d in READY: success", rate, _clockId.value);
        }

        /* Test 5: Set playback rate to 1.5 in STARTED state */
        {
            AVCLOCK_START_SUCCESS(avClock, avClockController, true);
            double rate = 1.5; // Fast with pitch correction
            AVCLOCK_SET_PLAYBACK_RATE_SUCCESS(avClock, avClockController, rate, true);
            UT_LOG_INFO("setPlaybackRate to %.2f for clock ID %d in STARTED: success", rate, _clockId.value);

            /* Stop the AV Clock */
            AVCLOCK_STOP_SUCCESS(avClock, avClockController, true);
        }

        /* Close the AV Clock */
        AVCLOCK_CLOSE_SUCCESS(avClock, avClockController, true);
    }
}

/**
 * @brief Test the AV Clock's setPlaybackRate function in invalid scenarios.
 *
 * This test verifies if the setPlaybackRate function handles invalid rates, states, and PCR mode correctly.
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
 * |Variation / Step|Description|Test Data|Expected Result|Notes|
 * |----------------|-----------|---------|---------------|-----|
 * |01|Retrieve available AV Clock IDs|None|clockIds should not be empty, status.isOk() should be true|Should Pass|
 * |02|Get AV Clock handle for each ID|_clockId|avClock should be valid, status.isOk() should be true|Should Pass|
 * |03|Open the AV Clock|IAVClockControllerListener|avClockController should be valid, status.isOk() should be true, state transitions to READY|Should Pass|
 * |04|Set clock mode to AUTO in READY state|ClockMode.AUTO (0)|status.isOk() should be true, return true|Should Pass|
 * |05|Set playback rate to invalid negative rate in READY state|rate=-1.0|status.isOk() should be true, return false|Should Pass|
 * |06|Set playback rate to high rate in READY state|rate=3.0|status.isOk() should be true, return true|Mock accepts, but real impl may reject|
 * |07|Set clock mode to PCR in READY state|ClockMode.PCR (1)|status.isOk() should be true, return true|Should Pass|
 * |08|Set playback rate in READY state with PCR mode|rate=1.0|status.isOk() should be false, exception EX_ILLEGAL_STATE, return false|Should Pass|
 * |09|Start the AV Clock|None|status.isOk() should be true, state transitions to STARTING then STARTED|Should Pass|
 * |10|Set playback rate in STARTED state with PCR mode|rate=1.0|status.isOk() should be false, exception EX_ILLEGAL_STATE, return false|Should Pass|
 * |11|Stop the AV Clock|None|status.isOk() should be true, state transitions to STOPPING then READY|Should Pass|
 * |12|Close the AV Clock|avClockController|status.isOk() should be true, state transitions to CLOSED|Should Pass|
 * |13|Open a new controller|IAVClockControllerListener|avClockController should be valid, status.isOk() should be true|Should Pass|
 * |14|Set playback rate in CLOSED state|rate=1.0|status.isOk() should be false, exception EX_ILLEGAL_STATE, return false|Should Pass|
 */
UT_ADD_TEST(VTS_L1_AVClock, setPlaybackRate_neg)
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

        /* Set clock mode to AUTO */
        com::rdk::hal::avclock::ClockMode clockMode = com::rdk::hal::avclock::ClockMode::AUTO;
        AVCLOCK_SET_CLOCK_MODE_SUCCESS(avClock, avClockController, clockMode, true);

        /* Test 1: Set playback rate to invalid negative rate in READY state */
        {
            double rate = -1.0;
            AVCLOCK_SET_PLAYBACK_RATE_FAILURE_INVALID_RATE(avClock, avClockController, rate);
            UT_LOG_INFO("setPlaybackRate to %.2f for clock ID %d in READY: failed as expected", rate, _clockId.value);
        }

        /* Test 2: Set playback rate to high rate in READY state */
        {
            double rate = 3.0; // Mock accepts, but real impl may reject
            AVCLOCK_SET_PLAYBACK_RATE_FAILURE_INVALID_RATE(avClock, avClockController, rate); // Note: Mock-specific behavior
            UT_LOG_INFO("setPlaybackRate to %.2f for clock ID %d in READY: success (mock accepts, real impl may reject)", rate, _clockId.value);
        }

        /* Test 3: Set playback rate in READY state with PCR mode */
        {
            clockMode = com::rdk::hal::avclock::ClockMode::PCR;
            AVCLOCK_SET_CLOCK_MODE_SUCCESS(avClock, avClockController, clockMode, true);

            double rate = 1.0;
            AVCLOCK_SET_PLAYBACK_RATE_FAILURE_STATE_OR_MODE(avClock, avClockController, rate);
            UT_LOG_INFO("setPlaybackRate to %.2f in READY state with PCR mode for clock ID %d: failed as expected", rate, _clockId.value);
        }

        /* Test 4: Set playback rate in STARTED state with PCR mode */
        {
            AVCLOCK_START_SUCCESS(avClock, avClockController, true);

            double rate = 1.0;
            AVCLOCK_SET_PLAYBACK_RATE_FAILURE_STATE_OR_MODE(avClock, avClockController, rate);
            UT_LOG_INFO("setPlaybackRate to %.2f in STARTED state with PCR mode for clock ID %d: failed as expected", rate, _clockId.value);

            /* Stop the AV Clock */
            AVCLOCK_STOP_SUCCESS(avClock, avClockController, true);
        }

        /* Test 5: Set playback rate in CLOSED state */
        {
            /* Close the AV Clock to reach CLOSED */
            AVCLOCK_CLOSE_SUCCESS(avClock, avClockController, true);

            double rate = 1.0;
            AVCLOCK_SET_PLAYBACK_RATE_FAILURE_STATE_OR_MODE(avClock, avClockController, rate);
            UT_LOG_INFO("setPlaybackRate to %.2f in CLOSED state for clock ID %d: failed as expected", rate, _clockId.value);
        }
    }
}

/**
 * @brief Test the AV Clock's getPlaybackRate function in valid scenarios.
 *
 * This test verifies if the getPlaybackRate function correctly returns the set playback rate in READY and STARTED states with non-PCR clock modes.
 *
 * **Test Group ID:** Basic: 01@n
 * **Test Case ID:** 041@n
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
 * |04|Set clock mode to AUTO in READY state|ClockMode.AUTO (0)|status.isOk() should be true, return true|Should Pass|
 * |05|Get playback rate in READY state|None|status.isOk() should be true, return 1.0|Should Pass|
 * |06|Set playback rate to 0.5 in READY state|rate=0.5|status.isOk() should be true, return true|Should Pass|
 * |07|Get playback rate in READY state|None|status.isOk() should be true, return 0.5|Should Pass|
 * |08|Start the AV Clock|None|status.isOk() should be true, state transitions to STARTING then STARTED|Should Pass|
 * |09|Get playback rate in STARTED state|None|status.isOk() should be true, return 0.5|Should Pass|
 */
UT_ADD_TEST(VTS_L1_AVClock, getPlaybackRate_pos)
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

        /* Set clock mode to AUTO */
        com::rdk::hal::avclock::ClockMode clockMode = com::rdk::hal::avclock::ClockMode::AUTO;
        AVCLOCK_SET_CLOCK_MODE_SUCCESS(avClock, avClockController, clockMode, true);

        /* Test 1: Get playback rate in READY state (default 1.0) */
        {
            double expectedRate = 1.0;
            AVCLOCK_GET_PLAYBACK_RATE_SUCCESS(avClock, avClockController, expectedRate);
            UT_LOG_INFO("getPlaybackRate in READY state for clock ID %d: returned %.2f", _clockId.value, expectedRate);
        }

        /* Test 2: Set and get playback rate in READY state */
        {
            double rate = 0.5;
            AVCLOCK_SET_PLAYBACK_RATE_SUCCESS(avClock, avClockController, rate, true);

            double expectedRate = 0.5;
            AVCLOCK_GET_PLAYBACK_RATE_SUCCESS(avClock, avClockController, expectedRate);
            UT_LOG_INFO("getPlaybackRate in READY state for clock ID %d: returned %.2f", _clockId.value, expectedRate);
        }

        /* Test 3: Get playback rate in STARTED state */
        {
            AVCLOCK_START_SUCCESS(avClock, avClockController, true);
            double expectedRate = 0.5;
            AVCLOCK_GET_PLAYBACK_RATE_SUCCESS(avClock, avClockController, expectedRate);
            UT_LOG_INFO("getPlaybackRate in STARTED state for clock ID %d: returned %.2f", _clockId.value, expectedRate);

            /* Stop the AV Clock */
            AVCLOCK_STOP_SUCCESS(avClock, avClockController, true);
        }

        /* Close the AV Clock */
        AVCLOCK_CLOSE_SUCCESS(avClock, avClockController, true);
    }
}

/**
 * @brief Test the AV Clock's getPlaybackRate function in invalid scenarios.
 *
 * This test verifies if the getPlaybackRate function handles invalid states and PCR mode correctly by throwing EX_ILLEGAL_STATE.
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
 * |Variation / Step|Description|Test Data|Expected Result|Notes|
 * |----------------|-----------|---------|---------------|-----|
 * |01|Retrieve available AV Clock IDs|None|clockIds should not be empty, status.isOk() should be true|Should Pass|
 * |02|Get AV Clock handle for each ID|_clockId|avClock should be valid, status.isOk() should be true|Should Pass|
 * |03|Open the AV Clock|IAVClockControllerListener|avClockController should be valid, status.isOk() should be true, state transitions to READY|Should Pass|
 * |04|Set clock mode to PCR in READY state|ClockMode.PCR (1)|status.isOk() should be true, return true|Should Pass|
 * |05|Get playback rate in READY state with PCR mode|None|status.isOk() should be false, exception EX_ILLEGAL_STATE|Should Pass|
 * |06|Start the AV Clock|None|status.isOk() should be true, state transitions to STARTING then STARTED|Should Pass|
 * |07|Get playback rate in STARTED state with PCR mode|None|status.isOk() should be false, exception EX_ILLEGAL_STATE|Should Pass|
 * |08|Stop the AV Clock|None|status.isOk() should be true, state transitions to STOPPING then READY|Should Pass|
 * |09|Close the AV Clock|avClockController|status.isOk() should be true, state transitions to CLOSED|Should Pass|
 * |10|Open a new controller|IAVClockControllerListener|avClockController should be valid, status.isOk() should be true|Should Pass|
 * |11|Get playback rate in CLOSED state|None|status.isOk() should be false, exception EX_ILLEGAL_STATE|Should Pass|
 */
UT_ADD_TEST(VTS_L1_AVClock, getPlaybackRate_neg)
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

        /* Test 1: Get playback rate in READY state with PCR mode */
        {
            com::rdk::hal::avclock::ClockMode clockMode = com::rdk::hal::avclock::ClockMode::PCR;
            AVCLOCK_SET_CLOCK_MODE_SUCCESS(avClock, avClockController, clockMode, true);

            AVCLOCK_GET_PLAYBACK_RATE_FAILURE_STATE_OR_MODE(avClock, avClockController);
            UT_LOG_INFO("getPlaybackRate in READY state with PCR mode for clock ID %d: failed as expected", _clockId.value);
        }

        /* Test 2: Get playback rate in STARTED state with PCR mode */
        {
            AVCLOCK_START_SUCCESS(avClock, avClockController, true);

            AVCLOCK_GET_PLAYBACK_RATE_FAILURE_STATE_OR_MODE(avClock, avClockController);
            UT_LOG_INFO("getPlaybackRate in STARTED state with PCR mode for clock ID %d: failed as expected", _clockId.value);

            /* Stop the AV Clock */
            AVCLOCK_STOP_SUCCESS(avClock, avClockController, true);
        }

        /* Close the AV Clock */
        AVCLOCK_CLOSE_SUCCESS(avClock, avClockController, true);
    }
}

/** @} */ // End of AV_CLOCK_HALTEST_L1
/** @} */ // End of AV_CLOCK_HALTEST
/** @} */ // End of AV_CLOCK
/** @} */ // End of HPK
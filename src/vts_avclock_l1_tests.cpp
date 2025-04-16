
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

 #include <ut_kvp_profile.h>
 #include <ut.h>
 #include <ut_log.h>
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
 
 static sp<IAVClockManager> gAVClockManagerService = nullptr;
 
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
 
 int getService()
 {
     gAVClockManagerService = android::interface_cast<IAVClockManager>(
         defaultServiceManager()->getService(
             String16(IAVClockManager::serviceName().c_str())));
     if (gAVClockManagerService == nullptr)
     {
         printf("Service not available\n");
         return -1;
     }
     return 0;
 }
 
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
         UT_ASSERT_EQUAL(getService(), 0);
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
 
 UT_ADD_TEST(AVClockManagerBaseTest, test_l1_AVClockManager_positive_getAVClockIds)
 {
     UT_LOG_INFO("Calling getAVClockIds(OUT:_aidl_return[])");
     std::vector<IAVClock::Id> avClockIds;
     Status status = gAVClockManagerService->getAVClockIds(&avClockIds);
 
     UT_LOG_INFO("Result getAVClockIds(OUT:_aidl_return[%zu]), status:[isOK:%s, exception:%s]",
                 avClockIds.size(),
                 UT_Control_GetMapString(bool_mapTable, status.isOk()),
                 UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
 
     UT_ASSERT_TRUE(status.isOk());
     UT_ASSERT_FALSE(avClockIds.empty());
 
     std::string avclockcount_path = "IAVClockManager.IAVClock";
     size_t expectedCount = UT_KVP_PROFILE_GET_LIST_COUNT(avclockcount_path.c_str());
     UT_ASSERT_EQUAL(avClockIds.size(), expectedCount);
 
     for (const auto& id : avClockIds)
     {
         bool found = false;
         for (size_t i = 0; i < expectedCount; ++i)
         {
             std::string avclockid_path = "IAVClockManager.IAVClock."+std::to_string(i);
             if (id.value == UT_KVP_PROFILE_GET_UINT32(avclockid_path.c_str()));
             {
                 found = true;
                 break;
             }
         }
         UT_ASSERT_TRUE(found);
     }
 }
 
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
 UT_ADD_TEST(AVClockManagerBaseTest, test_l1_AVClockManager_positive_getAVClock)
 {
     Status status;
     IAVClock::Id avClockId;
     std::vector<uint32_t> Ids;
 
     std::string avclockcount_path = "IAVClockManager.IAVClock";
     size_t expectedCount = UT_KVP_PROFILE_GET_LIST_COUNT(avclockcount_path.c_str());
 
     for (size_t i = 0; i < expectedCount; ++i)
     {
         std::string avclockid_path = "IAVClockManager.IAVClock."+std::to_string(i);
         Ids.push_back(UT_KVP_PROFILE_GET_UINT32(avclockid_path.c_str()));
     }
 
     for (const auto& Id : Ids)
     {
         avClockId.value = Id;
         sp<IAVClock> avClock;
         UT_LOG_INFO("Calling getAVClock(IN:avClockId:[%d], OUT:avClock[])", avClockId.value);
         status = gAVClockManagerService->getAVClock(avClockId, &avClock);
         UT_LOG_INFO("Result getAVClock(IN:avClockId:[%d], OUT:avClock[0x%X]), status:[isOK:%s, exception:%s]",
                     avClockId.value, avClock.get(), UT_Control_GetMapString(bool_mapTable, status.isOk()),
                     UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
         UT_ASSERT_TRUE(status.isOk());
         UT_ASSERT_NOT_EQUAL(avClock, nullptr);
     }
 }
 
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
 UT_ADD_TEST(AVClockManagerBaseTest, test_l1_AVClockManager_negative_getAVClock)
 {
     Status status;
     std::vector<IAVClock::Id> avClockIds;
     std::vector<uint32_t> Ids;
 
     std::string avclockcount_path = "IAVClockManager.IAVClock";
     size_t expectedCount = UT_KVP_PROFILE_GET_LIST_COUNT(avclockcount_path.c_str());
 
     for (size_t i = 0; i < expectedCount; ++i)
     {
         std::string avclockid_path = "IAVClockManager.IAVClock."+std::to_string(i);
         Ids.push_back(UT_KVP_PROFILE_GET_UINT32(avclockid_path.c_str()));
     }
     std::sort(Ids.begin(),Ids.end());
 
     // Calculate invalid IDs
     IAVClock::Id invalidLowId;
     invalidLowId.value = Ids.front() - 1;
     IAVClock::Id invalidHighId;
     invalidHighId.value = Ids.back() + 1;
 
     // Test with invalid low ID
     sp<IAVClock> avClock;
     UT_LOG_INFO("Calling getAVClock(IN:avClockId:[%d], OUT:avClock[])", invalidLowId.value);
     status = gAVClockManagerService->getAVClock(invalidLowId, &avClock);
     UT_LOG_INFO("Result getAVClock(IN:avClockId:[%d], OUT:avClock[0x%X]), status:[isOK:%s, exception:%s]",
                 invalidLowId.value, avClock.get(), UT_Control_GetMapString(bool_mapTable, status.isOk()),
                 UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
     UT_ASSERT_FALSE(status.isOk());
     UT_ASSERT_EQUAL(avClock, nullptr);
 
     // Test with invalid high ID
     UT_LOG_INFO("Calling getAVClock(IN:avClockId:[%d], OUT:avClock[])", invalidHighId.value);
     status = gAVClockManagerService->getAVClock(invalidHighId, &avClock);
     UT_LOG_INFO("Result getAVClock(IN:avClockId:[%d], OUT:avClock[0x%X]), status:[isOK:%s, exception:%s]",
                 invalidHighId.value, avClock.get(), UT_Control_GetMapString(bool_mapTable, status.isOk()),
                 UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
     UT_ASSERT_FALSE(status.isOk());
     UT_ASSERT_EQUAL(avClock, nullptr);
 }
 
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
 UT_ADD_TEST(AVClockBaseTest, test_l1_AVClock_positive_getCapabilities) {
     Status status;
     ::com::rdk::hal::avclock::Capabilities capabilities;
     
     for (const auto& clock : mAVClocks) {
         UT_LOG_INFO("Calling getCapabilities(OUT:_aidl_return)");
         status = clock->getCapabilities(&capabilities);
         UT_LOG_INFO("Result getCapabilities(OUT:_aidl_return), status:[isOK:%s, exception:%s]",
                     UT_Control_GetMapString(bool_mapTable, status.isOk()),
                     UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
         UT_ASSERT_TRUE(status.isOk());
         UT_ASSERT_NOT_EQUAL(&capabilities, nullptr);
     }
 }
 
 /**
  * @brief Test the positive scenario of getting the state from AVClock
  *
  * This test verifies that the getState function of AVClock returns a valid state and the status is OK. It ensures that the state is one of the expected values.
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
  * | 01 | Initialize the test and log the start of the test case | None | Should be successful | Should be successful |
  * | 02 | Iterate over each AVClock instance in mAVClocks | None | Should be successful | Should be successful |
  * | 03 | For each clock, call the getState function to retrieve the current state | clock = current AVClock instance, state = uninitialized | Status should be OK, state should be a valid State enum value | Should Pass |
  * | 04 | Log the result of the getState call, including the state and status | state = retrieved state, status = retrieved status | Should be successful | Should be successful |
  * | 05 | Assert that the status is OK and the state is one of the expected values | status.isOk() = true, state = valid State enum | Assertions should pass | Should Pass |
  */
 UT_ADD_TEST(AVClockBaseTest, test_l1_AVClock_positive_getState) {
     UT_LOG_INFO("Starting test_l1_AVClock_positive_getState");
 
     for (const auto& clock : mAVClocks) {
         State state;
         UT_LOG_INFO("Calling getState(OUT:_aidl_return:[State])");
         Status status = clock->getState(&state);
         UT_LOG_INFO("Result getState(OUT:_aidl_return:[State:%d]), status:[isOK:%s, exception:%s]",
                     static_cast<int32_t>(state),
                     UT_Control_GetMapString(bool_mapTable, status.isOk()),
                     UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
         UT_ASSERT_TRUE(status.isOk());
         UT_ASSERT_TRUE(state == State::UNKNOWN || state == State::CLOSED || state == State::OPENING ||
                     state == State::READY || state == State::STARTING || state == State::STARTED ||
                     state == State::FLUSHING || state == State::STOPPING || state == State::CLOSING);
     }
 }
 
 /**
  * @brief Test the positive scenario of getting property values from AVClock.
  *
  * This test verifies the functionality of the `getProperty` method in the AVClockManagerService. It checks whether the method correctly retrieves property values and ensures that the returned values are valid and within expected ranges.
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
  * | 01 | Initialize the properties vector with RESOURCE_ID, METRIC_AV_SYNC_ACCURACY_MS, and METRIC_AV_RESYNC_COUNT. | properties = {RESOURCE_ID, METRIC_AV_SYNC_ACCURACY_MS, METRIC_AV_RESYNC_COUNT} | Properties vector initialized successfully | Should be successful |
  * | 02 | Iterate over each property in the properties vector and call the getProperty method. | property = RESOURCE_ID, aidlReturn = std::optional<PropertyValue> | Status should be OK, aidlReturn should have value | Should Pass |
  * | 03 | Check if the status is OK and aidlReturn has a value. | status = OK, aidlReturn.has_value() = true | Assertions should pass | Should Pass |
  * | 04 | Verify the tag of the returned value is intValue and check if the value is greater than or equal to 0. | aidlReturn->value->getTag() = intValue, value >= 0 | Assertions should pass | Should Pass |
  */
 UT_ADD_TEST(AVClockBaseTest, test_l1_AVClock_positive_getProperty) {
     Status status;
     std::optional<PropertyValue> aidlReturn;
     std::vector<::com::rdk::hal::avclock::Property> properties = {
         ::com::rdk::hal::avclock::Property::RESOURCE_ID,
         ::com::rdk::hal::avclock::Property::METRIC_AV_SYNC_ACCURACY_MS,
         ::com::rdk::hal::avclock::Property::METRIC_AV_RESYNC_COUNT
     };
 
     for (const auto& clock : mAVClocks) 
     {
         for (const auto& property : properties) 
         {
             UT_LOG_INFO("Calling getProperty(IN:property:[%d], OUT:_aidl_return[])", static_cast<int32_t>(property));
             status = clock->getProperty(property, &aidlReturn);
             UT_LOG_INFO("Result getProperty(IN:property:[%d], OUT:_aidl_return[0x%X]), status:[isOK:%s, exception:%s]",
                         static_cast<int32_t>(property), aidlReturn.has_value() ? aidlReturn->value->get<com::rdk::hal::PropertyValue::Value::Tag::intValue>() : 0,
                         UT_Control_GetMapString(bool_mapTable, status.isOk()),
                         UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
 
             UT_ASSERT_TRUE(status.isOk());
             UT_ASSERT_TRUE(aidlReturn.has_value());
             UT_ASSERT_EQUAL(aidlReturn->value->getTag(), PropertyValue::Value::intValue);
         }
     }
 }
 
 /**
  * @brief Test the behavior of the getProperty API when an invalid property is provided.
  *
  * This test checks the response of the getProperty API when called with a property value that is not defined in the Property enum. The test ensures that the API returns a failure status and the output parameter is null.
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
  * | Variation / Step | Description | Test Data | Expected Result | Notes |
  * | :----: | --------- | ---------- |-------------- | ----- |
  * | 01 | Log the API call with invalid property value | invalidProperty = -1 | Log should be successful | Should be successful |
  * | 02 | Call the getProperty API with invalid property | invalidProperty = -1, aidlReturn = null | status.isOk() = false, aidlReturn = std::nullopt | Should Fail |
  * | 03 | Log the result of the API call | invalidProperty = -1, aidlReturn = null | Log should be successful | Should be successful |
  * | 04 | Verify the return value indicates failure | status.isOk() = false | Assertion should pass | Should be successful |
  * | 05 | Check that the output parameter _aidl_return is null | aidlReturn = std::nullopt | Assertion should pass | Should be successful |
  */
 UT_ADD_TEST(AVClockBaseTest, test_l1_AVClock_negative_getProperty) {
     Status status;
     std::optional<PropertyValue> aidlReturn;
     
     ::com::rdk::hal::avclock::Property invalidProperty = static_cast<::com::rdk::hal::avclock::Property>(-1);
     for (const auto& clock : mAVClocks) 
     {
         UT_LOG_INFO("Calling getProperty(IN:property:[%d], OUT:_aidl_return[])", static_cast<int32_t>(invalidProperty));
         status = clock->getProperty(invalidProperty, &aidlReturn);
         UT_LOG_INFO("Result getProperty(IN:property:[%d], OUT:_aidl_return[null]), status:[isOK:%s, exception:%s]",
                     static_cast<int32_t>(invalidProperty),
                     UT_Control_GetMapString(bool_mapTable, status.isOk()),
                     UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
 
         UT_ASSERT_FALSE(status.isOk());
         UT_ASSERT_EQUAL(aidlReturn, std::nullopt);
     }
 }
 
 /**
  * @brief Test the functionality of setting properties in AVClock.
  *
  * This test verifies the ability to set properties on AVClock objects, ensuring that valid properties are set correctly and invalid properties are handled appropriately.
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
  * | Variation / Step | Description | Test Data | Expected Result | Notes |
  * | :----: | --------- | ---------- |-------------- | ----- |
  * | 01 | Check if mAVClocks is not empty | mAVClocks | Should be successful | |
  * | 02 | Iterate over each clock in mAVClocks | clock | Should be successful | |
  * | 03 | Set property RESOURCE_ID with valid integer value 123 | property = RESOURCE_ID, propertyValue = intValue:123, _aidl_return = true | _aidl_return should be false | Should Pass |
  * | 04 | Log the result of setting property RESOURCE_ID | property = RESOURCE_ID, propertyValue = intValue:123, _aidl_return = false | Log should indicate success | Should be successful |
  * | 05 | Set unsupported property with integer value 456 | property = 9999, propertyValue = intValue:456, _aidl_return = true | _aidl_return should be false | Should Fail |
  * | 06 | Log the result of setting unsupported property | property = 9999, propertyValue = intValue:456, _aidl_return = false | Log should indicate failure | Should be successful |
  */
 UT_ADD_TEST(AVClockBaseTest, test_l1_AVClock_negative_setProperty) {
     UT_ASSERT_FALSE(mAVClocks.empty());
 
     for (auto& clock : mAVClocks) {
         // negative Test for Setting Property RESOURCE_ID which is read-only
         ::com::rdk::hal::avclock::Property property = ::com::rdk::hal::avclock::Property::RESOURCE_ID;
         PropertyValue propertyValue;
         propertyValue.value = PropertyValue::Value::make<PropertyValue::Value::Tag::intValue>(1); // Example integer value
         bool _aidl_return = true;
 
         UT_LOG_INFO("Calling setProperty(IN:property:[RESOURCE_ID], IN:propertyValue:[intValue:1], OUT:_aidl_return)");
         Status status = clock->setProperty(property, propertyValue, &_aidl_return);
         UT_LOG_INFO("Result setProperty(IN:property:[RESOURCE_ID], IN:propertyValue:[intValue:1], OUT:_aidl_return:[%s]), status:[isOK:%s, exception:%s]",
                     UT_Control_GetMapString(bool_mapTable, _aidl_return),
                     UT_Control_GetMapString(bool_mapTable, status.isOk()),
                     UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
         UT_ASSERT_FALSE(_aidl_return);
 
         // Negative Test for Setting Property with Unsupported Property
         property = static_cast<::com::rdk::hal::avclock::Property>(9999); // Unsupported property value
         propertyValue.value = PropertyValue::Value::make<PropertyValue::Value::Tag::intValue>(4); // Example integer value
         _aidl_return = true;
 
         UT_LOG_INFO("Calling setProperty(IN:property:[9999], IN:propertyValue:[intValue:4], OUT:_aidl_return)");
         status = clock->setProperty(property, propertyValue, &_aidl_return);
         UT_LOG_INFO("Result setProperty(IN:property:[9999], IN:propertyValue:[intValue:4], OUT:_aidl_return:[%s]), status:[isOK:%s, exception:%s]",
                     UT_Control_GetMapString(bool_mapTable, _aidl_return),
                     UT_Control_GetMapString(bool_mapTable, status.isOk()),
                     UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
         UT_ASSERT_FALSE(_aidl_return);
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
 UT_ADD_TEST(AVClockBaseTest, test_l1_AVClock_positive_open)
 {
 
     UT_ASSERT_FALSE(mAVClocks.empty());
 
     // Test Case: Positive Test for Successful Open
     for (auto& clock : mAVClocks)
     {
         sp<IAVClockController> controller;
         sp<IAVClockControllerListener> controllerListener = new AVClockControllerListenerImpl;
 
         UT_LOG_INFO("Calling open(IN:avClockControllerListener:[0x%X], OUT:_aidl_return[])", controllerListener.get());
         Status status = clock->open(controllerListener, &controller);
         UT_ASSERT_TRUE(status.isOk());
         UT_ASSERT_NOT_EQUAL(controller, nullptr);
         UT_LOG_INFO("Result open(IN:avClockControllerListener:[0x%X], OUT:_aidl_return[0x%X]), status:[isOK:%s, exception:%s]",
                     controllerListener.get(), controller.get(), UT_Control_GetMapString(bool_mapTable, status.isOk()), UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
 
         // Teardown: Close the controller
         bool closed = false;
         status = clock->close(controller, &closed);
         UT_ASSERT_TRUE(status.isOk());
         UT_ASSERT_TRUE(closed);
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
 UT_ADD_TEST(AVClockBaseTest, test_l1_AVClock_negative_open) {
     UT_ASSERT_FALSE(mAVClocks.empty());
     // Negative Test for Open with Null Listener
     UT_LOG_INFO("Calling open(IN:avClockControllerListener:[nullptr], OUT:_aidl_return:[nullptr])");
     sp<IAVClockController> controller;
     Status status = mAVClocks[0]->open(nullptr, &controller);
     UT_LOG_INFO("Result open(IN:avClockControllerListener:[nullptr], OUT:_aidl_return:[nullptr]), status:[isOK:%s, exception:%s]",
                 UT_Control_GetMapString(bool_mapTable, status.isOk()),
                 UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
     UT_ASSERT_EQUAL(controller, nullptr);
 
 
     //test 2 
     sp<IAVClockControllerListener> controllerListener = new AVClockControllerListenerImpl;
     UT_LOG_INFO("Calling open(IN:avClockControllerListener:[0x%X], OUT:_aidl_return[])", controllerListener.get());
     status = mAVClocks[0]->open(controllerListener, &controller);
     UT_ASSERT_TRUE(status.isOk());
     UT_ASSERT_NOT_EQUAL(controller, nullptr);
     UT_LOG_INFO("Result open(IN:avClockControllerListener:[0x%X], OUT:_aidl_return[0x%X]), status:[isOK:%s, exception:%s]",
                 controllerListener.get(), controller.get(), UT_Control_GetMapString(bool_mapTable, status.isOk()), UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
 
 
     sp<IAVClockControllerListener> controllerListener2 = new AVClockControllerListenerImpl;
     sp<IAVClockController> controller2;
     UT_LOG_INFO("Calling open(IN:avClockControllerListener:[0x%X], OUT:_aidl_return[])", controllerListener2.get());
     status = mAVClocks[0]->open(controllerListener2, &controller2);
     UT_ASSERT_EQUAL(status.exceptionCode(), android::binder::Status::EX_ILLEGAL_STATE);
     UT_LOG_INFO("Result open(IN:avClockControllerListener:[0x%X], OUT:_aidl_return[0x%X]), status:[isOK:%s, exception:%s]",
                 controllerListener2.get(), controller2.get(), UT_Control_GetMapString(bool_mapTable, status.isOk()), UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
 
     // Teardown: Close the controller
     bool closed = false;
     status = mAVClocks[0]->close(controller, &closed);
     UT_ASSERT_TRUE(status.isOk());
     UT_ASSERT_TRUE(closed);
 }
 
 /**
  * @brief Test the positive scenario of closing AVClock controllers.
  *
  * This test verifies that the AVClock controllers can be closed successfully. It checks that the controllers are not empty and iterates through each controller to ensure that the close operation returns a successful status and the controller is marked as closed.
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
  * | Variation / Step | Description | Test Data | Expected Result | Notes |
  * | :----: | --------- | ---------- |-------------- | ----- |
  * | 01 | Verify that the controllers list is not empty | mControllers | Should be non-empty | Should be successful |
  * | 02 | Iterate through each controller and attempt to close it | controller = current controller, closed = false | status.isOk() = true, closed = true | Should Pass |
  */
 UT_ADD_TEST(AVClockBaseTest, test_l1_AVClock_positive_close) {
     UT_ASSERT_FALSE(mAVClocks.empty());
 
     for (auto& clock : mAVClocks)
     {
         sp<IAVClockControllerListener> controllerListener = new AVClockControllerListenerImpl;
         sp<IAVClockController> controller;
         
         UT_LOG_INFO("Calling open(IN:avClockControllerListener:[0x%X], OUT:_aidl_return[])", controllerListener.get());
         Status status = clock->open(controllerListener, &controller);
         UT_ASSERT_TRUE(status.isOk());
         UT_ASSERT_NOT_EQUAL(controller, nullptr);
         UT_LOG_INFO("Result open(IN:avClockControllerListener:[0x%X], OUT:_aidl_return[0x%X]), status:[isOK:%s, exception:%s]",
                     controllerListener.get(), controller.get(), UT_Control_GetMapString(bool_mapTable, status.isOk()), UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
 
         bool closed = false;
         UT_LOG_INFO("Calling close(IN:avClockController:[0x%X], OUT:_aidl_return)", controller.get());
         status = clock->close(controller, &closed);
         UT_LOG_INFO("Result close(IN:avClockController:[0x%X], OUT:_aidl_return:[%s]), status:[isOK:%s, exception:%s]",
                     controller.get(), UT_Control_GetMapString(bool_mapTable, closed),
                     UT_Control_GetMapString(bool_mapTable, status.isOk()),
                     UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
         UT_ASSERT_TRUE(status.isOk());
         UT_ASSERT_TRUE(closed);
     }
 }
 
 /**
  * @brief Test the negative scenarios for closing AVClockController instances
  *
  * This test evaluates the behavior of the AVClockManagerService when attempting to close AVClockController instances that are either null or invalid. It ensures that the service correctly handles these erroneous inputs without crashing and returns appropriate error statuses.
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
  * | Variation / Step | Description | Test Data | Expected Result | Notes |
  * | :----: | --------- | ---------- |-------------- | ----- |
  * | 01 | Initialize a null AVClockController instance and attempt to close it using the AVClockManagerService. | avClockController = nullptr, aidlReturn = false | aidlReturn = false, status.isOk() = false | Should Fail |
  * | 02 | Initialize an invalid AVClockController instance and attempt to close it using the AVClockManagerService. | avClockController = invalid, aidlReturn = false | aidlReturn = false, status.isOk() = false | Should Fail |
  */
 UT_ADD_TEST(AVClockBaseTest, test_l1_AVClock_negative_close) {
     UT_ASSERT_FALSE(mAVClocks.empty());
     // Test Case 1: Negative Test for Closing a Null AVClockController Instance
     {
         sp<IAVClockController> nullController = nullptr;
         bool aidlReturn = false;
         UT_LOG_INFO("Calling close(IN:avClockController:[nullptr], OUT:_aidl_return[])");
         Status status = mAVClocks[0]->close(nullController, &aidlReturn);
         UT_LOG_INFO("Result close(IN:avClockController:[nullptr], OUT:_aidl_return[%s]), status:[isOK:%s, exception:%s]",
                     UT_Control_GetMapString(bool_mapTable, aidlReturn),
                     UT_Control_GetMapString(bool_mapTable, status.isOk()),
                     UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
         UT_ASSERT_FALSE(aidlReturn);
         UT_ASSERT_FALSE(status.isOk());
     }
  
     // Test Case 2: Negative Test for Closing an Invalid AVClockController Instance
     {
         sp<IAVClockController> invalidController; // Simulating an invalid instance
         bool aidlReturn = false;
         UT_LOG_INFO("Calling close(IN:avClockController:[invalid], OUT:_aidl_return[])");
         Status status = mAVClocks[0]->close(invalidController, &aidlReturn);
         UT_LOG_INFO("Result close(IN:avClockController:[invalid], OUT:_aidl_return[%s]), status:[isOK:%s, exception:%s]",
                     UT_Control_GetMapString(bool_mapTable, aidlReturn),
                     UT_Control_GetMapString(bool_mapTable, status.isOk()),
                     UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
         UT_ASSERT_FALSE(aidlReturn);
         UT_ASSERT_FALSE(status.isOk());
     }
  
     {
         sp<IAVClockControllerListener> controllerListener = new AVClockControllerListenerImpl;
         sp<IAVClockController> controller;
        
         UT_LOG_INFO("Calling open(IN:avClockControllerListener:[0x%X], OUT:_aidl_return[])", controllerListener.get());
         Status status = mAVClocks[0]->open(controllerListener, &controller);
         UT_ASSERT_TRUE(status.isOk());
         UT_ASSERT_NOT_EQUAL(controller, nullptr);
         UT_LOG_INFO("Result open(IN:avClockControllerListener:[0x%X], OUT:_aidl_return[0x%X]), status:[isOK:%s, exception:%s]",
                     controllerListener.get(), controller.get(), UT_Control_GetMapString(bool_mapTable, status.isOk()), UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
  
         status = controller->start();
         UT_ASSERT_TRUE(status.isOk());
        
         //close from invalid state
         bool closed = false;
         UT_LOG_INFO("Calling close(IN:avClockController:[0x%X], OUT:_aidl_return)", controller.get());
         status = mAVClocks[0]->close(controller, &closed);
         UT_LOG_INFO("Result close(IN:avClockController:[0x%X], OUT:_aidl_return:[%s]), status:[isOK:%s, exception:%s]",
                     controller.get(), UT_Control_GetMapString(bool_mapTable, closed),
                     UT_Control_GetMapString(bool_mapTable, status.isOk()),
                     UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
         UT_ASSERT_EQUAL(status.exceptionCode(), android::binder::Status::EX_ILLEGAL_STATE);
  
         status = controller->stop();
         UT_ASSERT_TRUE(status.isOk());
  
         //teardown
         UT_LOG_INFO("Calling close(IN:avClockController:[0x%X], OUT:_aidl_return)", controller.get());
         status = mAVClocks[0]->close(controller, &closed);
         UT_LOG_INFO("Result close(IN:avClockController:[0x%X], OUT:_aidl_return:[%s]), status:[isOK:%s, exception:%s]",
                     controller.get(), UT_Control_GetMapString(bool_mapTable, closed),
                     UT_Control_GetMapString(bool_mapTable, status.isOk()),
                     UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
         UT_ASSERT_TRUE(status.isOk());
         UT_ASSERT_TRUE(closed);
     }
 }
 
 /**
 * @brief Test the positive scenario of registering an event listener with AVClockManagerService
 *
 * This test verifies the successful registration of a valid AVClockEventListener instance with the AVClockManagerService. It ensures that the service is available, the listener is valid, and the registration process completes without errors.
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
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Ensure the AVClockManagerService is available | gAVClockManagerService = nullptr | Service should not be null | Should be successful |
 * | 02 | Create a valid instance of AVClockEventListenerImpl | avClockEventListener = new AVClockEventListenerImpl | Listener should not be null | Should be successful |
 * | 03 | Prepare the output parameter for the API call | aidl_return = false | Initial value set | Should be successful |
 * | 04 | Call the registerEventListener API with valid listener | avClockEventListener, aidl_return | Status should be OK, aidl_return should be true | Should Pass |
 * | 05 | Verify the return status is successful | status.isOk() | Status should be true | Should be successful |
 * | 06 | Verify the output parameter indicates successful registration | aidl_return | aidl_return should be true | Should be successful |
 */
 UT_ADD_TEST(AVClockBaseTest, test_l1_AVClock_positive_registerEventListener) {
     UT_ASSERT_FALSE(mAVClocks.empty());
     UT_LOG_INFO("Starting test_l1_AVClock_positive_registerEventListener");
     for (auto& clock : mAVClocks)
     {
         sp<IAVClockEventListener> avClockEventListener = new AVClockEventListenerImpl;
         UT_ASSERT_NOT_EQUAL(avClockEventListener, nullptr);
         bool aidl_return = false;
 
         UT_LOG_INFO("Calling registerEventListener(IN:avClockEventListener:[0x%X], OUT:_aidl_return:[%d])", avClockEventListener.get(), aidl_return);
         Status status = clock->registerEventListener(avClockEventListener, &aidl_return);
         UT_ASSERT_TRUE(status.isOk());
         UT_ASSERT_TRUE(aidl_return);
         UT_LOG_INFO("Result registerEventListener(IN:avClockEventListener:[0x%X], OUT:_aidl_return:[%d]), status:[isOK:%s, exception:%s]", avClockEventListener.get(), aidl_return, UT_Control_GetMapString(bool_mapTable, status.isOk()), UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
     }
 }
 
 /**
  * @brief Test the registration of an event listener with the AVClockManagerService.
  *
  * This test verifies the behavior of the registerEventListener function in the AVClockManagerService. 
  * It checks the successful registration of an event listener and ensures that a second registration attempt fails.
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
  * | 01 | Create an instance of AVClockEventListenerImpl | listener = new AVClockEventListenerImpl | Instance should be created successfully | Should be successful |
  * | 02 | Call registerEventListener for the first time with the created listener | listener = [0x%X], aidlReturn = false | status.isOk() = true, aidlReturn = true | Should Pass |
  * | 03 | Call registerEventListener for the second time with the same listener | listener = [0x%X], aidlReturn = false | status.isOk() = false, aidlReturn = false | Should Fail |
  */
 UT_ADD_TEST(AVClockBaseTest, test_l1_AVClock_negative_registerEventListener) {
     Status status;
     bool aidlReturn = false;
     sp<IAVClockEventListener> listener = new AVClockEventListenerImpl;
     UT_ASSERT_FALSE(mAVClocks.empty());
     // First registration attempt
     UT_LOG_INFO("Calling registerEventListener(IN:avClockEventListener:[0x%X], OUT:_aidl_return)", listener.get());
     status = mAVClocks[0]->registerEventListener(listener, &aidlReturn);
     UT_LOG_INFO("Result registerEventListener(IN:avClockEventListener:[0x%X], OUT:_aidl_return:[%s]), status:[isOK:%s, exception:%s]",
                 listener.get(), UT_Control_GetMapString(bool_mapTable, aidlReturn),
                 UT_Control_GetMapString(bool_mapTable, status.isOk()),
                 UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
     UT_ASSERT_TRUE(status.isOk());
     UT_ASSERT_TRUE(aidlReturn);
 
     // Second registration attempt
     UT_LOG_INFO("Calling registerEventListener(IN:avClockEventListener:[0x%X], OUT:_aidl_return)", listener.get());
     status = mAVClocks[0]->registerEventListener(listener, &aidlReturn);
     UT_LOG_INFO("Result registerEventListener(IN:avClockEventListener:[0x%X], OUT:_aidl_return:[%s]), status:[isOK:%s, exception:%s]",
                 listener.get(), UT_Control_GetMapString(bool_mapTable, aidlReturn),
                 UT_Control_GetMapString(bool_mapTable, status.isOk()),
                 UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
     UT_ASSERT_FALSE(status.isOk());
     UT_ASSERT_FALSE(aidlReturn);
 }
 
 /**
  * @brief Test the positive scenario of unregistering an event listener from AVClock.
  *
  * This test verifies the functionality of unregistering an event listener from AVClock instances. It ensures that the listener is successfully unregistered and the operation returns the expected results.
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
  * | Variation / Step | Description | Test Data | Expected Result | Notes |
  * | :----: | --------- | ---------- |-------------- | ----- |
  * | 01 | Verify mAVClocks is not empty | mAVClocks | Should be successful | Should be successful |
  * | 02 | Create AVClockEventListenerImpl instance | avClockEventListener = new AVClockEventListenerImpl | Should be successful | Should be successful |
  * | 03 | Iterate over each AVClock and call unregisterEventListener | avClockEventListener, aidl_return = false | status.isOk() = true, aidl_return = true | Should Pass |
  * | 04 | Log the result of unregisterEventListener | avClockEventListener, aidl_return, status | Log output should match expected format | Should be successful |
  * | 05 | Assert status is OK and aidl_return is true | status.isOk(), aidl_return | Assertions should pass | Should Pass |
  */
 UT_ADD_TEST(AVClockBaseTest, test_l1_AVClock_positive_unregisterEventListener)
 {
     UT_ASSERT_FALSE(mAVClocks.empty());
 
     for (auto& clock : mAVClocks)
     {
         Status status;
         sp<AVClockEventListenerImpl> avClockEventListener = new AVClockEventListenerImpl;
         bool aidl_return = false;
 
         UT_LOG_INFO("Calling registerEventListener(IN:avClockEventListener:[0x%X], OUT:_aidl_return:[%d])", avClockEventListener.get(), aidl_return);
         status = clock->registerEventListener(avClockEventListener, &aidl_return);
         UT_ASSERT_TRUE(status.isOk());
         UT_ASSERT_TRUE(aidl_return);
         UT_LOG_INFO("Result registerEventListener(IN:avClockEventListener:[0x%X], OUT:_aidl_return:[%d]), status:[isOK:%s, exception:%s]", avClockEventListener.get(), aidl_return, UT_Control_GetMapString(bool_mapTable, status.isOk()), UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
 
 
         // Assuming the listener is already registered, we proceed to unregister
         UT_LOG_INFO("Calling unregisterEventListener(IN:avClockEventListener:[0x%X], OUT:_aidl_return)", avClockEventListener.get());
         status = clock->unregisterEventListener(avClockEventListener, &aidl_return);
         UT_LOG_INFO("Result unregisterEventListener(IN:avClockEventListener:[0x%X], OUT:_aidl_return:[%s]), status:[isOK:%s, exception:%s]",
                     avClockEventListener.get(),
                     UT_Control_GetMapString(bool_mapTable, aidl_return),
                     UT_Control_GetMapString(bool_mapTable, status.isOk()),
                     UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
 
         UT_ASSERT_TRUE(status.isOk());
         UT_ASSERT_TRUE(aidl_return);
     }
 }
 
 /**
 * @brief Test the behavior of unregisterEventListener with valid and invalid listeners
 *
 * This test verifies the behavior of the unregisterEventListener function when provided with a valid but unregistered listener and an invalid listener. It ensures that the function handles these scenarios correctly by returning appropriate status and aidlReturn values.
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
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Create a valid but unregistered event listener instance | validUnregisteredListener = new AVClockEventListenerImpl | Should be successful | Should be successful |
 * | 02 | Call unregisterEventListener with a valid unregistered listener | avClockEventListener = validUnregisteredListener, _aidl_return = false | status.isOk() = true, aidlReturn = false | Should Pass |
 * | 03 | Create an invalid event listener instance | invalidListener = nullptr | Should be successful | Should be successful |
 * | 04 | Call unregisterEventListener with an invalid listener | avClockEventListener = invalidListener, _aidl_return = false | aidlReturn = false | Should Fail |
 */
 UT_ADD_TEST(AVClockBaseTest, test_l1_AVClock_negative_unregisterEventListener) {
     Status status;
     bool aidlReturn = false;
     UT_ASSERT_FALSE(mAVClocks.empty());
 
     // Create a valid but unregistered event listener instance
     sp<IAVClockEventListener> validUnregisteredListener = new AVClockEventListenerImpl;
 
     // Log and call unregisterEventListener with a valid unregistered listener
     UT_LOG_INFO("Calling unregisterEventListener(IN:avClockEventListener:[validUnregisteredListener], OUT:_aidl_return)");
     status = mAVClocks[0]->unregisterEventListener(validUnregisteredListener, &aidlReturn);
     UT_LOG_INFO("Result unregisterEventListener(IN:avClockEventListener:[validUnregisteredListener], OUT:_aidl_return:[%d]), status:[isOK:%s, exception:%s]", aidlReturn, UT_Control_GetMapString(bool_mapTable, status.isOk()), UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
     UT_ASSERT_TRUE(status.isOk());
     UT_ASSERT_FALSE(aidlReturn);
 
     // Create an invalid event listener instance
     sp<IAVClockEventListener> invalidListener = nullptr; // Simulating an invalid instance
 
     // Log and call unregisterEventListener with an invalid listener
     UT_LOG_INFO("Calling unregisterEventListener(IN:avClockEventListener:[invalidListener], OUT:_aidl_return)");
     status = mAVClocks[0]->unregisterEventListener(invalidListener, &aidlReturn);
     UT_LOG_INFO("Result unregisterEventListener(IN:avClockEventListener:[invalidListener], OUT:_aidl_return:[%d]), status:[isOK:%s, exception:%s]", aidlReturn, UT_Control_GetMapString(bool_mapTable, status.isOk()), UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
     UT_ASSERT_FALSE(aidlReturn);
 
 
     //unregister a listner which is allready unregistered
     sp<AVClockEventListenerImpl> avClockEventListener = new AVClockEventListenerImpl;
     bool aidl_return = false;
 
     UT_LOG_INFO("Calling registerEventListener(IN:avClockEventListener:[0x%X], OUT:_aidl_return:[%d])", avClockEventListener.get(), aidl_return);
     status = mAVClocks[0]->registerEventListener(avClockEventListener, &aidl_return);
     UT_ASSERT_TRUE(status.isOk());
     UT_ASSERT_TRUE(aidl_return);
     UT_LOG_INFO("Result registerEventListener(IN:avClockEventListener:[0x%X], OUT:_aidl_return:[%d]), status:[isOK:%s, exception:%s]", avClockEventListener.get(), aidl_return, UT_Control_GetMapString(bool_mapTable, status.isOk()), UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
 
 
     //unregister for first time
     UT_LOG_INFO("Calling unregisterEventListener(IN:avClockEventListener:[0x%X], OUT:_aidl_return)", avClockEventListener.get());
     status = mAVClocks[0]->unregisterEventListener(avClockEventListener, &aidl_return);
     UT_LOG_INFO("Result unregisterEventListener(IN:avClockEventListener:[0x%X], OUT:_aidl_return:[%s]), status:[isOK:%s, exception:%s]",
                 avClockEventListener.get(),
                 UT_Control_GetMapString(bool_mapTable, aidl_return),
                 UT_Control_GetMapString(bool_mapTable, status.isOk()),
                 UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
     UT_ASSERT_TRUE(status.isOk());
     UT_ASSERT_TRUE(aidl_return);
 
     //unregister for second time
     UT_LOG_INFO("Calling unregisterEventListener(IN:avClockEventListener:[0x%X], OUT:_aidl_return)", avClockEventListener.get());
     status = mAVClocks[0]->unregisterEventListener(avClockEventListener, &aidl_return);
     UT_LOG_INFO("Result unregisterEventListener(IN:avClockEventListener:[0x%X], OUT:_aidl_return:[%s]), status:[isOK:%s, exception:%s]",
                 avClockEventListener.get(),
                 UT_Control_GetMapString(bool_mapTable, aidl_return),
                 UT_Control_GetMapString(bool_mapTable, status.isOk()),
                 UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
     UT_ASSERT_FALSE(aidlReturn);   
 }
 
 /**
  * @brief Test the positive start functionality of AVClockController
  *
  * This test verifies that the AVClockController can successfully transition from the READY state to the STARTED state by invoking the start() method. It ensures that the controller is in the READY state before starting and checks that the start operation completes successfully. The test also includes a teardown step to stop the controller after the start operation.
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
  * | 01 | Check if mControllers is not empty | mControllers | Should not be empty | Should be successful |
  * | 02 | Iterate over each controller in mControllers | controller | - | Should be successful |
  * | 03 | Ensure the AV Clock is in the READY state by calling isReady() | isReady = false | status.isOk() = true, isReady = true | Should be successful |
  * | 04 | Log before calling the start API | - | - | Should be successful |
  * | 05 | Call the start API on the controller | - | status.isOk() = true | Should Pass |
  * | 06 | Log the result of the start API call | - | - | Should be successful |
  * | 07 | Verify that the return value is ::android::binder::Status::OK | - | status.isOk() = true | Should Pass |
  * | 08 | Teardown: Call stop on the controller | - | status.isOk() = true | Done by Pre-requisite TearDown function |
  */
 UT_ADD_TEST(AVClockControllerBaseTest, test_l1_AVClockController_positive_start) {
     Status status;
     UT_ASSERT_FALSE(mControllers.empty());
 
     for (auto& controller : mControllers) {
         // Log before calling the start API
         UT_LOG_INFO("Calling start()");
         status = controller->start();
         UT_LOG_INFO("Result start(), status:[isOK:%s, exception:%s]",
                     UT_Control_GetMapString(bool_mapTable, status.isOk()),
                     UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
         UT_ASSERT_TRUE(status.isOk());
 
         // Teardown: Call stop
         UT_LOG_INFO("Calling stop()");
         status = controller->stop();
         UT_LOG_INFO("Result stop(), status:[isOK:%s, exception:%s]",
                     UT_Control_GetMapString(bool_mapTable, status.isOk()),
                     UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
         UT_ASSERT_TRUE(status.isOk());
     }
 }
 
 /**
  * @brief Test the negative start behavior of AVClockController
  *
  * This test verifies that the start function of AVClockController behaves correctly when the controller is already in a STARTED state. It ensures that the function returns an EX_ILLEGAL_STATE exception and that the state of the AV Clock remains unchanged.
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
  * | Variation / Step | Description | Test Data | Expected Result | Notes |
  * | :----: | --------- | ---------- |-------------- | ----- |
  * | 01 | Check if mControllers is not empty | mControllers | Should not be empty | Should be successful |
  * | 02 | Iterate over each controller in mControllers | controller | Should iterate over all controllers | Should be successful |
  * | 03 | Ensure the AV Clock is in a non-READY state by starting it | started = false | status.isOk() = true, started = true | Should be successful |
  * | 04 | Call the start API on the controller |  | status.exceptionCode() = Status::EX_ILLEGAL_STATE | Should Pass |
  * | 05 | Verify the AV Clock state remains STARTED | currentState | currentState = State::STARTED | Should Pass |
  */
 UT_ADD_TEST(AVClockControllerBaseTest, test_l1_AVClockController_negative_start) {
     UT_ASSERT_FALSE(mControllers.empty());
 
     for (auto& controller : mControllers) {
         // Ensure the AV Clock is in a non-READY state, e.g., STARTED
         Status status = controller->start();
         UT_ASSERT_TRUE(status.isOk());
 
         // Call the start API
         UT_LOG_INFO("Calling start()");
         status = controller->start();
         UT_LOG_INFO("Result start(), status:[isOK:%s, exception:%s]",
                     UT_Control_GetMapString(bool_mapTable, status.isOk()),
                     UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
         UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_ILLEGAL_STATE);
 
         status = controller->stop();
         UT_ASSERT_TRUE(status.isOk());
     }
 }
 
 /**
  * @brief Test the positive scenario of stopping AVClockController
  *
  * This test verifies the successful execution of the stop() function on AVClockController instances. It ensures that the stop() function returns a successful status without any exceptions.
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
  * | 01 | Iterate over each AVClockController instance in mControllers | mControllers = [controller1, controller2, ...] | Should be successful | |
  * | 02 | Log the message before calling stop() | message = "Calling stop()" | Should be successful | |
  * | 03 | Call the stop() API on the controller | controller = current controller, status = controller->stop() | status.isOk() = true, status.exceptionCode() = none | Should Pass |
  * | 04 | Log the result of the stop() API call | status = result of stop() | Should be successful | |
  * | 05 | Verify the expected output using assertion | status.isOk() = true | Assertion should pass | |
  */
 UT_ADD_TEST(AVClockControllerBaseTest, test_l1_AVClockController_positive_stop)
 {
     for (auto& controller : mControllers)
     {
         UT_LOG_INFO("Calling start()");
         Status status = controller->start();
         UT_LOG_INFO("Result start(), status:[isOK:%s, exception:%s]",
                     UT_Control_GetMapString(bool_mapTable, status.isOk()),
                     UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));        
         UT_ASSERT_TRUE(status.isOk());
 
 
         UT_LOG_INFO("Calling stop()");
         status = controller->stop();
         UT_LOG_INFO("Result stop(), status:[isOK:%s, exception:%s]",
                     UT_Control_GetMapString(bool_mapTable, status.isOk()),
                     UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
         UT_ASSERT_TRUE(status.isOk());
     }
 }
 
 /**
  * @brief Test the negative scenario of stopping AV Clock Controller when not in STARTED state
  *
  * This test verifies that the stop() function of AV Clock Controller returns an illegal state exception when the controller is not in the STARTED state. This ensures that the stop operation is only valid when the controller is actively running.
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
  * | Variation / Step | Description | Test Data | Expected Result | Notes |
  * | :----: | --------- | ---------- |-------------- | ----- |
  * | 01 | Check if mControllers is not empty | mControllers | Should be successful | |
  * | 02 | Iterate over each controller in mControllers | controller | Should be successful | |
  * | 03 | Get the current state of the controller | currentState | status.isOk() = true, currentState != State::STARTED | Should be successful |
  * | 04 | Log the call to stop() | | | Should be successful |
  * | 05 | Call the stop() API on the controller | status | status.exceptionCode() = Status::EX_ILLEGAL_STATE | Should Pass |
  * | 06 | Log the result of stop() | | | Should be successful |
  * | 07 | Verify the expected output | status.exceptionCode() = Status::EX_ILLEGAL_STATE | | Should Pass |
  */
 UT_ADD_TEST(AVClockControllerBaseTest, test_l1_AVClockController_negative_stop)
 {
     UT_ASSERT_FALSE(mControllers.empty());
     for (auto& controller : mControllers)
     {
         UT_LOG_INFO("Calling stop() without prior start()");
         Status status = controller->stop();
         UT_LOG_INFO("Result stop(), status:[isOK:%s, exception:%s]",
                     UT_Control_GetMapString(bool_mapTable, status.isOk()),
                     UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
         UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_ILLEGAL_STATE);
     }
 }
 
 /**
  * @brief Test the functionality of setting audio sink in AVClockController.
  *
  * This test verifies the behavior of the AVClockController when setting an audio sink with both valid and undefined IDs. It ensures that the controller correctly handles these scenarios in different states.
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
  * | Variation / Step | Description | Test Data | Expected Result | Notes |
  * | :----: | --------- | ---------- |-------------- | ----- |
  * | 01 | Verify controllers are not empty | mControllers | Should be successful | |
  * | 02 | Retrieve list count of audio sinks | listCount = UT_KVP_PROFILE_GET_LIST_COUNT(IAudioSinkManager.IAudioSink) | Should be successful | |
  * | 03 | Populate audioSinkIds with valid IDs | audioSinkIds | Should be successful | |
  * | 04 | Ensure AV Clock is in READY state | started = false, status = controller->start(&started) | status.isOk() = true, started = true | Should be successful |
  * | 05 | Set Audio Sink with valid ID | audioSinkId.value = audioSinkIdValue, aidlReturn = false, status = controller->setAudioSink(audioSinkId, &aidlReturn) | status.isOk() = true, aidlReturn = true | Should Pass |
  * | 06 | Set Audio Sink with UNDEFINED ID in STARTED state | undefinedAudioSinkId.value = IAudioSink::Id::UNDEFINED, aidlReturn = false, status = controller->setAudioSink(undefinedAudioSinkId, &aidlReturn) | status.isOk() = true, aidlReturn = true | Should Pass |
  */
 UT_ADD_TEST(AVClockControllerBaseTest, test_l1_AVClockController_positive_setAudioSink) {
     UT_ASSERT_FALSE(mControllers.empty());
  
     Status status;
     std::vector<int32_t> audioSinkIds;
     std::string audiosinkcount_path = "IAudioSinkManager.IAudioSink";
     size_t expectedCount = UT_KVP_PROFILE_GET_LIST_COUNT(audiosinkcount_path.c_str());
 
     for (size_t i = 0; i < expectedCount; ++i)
     {
         std::string audiosinkid_path = "IAudioSinkManager.IAudioSink."+std::to_string(i);
         audioSinkIds.push_back(UT_KVP_PROFILE_GET_UINT32(audiosinkid_path.c_str()));
     }
 
     for (auto& controller : mControllers) {
         // Test Case 1: Set Audio Sink with Valid ID in READY State
         for (const auto& audioSinkIdValue : audioSinkIds) 
         {
             IAudioSink::Id audioSinkId;
             audioSinkId.value = audioSinkIdValue;
 
             // Ensure the AV Clock is in the READY state
             bool aidlReturn = false;
             UT_LOG_INFO("Calling setAudioSink(IN:audioSinkId:[%d], OUT:_aidl_return)", audioSinkId.value);
             status = controller->setAudioSink(audioSinkId, &aidlReturn);
             UT_LOG_INFO("Result setAudioSink(IN:audioSinkId:[%d], OUT:_aidl_return:[%s]), status:[isOK:%s, exception:%s]",
                         audioSinkId.value, UT_Control_GetMapString(bool_mapTable, aidlReturn),
                         UT_Control_GetMapString(bool_mapTable, status.isOk()),
                         UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
             UT_ASSERT_TRUE(status.isOk());
             UT_ASSERT_TRUE(aidlReturn);
         }
 
         // Test Case 2: setting audiosink to UNDEFINED when state is READY
         IAudioSink::Id undefinedAudioSinkId;
         undefinedAudioSinkId.value = IAudioSink::Id::UNDEFINED;
 
         bool aidlReturn = false;
         UT_LOG_INFO("Calling setAudioSink(IN:audioSinkId:[%d], OUT:_aidl_return)", undefinedAudioSinkId.value);
         status = controller->setAudioSink(undefinedAudioSinkId, &aidlReturn);
         UT_LOG_INFO("Result setAudioSink(IN:audioSinkId:[%d], OUT:_aidl_return:[%s]), status:[isOK:%s, exception:%s]",
                     undefinedAudioSinkId.value, UT_Control_GetMapString(bool_mapTable, aidlReturn),
                     UT_Control_GetMapString(bool_mapTable, status.isOk()),
                     UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
         UT_ASSERT_TRUE(status.isOk());
         UT_ASSERT_TRUE(aidlReturn);
 
         // Test Case 3: Set Audio Sink with Valid ID in STARTED State
         status = controller->start();
         UT_ASSERT_TRUE(status.isOk());
 
         for (const auto& audioSinkIdValue : audioSinkIds) {
             IAudioSink::Id audioSinkId;
             audioSinkId.value = audioSinkIdValue;
 
             // Ensure the AV Clock is in the READY state
             bool aidlReturn = false;
             UT_LOG_INFO("Calling setAudioSink(IN:audioSinkId:[%d], OUT:_aidl_return)", audioSinkId.value);
             status = controller->setAudioSink(audioSinkId, &aidlReturn);
             UT_LOG_INFO("Result setAudioSink(IN:audioSinkId:[%d], OUT:_aidl_return:[%s]), status:[isOK:%s, exception:%s]",
                         audioSinkId.value, UT_Control_GetMapString(bool_mapTable, aidlReturn),
                         UT_Control_GetMapString(bool_mapTable, status.isOk()),
                         UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
             UT_ASSERT_TRUE(status.isOk());
             UT_ASSERT_TRUE(aidlReturn);
         }
 
         // Test Case 4: setting audiosink to UNDEFINED when state is STARTED        
 
         aidlReturn = false;
         UT_LOG_INFO("Calling setAudioSink(IN:audioSinkId:[%d], OUT:_aidl_return)", undefinedAudioSinkId.value);
         status = controller->setAudioSink(undefinedAudioSinkId, &aidlReturn);
         UT_LOG_INFO("Result setAudioSink(IN:audioSinkId:[%d], OUT:_aidl_return:[%s]), status:[isOK:%s, exception:%s]",
                     undefinedAudioSinkId.value, UT_Control_GetMapString(bool_mapTable, aidlReturn),
                     UT_Control_GetMapString(bool_mapTable, status.isOk()),
                     UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
         UT_ASSERT_TRUE(status.isOk());
         UT_ASSERT_TRUE(aidlReturn);
 
         
         status = controller->stop();
         UT_ASSERT_TRUE(status.isOk());        
 
     }
 }
 
 /**
  * @brief Test the negative scenario of setting an audio sink in AVClockController
  *
  * This test verifies the behavior of the AVClockController when attempting to set an invalid audio sink ID. 
  * It ensures that the function correctly handles invalid inputs by returning appropriate error statuses.
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
  * | 01 | Check if mControllers is not empty | mControllers | Should not be empty | Should be successful |
  * | 02 | Iterate over each controller and check if AV Clock is in READY state | controller, isReady = false | status.isOk() = true, isReady = true | Should be successful |
  * | 03 | Retrieve audioSinkId from YAML and store in a list | audioSinkCount, audioSinkIds | List populated with audio sink IDs | Should be successful |
  * | 04 | Sort audioSinkIds and determine invalid IDs | audioSinkIds, invalidIdLow, invalidIdHigh | invalidIdLow and invalidIdHigh determined | Should be successful |
  * | 05 | Test setAudioSink with invalidIdLow | audioSinkId = invalidIdLow, aidlReturn = false | aidlReturn = false, status.isOk() = false | Should Fail |
  * | 06 | Test setAudioSink with invalidIdHigh | audioSinkId = invalidIdHigh, aidlReturn = false | aidlReturn = false, status.isOk() = false | Should Fail |
  */
 UT_ADD_TEST(AVClockControllerBaseTest, test_l1_AVClockController_negative_setAudioSink) {
     UT_ASSERT_FALSE(mControllers.empty());
 
     std::vector<int32_t> audioSinkIds;
     std::string audiosinkcount_path = "IAudioSinkManager.IAudioSink";
     size_t expectedCount = UT_KVP_PROFILE_GET_LIST_COUNT(audiosinkcount_path.c_str());
 
     for (size_t i = 0; i < expectedCount; ++i)
     {
         std::string audiosinkid_path = "IAudioSinkManager.IAudioSink."+std::to_string(i);
         audioSinkIds.push_back(UT_KVP_PROFILE_GET_UINT32(audiosinkid_path.c_str()));
     }
 
     for (auto& controller : mControllers) {
         // Sort the IDs and determine invalid IDs
         std::sort(audioSinkIds.begin(), audioSinkIds.end());
         int32_t invalidIdLow = audioSinkIds.front() - 1;
         int32_t invalidIdHigh = audioSinkIds.back() + 1;
 
         // Test with invalid IDs
         std::vector<int32_t> invalidIds = {invalidIdLow, invalidIdHigh};
         for (auto& invalidId : invalidIds) {
             IAudioSink::Id audioSinkId;
             audioSinkId.value = invalidId;
             bool aidlReturn = false;
 
             UT_LOG_INFO("Calling setAudioSink(IN:audioSinkId:[%d], OUT:_aidl_return)", audioSinkId.value);
             Status status = controller->setAudioSink(audioSinkId, &aidlReturn);
             UT_LOG_INFO("Result setAudioSink(IN:audioSinkId:[%d], OUT:_aidl_return:[%s]), status:[isOK:%s, exception:%s]",
                         audioSinkId.value, UT_Control_GetMapString(bool_mapTable, aidlReturn),
                         UT_Control_GetMapString(bool_mapTable, status.isOk()),
                         UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
 
             UT_ASSERT_FALSE(aidlReturn);
             UT_ASSERT_FALSE(status.isOk());
         }
     }
 }
 
 /**
  * @brief Test the positive scenario of getting an audio sink from AVClockController
  *
  * This test verifies the functionality of the getAudioSink method in the AVClockController. It ensures that the method returns a valid audio sink ID when the controller is in the READY and STARTED states.
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
  * | 01 | Check if mControllers is not empty | mControllers | Should not be empty | Should be successful |
  * | 02 | Set AV Clock state to READY for each controller | State = READY | Status is OK | Should be successful |
  * | 03 | Call getAudioSink API in READY state | OUT: audioSinkId | Status is OK, audioSinkId is valid | Should Pass |
  * | 04 | Verify audioSinkId in READY state | audioSinkId | audioSinkId is UNDEFINED or valid | Should Pass |
  * | 05 | Set AV Clock state to STARTED for each controller | State = STARTED | Status is OK | Should be successful |
  * | 06 | Call getAudioSink API in STARTED state | OUT: audioSinkId | Status is OK, audioSinkId is valid | Should Pass |
  * | 07 | Verify audioSinkId in STARTED state | audioSinkId | audioSinkId is UNDEFINED or valid | Should Pass |
  */
 UT_ADD_TEST(AVClockControllerBaseTest, test_l1_AVClockController_positive_getAudioSink)
 {
     UT_ASSERT_FALSE(mControllers.empty());
     
     Status status;
     std::vector<int32_t> audioSinkIds;
     std::string audiosinkcount_path = "IAudioSinkManager.IAudioSink";
     size_t expectedCount = UT_KVP_PROFILE_GET_LIST_COUNT(audiosinkcount_path.c_str());
 
     for (size_t i = 0; i < expectedCount; ++i)
     {
         std::string audiosinkid_path = "IAudioSinkManager.IAudioSink."+std::to_string(i);
         audioSinkIds.push_back(UT_KVP_PROFILE_GET_UINT32(audiosinkid_path.c_str()));
     }
 
     for (auto& controller : mControllers)
     {
         IAudioSink::Id audioSinkId;
         UT_LOG_INFO("Calling getAudioSink(OUT:audioSinkId)");
         status = controller->getAudioSink(&audioSinkId);
         UT_LOG_INFO("Result getAudioSink(OUT:audioSinkId:[%d]), status:[isOK:%s, exception:%s]",
                     audioSinkId.value,
                     UT_Control_GetMapString(bool_mapTable, status.isOk()),
                     UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
         UT_ASSERT_TRUE(status.isOk());
         UT_ASSERT_TRUE(audioSinkId.value == IAudioSink::Id::UNDEFINED || std::find(audioSinkIds.begin(), audioSinkIds.end(), audioSinkId.value) != audioSinkIds.end());
 
         status = controller->start();
         UT_ASSERT_TRUE(status.isOk());
 
         UT_LOG_INFO("Calling getAudioSink(OUT:audioSinkId)");
         status = controller->getAudioSink(&audioSinkId);
         UT_LOG_INFO("Result getAudioSink(OUT:audioSinkId:[%d]), status:[isOK:%s, exception:%s]",
                     audioSinkId.value,
                     UT_Control_GetMapString(bool_mapTable, status.isOk()),
                     UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
         UT_ASSERT_TRUE(status.isOk());
         UT_ASSERT_TRUE(audioSinkId.value == IAudioSink::Id::UNDEFINED || std::find(audioSinkIds.begin(), audioSinkIds.end(), audioSinkId.value) != audioSinkIds.end());
 
         status = controller->stop();
         UT_ASSERT_TRUE(status.isOk());
     }
 }
 
 /**
  * @brief Test the positive scenario of setting a supplementary audio sink in AVClockController.
  *
  * This test verifies that the AVClockController can successfully set a supplementary audio sink
  * when the controller is in a valid state (READY or STARTED). It ensures that the function
  * setSupplementaryAudioSink is called with valid parameters and returns the expected results.
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
  * | Variation / Step | Description | Test Data | Expected Result | Notes |
  * | :----: | --------- | ---------- |-------------- | ----- |
  * | 01 | Check if mControllers is not empty | mControllers | Should not be empty | Should be successful |
  * | 02 | Iterate over each controller in mControllers | controller | Should iterate successfully | Should be successful |
  * | 03 | Get the current state of the controller | currentState | State should be READY or STARTED | Should be successful |
  * | 04 | Retrieve the audio sink count from YAML | audioSinkCount | Should be greater than 0 | Should be successful |
  * | 05 | Iterate over each audio sink index | index | Should iterate successfully | Should be successful |
  * | 06 | Retrieve supplementaryAudioSinkId from YAML | supplementaryAudioSinkIdValue | Valid ID retrieved | Should be successful |
  * | 07 | Call setSupplementaryAudioSink with supplementaryAudioSinkId | supplementaryAudioSinkId, _aidl_return | status.isOk() should be true, _aidl_return should be true | Should Pass |
  */
 UT_ADD_TEST(AVClockControllerBaseTest, test_l1_AVClockController_positive_setSupplementaryAudioSink)
 {
     UT_ASSERT_FALSE(mControllers.empty());
     
     Status status;
     std::vector<int32_t> audioSinkIds;
     std::string audiosinkcount_path = "IAudioSinkManager.IAudioSink";
     size_t expectedCount = UT_KVP_PROFILE_GET_LIST_COUNT(audiosinkcount_path.c_str());
 
     for (size_t i = 0; i < expectedCount; ++i)
     {
         std::string audiosinkid_path = "IAudioSinkManager.IAudioSink."+std::to_string(i);
         audioSinkIds.push_back(UT_KVP_PROFILE_GET_UINT32(audiosinkid_path.c_str()));
     }
     UT_ASSERT_GREATER(audioSinkIds.size(), 0);
 
     //test 1 : test from READY state
     for (auto& controller : mControllers)
     {
         for (auto& supplementaryAudioSinkIdValue : audioSinkIds)
         {
             IAudioSink::Id supplementaryAudioSinkId;
             supplementaryAudioSinkId.value = supplementaryAudioSinkIdValue;
             bool _aidl_return = false;
 
             // Log before calling the API
             UT_LOG_INFO("Calling setSupplementaryAudioSink(IN:supplementaryAudioSinkId:[%d], OUT:_aidl_return)", supplementaryAudioSinkId.value);
             status = controller->setSupplementaryAudioSink(supplementaryAudioSinkId, &_aidl_return);
             UT_LOG_INFO("Result setSupplementaryAudioSink(IN:supplementaryAudioSinkId:[%d], OUT:_aidl_return:[%s]), status:[isOK:%s, exception:%s]",
                         supplementaryAudioSinkId.value, _aidl_return ? "true" : "false",
                         UT_Control_GetMapString(bool_mapTable, status.isOk()),
                         UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
 
             UT_ASSERT_TRUE(status.isOk());
             UT_ASSERT_TRUE(_aidl_return);
         }
     }
 
     //test 2 : test from STARTED state
     for (auto& controller : mControllers)
     {
         status = controller->start();
         UT_ASSERT_TRUE(status.isOk());
         
         for (auto& supplementaryAudioSinkIdValue : audioSinkIds)
         {
             IAudioSink::Id supplementaryAudioSinkId;
             supplementaryAudioSinkId.value = supplementaryAudioSinkIdValue;
             bool _aidl_return = false;
 
             // Log before calling the API
             UT_LOG_INFO("Calling setSupplementaryAudioSink(IN:supplementaryAudioSinkId:[%d], OUT:_aidl_return)", supplementaryAudioSinkId.value);
             status = controller->setSupplementaryAudioSink(supplementaryAudioSinkId, &_aidl_return);
             UT_LOG_INFO("Result setSupplementaryAudioSink(IN:supplementaryAudioSinkId:[%d], OUT:_aidl_return:[%s]), status:[isOK:%s, exception:%s]",
                         supplementaryAudioSinkId.value, _aidl_return ? "true" : "false",
                         UT_Control_GetMapString(bool_mapTable, status.isOk()),
                         UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
 
             UT_ASSERT_TRUE(status.isOk());
             UT_ASSERT_TRUE(_aidl_return);
         }
 
         status = controller->stop();
         UT_ASSERT_TRUE(status.isOk());
     }
 }
 
 /**
  * @brief Test the negative scenario for setting a supplementary audio sink in AVClockController.
  *
  * This test verifies the behavior of the AVClockController when attempting to set a supplementary audio sink with an invalid ID. It ensures that the operation fails as expected when the ID is not valid.
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
  * | Variation / Step | Description | Test Data | Expected Result | Notes |
  * | :----: | --------- | ---------- |-------------- | ----- |
  * | 01 | Verify that mControllers is not empty | mControllers | Should be successful | |
  * | 02 | Iterate over each controller and get the current state | controller, currentState | Should be successful | |
  * | 03 | Check if the current state is READY or STARTED | currentState | Should be successful | |
  * | 04 | Retrieve audio sink IDs from YAML using macros | audioSinkIds, count | Should be successful | |
  * | 05 | Sort audio sink IDs and determine invalid ID | audioSinkIds, invalidId | Should be successful | |
  * | 06 | Prepare supplementary audio sink ID with invalid ID | supplementaryAudioSinkId | Should be successful | |
  * | 07 | Call setSupplementaryAudioSink with invalid ID | supplementaryAudioSinkId, aidlReturn | Should Fail | |
  * | 08 | Verify the results of setSupplementaryAudioSink call | supplementaryAudioSinkId, aidlReturn, status | Should be successful | |
  */
 UT_ADD_TEST(AVClockControllerBaseTest, test_l1_AVClockController_negative_setSupplementaryAudioSink) {
     UT_ASSERT_FALSE(mControllers.empty());
     Status status;
     // Retrieve audio sink IDs from YAML using macros
     std::vector<int32_t> audioSinkIds;
     std::string audiosinkcount_path = "IAudioSinkManager.IAudioSink";
     size_t expectedCount = UT_KVP_PROFILE_GET_LIST_COUNT(audiosinkcount_path.c_str());
 
     for (size_t i = 0; i < expectedCount; ++i)
     {
         std::string audiosinkid_path = "IAudioSinkManager.IAudioSink."+std::to_string(i);
         audioSinkIds.push_back(UT_KVP_PROFILE_GET_UINT32(audiosinkid_path.c_str()));
     }
     UT_ASSERT_GREATER(audioSinkIds.size(), 0);
 
    // Sort IDs and determine invalid ID
    std::sort(audioSinkIds.begin(), audioSinkIds.end());
    int32_t invalidId_low = audioSinkIds.front() - 1;
    IAudioSink::Id supplementaryAudioSinkId;
    supplementaryAudioSinkId.value = invalidId_low;
     
     for (auto& controller : mControllers) 
     {
         //test 1: using invalid lower id
         bool aidlReturn = false;
         UT_LOG_INFO("Calling setSupplementaryAudioSink(IN:supplementaryAudioSinkId:[%d], OUT:_aidl_return)", supplementaryAudioSinkId.value);
         status = controller->setSupplementaryAudioSink(supplementaryAudioSinkId, &aidlReturn);
         UT_LOG_INFO("Result setSupplementaryAudioSink(IN:supplementaryAudioSinkId:[%d], OUT:_aidl_return:[%d]), status:[isOK:%s, exception:%s]",
                     supplementaryAudioSinkId.value, aidlReturn, UT_Control_GetMapString(bool_mapTable, status.isOk()), UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
         UT_ASSERT_FALSE(aidlReturn);
         UT_ASSERT_TRUE(status.isOk());
 
 
         //test 2: using invalid higher id
         int32_t invalidId_high = audioSinkIds.back() + 1;
         supplementaryAudioSinkId.value = invalidId_high;
 
         UT_LOG_INFO("Calling setSupplementaryAudioSink(IN:supplementaryAudioSinkId:[%d], OUT:_aidl_return)", supplementaryAudioSinkId.value);
         status = controller->setSupplementaryAudioSink(supplementaryAudioSinkId, &aidlReturn);
         UT_LOG_INFO("Result setSupplementaryAudioSink(IN:supplementaryAudioSinkId:[%d], OUT:_aidl_return:[%d]), status:[isOK:%s, exception:%s]",
                     supplementaryAudioSinkId.value, aidlReturn, UT_Control_GetMapString(bool_mapTable, status.isOk()), UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
         UT_ASSERT_FALSE(aidlReturn);
         UT_ASSERT_TRUE(status.isOk());
     }
 }
 
 /**
  * @brief Test the positive scenario of retrieving supplementary audio sink in different states.
  *
  * This test verifies the functionality of the `getSupplementaryAudioSink` method in the `AVClockController` class.
  * It checks whether the method correctly retrieves a valid audio sink ID or returns an undefined ID when the controller
  * is in the READY and STARTED states. The test ensures that the method behaves as expected and returns a valid status.
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
  * | :----: | --------- | ---------- |-------------- | ----- |
  * | 01 | Retrieve valid audio sink IDs from YAML using macros | audioSinkCount = UT_KVP_PROFILE_GET_LIST_COUNT(IAudioSinkManager.IAudioSink) | Should be successful | Should be successful |
  * | 02 | Insert valid audio sink IDs into a set for validation | validAudioSinkIds.insert(UT_KVP_PROFILE_GET_UINT32(IAudioSinkManager.IAudioSink.<index>)) | Should be successful | Should be successful |
  * | 03 | Set controller state to READY | state = IAVClock::State::READY | Status isOk = true | Should be successful |
  * | 04 | Call getSupplementaryAudioSink in READY state | audioSinkId = controller->getSupplementaryAudioSink() | Status isOk = true, audioSinkId is valid or UNDEFINED | Should Pass |
  * | 05 | Set controller state to STARTED | state = IAVClock::State::STARTED | Status isOk = true | Should be successful |
  * | 06 | Call getSupplementaryAudioSink in STARTED state | audioSinkId = controller->getSupplementaryAudioSink() | Status isOk = true, audioSinkId is valid or UNDEFINED | Should Pass |
  */
 UT_ADD_TEST(AVClockControllerBaseTest, test_l1_AVClockController_positive_getSupplementaryAudioSink)
 {
     Status status;
     IAudioSink::Id audioSinkId;
     std::unordered_set<int32_t> validAudioSinkIds;
 
     std::vector<int32_t> audioSinkIds;
     std::string audiosinkcount_path = "IAudioSinkManager.IAudioSink";
     size_t expectedCount = UT_KVP_PROFILE_GET_LIST_COUNT(audiosinkcount_path.c_str());
 
     for (size_t i = 0; i < expectedCount; ++i)
     {
         std::string audiosinkid_path = "IAudioSinkManager.IAudioSink."+std::to_string(i);
         audioSinkIds.push_back(UT_KVP_PROFILE_GET_UINT32(audiosinkid_path.c_str()));
     }
     UT_ASSERT_GREATER(audioSinkIds.size(), 0);
 
     for (auto& controller : mControllers) 
     {
         //test from READY state
         UT_LOG_INFO("Calling getSupplementaryAudioSink(OUT:audioSinkId)");
         status = controller->getSupplementaryAudioSink(&audioSinkId);
         UT_LOG_INFO("Result getSupplementaryAudioSink(OUT:audioSinkId:[%d]), status:[isOK:%s, exception:%s]",
                     audioSinkId.value, UT_Control_GetMapString(bool_mapTable, status.isOk()),
                     UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
 
         UT_ASSERT_TRUE(status.isOk());
         UT_ASSERT_TRUE(audioSinkId.value == IAudioSink::Id::UNDEFINED || std::find(audioSinkIds.begin(), audioSinkIds.end(), audioSinkId.value) != audioSinkIds.end());
 
         // Test from STARTED state
         status = controller->start();
         UT_ASSERT_TRUE(status.isOk());
 
         IAudioSink::Id audioSinkId2;
         UT_LOG_INFO("Calling getSupplementaryAudioSink(OUT:audioSinkId)");
         status = controller->getSupplementaryAudioSink(&audioSinkId2);
         UT_LOG_INFO("Result getSupplementaryAudioSink(OUT:audioSinkId:[%d]), status:[isOK:%s, exception:%s]",
                     audioSinkId2.value, UT_Control_GetMapString(bool_mapTable, status.isOk()),
                     UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
 
         UT_ASSERT_TRUE(status.isOk());
         UT_ASSERT_TRUE(audioSinkId2.value == IAudioSink::Id::UNDEFINED || std::find(audioSinkIds.begin(), audioSinkIds.end(), audioSinkId2.value) != audioSinkIds.end());
     
         status = controller->stop();
         UT_ASSERT_TRUE(status.isOk()); 
     }
 }

 /**
  * @brief Test the functionality of setting a video sink in AVClockController.
  *
  * This test verifies the ability of the AVClockController to correctly set a video sink using valid and undefined IDs. It ensures that the controller behaves as expected when provided with different video sink IDs.
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
  * | :----: | --------- | ---------- |-------------- | ----- |
  * | 01 | Verify that mControllers is not empty | mControllers | Should be successful | Should be successful |
  * | 02 | Retrieve the count of video sinks from IVideoSinkManager | videoSinkCount = UT_KVP_PROFILE_GET_LIST_COUNT(IVideoSinkManager.IVideoSink) | Should be successful | Should be successful |
  * | 03 | Iterate over each video sink ID and set it using setVideoSink | videoSinkId.value = UT_KVP_PROFILE_GET_UINT32(IVideoSinkManager.IVideoSink.<index>), aidlReturn = false | status.isOk() = true, aidlReturn = true | Should Pass |
  * | 04 | Set video sink with UNDEFINED ID using setVideoSink | undefinedVideoSinkId.value = IVideoSink::Id::UNDEFINED, aidlReturnUndefined = false | statusUndefined.isOk() = true, aidlReturnUndefined = true | Should Pass |
  */
 UT_ADD_TEST(AVClockControllerBaseTest, test_l1_AVClockController_positive_setVideoSink) {
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
 
     // Test from READY state
     for (auto& controller : mControllers) 
     {
         // Test Case 1: Set Video Sink with Valid ID
         for(auto& id : videoSinkIds)
         {
             IVideoSink::Id videoSinkId;
             videoSinkId.value = id;
             bool aidlReturn = false;
             UT_LOG_INFO("Calling setVideoSink(IN:videoSinkIds:[%d], OUT:_aidl_return)", videoSinkId.value);
             status = controller->setVideoSink(videoSinkId, &aidlReturn);
             UT_LOG_INFO("Result setVideoSink(IN:videoSinkIds:[%d], OUT:_aidl_return:[%s]), status:[isOK:%s, exception:%s]",
                         videoSinkId.value, aidlReturn ? "true" : "false",
                         UT_Control_GetMapString(bool_mapTable, status.isOk()),
                         UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
 
             UT_ASSERT_TRUE(status.isOk());
             UT_ASSERT_TRUE(aidlReturn);
         }
 
         // Test Case 2: Set Video Sink with UNDEFINED ID
         IVideoSink::Id undefinedVideoSinkId;
         undefinedVideoSinkId.value = IVideoSink::Id::UNDEFINED;
 
         bool aidlReturnUndefined = false;
         UT_LOG_INFO("Calling setVideoSink(IN:videoSinkId:[%d], OUT:_aidl_return)", undefinedVideoSinkId.value);
         Status statusUndefined = controller->setVideoSink(undefinedVideoSinkId, &aidlReturnUndefined);
 
         UT_LOG_INFO("Result setVideoSink(IN:videoSinkId:[%d], OUT:_aidl_return:[%s]), status:[isOK:%s, exception:%s]",
                     undefinedVideoSinkId.value, aidlReturnUndefined ? "true" : "false",
                     UT_Control_GetMapString(bool_mapTable, statusUndefined.isOk()),
                     UT_Control_GetMapString(statusException_mapTable, statusUndefined.exceptionCode()));
 
         UT_ASSERT_TRUE(statusUndefined.isOk());
         UT_ASSERT_TRUE(aidlReturnUndefined);
     }
 

 
     for (auto& controller : mControllers) 
     {
        // Test from STARTED state
        status = controller->start();
        UT_ASSERT_TRUE(status.isOk());

         // Test Case 1: Set Video Sink with Valid ID
         for(auto& id : videoSinkIds)
         {
            IVideoSink::Id videoSinkId;
            videoSinkId.value = id;
            bool aidlReturn = false;
            UT_LOG_INFO("Calling setVideoSink(IN:videoSinkIds:[%d], OUT:_aidl_return)", videoSinkId.value);
            status = controller->setVideoSink(videoSinkId, &aidlReturn);
            UT_LOG_INFO("Result setVideoSink(IN:videoSinkIds:[%d], OUT:_aidl_return:[%s]), status:[isOK:%s, exception:%s]",
                videoSinkId.value, aidlReturn ? "true" : "false",
                        UT_Control_GetMapString(bool_mapTable, status.isOk()),
                        UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));

            UT_ASSERT_TRUE(status.isOk());
            UT_ASSERT_TRUE(aidlReturn);
         }
 
         // Test Case 2: Set Video Sink with UNDEFINED ID
         IVideoSink::Id undefinedVideoSinkId;
         undefinedVideoSinkId.value = IVideoSink::Id::UNDEFINED;
 
         bool aidlReturnUndefined = false;
         UT_LOG_INFO("Calling setVideoSink(IN:videoSinkId:[%d], OUT:_aidl_return)", undefinedVideoSinkId.value);
         Status statusUndefined = controller->setVideoSink(undefinedVideoSinkId, &aidlReturnUndefined);
 
         UT_LOG_INFO("Result setVideoSink(IN:videoSinkId:[%d], OUT:_aidl_return:[%s]), status:[isOK:%s, exception:%s]",
                     undefinedVideoSinkId.value, aidlReturnUndefined ? "true" : "false",
                     UT_Control_GetMapString(bool_mapTable, statusUndefined.isOk()),
                     UT_Control_GetMapString(statusException_mapTable, statusUndefined.exceptionCode()));
 
         UT_ASSERT_TRUE(statusUndefined.isOk());
         UT_ASSERT_TRUE(aidlReturnUndefined);

         status = controller->stop();
         UT_ASSERT_TRUE(status.isOk());
     }
 
 }

 /**
  * @brief Test the negative scenario for setting a video sink in AVClockController
  *
  * This test verifies the behavior of the AVClockController when attempting to set a video sink with an invalid ID. It ensures that the function correctly handles invalid input by returning appropriate error codes and statuses.
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
  * | Variation / Step | Description | Test Data | Expected Result | Notes |
  * | :----: | --------- | ---------- |-------------- | ----- |
  * | 01 | Check if mControllers is not empty | mControllers | Should not be empty | Should be successful |
  * | 02 | Retrieve video sink IDs from YAML using macros | IVideoSinkManager.IVideoSink | List of video sink IDs | Should be successful |
  * | 03 | Sort video sink IDs and determine invalid videoSinkId | videoSinkIds | invalidVideoSinkId = smallest ID - 1 | Should be successful |
  * | 04 | Iterate over all controllers and set invalid videoSinkId | videoSinkId = invalidVideoSinkId | aidlReturn = false, status.isOk() = false, status.exceptionCode() = EX_ILLEGAL_STATE | Should Fail |
  */
 UT_ADD_TEST(AVClockControllerBaseTest, test_l1_AVClockController_negative_setVideoSink) {
     UT_ASSERT_FALSE(mControllers.empty());
 
 
     std::vector<int32_t> videoSinkIds;
     std::string videosinkcount_path = "IVideoSinkManager.IVideoSink";
     size_t expectedCount = UT_KVP_PROFILE_GET_LIST_COUNT(videosinkcount_path.c_str());
 
     for (size_t i = 0; i < expectedCount; ++i)
     {
         std::string videosinkid_path = "IVideoSinkManager.IVideoSink."+std::to_string(i);
         videoSinkIds.push_back(UT_KVP_PROFILE_GET_UINT32(videosinkid_path.c_str()));
     }
     UT_ASSERT_GREATER(videoSinkIds.size(), 0);  
 
     // Sort and determine invalid videoSinkId
     std::sort(videoSinkIds.begin(), videoSinkIds.end());
     int32_t invalidVideoSinkIdlow = videoSinkIds.front() - 1;
     int32_t invalidVideoSinkIdhigh = videoSinkIds.back() + 1;
 
 
     // Iterate over all controllers
     for (auto& controller : mControllers) {
         bool aidlReturn = false;
         IVideoSink::Id videoSinkId;
         videoSinkId.value = invalidVideoSinkIdlow;
 
         UT_LOG_INFO("Calling setVideoSink(IN:videoSinkId:[%d], OUT:_aidl_return)", videoSinkId.value);
         Status status = controller->setVideoSink(videoSinkId, &aidlReturn);
         UT_LOG_INFO("Result setVideoSink(IN:videoSinkId:[%d], OUT:_aidl_return:[%s]), status:[isOK:%s, exception:%s]",
                     videoSinkId.value, aidlReturn ? "true" : "false",
                     UT_Control_GetMapString(bool_mapTable, status.isOk()),
                     UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
         UT_ASSERT_FALSE(aidlReturn);
         UT_ASSERT_FALSE(status.isOk());
 
         videoSinkId.value = invalidVideoSinkIdhigh;
 
         UT_LOG_INFO("Calling setVideoSink (IN:videoSinkId:[%d], OUT:_aidl_return)", videoSinkId.value);
         status = controller->setVideoSink(videoSinkId, &aidlReturn);
         UT_LOG_INFO("Result setVideoSink(IN:videoSinkId:[%d], OUT:_aidl_return:[%s]), status:[isOK:%s, exception:%s]",
                     videoSinkId.value, aidlReturn ? "true" : "false",
                     UT_Control_GetMapString(bool_mapTable, status.isOk()),
                     UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
         UT_ASSERT_FALSE(aidlReturn);
         UT_ASSERT_FALSE(status.isOk());
     }
 }
 
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
 UT_ADD_TEST(AVClockControllerBaseTest, test_l1_AVClockController_positive_getVideoSink)
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
 UT_ADD_TEST(AVClockControllerBaseTest, test_l1_AVClockController_positive_setClockMode)
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
 UT_ADD_TEST(AVClockControllerBaseTest, test_l1_AVClockController_negative_setClockMode) {
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
 UT_ADD_TEST(AVClockControllerBaseTest, test_l1_AVClockController_positive_getClockMode) {
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
 UT_ADD_TEST(AVClockControllerBaseTest, test_l1_AVClockController_positive_notifyPCRSample) {
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
 UT_ADD_TEST(AVClockControllerBaseTest, test_l1_AVClockController_negative_notifyPCRSample) {
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
 UT_ADD_TEST(AVClockControllerBaseTest, test_l1_AVClockController_positive_getCurrentClockTime)
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
 UT_ADD_TEST(AVClockControllerBaseTest, test_l1_AVClockController_negative_getCurrentClockTime) 
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
 UT_ADD_TEST(AVClockControllerBaseTest, test_l1_AVClockController_positive_setPlaybackRate) {
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
 UT_ADD_TEST(AVClockControllerBaseTest, test_l1_AVClockController_negative_setPlaybackRate) {
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
 UT_ADD_TEST(AVClockControllerBaseTest, test_l1_AVClockController_positive_getPlaybackRate) {
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
 UT_ADD_TEST(AVClockControllerBaseTest, test_l1_AVClockController_negative_getPlaybackRate) {
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
 
 int main(int argc, char **argv)
 {
     UT_status_t result;
     UT_init(argc, argv);
     result = UT_run_tests();
     return result;
 }
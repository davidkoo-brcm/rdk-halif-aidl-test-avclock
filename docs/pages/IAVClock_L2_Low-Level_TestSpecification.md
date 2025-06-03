# IAVCLOCK L2 Low Level Test Specification and Procedure Documentation

## Table of Contents

    -[IAVCLOCK L2 Low Level Test Specification and Procedure Documentation](#iavclock-l2-low-level-test-specification-and-procedure-documentation)

    -[Table of Contents](#table-of-contents)
         - [Overview](#overview)
           - [Acronyms, Terms and Abbreviations](#acronyms-terms-and-abbreviations)
           - [Definitions](#definitions)
           - [References](#references)
         - [Level 2 Test Procedure](#level-2-test-procedure)

## Overview

    This document describes the level 2 testing suite for the IAVCLOCK module.

### Acronyms, Terms and Abbreviations

    -`HAL` \- Hardware Abstraction Layer, may include some common components
       - `UT`  \- Unit Test(s)
       - `OEM`  \- Original Equipment Manufacture
       - `SoC`  \- System on a Chip

### Definitions

    -`ut-core` \- Common Testing Framework [https://github.com/rdkcentral/ut-core](https://github.com/rdkcentral/ut-core), which wraps a open-source framework that can be expanded to the requirements for future framework.

### References

- `High Level Test Specification` - [avClock_High-Level-2_TestSpecification.md](avClock_High-Level-2_TestSpecification.md)

## Level 2 Test Procedure

    The following functions are expecting to test the module operates correctly.

### Test 1

| Title         | Details                                                            |
| ------------- | ------------------------------------------------------------------ |
| Function Name | `test_l2_AVClockGetProperty_GetReadOnlyProperties`               |
| Description   | Get read only properties (Property) of AV Clock using get function |
| Test Group    | Module : 02                                                        |
| Test Case ID  | 001                                                                |
| Priority      | High                                                               |

**Pre-Conditions :**
None

**Dependencies :**
None

**User Interaction :**
If user chose to run the test in interactive mode, then the test case has to be selected via console.

#### Test Procedure :

| Variation / Steps | Description                                              | Test Data (input → output)                      | Expected Result                          | Notes                |
| ----------------- | -------------------------------------------------------- | ----------------------------------------------- | ---------------------------------------- | -------------------- |
| 00                | **Setup:** Initialize AVClocks and open controllers      | `getAVClockIds()` → list, `open()` → controller | AVClocks opened, state becomes READY     | Prerequisite         |
| 01                | Iterate over each AVClock in `mAvClocks`                 | `clk` = `AVClock` object                        | Status is **OK**                         | Should be successful |
| 02                | Call `getProperty` with `RESOURCE_ID`                    | `RESOURCE_ID` → `propertyValue`                 | Status is **OK**, `propertyValue` is set | Should be successful |
| 03                | Assert status and value for `RESOURCE_ID`                | `status`, `propertyValue.has_value()`           | Both assertions pass                     | Should be successful |
| 04                | Call `getProperty` with `METRIC_AV_SYNC_ACCURACY_MS`     | `METRIC_AV_SYNC_ACCURACY_MS` → `propertyValue`  | Status is **OK**, `propertyValue` is set | Should be successful |
| 05                | Assert status and value for `METRIC_AV_SYNC_ACCURACY_MS` | `status`, `propertyValue.has_value()`           | Both assertions pass                     | Should be successful |
| 06                | Call `getProperty` with `METRIC_AV_RESYNC_COUNT`         | `METRIC_AV_RESYNC_COUNT` → `propertyValue`      | Status is **OK**, `propertyValue` is set | Should be successful |
| 07                | Assert status and value for `METRIC_AV_RESYNC_COUNT`     | `status`, `propertyValue.has_value()`           | Both assertions pass                     | Should be successful |
| 08                | **Teardown:** Close controllers, unregister listeners    | `close()` → true, state transitions to CLOSED   | Cleanup completed successfully           | Post-requisite       |



### Test 2

| Title         | Details                                                                                                                        |
| ------------- | ------------------------------------------------------------------------------------------------------------------------------ |
| Function Name | `test_l2_AVClockSetPropertyTest_SetReadOnlyPropertyTest`                                                                     |
| Description   | Set read only properties (Property) of AV Clock using setProperty function. Verify return value from the setProperty is false. |
| Test Group    | Module : 02                                                                                                                    |
| Test Case ID  | 002                                                                                                                            |
| Priority      | High                                                                                                                           |

**Pre-Conditions :**
None

**Dependencies :**
None

**User Interaction :**
If user chose to run the test in interactive mode, then the test case has to be selected via console.

#### Test Procedure :

| Variation / Steps | Description                                               | Test Data (input → output)                                                                 | Expected Result                            | Notes                |
| ----------------- | --------------------------------------------------------- | ------------------------------------------------------------------------------------------ | ------------------------------------------ | -------------------- |
| 00                | **Setup:** Initialize and open all AVClocks               | `getAVClockIds()` → non-empty list, register listener, `open()` → `controller != nullptr`  | All AVClocks opened and state is READY     | Prerequisite         |
| 01                | For each AVClock in `mAvClocks`                           | `clk` = AVClock instance                                                                   | Loop starts successfully                   | Applies to all steps |
| 02                | Attempt to set `RESOURCE_ID` property                     | `property = RESOURCE_ID`, `propertyValue = intValue:123` → `result = false`                | `status.isOk() == true`, `result == false` | Should be rejected   |
| 03                | Attempt to set `METRIC_AV_SYNC_ACCURACY_MS` property      | `property = METRIC_AV_SYNC_ACCURACY_MS`, `propertyValue = intValue:123` → `result = false` | `status.isOk() == true`, `result == false` | Should be rejected   |
| 04                | Attempt to set `METRIC_AV_RESYNC_COUNT` property          | `property = METRIC_AV_RESYNC_COUNT`, `propertyValue = intValue:123` → `result = false`     | `status.isOk() == true`, `result == false` | Should be rejected   |
| 05                | **Teardown:** Close all AVClocks and unregister listeners | `close()` → `true`, state transitions `READY → CLOSING → CLOSED`, unregister → `true`      | All clocks closed and cleaned up properly  | Post-requisite       |


### Test 3

| Title         | Details                                                                                                                                                                              |
| ------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| Function Name | `test_l2_AVClockStateTransitionTest_TestStateTransitions`                                                                                                                          |
| Description   | Ensure state transitions (open->start->stop->close) of AV Clock. Validate state change using onStateChanged using of IAVClockEventListener, IAVClockControllerListener and getState. |
| Test Group    | Module : 02                                                                                                                                                                          |
| Test Case ID  | 003                                                                                                                                                                                  |
| Priority      | High                                                                                                                                                                                 |

**Pre-Conditions :**
None

**Dependencies :**
None

**User Interaction :**
If user chose to run the test in interactive mode, then the test case has to be selected via console.

#### Test Procedure :

| Variation / Step | Description                                                                                                       | Test Data                                                            | Expected Result                      | Notes          |
| ---- | ----------------------------------------------------------------------------------------------------------------- | -------------------------------------------------------------------- | ------------------------------------ | -------------- |
| 00   | **Setup:** Retrieve AVClock IDs and verify not empty                                                              | Call `getAVClockIds()`                                               | Status is OK; IDs list is not empty  | Prerequisite   |
| 01   | For each AVClock ID, get AVClock instance and register event listener                                             | AVClock ID                                                           | Status is OK; listener registered    | Prerequisite   |
| 02   | For each AVClock, create controller listener, open AVClock to get controller, and wait for state `CLOSED`→`READY` | Controller listener; AVClock instance                                | Status is OK; AVClock state == READY | Prerequisite   |
| 03   | Start the AV Clock using its controller                                                                           | Valid AVClockController                                              | Status is OK                         | Test step      |
| 04   | Wait for state transition from `STARTING` to `STARTED`                                                            | Initial=STARTING, Final=STARTED, timeout=MAX\_FINAL\_STATE\_WAIT\_MS | State transition successful          | Test step      |
| 05   | Verify AVClock state is `STARTED` after start                                                                     | Call `getState()`                                                    | State == STARTED                     | Test step      |
| 06   | Stop the AV Clock using its controller                                                                            | Valid AVClockController                                              | Status is OK                         | Test step      |
| 07   | Wait for state transition from `STOPPING` to `READY`                                                              | Initial=STOPPING, Final=READY, timeout=MAX\_FINAL\_STATE\_WAIT\_MS   | State transition successful          | Test step      |
| 08   | Verify AVClock state is `READY` after stop                                                                        | Call `getState()`                                                    | State == READY                       | Test step      |
| 09   | **Teardown:** Close each AVClock with corresponding controller and verify closed status                           | Valid controller and AVClock                                         | Status is OK; closed = true          | Post-requisite |
| 10   | Wait for state transition from `READY` to `CLOSING` to `CLOSED`                                                   | Initial=READY, Intermediate=CLOSING, Final=CLOSED                    | State transitions successful         | Post-requisite |
| 11   | Verify AVClock state is `CLOSED` after close                                                                      | Call `getState()`                                                    | State == CLOSED                      | Post-requisite |
| 12   | Unregister event listeners from each AVClock and verify unregistration                                            | Event listener                                                       | Unregistered successfully            | Post-requisite |
| 13   | Clear internal storage vectors for controllers, listeners, clocks, and IDs                                        | None                                                                 | All internal containers cleared      | Post-requisite |


### Test 4

| Title         | Details                                                                                                                                                                                 |
| ------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Function Name | `test_l2_AVClockController_VerifyClockModeSetToAutoOnOpenAndSetGetClockMode`                                                                                                          |
| Description   | Check and verify clock mode should set to AUTO upon open each AV Clock instance. Set clock mode using setClockMode and verify using getClockMode set and get ClockMode should be equal. |
| Test Group    | Module : 02                                                                                                                                                                             |
| Test Case ID  | 004                                                                                                                                                                                     |
| Priority      | High                                                                                                                                                                                    |

**Pre-Conditions :**
None

**Dependencies :**
None

**User Interaction :**
If user chose to run the test in interactive mode, then the test case has to be selected via console.

#### Test Procedure :

| Variation / Steps | Description                                                 | Test Data (input → output)                                                                                                | Expected Result                                                             | Notes                                |
| ----------------- | ----------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------- | --------------------------------------------------------------------------- | ------------------------------------ |
| **00**            | **Setup**: Prepare AVClock test environment                 | Call `SetUp()` → retrieve clock IDs, create `mAvClocks` and `mControllers`, register listeners, transition to READY state | All clocks initialized, listeners registered, controllers in READY state    | Executed before test logic begins    |
| **01**            | For each `AVClockController` in `mControllers` (begin loop) | `ctrl` = AVClockController instance                                                                                       | Loop starts successfully                                                    | Applies to all steps                 |
| **02**            | Verify initial clock mode is **AUTO** after open            | `getClockMode()` → `initialMode`                                                                                          | `status.isOk()` = true;<br>`initialMode` = AUTO                             | Baseline check after controller open |
| **03**            | Set clock mode **AUTO** and verify                          | `setClockMode(AUTO)` → `setResult = true`;<br>`getClockMode()` → `setMode`                                                | `status.isOk()` = true;<br>`setResult` = true;<br>`setMode` = AUTO          | Should be successful                 |
| **04**            | Set clock mode **PCR** and verify                           | `setClockMode(PCR)` → `setResult = true`;<br>`getClockMode()` → `setMode`                                                 | `status.isOk()` = true;<br>`setResult` = true;<br>`setMode` = PCR           | Should be successful                 |
| **05**            | Set clock mode **AUDIO\_MASTER** and verify                 | `setClockMode(AUDIO_MASTER)` → `setResult = true`;<br>`getClockMode()` → `setMode`                                        | `status.isOk()` = true;<br>`setResult` = true;<br>`setMode` = AUDIO\_MASTER | Should be successful                 |
| **06**            | Set clock mode **VIDEO\_MASTER** and verify                 | `setClockMode(VIDEO_MASTER)` → `setResult = true`;<br>`getClockMode()` → `setMode`                                        | `status.isOk()` = true;<br>`setResult` = true;<br>`setMode` = VIDEO\_MASTER | Should be successful                 |
| **07**            | End loop for all controllers                                | —                                                                                                                         | All controllers tested without failure                                      | Loop completes successfully          |
| **08**            | **Teardown**: Reset state and clean up test environment     | Call `TearDown()` → close each controller, verify CLOSED state, unregister listeners, clear vectors                       | All controllers closed, listeners unregistered, internal state reset        | Executed after test logic completes  |


### Test 5

| Title         | Details                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               |
| ------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Function Name | `test_l2_AVClockPlaybackRateTest_VerifyPlaybackRateAndStateTransitions`                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             |
| Description   | Check and verify playback state after open it should be set as 1.0 /n set supported playback rate like (0.0 , 1.0 and 2.0) using setPlaybackRate and verify using getPlaybackRate set and get Playback Rate should be equal /n start opened AV Clock instance verify the state using getState or onStateChanged and AV Clock instance state should be in STARTED /n Again set playback rate using using setPlaybackRate and verify using getPlaybackRate set and get Playback Rate should be equal /n Stop AV clock instance using stop and check the state using getState or onStateChanged should be in READY state |
| Test Group    | Module : 02                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           |
| Test Case ID  | 005                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   |
| Priority      | High                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  |

**Pre-Conditions :**
None

**Dependencies :**
None

**User Interaction :**
If user chose to run the test in interactive mode, then the test case has to be selected via console.

#### Test Procedure :

| Variation / Steps | Description                                           | Test Data (input → output)                                                                   | Expected Result                                        | Notes                                |
| ----------------- | ----------------------------------------------------- | -------------------------------------------------------------------------------------------- | ------------------------------------------------------ | ------------------------------------ |
| **00**            | **Setup**: Initialize clocks and controllers          | Call `SetUp()` → creates `mAvClocks`, `mControllers`, registers listeners, opens controllers | All clocks opened, listeners registered, state = READY | Common setup for AVClock tests       |
| **01**            | Verify that the controller list is not empty          | None                                                                                         | `ASSERT_FALSE(mControllers.empty())`                   | Should be successful                 |
| **02**            | Verify default playback rate is 1.0                   | `getPlaybackRate()` → `playbackRate`                                                         | `playbackRate == 1.0`                                  | Before any set operation             |
| **03**            | Set playback rate to 0.0 and verify                   | `setPlaybackRate(0.0)` → `success = true`;<br>`getPlaybackRate()` → `playbackRate`           | `success == true`, `playbackRate == 0.0`               | Within controller loop               |
| **04**            | Set playback rate to 1.0 and verify                   | `setPlaybackRate(1.0)` → `success = true`;<br>`getPlaybackRate()` → `playbackRate`           | `success == true`, `playbackRate == 1.0`               | Valid rate                           |
| **05**            | Set playback rate to 2.0 and verify                   | `setPlaybackRate(2.0)` → `success = true`;<br>`getPlaybackRate()` → `playbackRate`           | `success == true`, `playbackRate == 2.0`               | Valid rate                           |
| **06**            | Start the AV Clock and verify transition to STARTED   | `start()`;<br>`getState()` → `state`                                                         | `status.isOk() == true`, `state == State::STARTED`     | Verifies `start()` behavior          |
| **07**            | Set playback rate to 0.0 again after START and verify | `setPlaybackRate(0.0)` → `success = true`;<br>`getPlaybackRate()` → `playbackRate`           | `success == true`, `playbackRate == 0.0`               | Confirms dynamic setting is allowed  |
| **08**            | Set playback rate to 1.0 again after START and verify | `setPlaybackRate(1.0)` → `success = true`;<br>`getPlaybackRate()` → `playbackRate`           | `success == true`, `playbackRate == 1.0`               | Redundant validation after START     |
| **09**            | Set playback rate to 2.0 again after START and verify | `setPlaybackRate(2.0)` → `success = true`;<br>`getPlaybackRate()` → `playbackRate`           | `success == true`, `playbackRate == 2.0`               | Confirms idempotency and flexibility |
| **10**            | Stop the AV Clock and verify transition to READY      | `stop()`;<br>`getState()` → `state`                                                          | `status.isOk() == true`, `state == State::READY`       | End of test sequence                 |
| **11**            | **Teardown**: Close controllers and cleanup resources | Call `TearDown()` → close controllers, unregister listeners, clear vectors                   | All clocks CLOSED, listeners unregistered              | Restores system to pre-test state    |

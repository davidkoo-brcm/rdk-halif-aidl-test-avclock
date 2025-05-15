#AV Clock High Level Test Specification Document

## Table of Contents

- [Acronyms, Terms and Abbreviations](#acronyms-terms-and-abbreviations)
- [Introduction](#introduction)
- [Module Description](#module-description)
- [Test Frame Specifications](#test-frame-specifications)
- [L2 Test Specification](#l2-test-specification)
- [L3 Test Specification](#l3-test-specification)

## Acronyms, Terms and Abbreviations

- `HAL`    - Hardware Abstraction Layer
- `API`    - Application Programming Interface
- `L2`     - Level 2 Testing
- `L3`     - Level 3 Testing
- `NA`     - Not Applicable
- `Y`      - Yes
- `CB`     - Call-back function (suffix)
- `RDK`    - Reference Design Kit
- `EOS`    - End Of Stream
- `PCR`    - Program Clock Reference

## Introduction

This document provides an overview of the testing requirements for the AV Clock module. It outlines the scope of testing, objectives, testing levels and approaches, specific test requirements, and expected deliverables.

- AV Clock Interface Definitions - [AV Clock HAL](https://github.com/rdkcentral/rdk-halif-aidl/tree/v0.6/avclock/current/com/rdk/hal/avclock)

- AV Clock Overview - [AV Clock Overview](https://wiki.rdkcentral.com/display/RDKHAL/AV+Clock)

## Module Description

The AV Clock HAL service links audio sinks and video sinks together to provide lip sync playback against a shared clock.

The audio and video sinks that are linked by an AV Clock form a sync group which share a common clock for presentation.

The AV Clock HAL allows the application to specify which video sink and audio sink(s) are linked in the sync group.

## Test Frame Specifications

TBD

## L2 Test Specification
Key Notes:
Pre-Requirements and Post-Requirements apply to all L2 tests.

|Requirement|Description|HAL APIs|
|-----------|-----------|--------|
|Pre-Requirements|<ul><li>Read supported AV Clock Id's using `IAVClockManager` class</li><li>Loop through supported AV Clock Id's</li><li>Register event listeners using `registerEventListener`</li><li>`open` AV Clock instance and verify the state using `getState` or `onStateChanged` state should be in `READY`</li>|<ul><li>IAVClockManager - getAVClockIds, getAVClock</li><li>IAVClock  - open, registerEventListener, getState</li><li>IAVClockEventListener - onStateChanged</li></ul>|

|Requirement|Description|HAL APIs|
|-----------|-----------|--------|
|Post-Requirements|<ul><li>Close the AV Clock instance using `close` check state should be in `CLOSED` using `getState` or `onStateChanged`</li><li>Unregister event listeners using `unregisterEventListener`</li></ul>|<ul><li>IAVClock  - close, getState, unregisterEventListener</li><li>IAVClockEventListener - onStateChanged</li></ul>|

|Test|Description|HAL APIs| 
|----|-----------|--------|
|Get AV Clock Properties|<ul><li>get read only properties (`Property`) of AV Clock using get function</li></ul>|<ul><li>IAVClockManager - getAVClockIds</li><li>IAVClock - getProperty</li></ul>|
|Set AV Clock Properties|<ul><li>set read only properties (`Property`) of AV Clock using `setProperty` function</li><li>Verify return value from the `setProperty` is false </li></ul>|<ul><li>IAVClockManager - getAVClockIds</li><li>IAVClock - setProperty</li></ul>|
|States of AV Clock|<ul><li>Ensure state transitions (`open`->`start`->`stop`->`close`) of AV Clock</li><li>Validate state change using `onStateChanged` using  of `IAVClockEventListener`,`IAVClockControllerListener` and `getState`</li></ul>|<ul><li>IAVClock  - getState </li><li>IAVClockEventListener - onStateChanged</li><li>IAVClockController - start, stop</li></ul>|
|Set and Get Clock Mode|<ul><li>Check and verify clock mode should set to `AUTO` upon open each AV Clock instance</li><li>set clock mode using `setClockMode` and verify using `getClockMode` set and get ClockMode should be equal</li></ul>|<ul></li><li>IAVClockController - setClockMode, getClockMode</li></ul>|
|Set and Get Playback Rate|<ul><li>Check and verify playback state after open it should be set as `1.0`</li><li>set supported playback rate like (0.0 , 1.0 and 2.0) using `setPlaybackRate` and verify using `getPlaybackRate` set and get Playback Rate should be equal</li><li>`start` opened AV Clock instance verify the state using `getState` or `onStateChanged` and AV Clock instance state should be in `STARTED`</li><li>Again set playback rate using using `setPlaybackRate` and verify using `getPlaybackRate` set and get Playback Rate should be equal</li><li>Stop AV clock instance using `stop` and check the state using `getState` or `onStateChanged` should be in `READY` state<></ul>|<ul><li>IAVClockController - start, setPlaybackRate, getPlaybackRate, stop</li></ul>|

## L3 Test Specification
Key Notes:
Pre-Requirements and Post-Requirements apply to all L3 tests.

|Requirement|Description|HAL APIs|
|-----------|-----------|--------|
|Pre-Requirements|<ul><li>Read supported AV Clock Id's using `IAVClockManager` class</li><li>Loop through supported AV Clock Id's</li><li>Register event listeners using `registerEventListener`</li><li>`open` AV Clock instance and verify the state using `getState` or `onStateChanged` state should be in `READY`</li><li>set Audio Sink using setAudioSink verify using getAudioSink</li><li>set Video Sink using setVideoSink verify using getVideoSink</li></ul>|<ul><li>IAVClockManager - getAVClockIds, getAVClock</li><li>IAVClock - open, registerEventListener, getState</li><li>IAVClockController - setAudioSink, getAudioSink, setVideoSink, getVideoSink</li><li>IAVClockEventListener - onStateChanged</li></ul>|

|Requirement|Description|HAL APIs|
|-----------|-----------|--------|
|Post-Requirements|<ul><li>Stop and Close the AV Clock instance using `stop`,`close` check state should be in `CLOSED` using `getState` or `onStateChanged`</li><li>Unregister event listeners using `unregisterEventListener`</li></ul>|<ul><li>IAVClockController - stop</li><li>IAVClock  - close, getState, unregisterEventListener</li><li>IAVClockEventListener - onStateChanged</li></ul>|


|Test|Description|HAL APIs|
|----|-----------|--------|
|Clock Mode Auto (Video stream with Audio)|<ul><li>Check and verify the clock mode should be in `AUTO` by default using `getClockMode`</li><li>Start playback</li><li>`start` opened AV Clock instance verify the state using `getState` or `onStateChanged` and AV Clock instance state should be in `STARTED`</li><li>Get current clock time using `getCurrentClockTime` and verify against `PTS` of AV Stream should be in range</li><li>check the playback video and audio should be properly played back</li><li>Get current clock time  again after playback complete using `getCurrentClockTime` and verify against `PTS` of AV Stream</li></ul>|<ul><li>IAVClock - getState</li><li>IAVClockController - start, getClockMode, getCurrentClockTime</li><li>IAVClockEventListener - onStateChanged</li></ul>|
|Clock Mode Auto (Video stream with no Audio)|<ul><li>Check and verify the clock mode should be in `AUTO` by default using `getClockMode`</li><li>Start playback</li><li>`start` opened AV Clock instance verify the state using `getState` or `onStateChanged` and AV Clock instance state should be in `STARTED`</li><li>Get current clock time using `getCurrentClockTime` and verify against `PTS` of Video Stream should be in range</li><li>Check the playback video should be properly played with no audio</li><li>Get current clock time  again after playback complete using `getCurrentClockTime` and verify against `PTS` of Video Stream</li></ul>|<ul><li>IAVClock - getState</li><li>IAVClockController - start, getCurrentClockTime, getClockMode</li><li>IAVClockEventListener - onStateChanged</li></ul>|
|Clock Mode Auto Unsetting AudioSink (AV stream)|<ul><li>Check and verify the clock mode should be in `AUTO` by default using `getClockMode`</li><li>Start playback</li><li>`start` opened AV Clock instance verify the state using `getState` or `onStateChanged` and AV Clock instance state should be in `STARTED`</li><li>Get current clock time using `getCurrentClockTime` and verify against `PTS` of AV Stream should be in range</li><li>Verify audio and video should be properly played</li><li>Unset the audio sink to undefined with `IAudioSink.Id.UNDEFINED` while playback using `setAudioSink` and verify video is being played with no audio</li><li>Get current clock time using `getCurrentClockTime` and verify against `PTS` of Video Stream should be in range</li><li>Get current clock time  again after playback complete using `getCurrentClockTime` and verify against `PTS` of Video Stream</li></ul>|<ul><li>IAVClock - getState</li><li>IAVClockController - start, setAudioSink, getClockMode,getCurrentClockTime </li><li>IAVClockEventListener - onStateChanged</li></ul>|
|Clock Mode Auto Unsetting VideoSink (AV stream)|<ul><li>Check and verify the clock mode should be in `AUTO` by default using `getClockMode`</li><li>Start playback</li><li>`start` opened AV Clock instance verify the state using `getState` or `onStateChanged` and AV Clock instance state should be in `STARTED`</li><li>Get current clock time using `getCurrentClockTime` and verify against `PTS` of AV Stream should be in range</li><li>Verify audio and video should be properly played</li><li>Unset the video sink to undefined with `IVideoSink.Id.UNDEFINED` while playback using `setVideoSink` and verify audio is being played with no video</li><li>Get current clock time using `getCurrentClockTime` and verify against `PTS` of Audio Stream should be in range</li><li>Get current clock time  again after playback complete using `getCurrentClockTime` and verify against `PTS` of Audio Stream</li></ul>|<ul><li>IAVClock - getState</li><li>IAVClockController - start, setVideoSink, getClockMode, getCurrentClockTime</li><li>IAVClockEventListener - onStateChanged</li></ul>|
|Clock Mode Audio Master (AVStream)|<ul><li>set AV Clock instance to Audio Master using `setClockMode` and verify the same using `getClockMode`</li><li>Start play back</li><li>`start` opened AV Clock instance verify the state using `getState` or `onStateChanged` and AV Clock instance state should be in `STARTED`</li><li>Get current clock time using `getCurrentClockTime` and verify against `PTS` of Audio Stream should be in range</li><li>Verify audio should be properly played</li><li>Get current clock time  again after playback complete using `getCurrentClockTime` and verify against `PTS` of Audio Stream</li></ul>|<ul><li>IAVClock - getState</li><li>IAVClockController - start, setClockMode, getClockMode, getCurrentClockTime</li><li>IAVClockEventListener - onStateChanged</li></ul>|
|Clock Mode Video Master (AVStream)|<ul><li>set AV Clock instance to Video Master using `setClockMode` verify the same using `getClockMode`</li><li>Start play back</li><li>`start` opened AV Clock instance verify the state using `getState` or `onStateChanged` and AV Clock instance state should be in `STARTED`</li><li>Get current clock time using `getCurrentClockTime` and verify against `PTS` of Video Stream should be in range</li><li>Check the playback only video is being played , Audio should not be played</li><li>Get current clock time  again after playback complete using `getCurrentClockTime` and verify against `PTS` of Video Stream</li></ul>|<ul><li>IAVClock - getState</li><li>IAVClockController - start, setClockMode, getClockMode, getCurrentClockTime </li><li>IAVClockEventListener - onStateChanged</li></ul>|
|Clock Mode PCR (PCR Sample playback)|<ul><li>set AV Clock instance to `PCR` using `setClockMode` and verify the same using `getClockMode`</li><li>Start play back</li><li>`start` opened AV Clock instance verify the state using `getState` or `onStateChanged` and AV Clock instance state should be in `STARTED`</li><li>Get the `pcrTimeNs	` and `sampleTimestampNs`</li><li>While playback continues notify PCR sample using `notifyPCRSample` and get current clock time using `getCurrentClockTime` verify should be in range of `STC`</li><li>Verify current time clock expected to play at play rate `1.0`</li></ul>|<ul><li>IAVClock  - getState </li><li>IAVClockController - start, setClockMode, notifyPCRSample, getCurrentClockTime, getClockMode</li><li>IAVClockEventListener - onStateChanged</li></ul>|
|playback with Supplementary AudioSink (AV Stream)|<ul><li>Check the `ClockMode` should not be in `PCR` using `getClockMode`</li><li>set supplementary audio sink using `setSupplementaryAudioSink` verify using `getSupplementaryAudioSink`</li><li>Start play back</li><li>`start` opened AV Clock instance verify the state using `getState` or `onStateChanged` and AV Clock instance state should be in `STARTED`</li><li>While playback continues verify audio , video properly played</li><li>Get current clock time using `getCurrentClockTime` and verify against `PTS` of Audio Stream should be in range</li><li>Get current clock time  again after playback complete using `getCurrentClockTime` and verify against `PTS` of Audio Stream</li></ul>|<ul><li>IAVClock  - getState</li><li>IAVClockController - start, setClockMode, getClockMode, getCurrentClockTime, setSupplementaryAudioSink, getSupplementaryAudioSink</li><li>IAVClockEventListener - onStateChanged</li></ul>|
|Set Play Back Rate to Pause (AV stream playback)|<ul><li>Check `ClockMode` should not be in PCR using `getClockMode`</li><li>Start play back</li><li>set play back rate to pause i.e(0.0) using `setPlaybackRate`</li><li>`start` opened AV Clock instance verify the state using `getState` or `onStateChanged` and AV Clock instance state should be in `STARTED`</li><li>verify playback rate using `getPlaybackRate` and should be in paused state i.e(0.0)</li></ul>|<ul><li>IAVClock  - getState</li><li>IAVClockController - start, setPlaybackRate,</li><li>IAVClockEventListener - onStateChanged</li></ul>|
|Set Play Back Rate to Normal play (AV Stream playback)|<ul><li>Check `ClockMode` should not be in PCR using `getClockMode`</li><li>Start play back</li><li>set play back rate to pause i.e(0.0) using `setPlaybackRate` and veriy the playback rate using `getPlaybackRate`</li><li>`start` opened AV Clock instance verify the state using `getState` or `onStateChanged` and AV Clock instance state should be in `STARTED`</li><li>While playback continues set play back rate to pause i.e(1.0) using `setPlaybackRate` and verify using `getPlaybackRate`and playback should be played with normal speed</li></ul>|<ul><li>IAVClock  - getState</li><li>IAVClockController - start, setPlaybackRate</li><li>IAVClockEventListener - onStateChanged</li></ul>|
|Set Play Back Rate to Fast Forward (AV Stream playback)|<ul><li>Check `ClockMode` should not be in PCR using `getClockMode`</li><li>Start play back</li><li>`start` opened AV Clock instance verify the state using `getState` or `onStateChanged` and AV Clock instance state should be in `STARTED`</li><li>While playback continues set play back rate  i.e(0.5) using `setPlaybackRate`, verify using `getPlaybackRate` and verify Video is playing at half speed and audio is heard at half speed</li><li>While playback continues set play back rate  i.e(2.0) using `setPlaybackRate`, verify using `getPlaybackRate` and verify Video and audio is playing in fast forward mode when compared to normal speed</li></ul>|<ul><li>IAVClock  - getState</li><li>IAVClockController - start, setPlaybackRate</li><li>IAVClockEventListener - onStateChanged</li></ul>|
|AV Re Sync Events Count (AV Stream playback)|<ul><li>Check `ClockMode` should not be in PCR using `getClockMode`</li><li>Start play back</li><li>`start` opened AV Clock instance verify the state using `getState` or `onStateChanged` and AV Clock instance state should be in `STARTED`</li><li>While playback continues get AV Resync count `METRIC_AV_RESYNC_COUNT` metrics using `getProperty`</li></ul>|<ul><li>IAVClock  - getState, getProperty </li><li>IAVClockController - start, setPlaybackRate, stop</li><li>IAVClockEventListener - onStateChanged</li></ul>|
|AV Sync Accuracy (AV Stream playback)|<ul><li>Check `ClockMode` should not be in PCR using `getClockMode`</li><li>Start play back</li><li>`start` opened AV Clock instance verify the state using `getState` or `onStateChanged` and AV Clock instance state should be in `STARTED`</li><li>While playback continues get AV sync accuracy using `METRIC_AV_SYNC_ACCURACY_MS` metrics using `getProperty`and it should not outside of the target tolerance of -20ms to +40ms</li></ul>|<ul><li>IAVClock  - getState, getProperty</li><li>IAVClockController - start</li><li>IAVClockEventListener - onStateChanged</li></ul>|

# AV Clock High Level Test Specification Document

## Table of Contents

- [Acronyms, Terms and Abbreviations](#acronyms-terms-and-abbreviations)
- [Introduction](#introduction)
- [Module Description](#module-description)
- [L2 Test Specification](#l2-test-specification)

## Acronyms, Terms and Abbreviations

- `HAL`    - Hardware Abstraction Layer
- `API`    - Application Programming Interface
- `L2`     - Level 2 Testing
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

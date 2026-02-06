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

/** @brief
* AVClock.cpp
*
* Implementation of AVClock service
*/

#include "AVClock.h"
#include <thread>
#include <android-base/logging.h>
namespace com {
    namespace rdk {
        namespace hal {
            namespace avclock {

                ::com::rdk::hal::avclock::IAVClock::Id g_supported_id[NUM_AV_CLOCK];

                AVClockManager::AVClockManager()
                {
                    if (NUM_AV_CLOCK <= 0) {
                        LOG(ERROR) << "NUM_AV_CLOCK is invalid: " << NUM_AV_CLOCK;
                    }
                    for (int32_t i = 0; i < NUM_AV_CLOCK; i++) {
                        g_supported_id[i].value = i;
                        LOG(DEBUG) << "Initialized g_supported_id[" << i << "] = " << g_supported_id[i].value;
                    }
                    printf("[SERVICE] AVClockManager service initialized\n");
                }

                AVClockManager::~AVClockManager()
                {
                    printf("[SERVICE] AVClockManager service destroyed\n");
                }

                android::binder::Status AVClockManager::getAVClockIds(  std::vector<IAVClock::Id>* _aidl_return  )
                {
                    for (int32_t i = 0;i < NUM_AV_CLOCK; i++)
                    {
                        _aidl_return->push_back(g_supported_id[i]);
                    }
                    return android::binder::Status::ok();
                }

                android::binder::Status AVClockManager::getAVClock(const IAVClock::Id& avClockId, android::sp<IAVClock>* _aidl_return)
                {
                    LOG(DEBUG) << "getAVClock called with ID=" << avClockId.value;
                    if (avClockId.value < 0 || avClockId.value >= NUM_AV_CLOCK) {
                        LOG(ERROR) << "Invalid ID: " << avClockId.value << " (must be 0 to " << (NUM_AV_CLOCK - 1) << ")";
                        *_aidl_return = nullptr;
                        return android::binder::Status::fromExceptionCode(android::binder::Status::EX_ILLEGAL_ARGUMENT);
                    }

                    LOG(DEBUG) << "Creating AVClock with ID=" << avClockId.value;
                    android::sp<IAVClock> avClock = new AVClock(avClockId);
                    *_aidl_return = avClock;
                    return android::binder::Status::ok();
                }

                AVClock::AVClock(IAVClock::Id id)
                {
                    this->mId = id;
                    this->setState(State::CLOSED);
                    this->mMetricAVSyncAccuracyMs = -1;
                    this->mMetricAVResyncCount = -1;

                    this->mEventListener = false;
                    this->mAVClockEventListener = nullptr;

                    printf("[SERVICE] AVClock service initialized\n");
                }

                AVClock::~AVClock()
                {
                    printf("[SERVICE] AVClock service destroyed\n");
                }

                State AVClock::getState()
                {
                    std::lock_guard<std::mutex> lock(this->mStateMutex);
                    State state = this->mAVClockState;
                    return state;
                }
                void AVClock::setState(State newState)
                {
                    std::lock_guard<std::mutex> lock(this->mStateMutex);
                    this->mAVClockState = newState;
                }

                android::binder::Status AVClock::getCapabilities(avclock::Capabilities* _aidl_return)
                {
                    LOG(DEBUG) << "getCapabilities called for AVClock ID=" << mId.value;
                    if (!_aidl_return) {
                        LOG(ERROR) << "getCapabilities: null _aidl_return";
                        return android::binder::Status::fromExceptionCode(android::binder::Status::EX_NULL_POINTER);
                    }

                    avclock::Capabilities capabilities;


                    *_aidl_return = capabilities;
                    LOG(DEBUG) << "getCapabilities returned audioSinkIds={0, 1}, videoSinkIds={0, 1}";
                    return android::binder::Status::ok();
                }

                android::binder::Status AVClock::getState(State* _aidl_return)
                {
                    *_aidl_return = this->getState();
                    return android::binder::Status::ok();
                }

                android::binder::Status AVClock::getProperty(avclock::Property property, std::optional<PropertyValue>* _aidl_return)
                {
                    LOG(DEBUG) << "getProperty called for property=" << static_cast<int32_t>(property);
                    if (!_aidl_return) {
                        LOG(ERROR) << "getProperty: null _aidl_return";
                        return android::binder::Status::fromExceptionCode(android::binder::Status::EX_NULL_POINTER);
                    }

                    com::rdk::hal::PropertyValue value;
                    switch (property) {
                        case avclock::Property::RESOURCE_ID:
                            value.value = com::rdk::hal::PropertyValue::Value::make<com::rdk::hal::PropertyValue::Value::intValue>(this->mId.value);
                            LOG(DEBUG) << "getProperty: RESOURCE_ID = " << this->mId.value;
                            break;
                        case avclock::Property::METRIC_AV_SYNC_ACCURACY_MS:
                            value.value = com::rdk::hal::PropertyValue::Value::make<com::rdk::hal::PropertyValue::Value::intValue>(this->mMetricAVSyncAccuracyMs);
                            LOG(DEBUG) << "getProperty: METRIC_AV_SYNC_ACCURACY_MS = " << this->mMetricAVSyncAccuracyMs;
                            break;
                        case avclock::Property::METRIC_AV_RESYNC_COUNT:
                            value.value = com::rdk::hal::PropertyValue::Value::make<com::rdk::hal::PropertyValue::Value::intValue>(this->mMetricAVResyncCount);
                            LOG(DEBUG) << "getProperty: METRIC_AV_RESYNC_COUNT = " << this->mMetricAVResyncCount;
                            break;
                        default:
                            LOG(ERROR) << "getProperty: invalid property=" << static_cast<int32_t>(property);
                            *_aidl_return = std::nullopt;
                            return android::binder::Status::fromExceptionCode(android::binder::Status::EX_ILLEGAL_ARGUMENT);
                    }

                    *_aidl_return = value;
                    return android::binder::Status::ok();
                }

                android::binder::Status AVClock::setProperty(avclock::Property property, const PropertyValue& propertyValue, bool* _aidl_return)
                {
                    if (!propertyValue.value)
                    {
                        *_aidl_return = false;
                        return android::binder::Status::fromExceptionCode(android::binder::Status::EX_ILLEGAL_ARGUMENT);
                    }

                    switch (property) {
                        case ::com::rdk::hal::avclock::Property::RESOURCE_ID:
                        case ::com::rdk::hal::avclock::Property::METRIC_AV_SYNC_ACCURACY_MS:
                        case ::com::rdk::hal::avclock::Property::METRIC_AV_RESYNC_COUNT:
                            *_aidl_return = false; // readonly properties
                            break; // valid property
                        default:
                            *_aidl_return = false;
                            return android::binder::Status::fromExceptionCode(android::binder::Status::EX_ILLEGAL_ARGUMENT);
                    }
                    return android::binder::Status::ok();
                }

                android::binder::Status AVClock::open(  const android::sp<avclock::IAVClockControllerListener>& avClockControllerListener, android::sp<avclock::IAVClockController>* _aidl_return  )
                {
                    if(avClockControllerListener == nullptr)
                    {
                        *_aidl_return = nullptr;
                        return android::binder::Status::fromExceptionCode(android::binder::Status::EX_ILLEGAL_ARGUMENT);
                    }
                    if(this->getState() != State::CLOSED)
                    {
                        *_aidl_return = nullptr;
                        return android::binder::Status::fromExceptionCode(android::binder::Status::EX_ILLEGAL_STATE);
                    }

                    std::this_thread::sleep_for(std::chrono::milliseconds(5));

                    if (this->mEventListener)
                    {
                        this->mAVClockEventListener->onStateChanged(this->getState(), State::OPENING);
                    }

                    this->setState(State::OPENING);

                    *_aidl_return = new AVClockController(this->mId, this);

                    std::this_thread::sleep_for(std::chrono::milliseconds(250));

                    if (this->mEventListener)
                    {
                        this->mAVClockEventListener->onStateChanged(this->getState(), State::READY);
                    }

                    this->setState(State::READY);
                    return android::binder::Status::ok();
                }

                android::binder::Status AVClock::close(const android::sp<avclock::IAVClockController>& avClockController, bool* _aidl_return)
                {
                    if (avClockController == nullptr)
                    {
                        *_aidl_return = false;
                        return android::binder::Status::fromExceptionCode(android::binder::Status::EX_ILLEGAL_ARGUMENT);
                    }

                    if (this->getState() != State::READY)
                    {
                        *_aidl_return = false;
                        return android::binder::Status::fromExceptionCode(android::binder::Status::EX_ILLEGAL_STATE);
                    }

                    /* To simulate State Transitions */
                    std::this_thread::sleep_for(std::chrono::milliseconds(5));

                    if (this->mAVClockEventListener)
                    {
                        this->mAVClockEventListener->onStateChanged(this->getState(), State::CLOSING);
                    }

                    this->setState(State::CLOSING);

                    std::this_thread::sleep_for(std::chrono::milliseconds(250));
                    *_aidl_return = true;

                    if (this->mAVClockEventListener)
                    {
                        this->mAVClockEventListener->onStateChanged(this->getState(), State::CLOSED);
                    }
                    this->setState(State::CLOSED);
                    return android::binder::Status::ok();
                }

                android::binder::Status AVClock::registerEventListener(const android::sp<avclock::IAVClockEventListener>& avClockEventListener, bool* _aidl_return)
                {
                    if (avClockEventListener == nullptr)
                    {
                        *_aidl_return = false;
                        return android::binder::Status::fromExceptionCode(android::binder::Status::EX_ILLEGAL_ARGUMENT);
                    }
                    if (this->mAVClockEventListener != nullptr)
                    {
                        *_aidl_return = false;
                        return android::binder::Status::ok();
                    }
                    *_aidl_return = true;
                    this->mAVClockEventListener = avClockEventListener;
                    this->mEventListener = true;
                    return android::binder::Status::ok();
                }

                android::binder::Status AVClock::unregisterEventListener(const android::sp<avclock::IAVClockEventListener>& avClockEventListener, bool* _aidl_return)
                {
                    if (avClockEventListener == nullptr)
                    {
                        *_aidl_return = false;
                        return android::binder::Status::fromExceptionCode(android::binder::Status::EX_ILLEGAL_ARGUMENT);
                    }
                    if (this->mAVClockEventListener == nullptr)
                    {
                        *_aidl_return = false;
                        return android::binder::Status::ok();
                    }
                    
                    // Compare the underlying IBinder objects with additional null safety
                    android::sp<android::IBinder> registeredBinder = IInterface::asBinder(this->mAVClockEventListener);
                    android::sp<android::IBinder> providedBinder = IInterface::asBinder(avClockEventListener);
                    
                    // Check if binders are valid
                    if (registeredBinder == nullptr || providedBinder == nullptr)
                    {
                        *_aidl_return = false;
                        return android::binder::Status::ok();
                    }
                    
                    if (registeredBinder.get() != providedBinder.get())
                    {
                        *_aidl_return = false;
                        return android::binder::Status::ok();
                    }
                    
                    *_aidl_return = true;
                    this->mAVClockEventListener = nullptr;
                    this->mEventListener = false;
                    return android::binder::Status::ok();
                }

                AVClockController::AVClockController(IAVClock::Id id, AVClock *parentClock)
                {
                    this->mId = id;
                    this->mParentClock = parentClock;

                    this->mClockMode = ClockMode::AUTO;
                    this->mPlayBackRate = 1.0;
                    this->mAudioSinkId.value = -1;
                    this->mSupplementaryAudioSinkId.value = -1;
                    this->mVideoSinkId.value = -1;

                    this->mPCRSamples = {{0, 0}};

                    this->mPlayBackPaused = false;
                    this->mPlayBackAudioMuted = false;
                    this->mPlayBackHoldVideoFrame = false;
                    this->mPlayBackAudioPitchCorrectionEnabled = false;

                    printf("[SERVICE] AVClockController service initialized\n");
                }

                AVClockController::~AVClockController()
                {
                    printf("[SERVICE] AVClockController service destroyed\n");
                }

                android::binder::Status AVClockController::start()
                {
                    if(this->mParentClock->getState() != State::READY)
                    {
                        return android::binder::Status::fromExceptionCode(android::binder::Status::EX_ILLEGAL_STATE);
                    }
                    /* To simulate State Transitions */
                    std::this_thread::sleep_for(std::chrono::milliseconds(5));

                    if (this->mParentClock->mEventListener)
                    {
                        this->mParentClock->mAVClockEventListener->onStateChanged(this->mParentClock->getState(), State::STARTING);
                    }
                    this->mParentClock->setState(State::STARTING);

                    std::this_thread::sleep_for(std::chrono::milliseconds(250));
                    if (this->mParentClock->mEventListener)
                    {
                        this->mParentClock->mAVClockEventListener->onStateChanged(this->mParentClock->getState(), State::STARTED);
                    }
                    this->mParentClock->setState(State::STARTED);
                    return android::binder::Status::ok();
                }

                android::binder::Status AVClockController::stop()
                {
                    if(this->mParentClock->getState() != State::STARTED)
                    {
                        return android::binder::Status::fromExceptionCode(android::binder::Status::EX_ILLEGAL_STATE);
                    }

                    /* To simulate State Transitions */
                    std::this_thread::sleep_for(std::chrono::milliseconds(5));

                    if (this->mParentClock->mEventListener)
                    {
                        this->mParentClock->mAVClockEventListener->onStateChanged(this->mParentClock->getState(), State::STOPPING);
                    }
                    this->mParentClock->setState(State::STOPPING);

                    std::this_thread::sleep_for(std::chrono::milliseconds(250));

                    if (this->mParentClock->mEventListener)
                    {
                        this->mParentClock->mAVClockEventListener->onStateChanged(this->mParentClock->getState(), State::READY);
                    }
                    this->mParentClock->setState(State::READY);
                    return android::binder::Status::ok();
                }

                android::binder::Status AVClockController::setAudioSink(  const com::rdk::hal::audiosink::IAudioSink::Id& audioSinkId, bool* _aidl_return  )
                {
                    *_aidl_return = false;
                    if(this->mParentClock->getState() != State::READY && this->mParentClock->getState() != State::STARTED)
                    {
                        *_aidl_return = false;
                        return android::binder::Status::fromExceptionCode(android::binder::Status::EX_ILLEGAL_STATE);
                    }

                    if(audioSinkId.value >=-1 && audioSinkId.value <= NUM_AUDIO_SINK)
                    {
                        this->mAudioSinkId = audioSinkId;
                        *_aidl_return = true;
                    }
                    return android::binder::Status::ok();
                }

                android::binder::Status AVClockController::getAudioSink(  com::rdk::hal::audiosink::IAudioSink::Id* _aidl_return  )
                {
                    if(this->mParentClock->getState() != State::READY && this->mParentClock->getState() != State::STARTED)
                    {
                        return android::binder::Status::fromExceptionCode(android::binder::Status::EX_ILLEGAL_STATE);
                    }
                    *_aidl_return = this->mAudioSinkId;
                    return android::binder::Status::ok();
                }

                android::binder::Status AVClockController::setSupplementaryAudioSink(  const com::rdk::hal::audiosink::IAudioSink::Id& supplementaryAudioSinkId, bool* _aidl_return  )
                {
                    *_aidl_return = false;
                    if(this->mParentClock->getState() != State::READY && this->mParentClock->getState() != State::STARTED)
                    {
                        *_aidl_return = false;
                        return android::binder::Status::fromExceptionCode(android::binder::Status::EX_ILLEGAL_STATE);
                    }

                    if(supplementaryAudioSinkId.value >=-1 && supplementaryAudioSinkId.value <= NUM_AUDIO_SINK_SUPPLEMENTARY)
                    {
                        this->mSupplementaryAudioSinkId = supplementaryAudioSinkId;
                        *_aidl_return = true;
                    }
                    return android::binder::Status::ok();
                }

                android::binder::Status AVClockController::getSupplementaryAudioSink(  com::rdk::hal::audiosink::IAudioSink::Id* _aidl_return  )
                {
                    if(this->mParentClock->getState() != State::READY && this->mParentClock->getState() != State::STARTED)
                    {
                        return android::binder::Status::fromExceptionCode(android::binder::Status::EX_ILLEGAL_STATE);
                    }
                    *_aidl_return = this->mSupplementaryAudioSinkId;
                    return android::binder::Status::ok();
                }

                android::binder::Status AVClockController::setVideoSink(  const com::rdk::hal::videosink::IVideoSink::Id& videoSinkId, bool* _aidl_return  )
                {
                    *_aidl_return = false;
                    if(this->mParentClock->getState() != State::READY && this->mParentClock->getState() != State::STARTED)
                    {
                        return android::binder::Status::fromExceptionCode(android::binder::Status::EX_ILLEGAL_STATE);
                    }

                    if(videoSinkId.value >=-1 && videoSinkId.value <= NUM_VIDEO_SINK)
                    {
                        this->mVideoSinkId = videoSinkId;
                        *_aidl_return = true;
                    }
                    return android::binder::Status::ok();
                }

                android::binder::Status AVClockController::getVideoSink(  com::rdk::hal::videosink::IVideoSink::Id* _aidl_return  )
                {
                    if(this->mParentClock->getState() != State::READY && this->mParentClock->getState() != State::STARTED)
                    {
                        return android::binder::Status::fromExceptionCode(android::binder::Status::EX_ILLEGAL_STATE);
                    }
                    *_aidl_return = this->mVideoSinkId;
                    return android::binder::Status::ok();
                }

                android::binder::Status AVClockController::setClockMode(  ClockMode clockMode, bool* _aidl_return  )
                {
                    if(this->mParentClock->getState() != State::READY)
                    {
                        return android::binder::Status::fromExceptionCode(android::binder::Status::EX_ILLEGAL_STATE);
                    }

                    switch (clockMode) {
                        case ::com::rdk::hal::avclock::ClockMode::AUTO:
                        case ::com::rdk::hal::avclock::ClockMode::PCR:
                        case ::com::rdk::hal::avclock::ClockMode::AUDIO_MASTER:
                        case ::com::rdk::hal::avclock::ClockMode::VIDEO_MASTER:
                            break; // valid clockMode
                        default:
                            *_aidl_return = false;
                            return android::binder::Status::ok();
                    }
                    this->mClockMode = clockMode;
                    *_aidl_return = true;
                    return android::binder::Status::ok();
                }

                android::binder::Status AVClockController::getClockMode(  ClockMode* _aidl_return  )
                {
                    if(this->mParentClock->getState() != State::READY && this->mParentClock->getState() != State::STARTED)
                    {
                        return android::binder::Status::fromExceptionCode(android::binder::Status::EX_ILLEGAL_STATE);
                    }
                    *_aidl_return = this->mClockMode;
                    return android::binder::Status::ok();
                }

                android::binder::Status AVClockController::notifyPCRSample(  int64_t pcrTimeNs, int64_t sampleTimestampNs, bool* _aidl_return  )
                {
                    *_aidl_return = false;
                    if(this->mParentClock->getState() != State::STARTED || this->mClockMode != ClockMode::PCR)
                    {
                        return android::binder::Status::fromExceptionCode(android::binder::Status::EX_ILLEGAL_STATE);
                    }

                    if(pcrTimeNs < 0 ||sampleTimestampNs < 0){
                        *_aidl_return = false;
                        return android::binder::Status::ok();
                    }

                    this->mPCRSamples = { PCRSample{pcrTimeNs, sampleTimestampNs} };
                    *_aidl_return = true;
                    return android::binder::Status::ok();
                }

                android::binder::Status AVClockController::getCurrentClockTime(  ClockTime* _aidl_return  )
                {
                    if(this->mParentClock->getState() != State::STARTED)
                    {
                        return android::binder::Status::fromExceptionCode(android::binder::Status::EX_ILLEGAL_STATE);
                    }

                    struct timespec ts;
                    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
                    int64_t nowNs = static_cast<int64_t>(ts.tv_sec) * 1'000'000'000LL + ts.tv_nsec;

                    int64_t clockTimeNs = 0;
                    if (this->mClockMode == ClockMode::PCR) {
                        // Simulate STC by adding PCR base + elapsed time
                        const PCRSample& latestSample = mPCRSamples.back();
                        clockTimeNs = latestSample.pcrTimeNs + (nowNs - latestSample.sampleTimestampNs);
                    } else {
                        // System-based clock time = now
                        clockTimeNs = nowNs;
                    }

                    _aidl_return->clockTimeNs = clockTimeNs;
                    _aidl_return->sampleTimestampNs = nowNs;

                    return android::binder::Status::ok();
                }

                android::binder::Status AVClockController::setPlaybackRate(  double rate, bool* _aidl_return  )
                {
                    *_aidl_return = false;
                    if(this->mParentClock->getState() != State::READY && this->mParentClock->getState() != State::STARTED)
                    {
                        return android::binder::Status::fromExceptionCode(android::binder::Status::EX_ILLEGAL_STATE);
                    }

                    if(this->mClockMode == ClockMode::PCR)
                    {
                        return android::binder::Status::fromExceptionCode(android::binder::Status::EX_ILLEGAL_STATE);
                    }

                    if(rate < 0.0 || rate > 2.0)
                    {
                        *_aidl_return = false;
                        return android::binder::Status::ok();
                    }
                    this->mPlayBackRate = rate;
                    *_aidl_return = true;

                    if(rate == 0.0)
                    {
                        //paused state
                        this->mPlayBackPaused = true;
                        this->mPlayBackHoldVideoFrame = true;

                    }
                    if(rate < 0.5)
                        this->mPlayBackAudioMuted = true;
                    else
                        this->mPlayBackAudioPitchCorrectionEnabled = true;

                    return android::binder::Status::ok();
                }

                android::binder::Status AVClockController::getPlaybackRate(  double* _aidl_return  )
                {
                    if(this->mParentClock->getState() != State::READY && this->mParentClock->getState() != State::STARTED)
                    {
                        return android::binder::Status::fromExceptionCode(android::binder::Status::EX_ILLEGAL_STATE);
                    }

                    if(this->mClockMode == ClockMode::PCR)
                    {
                        return android::binder::Status::fromExceptionCode(android::binder::Status::EX_ILLEGAL_STATE);
                    }
                    *_aidl_return = this->mPlayBackRate;
                    return android::binder::Status::ok();
                }
            }
        }
    }
}

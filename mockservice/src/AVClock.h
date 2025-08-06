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
* AVClock.h
*
* Header file for the AVClock service implementation
*/

#ifndef AVCLOCK_H
#define AVCLOCK_H

#include <com/rdk/hal/avclock/BnAVClockManager.h>
#include <com/rdk/hal/avclock/IAVClockManager.h>
#include <com/rdk/hal/avclock/BnAVClock.h>
#include <com/rdk/hal/avclock/BnAVClockController.h>
#include <binder/BinderService.h>
#include <optional>
#include <string>
#include <utils/StrongPointer.h>
#include <vector>
#include <time.h>
#include <android/log.h>


namespace com {
    namespace rdk {
        namespace hal {
            namespace avclock {

#define NUM_AV_CLOCK 2
#define NUM_AUDIO_SINK 1
#define NUM_AUDIO_SINK_SUPPLEMENTARY 1
#define NUM_VIDEO_SINK 1


                class AVClockManager :public android::BinderService<AVClockManager>,
                                      public BnAVClockManager {
                public:
                    AVClockManager();
                    virtual ~AVClockManager();

                    static char const* getServiceName() { return IAVClockManager::serviceName().c_str(); }

                    android::binder::Status getAVClockIds(::std::vector<::com::rdk::hal::avclock::IAVClock::Id>* _aidl_return) override;
                    android::binder::Status getAVClock(const ::com::rdk::hal::avclock::IAVClock::Id& avClockId, ::android::sp<::com::rdk::hal::avclock::IAVClock>* _aidl_return) override;
                };

                class AVClock :public android::BinderService<AVClock>,
                               public BnAVClock {
                private:
                    IAVClock::Id mId;
                    State mAVClockState;
                    std::mutex mStateMutex;
                    int32_t mMetricAVSyncAccuracyMs;
                    int32_t mMetricAVResyncCount;

                public:
                    AVClock(IAVClock::Id id);
                    virtual ~AVClock();

                    android::binder::Status getCapabilities(::com::rdk::hal::avclock::Capabilities* _aidl_return) override;
                    android::binder::Status getState(::com::rdk::hal::State* _aidl_return) override;
                    android::binder::Status getProperty(::com::rdk::hal::avclock::Property property, ::std::optional<::com::rdk::hal::PropertyValue>* _aidl_return) override;
                    android::binder::Status setProperty(::com::rdk::hal::avclock::Property property, const ::com::rdk::hal::PropertyValue& propertyValue, bool* _aidl_return) override;
                    android::binder::Status open(const ::android::sp<::com::rdk::hal::avclock::IAVClockControllerListener>& avClockControllerListener, ::android::sp<::com::rdk::hal::avclock::IAVClockController>* _aidl_return) override;
                    android::binder::Status close(const ::android::sp<::com::rdk::hal::avclock::IAVClockController>& avClockController, bool* _aidl_return) override;
                    android::binder::Status registerEventListener(const ::android::sp<::com::rdk::hal::avclock::IAVClockEventListener>& avClockEventListener, bool* _aidl_return) override;
                    android::binder::Status unregisterEventListener(const ::android::sp<::com::rdk::hal::avclock::IAVClockEventListener>& avClockEventListener, bool* _aidl_return) override;

                    void setState(State state);
                    State getState();

                    bool mEventListener;
                    android::sp<avclock::IAVClockEventListener> mAVClockEventListener;
                };

                class AVClockController :public android::BinderService<AVClockController>,
                                         public BnAVClockController {
                private:
                    IAVClock::Id mId;
                    AVClock *mParentClock;
                    ClockMode mClockMode;
                    double mPlayBackRate;
                    ::com::rdk::hal::audiosink::IAudioSink::Id mAudioSinkId;
                    ::com::rdk::hal::audiosink::IAudioSink::Id mSupplementaryAudioSinkId;
                    ::com::rdk::hal::videosink::IVideoSink::Id mVideoSinkId;

                    struct PCRSample {
                        int64_t pcrTimeNs;
                        int64_t sampleTimestampNs;
                    };
                    std::vector<PCRSample> mPCRSamples;

                    bool mPlayBackPaused;
                    bool mPlayBackAudioMuted;
                    bool mPlayBackHoldVideoFrame;
                    bool mPlayBackAudioPitchCorrectionEnabled;

                public:
                    AVClockController(IAVClock::Id id, AVClock *parentClock);
                    virtual ~AVClockController();

                    android::binder::Status start() override;
                    android::binder::Status stop() override;
                    android::binder::Status setAudioSink(const ::com::rdk::hal::audiosink::IAudioSink::Id& audioSinkId, bool* _aidl_return) override;
                    android::binder::Status getAudioSink(::com::rdk::hal::audiosink::IAudioSink::Id* _aidl_return) override;
                    android::binder::Status setSupplementaryAudioSink(const ::com::rdk::hal::audiosink::IAudioSink::Id& supplementaryAudioSinkId, bool* _aidl_return) override;
                    android::binder::Status getSupplementaryAudioSink(::com::rdk::hal::audiosink::IAudioSink::Id* _aidl_return) override;
                    android::binder::Status setVideoSink(const ::com::rdk::hal::videosink::IVideoSink::Id& videoSinkId, bool* _aidl_return) override;
                    android::binder::Status getVideoSink(::com::rdk::hal::videosink::IVideoSink::Id* _aidl_return) override;
                    android::binder::Status setClockMode(::com::rdk::hal::avclock::ClockMode clockMode, bool* _aidl_return) override;
                    android::binder::Status getClockMode(::com::rdk::hal::avclock::ClockMode* _aidl_return) override;
                    android::binder::Status notifyPCRSample(int64_t pcrTimeNs, int64_t sampleTimestampNs, bool* _aidl_return) override;
                    android::binder::Status getCurrentClockTime(::com::rdk::hal::avclock::ClockTime* _aidl_return) override;
                    android::binder::Status setPlaybackRate(double rate, bool* _aidl_return) override;
                    android::binder::Status getPlaybackRate(double* _aidl_return) override;
                };
            }
        }
    }
}

#endif AVCLOCK_H



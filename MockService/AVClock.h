/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
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

namespace com {
    namespace rdk {
        namespace hal {
            namespace avclock {

                class AVClockManager :public android::BinderService<AVClockManager>,
                                      public BnAVClockManager {
                public:
                    AVClockManager();
                    virtual ~AVClockManager();

                    static char const* getServiceName() { return IAVClockManager::serviceName().c_str(); }

                    android::binder::Status getAVClockIds( std::vector<IAVClock::Id>* _aidl_return ) override;
                    android::binder::Status getAVClock( const IAVClock::Id& avClockId, android::sp<IAVClock>* _aidl_return ) override;
                };

                class AVClock :public android::BinderService<AVClock>,
                               public BnAVClock {
                public:
                    AVClock();
                    virtual ~AVClock();

                    android::binder::Status getCapabilities( avclock::Capabilities* _aidl_return ) override;
                    android::binder::Status getState( State* _aidl_return ) override;
                    android::binder::Status getProperty( avclock::Property property, std::optional<PropertyValue>* _aidl_return ) override;
                    android::binder::Status setProperty( avclock::Property property, const PropertyValue& propertyValue, bool* _aidl_return ) override;
                    android::binder::Status open( const android::sp<avclock::IAVClockControllerListener>& avClockControllerListener, android::sp<avclock::IAVClockController>* _aidl_return ) override;
                    android::binder::Status close( const android::sp<avclock::IAVClockController>& avClockController, bool* _aidl_return ) override;
                    android::binder::Status registerEventListener( const android::sp<avclock::IAVClockEventListener>& avClockEventListener, bool* _aidl_return ) override;
                    android::binder::Status unregisterEventListener( const android::sp<avclock::IAVClockEventListener>& avClockEventListener, bool* _aidl_return ) override;
                };

                class AVClockController :public android::BinderService<AVClockController>,
                                         public BnAVClockController {
                public:
                    AVClockController();
                    virtual ~AVClockController();

                    android::binder::Status start(  ) override;
                    android::binder::Status stop(  ) override;
                    android::binder::Status setAudioSink( const com::rdk::hal::audiosink::IAudioSink::Id& audioSinkId, bool* _aidl_return ) override;
                    android::binder::Status getAudioSink( com::rdk::hal::audiosink::IAudioSink::Id* _aidl_return ) override;
                    android::binder::Status setSupplementaryAudioSink( const com::rdk::hal::audiosink::IAudioSink::Id& supplementaryAudioSinkId, bool* _aidl_return ) override;
                    android::binder::Status getSupplementaryAudioSink( com::rdk::hal::audiosink::IAudioSink::Id* _aidl_return ) override;
                    android::binder::Status setVideoSink( const com::rdk::hal::videosink::IVideoSink::Id& videoSinkId, bool* _aidl_return ) override;
                    android::binder::Status getVideoSink( com::rdk::hal::videosink::IVideoSink::Id* _aidl_return ) override;
                    android::binder::Status setClockMode( ClockMode clockMode, bool* _aidl_return ) override;
                    android::binder::Status getClockMode( ClockMode* _aidl_return ) override;
                    android::binder::Status notifyPCRSample( int64_t pcrTimeNs, int64_t sampleTimestampNs, bool* _aidl_return ) override;
                    android::binder::Status getCurrentClockTime( ClockTime* _aidl_return ) override;
                    android::binder::Status setPlaybackRate( double rate, bool* _aidl_return ) override;
                    android::binder::Status getPlaybackRate( double* _aidl_return ) override;
                };
            }
        }
    }
}

#endif AVCLOCK_H



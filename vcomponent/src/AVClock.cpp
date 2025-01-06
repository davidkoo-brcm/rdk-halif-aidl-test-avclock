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

/** @brief
* AVClock.cpp
*
* Implementation of AVClock service
*/

#include "AVClock.h"

namespace com {
    namespace rdk {
        namespace hal {
            namespace avclock {

                AVClockManager::AVClockManager()
                {
                    printf("AVClockManager service initialized\n");
                }

                AVClockManager::~AVClockManager()
                {
                    printf("AVClockManager service destroyed\n");
                }

                android::binder::Status AVClockManager::getAVClockIds(  std::vector<IAVClock::Id>* _aidl_return  )
                {
                    IAVClock::Id id1;
                    id1.value = 0; // Set value
                    IAVClock::Id id2;
                    id2.value = 1; // Set value
                    _aidl_return->push_back(id1);
                    _aidl_return->push_back(id2);
                    return android::binder::Status::ok();
                }

                android::binder::Status AVClockManager::getAVClock(  const IAVClock::Id& avClockId, android::sp<IAVClock>* _aidl_return  )
                {
                    android::sp<IAVClock> avClock = new AVClock();
                    *_aidl_return = avClock;
                    return android::binder::Status::ok();
                }

                AVClock::AVClock() 
                {
                    printf("AVClock service initialized\n");
                }

                AVClock::~AVClock() 
                {
                    printf("AVClock service destroyed\n");
                }

                android::binder::Status AVClock::getCapabilities(  avclock::Capabilities* _aidl_return  ) 
                {
                    return android::binder::Status::ok();
                }
                
                android::binder::Status AVClock::getState(  State* _aidl_return  ) 
                {
                    *_aidl_return = State::CLOSED;
                    return android::binder::Status::ok();
                }
                
                android::binder::Status AVClock::getProperty(  avclock::Property property, std::optional<PropertyValue>* _aidl_return  ) 
                {
                    com::rdk::hal::PropertyValue value;
                    *_aidl_return = value;
                    return android::binder::Status::ok();
                }
                
                android::binder::Status AVClock::setProperty(  avclock::Property property, const PropertyValue& propertyValue, bool* _aidl_return  ) 
                {
                    return android::binder::Status::ok();
                }
                
                android::binder::Status AVClock::open(  const android::sp<avclock::IAVClockControllerListener>& avClockControllerListener, android::sp<avclock::IAVClockController>* _aidl_return  ) 
                {
                    *_aidl_return = new AVClockController();
                    return android::binder::Status::ok();
                }
                
                android::binder::Status AVClock::close(  const android::sp<avclock::IAVClockController>& avClockController, bool* _aidl_return  ) 
                {
                    return android::binder::Status::ok();
                }
                
                android::binder::Status AVClock::registerEventListener(  const android::sp<avclock::IAVClockEventListener>& avClockEventListener, bool* _aidl_return  ) 
                {
                    return android::binder::Status::ok();
                }
                
                android::binder::Status AVClock::unregisterEventListener(  const android::sp<avclock::IAVClockEventListener>& avClockEventListener, bool* _aidl_return  ) 
                {
                    return android::binder::Status::ok();
                }

                AVClockController::AVClockController()
                {
                    printf("AVClockController service initialized\n");
                }

                AVClockController::~AVClockController()
                {
                    printf("AVClockController service destroyed\n");
                }

                android::binder::Status AVClockController::start(    )
                {
                    return android::binder::Status::ok();
                }

                android::binder::Status AVClockController::stop(    )
                {
                    return android::binder::Status::ok();
                }

                android::binder::Status AVClockController::setAudioSink(  const com::rdk::hal::audiosink::IAudioSink::Id& audioSinkId, bool* _aidl_return  )
                {
                    return android::binder::Status::ok();
                }

                android::binder::Status AVClockController::getAudioSink(  com::rdk::hal::audiosink::IAudioSink::Id* _aidl_return  )
                {
                    return android::binder::Status::ok();
                }

                android::binder::Status AVClockController::setSupplementaryAudioSink(  const com::rdk::hal::audiosink::IAudioSink::Id& supplementaryAudioSinkId, bool* _aidl_return  ) 
                {
                    return android::binder::Status::ok();
                }
                
                android::binder::Status AVClockController::getSupplementaryAudioSink(  com::rdk::hal::audiosink::IAudioSink::Id* _aidl_return  ) 
                {
                    return android::binder::Status::ok();
                }
                
                android::binder::Status AVClockController::setVideoSink(  const com::rdk::hal::videosink::IVideoSink::Id& videoSinkId, bool* _aidl_return  ) 
                {
                    return android::binder::Status::ok();
                }
                
                android::binder::Status AVClockController::getVideoSink(  com::rdk::hal::videosink::IVideoSink::Id* _aidl_return  ) 
                {
                    return android::binder::Status::ok();
                }
                
                android::binder::Status AVClockController::setClockMode(  ClockMode clockMode, bool* _aidl_return  ) 
                {
                    return android::binder::Status::ok();
                }
                
                android::binder::Status AVClockController::getClockMode(  ClockMode* _aidl_return  ) 
                {
                    return android::binder::Status::ok();
                }
                
                android::binder::Status AVClockController::notifyPCRSample(  int64_t pcrTimeNs, int64_t sampleTimestampNs, bool* _aidl_return  ) 
                {
                    return android::binder::Status::ok();
                }
                
                android::binder::Status AVClockController::getCurrentClockTime(  ClockTime* _aidl_return  ) 
                {
                    return android::binder::Status::ok();
                }
                
                android::binder::Status AVClockController::setPlaybackRate(  double rate, bool* _aidl_return  ) 
                {
                    return android::binder::Status::ok();
                }
                
                android::binder::Status AVClockController::getPlaybackRate(  double* _aidl_return  ) 
                {
                    return android::binder::Status::ok();
                }
            }
        }
    }
}

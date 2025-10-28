/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2016 RDK Management
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

/**
 * @addtogroup HPK Hardware Porting Kit
 * @{
 *
 */

/**
 * @addtogroup AVClock AVClock Module
 * @{
 *
 */

/**
 * @defgroup AVCLOCK_HALTEST AVClock HAL Tests
 * @{
 *
 */

/**
 * @defgroup AVCLOCK_HALTEST_L3 AVClock HAL Tests L3 File
 * @{
 * @parblock
 *
 * ### L3 Test Cases for AVClock HAL :
 *
 * This module includes Level 3 functional tests (success and failure scenarios).
 * This is to ensure that the API meets the operational requirements of the module across all vendors.
 *
 * **Pre-Conditions:** @n
 * **Dependencies:** None@n
 *
 * @endparblock
 *
 */

/**
 * @file vts_avclock_l3_tests.cpp
 *
 */

#include <ut_kvp_profile.h>
#include <ut.h>
#include <ut_log.h>
#include <ut_control_plane.h>
#include <deque>
#include <binder/IServiceManager.h>
#include <binder/ProcessState.h>
#include <binder/IPCThreadState.h>
#include <binder/Status.h>
#include <utils/String16.h>
#include <vector>
#include <optional>
#include <thread>
#include <chrono>
#include <mutex>
#include <condition_variable>
#include <algorithm>
#include <unordered_set>
#include <atomic>
#include <tuple>

#include <map>

#include "com/rdk/hal/avbuffer/IAVBuffer.h"
#include "com/rdk/hal/avbuffer/BnAVBufferSpaceListener.h"

#include "vts_audiosink.h"
#include "vts_audiodecoder.h"
#include "vts_videodecoder.h"
#include "vts_audiodecoder_parse_testconfig.h"

#include "vts_audiosink_helper_functions.h"
#include "vts_audiosink_parse_hfp.h"
#include "vts_avbuffer.h"

#include "vts_videosink_helper_functions.h"
#include "vts_videosink_parse_hfp.h"
#include "vts_videosink_parse_testconfig.h"

#include "vts_avclock_parse_hfp.h"
#include "vts_avclock.h"

#include "IAVBufferHelper.h"

static const char *gModuleName = "IAVClockManager";
static const char *gAudioDecoderModuleName = "IAudioDecoderManager";
static const char *gAudioSinkModuleName = "IAudioSinkManager";
static const char *gVideoDecoderModuleName = "IVideoDecoderManager";
static const char *gVideoSinkModuleName = "IVideoSinkManager";
static const char *gTestGroup = "L3";

std::atomic<bool> threadStarted(false);

//static IAVBufferHelper* gAVbufferHelper;

static sp<IAVBuffer> vts_avbuffer_getService()
{
    return
        android::interface_cast<IAVBuffer>(
            android::defaultServiceManager()->getService(
                android::String16(IAVBuffer::serviceName().c_str())
            )
        );
}

/* Test class registration with UT_Core */
UT_ADD_TEST_TO_GROUP(VTS_L3_AVClock, UT_TESTS_L3);

class AVBufferSpaceListener : public BnAVBufferSpaceListener
{
public:

    AVBufferSpaceListener() {};
    virtual ~AVBufferSpaceListener() = default;

    Status onSpaceAvailable() override
    {
        return Status::ok();
    }
};

class VTS_L3_AVClock : public UTCore
{
    public:
    VTS_L3_AVClock() : UTCore() {};

    ~VTS_L3_AVClock() override = default;

    public:
        std::vector<std::tuple<IAVClock::Id, sp<IAVClock>, sp<AVClockControllerListener>, sp<AVClockEventListener>>> mAVClockList;
        std::vector<tuple<IAudioDecoder::Id, sp<IAudioDecoder>, sp<AudioDecoderControllerListener>, sp<AudioDecoderEventListener>, audiodecoder::Capabilities *>> mAudioDecoderList;
        std::vector<tuple<IVideoDecoder::Id, sp<IVideoDecoder>, sp<VideoDecoderControllerListener>, sp<VideoDecoderEventListener>, videodecoder::Capabilities *>> mVideoDecoderList;
        std::vector<std::tuple<IAudioSink::Id, sp<IAudioSink>, sp<AudioSinkControllerListener>, sp<AudioSinkEventListener>>> mAudioSinkList;
        std::vector<tuple<IVideoSink::Id, sp<IVideoSink>, sp<VideoSinkControllerListener>, sp<VideoSinkEventListener>>> mVideoSinkList;
        sp<IAVBuffer> mAVBufferManager;
        IAVBufferHelper* mAVbufferHelper;

        void SetUp() override
       {
        std::vector<IAVClock::Id> avClockIds;
        std::vector<IAudioSink::Id> audioSinkIds;
        std::vector<IVideoSink::Id> videoSinkIds;
        std::vector<IAudioDecoder::Id> audioDecoderIds;
        std::vector<IVideoDecoder::Id> videoDecoderIds;

        if(gAVClockManagerService== NULL)
        {
            UT_FAIL_FATAL("AVClock Service not available");
        }

        if(gAudioDecoderManager== NULL)
        {
            UT_FAIL_FATAL("AudioDecoder Service not available");
        }

        if(gAudioSinkManager == NULL)
        {
            UT_FAIL_FATAL("AudioSink Service not available");
        }

        if(gVideoDecoderManager== NULL)
        {
            UT_FAIL_FATAL("VideoDecoder Service not available");
        }

        if(gVideoSinkManager == NULL)
        {
            UT_FAIL_FATAL("VideoSink Service not available");
        }

        mAVBufferManager = vts_avbuffer_getService();
        if(mAVBufferManager == NULL)
        {
            UT_FAIL_FATAL("AVBufferManager Service not available");
        }

        mAVbufferHelper = getAVBufferHelperInstance();

        /* Get the AVClock IDs */
        AVCLOCK_MANAGER_GETAVCLOCKIDS(avClockIds);

        /* Get the Audio Sink IDs */
        AUDIOSINK_MANAGER_GETAUDIOSINKIDS(audioSinkIds);

        /* Get the Audio decoder IDs */
        AUDIODECODER_MANAGER_GETAUDIODECODERIDS(audioDecoderIds);

        /* Get the Video decoder IDs */
        VIDEODECODER_MANAGER_GETVIDEODECODERIDS(videoDecoderIds);

        /* Get the Video Sink IDs */
        VIDEOSINK_MANAGER_GETVIDEOSINKIDS(videoSinkIds);

        android::ProcessState::self()->startThreadPool();

        for(const auto& _avClockId: avClockIds)
        {
            sp<IAVClock> avClock;

            sp<AVClockEventListener> eventListener = new AVClockEventListener(_avClockId.value);

            sp<AVClockControllerListener> controlListener = new AVClockControllerListener(_avClockId.value);

            /* Get AVClock handle */
            AVCLOCK_MANAGER_GETAVCLOCK_SUCCESS(_avClockId, avClock);

            /* Register events */
            AVCLOCK_REGISTER_EVENT_LISTENER_SUCCESS(avClock, eventListener);

            /* Get Capabilities of AVClock */
            //TODO: Uncomment when capabilities are implemented in the HAL
            //AVCLOCK_GETCAPABILITIES(avClock, capabilities);

            mAVClockList.push_back({_avClockId, avClock, controlListener, eventListener});
        }

        for(const auto& _audioSinkId: audioSinkIds)
        {
            sp<IAudioSink> audioSink;

            sp<AudioSinkEventListener> eventListener = new AudioSinkEventListener(_audioSinkId.value);

            sp<AudioSinkControllerListener> controlListener = new AudioSinkControllerListener(_audioSinkId.value);

            /* Get Audio decoder handle */
            AUDIOSINK_MANAGER_GETAUDIOSINK(_audioSinkId, audioSink);

            /* Register events */
            AUDIOSINK_REGISTER_EVENT_SUCCESS(audioSink, eventListener);

            /* Get Capabilities of Audio Sink */
            //TODO: Uncomment when capabilities are implemented in the HAL
            //AUDIOSINK_GETCAPABILITIES(audioSink, capabilities);

            mAudioSinkList.push_back({_audioSinkId, audioSink, controlListener, eventListener});
        }

        for(const auto& _audioDecoderId: audioDecoderIds)
        {
            sp<IAudioDecoder> audioDecoder;

            audiodecoder::Capabilities *capabilities = new audiodecoder::Capabilities();

            sp<AudioDecoderEventListener> eventListener = new AudioDecoderEventListener();

            sp<AudioDecoderControllerListener> controlListener = new AudioDecoderControllerListener();

            controlListener->setDecoderID(_decoderId.value);
            eventListener->setDecoderID(_decoderId.value);

            /* Get Audio decoder handle */
            AUDIODECODER_MANAGER_GETAUDIODECODER(_audioDecoderId, audioDecoder);

            /* Register events */
            AUDIODECODER_REGISTER_EVENT_SUCCESS(audioDecoder, eventListener);

            /* Get Capabilities of Audio Decoder */
            AUDIODECODER_GETCAPABILITIES(audioDecoder, capabilities);

            mAudioDecoderList.push_back({_audioDecoderId, audioDecoder, controlListener, eventListener, capabilities});
        }

        for (const auto& _videoDecoderId : videoDecoderIds)
        {
            sp<IVideoDecoder> videoDecoder;
            videodecoder::Capabilities *capabilities = new videodecoder::Capabilities();
            sp<VideoDecoderEventListener> eventListener = new VideoDecoderEventListener();
            sp<VideoDecoderControllerListener> controlListener = new VideoDecoderControllerListener();

            VIDEODECODER_MANAGER_GETVIDEODECODER(_videoDecoderId, videoDecoder);

            VIDEODECODER_REGISTER_EVENT_SUCCESS(videoDecoder, eventListener);

            VIDEODECODER_GETCAPABILITIES(videoDecoder, capabilities);

            mVideoDecoderList.push_back({_videoDecoderId, videoDecoder, controlListener, eventListener, capabilities});
            UT_LOG_INFO("Added decoder ID %d to test list", decoderIdValue);
        }

        for(const auto& _videoSinkId: videoSinkIds)
        {

            sp<IVideoSink> videoSink;

            sp<VideoSinkEventListener> videoSinkEventListener = new VideoSinkEventListener();

            sp<VideoSinkControllerListener> videoSinkControlListener = new VideoSinkControllerListener();

            videoSinkControlListener->setSinkID(_sinkId.value);
            videoSinkEventListener->setSinkID(_sinkId.value);

            /* Get Video Sink handle */
            VIDEOSINK_MANAGER_GETVIDEOSINK(_sinkId, videoSink);

            /* Register events */
            VIDEOSINK_REGISTER_EVENT_SUCCESS(videoSink, videoSinkEventListener);

            //TODO: VideoSink Get capabilities are yet to implement in the video sink

            mVideoSinkList.push_back({_sinkId, videoSink, videoSinkControlListener, videoSinkEventListener});
        }
    }
};
    
void checkCompleteStateTransition(sp<IAVClock> avclock, sp<AVClockEventListener> eventListener,
                                      sp<AVClockControllerListener> controlListener, State oldState,
                                      State intermediateState, State newState)
    {
        ::com::rdk::hal::State currentState;

        UT_ASSERT_TRUE(eventListener->checkStateTransistion(oldState, intermediateState));
        UT_ASSERT_TRUE(controlListener->checkStateTransistion(oldState, intermediateState));

        eventListener->waitForState(newState, std::chrono::milliseconds(MAX_FINAL_STATE_WAIT_MS));
        controlListener->waitForState(newState, std::chrono::milliseconds(MAX_FINAL_STATE_WAIT_MS));

        AVCLOCK_GET_STATE(sink, currentState);

        UT_ASSERT_EQUAL(currentState, newState);

        UT_ASSERT_TRUE(eventListener->checkStateTransistion(intermediateState, newState));
        UT_ASSERT_TRUE(controlListener->checkStateTransistion(intermediateState, newState));

        eventListener->resetStateChanges();
        controlListener->resetStateChanges();
    }

    bool isClockUsable(const IAVClock::Id& clockId)
    {
        for (const auto& item : gAVClock_hfp)
        {
            if (std::find(item.availableIds.begin(), item.availableIds.end(), clockId.value) != item.availableIds.end())
            {
                return true;
            }
        }
        return false;
    }

class AVClockPrimeAudio
{
public:
    AVClockPrimeAudio(IAVClock::Id audioPrimeClockId , sp<IAVClock> audioPrimeClkHandle, avclock::ClockMode audioPrimeClockMode,
                     sp<AVClockControllerListener> audioPrimeClkController, IAudioDecoder::Id audioDecId, sp<IAudioDecoder> audioDecHandle,
                     sp<AudioDecoderControllerListener> audioDecControlListener,
                     sp<AudioDecoderEventListener> audioDecEventListener,IAudioSink::Id audioSinkId, 
                     sp<IAudioSink> audioSinkHandle,
                     sp<AudioSinkControllerListener> audioSinkControlListener,
                     sp<AudioSinkEventListener> audioSinkEventListener,
                     sp<IAVBuffer> AVBufferManager, IAVBufferHelper* avbufferHelper,
                     bool audioPrimeSecDecode, bool isTunnel, testConfiguration_t audioPrimeconfig)
    {
        maudioPrimeClockId = audioPrimeClockId;
        maudioPrimeClockHandle = audioPrimeClkHandle;
        maudioPrimeClockMode = audioPrimeClockMode;
        maudioPrimeClckController = audioPrimeClkController;
        maudioDecId = audioDecId;
        maudioDecHandle = audioDecHandle;
        maudioDecControlListener = audioDecControlListener;
        maudioDecEventListener = audioDecEventListener;
        maudioSinkId = audioSinkId;
        maudioSinkHandle = audioSinkHandle;
        maudioSinkControlListener = audioSinkControlListener;
        maudioSinkEventListener = audioSinkEventListener;
        maudioPrimeSecureDecode = audioPrimeSecDecode;
        maudioPrimeTestConfig = audioPrimeconfig;
        maudioTunnel = isTunnel;
        mAVBufferManager = AVBufferManager;
        mAVbufferHelper = avbufferHelper;
    }
    void run()
    {
        audioPrimeClockTest();
    }

public:
        IAVClock::ID maudioPrimeClockId = audioPrimeClockId;
        sp<IAVClock> maudioPrimeClockHandle = audioPrimeClkHandle;
        avclock::ClockMode maudioPrimeClockMode = audioPrimeClockMode;
        sp<AVClockControllerListener> maudioPrimeClckController = audioPrimeClkController;
        IAudioDecoder::Id maudioDecId = audioDecId;
        sp<IAudioDecoder> maudioDecHandle = audioDecHandle;
        sp<AudioDecoderControllerListener> maudioDecControlListener = audioDecControlListener;
        sp<AudioDecoderEventListener> maudioDecEventListener = audioDecEventListener;
        IAudioSink::Id maudioSinkId = audioSinkId;
        sp<IAudioSink> maudioSinkHandle = audioSinkHandle;
        sp<AudioSinkControllerListener> maudioSinkControlListener = audioSinkControlListener;
        sp<AudioSinkEventListener> maudioSinkEventListener = audioSinkEventListener;
        bool maudioPrimeSecureDecode = audioPrimeSecDecode;
        testConfiguration_t maudioPrimeTestConfig = audioPrimeconfig;
        bool maudioTunnel = istunnel;
        sp<IAVBuffer> mAVBufferManager = AVBufferManager;
        IAVBufferHelper* mAVbufferHelper = avbufferHelper

    void audioPrimeClockTest()
    {
        try {
                android::sp<com::rdk::hal::avclock::IAVClockController> avClockController;
                bool foundDecoder = false;
                
                /* Open the AVClock */
                AVCLOCK_OPEN_SUCCESS(maudioPrimeClockId, maudioPrimeClckController, avClockController, true);

                /* Set the AVClock Mode */
                AVCLOCK_SET_CLOCK_MODE_SUCCESS(maudioPrimeClockHandle, avClockController, maudioPrimeClockMode, true)
     
                /* Start the AVClock */
                AVCLOCK_START_SUCCESS(maudioPrimeClockHandle, avClockController, true);

                /*Set the Sink*/
                AVCLOCK_SET_AUDIO_SINK_SUCCESS(maudioPrimeClockHandle, avClockController, maudioSinkId, true)

                if (!isSinkUsable(maudioSinkId))
                {
                    UT_LOG_INFO("[SinkID: %d] is not usable for this test configuration, skipping.", maudioSinkId.value);
                    continue;
                }

                if (isDecoderUsable(audioDecId))
                {
                    foundDecoder = true;
                }

                if (!foundDecoder)
                {
                    UT_LOG_WARNING("No usable decoder found for Sink ID %d, skipping.", _sinkId.value);
                    return;
                }
                
                com::rdk::hal::avclock::ClockTime clockBeforeDecoder , clockAfterDecoder, clockAfterSink;

                AVCLOCK_GET_CURRENT_CLOCK_TIME_SUCCESS(avClock, avClockController, clockBeforeDecoder);

                HeapMetrics beforeHeapMetrics, afterHeapMetrics;
                AVBUFFER_GETMETRICS(mAVBufferManager, maudioPrimeSecureDecode, beforeHeapMetrics);

                AudioDecoderTest decodeTest(audioDecId, audioDecHandle, audioDecControlListener,
                                    audioDecEventListener, mAVBufferManager, mAVbufferHelper,  maudioPrimeSecureDecode, maudioPrimeTestConfig);

                AVCLOCK_GET_CURRENT_CLOCK_TIME_SUCCESS(avClock, avClockController, clockAfterDecoder);

                std::thread decodeThread(&AudioDecoderTest::run, &decodeTest, false);

                SinkTest sinkTest(audioSinkId, audioSinkHandle, audioSinkControlListener, audioSinkEventListener,
                                mAVBufferManager, audioDecId, maudioPrimeSecureDecode, maudioTunnel, maudioPrimeTestConfig);

                std::this_thread::sleep_for(std::chrono::milliseconds(500));
                sinkTest.run(maudioTunnel);

                AVCLOCK_GET_CURRENT_CLOCK_TIME_SUCCESS(avClock, avClockController, clockAfterSink);

                
                //Calculate latency
                long long decodeLatency = clockAfterDecoder - clockBeforeDecoder;
                long long totalLatency = clockAfterSink - clockBeforeDecoder;

                UT_LOG_INFO("Decode latency = %lld, TotalLatency = %lld", decodeLatency, totalLatency);
               
                if(total Latency > kMaxLatencyNs)
                     UT_LOG_INFO("Warning: High total latency\n");

                const auto& frame = maudioPrimeTestConfig.frameDetails;

                long long framePTS = std::get<1>(frame);

                long long deltaPTS = std::abs(framePTS - clockAfterSink);

                UT_LOG_INFO("PTS delta after sink = %lld", deltaPTS);

                if (deltaPTS > kMaxPTSDriftNs)
                {
                       UT_LOG_INFO("Warning: PTS misalignment\n");
                }

                if (decodeThread.joinable())
                    decodeThread.join();

                AVBUFFER_GETMETRICS(mAVBufferManager, maudioPrimeSecureDecode, afterHeapMetrics);

                // Check if the decoder freed all the buffer pools
                UT_ASSERT_TRUE(beforeHeapMetrics.secure == afterHeapMetrics.secure);
                UT_ASSERT_TRUE(beforeHeapMetrics.bytesUsed == afterHeapMetrics.bytesUsed);
                UT_ASSERT_TRUE(beforeHeapMetrics.bytesTotal == afterHeapMetrics.bytesTotal);

                /*Stop the AVClock*/
                AVCLOCK_STOP_SUCCESS(maudioPrimeClockHandle, avClockController, true);

                /*Close the AVClock*/
                AVCLOCK_CLOSE_SUCCESS(maudioPrimeClockHandle, avClockController, true);

        } catch (const std::exception& ex) {
            UT_LOG_ERROR("Exception in runSequentialSink: %s", ex.what());
            UT_ASSERT_TRUE(false);
        } catch (...) {
            UT_LOG_ERROR("Unknown exception in runSequentialSink");
            UT_ASSERT_TRUE(false);
        }
    }
};

class AVClockPrimeVideo
{
public:
    AVClockPrimeVideo(IAVClock::Id videoPrimeClockId , sp<IAVClock> videoPrimeClkHandle, avclock::ClockMode videoPrimeClockMode, 
                     sp<AVClockControllerListener> videoPrimeClkController, IVideoDecoder::Id videoDecId, IVideoDecoder::Id videoDecId, sp<IVideoDecoder> videoDecHandle,
                     sp<VideoDecoderControllerListener> videoDecControlListener,
                     sp<VideoDecoderEventListener> videoDecEventListener,IVideoSink::Id videoSinkId, 
                     sp<IVideoSink> videoSinkHandle,
                     sp<VideoSinkControllerListener> videoSinkControlListener,
                     sp<VideoSinkEventListener> videoSinkEventListener,
                     sp<IAVBuffer> AVBufferManager, IAVBufferHelper* avbufferHelper,
                     bool videoPrimeSecDecode, bool isTunnel, videoDecoderTestConfiguration_t videoDecTestconfig, videoTestConfiguration_t VideoSinkConfig)
    {
        mvideoPrimeClockId = videoPrimeClockId;
        mvideoPrimeClockHandle = videoPrimeClkHandle;
        mvideoPrimeClockMode = videoPrimeClockMode;
        mvideoPrimeClckController = videoPrimeClkController;
        mvideoDecId = videoDecId;
        mvideoDecHandle = videoDecHandle;
        mvideoDecControlListener = videoDecControlListener;
        mvideoDecEventListener = videoDecEventListener;
        mvideoSinkId = videoSinkId;
        mvideoSinkHandle = videoSinkHandle;
        mvideoSinkControlListener = videoSinkControlListener;
        mvideoSinkEventListener = videoSinkEventListener;
        mvideoPrimeSecureDecode = videoPrimeSecDecode;
        mvideoTunnel = isTunnel;
        mAVBufferManager = AVBufferManager;
        mAVbufferHelper = avbufferHelper;
        mvideoDecTestConfig = videoDecTestConfig;
        mvideoSinkConfig = VideoSinkConfig;
    }
    void run()
    {
        videoPrimeClockTest();
    }

public:
        IAVClock::ID mvideoPrimeClockId = videoPrimeClockId;
        sp<IAVClock> mvideoPrimeClockHandle = videoPrimeClkHandle;
        avclock::ClockMode mvideoPrimeClockMode = videoPrimeClockMode;
        sp<AVClockControllerListener> mvideoPrimeClckController = videoPrimeClkController;
        IVideoDecoder::Id mvideoDecId = videoDecId;
        sp<IVideoDecoder> mvideoDecHandle = videoDecHandle;
        sp<VideoDecoderControllerListener> mvideoDecControlListener = videoDecControlListener;
        sp<VideoDecoderEventListener> mvideoDecEventListener = videoDecEventListener;
        IVideoSink::Id mvideoSinkId = videoSinkId;
        sp<IVideoSink> mvideoSinkHandle = videoSinkHandle;
        sp<VideoSinkControllerListener> mvideoSinkControlListener = videoSinkControlListener;
        sp<VideoSinkEventListener> mvideoSinkEventListener = videoSinkEventListener;
        bool mvideoPrimeSecureDecode = videoPrimeSecDecode;
        bool mvideoTunnel = isTunnel;
        sp<IAVBuffer> mAVBufferManager = AVBufferManager;
        IAVBufferHelper* mAVbufferHelper = avbufferHelper;
        videoDecoderTestConfiguration_t mvideoDecTestConfig = videoDecTestConfig;
        videoTestConfiguration_t mvideoSinkConfig = VideoSinkConfig;

    void videoPrimeClockTest()
    {
        try {
                android::sp<com::rdk::hal::avclock::IAVClockController> avClockController;
                bool foundDecoder = false;

                /* Open the AVClock */
                AVCLOCK_OPEN_SUCCESS(mvideoPrimeClockId, videoPrimeClkController, avClockController, true);

                /* Set the AVClock Mode */
                AVCLOCK_SET_CLOCK_MODE_SUCCESS(mpcrPrimeClockHandle, avClockController, mpcrPrimeClockMode, true)
     
                /* Start the AVClock */
                AVCLOCK_START_SUCCESS(mpcrPrimeClockHandle, avClockController, true);

                /*Set the Sink*/
                AVCLOCK_SET_AUDIO_SINK_SUCCESS(mvideoPrimeClockHandle, avClockController, mvideoSinkId, true)

                if (!isSinkUsable(mvideoSinkId))
                {
                    UT_LOG_INFO("[SinkID: %d] is not usable for this test configuration, skipping.", maudioSinkId.value);
                    continue;
                }

                if (isDecoderUsable(videoDecId))
                {
                    foundDecoder = true;
                }

                if (!foundDecoder)
                {
                    UT_LOG_WARNING("No usable decoder found for Sink ID %d, skipping.", videoDecId.value);
                    return;
                }

                com::rdk::hal::avclock::ClockTime videoClockBeforeDecoder , videoClockAfterDecoder, videoClockAfterSink;

                AVCLOCK_GET_CURRENT_CLOCK_TIME_SUCCESS(avClock, avClockController, videoClockBeforeDecoder);

                HeapMetrics beforeHeapMetrics, afterHeapMetrics;
                AVBUFFER_GETMETRICS(mAVBufferManager, mvideoPrimeSecureDecode, beforeHeapMetrics);

                videoDecoderTest decodeTest(mvideoDecId, mvideoDecHandle, mvideoDecControlListener,
                                    mvideoDecEventListener, mAVBufferManager, mAVbufferHelper, mvideoPrimeSecureDecode, 
                                    mvideoTunnel, mvideoDecTestConfig, mAVbufferHelper);

                std::thread decodeThread(&AudioDecoderTest::run, &decodeTest, false);

                AVCLOCK_GET_CURRENT_CLOCK_TIME_SUCCESS(avClock, avClockController, videoClockAfterDecoder);

                SinkTest *sinkTest = new SinkTest(mvideoSinkId, mvideoSinkHandle, mvideoSinkControlListener, mvideoSinkEventListener,
                                                mvideoDecControlListener, mvideoDecTestConfig, mvideoSinkConfig);

                std::this_thread::sleep_for(std::chrono::milliseconds(500));
                sinkTest.run();

                AVCLOCK_GET_CURRENT_CLOCK_TIME_SUCCESS(avClock, avClockController, videoClockAfterSink);

                //Calculate latency
                long long decodeLatency = clockAfterDecoder - clockBeforeDecoder;
                long long totalLatency = clockAfterSink - clockBeforeDecoder;

                UT_LOG_INFO("Decode latency = %lld, TotalLatency = %lld", decodeLatency, totalLatency);
               
                if(totalLatency > kMaxLatencyNs)
                     UT_LOG_INFO("Warning: High total latency\n");

                const auto& frame = mvideoDecTestConfig.frameDetails;

                long framePTS = std::get<1>(frame);

                long deltaPTS = std::abs(framePTS - clockAfterSink);

                UT_LOG_INFO("PTS delta after sink = %lld", deltaPTS);

                if (deltaPTS > kMaxPTSDriftNs)
                {
                       UT_LOG_INFO("Warning: PTS misalignment\n");
                }

                if (decodeThread.joinable())
                    decodeThread.join();

                AVBUFFER_GETMETRICS(mAVBufferManager, mvideoPrimeSecureDecode, afterHeapMetrics);

                // Check if the decoder freed all the buffer pools
                UT_ASSERT_TRUE(beforeHeapMetrics.secure == afterHeapMetrics.secure);
                UT_ASSERT_TRUE(beforeHeapMetrics.bytesUsed == afterHeapMetrics.bytesUsed);
                UT_ASSERT_TRUE(beforeHeapMetrics.bytesTotal == afterHeapMetrics.bytesTotal);
                
        } catch (const std::exception& ex) {
            UT_LOG_ERROR("Exception in runSequentialSink: %s", ex.what());
            UT_ASSERT_TRUE(false);
        } catch (...) {
            UT_LOG_ERROR("Unknown exception in runSequentialSink");
            UT_ASSERT_TRUE(false);
        }
    }
};

class AVClockPrimePcr()
{
        AVClockPrimePcr(IAVClock::Id pcrPrimeClockId , sp<IAVClock> pcrPrimeClkHandle, avclock::ClockMode pcrPrimeClockMode, sp<AVClockControllerListener> pcrPrimeClkController, vector<int64_t> pcrValues)
    {
        mpcrPrimeClockId = pcrPrimeClockId;
        mpcrPrimeClockHandle = pcrPrimeClkHandle;
        mpcrPrimeClockMode = pcrPrimeClockMode;
        mpcrPrimeClckController = pcrPrimeClkController;
        vector<int64_t> mpcrValues = pcrValues;
    }

    long getMonotonicRawTimestamp() 
    {
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
        return static_cast<long>(ts.tv_sec) * 1'000'000'000LL + ts.tv_nsec;
    }

    void run()
    {
        pcrPrimeTest();
    }

public:
        IAVClock::ID mpcrPrimeClockId = pcrPrimeClockId;
        sp<IAVClock> mpcrPrimeClockHandle = pcrPrimeClkHandle;
        avclock::ClockMode mpcrPrimeClockMode = pcrPrimeClockMode;
        sp<AVClockControllerListener> mpcrPrimeClckController = pcrPrimeClkController;
        vector<long> mpcrValues = pcrValues;

    void pcrPrimeClockTest()
    {
        try {
                android::sp<com::rdk::hal::avclock::IAVClockController> avClockController;

                /* Open the AVClock */
                AVCLOCK_OPEN_SUCCESS(mpcrPrimeClockId, pcrPrimeClkController, avClockController, true);

                /* Set the AVClock Mode */
                AVCLOCK_SET_CLOCK_MODE_SUCCESS(mpcrPrimeClockHandle, avClockController, mpcrPrimeClockMode, true);
     
                /* Start the AVClock */
                AVCLOCK_START_SUCCESS(mpcrPrimeClockHandle, avClockController, true);

                for(const auto& pcrTimeNs = mpcrValues )
                {
                    // Get current timestamp
                    long sampleTimestampNs = getMonotonicRawTimestamp();
                    bool success = notifyPCRSample(pcrTimeNs, sampleTimestampNs);
                }
                
                std::this_thread::sleep_for(std::chrono::milliseconds(500));

                AVCLOCK_GET_CURRENT_CLOCK_TIME_SUCCESS(mvideoPrimeClockHandle, avClockController, expectedClockTimeNs)

        } catch (const std::exception& ex) {
            UT_LOG_ERROR("Exception in runSequentialSink: %s", ex.what());
            UT_ASSERT_TRUE(false);
        } catch (...) {
            UT_LOG_ERROR("Unknown exception in runSequentialSink");
            UT_ASSERT_TRUE(false);
        }
    }
}

    void runSequentialAudioClock(testConfiguration_t& testConfig, bool isSecure, bool isTunnel, avclock::ClockMode clockMode, contentprimetype primetype)
    {
       try {
            for (const auto& [_clockId, clockHandle, controlListener, eventListener] : mAVClockList)
             {  

                if (!isClockUsable(_clockId))
                {
                    UT_LOG_INFO("[ClockID: %d] is not usable for this test configuration, skipping.", _clockId.value);
                    continue;
                }

                if (!mAudioDecoderList.empty()) 
                {
                    auto& [audioDecId, audioDecHandle, audioDecControlListener, audioDecEventListener] = mAudioDecoderList[0];
                } else {
                    UT_LOG_INFO(" Audio Decoder List is empty Skipping Sequential Test");
                    break;
                }

                if (!mAudioSinkList.empty()) 
                {
                    auto& [audioSinkId, audioSinkHandle, audioSinkControlListener, audioSinkEventListener] = mAudioSinkList[0];
                } else {
                    UT_LOG_INFO("Audio Sink List is empty Skipping Sequential Test");
                    break;
                }

                AVClockPrimeAudio audioprimetest(_clockId, clockHandle, clockMode, audioDecId, audioDecHandle, audioDecControlListener,
                            audioDecEventListener, audioSinkId, audioSinkHandle, audioSinkControlListener,audioSinkEventListener,
                            mAVBufferManager, mAVbufferHelper, isSecure, isTunnel, testConfig);
                std::thread primeaudiothread(&AVClockPrimeAudio::run, &audioprimetest, false); 

                if (primeaudiothread.joinable())
                        primeaudiothread.join();

            } catch (const std::exception& ex) {
                UT_LOG_ERROR("Exception in runSequentialAudioClock: %s", ex.what());
                UT_ASSERT_TRUE(false);
            } catch (...) {
                UT_LOG_ERROR("Unknown exception in runSequentialAudioClock");
                UT_ASSERT_TRUE(false);
            }

        }
    }

    void runSequentialVideoClock(videoDecoderTestConfiguration_t vdecConfig, videoTestConfiguration_t sinkConfig, bool isSecure, bool isTunnel, avclock::ClockMode clockMode, contentprimetype primetype)
    {
       try {
            for (const auto& [_clockId, clockHandle, controlListener, eventListener] : mAVClockList)
             {  

                if (!isClockUsable(_clockId))
                {
                    UT_LOG_INFO("[ClockID: %d] is not usable for this test configuration, skipping.", _clockId.value);
                    continue;
                }

                if (!mVideoDecoderList.empty()) 
                {
                    auto& [videoDecId, videoDecHandle, videoDecControlListener, videoDecEventListener] = mVideoDecoderList[0];
                } else {
                    UT_LOG_INFO(" Video Decoder List is empty Skipping Sequential Test");
                    break;
                }

                if (!mVideoSinkList.empty()) 
                {
                    auto& [videoSinkId, videoSinkHandle, videoSinkControlListener, videoSinkEventListener] = mSinkDecoderList[0];
                } else {
                    UT_LOG_ERROR("Video Sink List is empty Skipping Sequential Test");
                    break;
                }
        
                AVClockPrimeVideo videoprimetest(_clockId, clockHandle, clockMode, videoDecId, videoDecHandle, videoDecControlListener,
                        videoDecEventListener, videoSinkId, videoSinkHandle, videoSinkControlListener, videoSinkEventListener,
                        mAVBufferManager, mAVbufferHelper, isSecure, isTunnel, vdecConfig, sinkConfig);
                std::thread primevideothread(&AVClockPrimeVideo::run, &videoprimetest, false);  
                
                if (primevideothread.joinable())
                        primevideothread.join();
                
            } catch (const std::exception& ex) {
                UT_LOG_ERROR("Exception in runSequentialVideoClock: %s", ex.what());
                UT_ASSERT_TRUE(false);
            } catch (...) {
                UT_LOG_ERROR("Unknown exception in runSequentialVideoClock");
                UT_ASSERT_TRUE(false);
            }

        }
    }

    void runSequentialPCRClock(long pcrValue, avclock::ClockMode clockMode)
    {
       try {
            for (const auto& [_clockId, clockHandle, controlListener, eventListener] : mAVClockList)
             {  

                if (!isClockUsable(_clockId))
                {
                    UT_LOG_INFO("[ClockID: %d] is not usable for this test configuration, skipping.", _clockId.value);
                    continue;
                }
        
                AVClockPrimePcr pcrprimetest(_clockId, clockHandle, clockMode, controlListener, pcrValue);
                std::thread primepcrthread(&AVClockPrimePcr::run, &pcrprimetest, false);  
                
                if (primepcrthread.joinable())
                        primepcrthread.join();
                
            } catch (const std::exception& ex) {
                UT_LOG_ERROR("Exception in runSequentialPCRClock: %s", ex.what());
                UT_ASSERT_TRUE(false);
            } catch (...) {
                UT_LOG_ERROR("Unknown exception in runSequentialPCRClock");
                UT_ASSERT_TRUE(false);
            }

        }
    }

    void runConcurrentAudioClock(testConfiguration_t& testConfig, bool secureDecode, bool isTunnel)
    {
        try {
            vector<thread> audioPrimeThreads;
            vector<unique_ptr<AVClockPrimeAudio>> audioPrimeTests;

                for(clockid = 0; clockid < mAVClockList.size(); clockid++)
                {
                    if(mAVClockList.size() == 1)
                    {
                        UT_LOG_INFO("Platform has once instace of Clock it is same as sequential skipping concurrent Tests");
                        continue;
                    }
     
                    auto result = getTupleElements(mAudioDecoderList, clockid, std::index_sequence<0, 1, 2, 3>{});
                    if (result)
                    {
                        auto& [audioDecId, audioDecHandle, audioDecControlListener, audioDecEventListener] = *result;
                    }

                    auto result = getTupleElements(mAudioSinkList, clockid, std::index_sequence<0, 1, 2, 3>{});
                    if (result)
                    {
                        auto& [audioSinkId, audioSinkHandle, audioSinkControlListener, audioSinkEventListener] = *result;
                    }
  
                    auto audioprimetest = make_unique<AVClockPrimeAudio>(_clockId, clockHandle, clockMode, audioDecId, audioDecHandle, audioDecControlListener,
                                audioDecEventListener, audioSinkId, audioSinkHandle, audioSinkControlListener,audioSinkEventListener,
                                mAVBufferManager, mAVbufferHelper, isSecure, isTunnel, testConfig);
                    audioPrimeThreads.emplace_back(&AVClockPrimeAudio::run, audioprimetest.get(), true); 
                    audioprimetest.push_back(move(audioprimetest));
                }
            } catch (const std::exception& ex) {
                UT_LOG_ERROR("Exception in runConcurrentAudioClock: %s", ex.what());
                UT_ASSERT_TRUE(false);
            } catch (...) {
                UT_LOG_ERROR("Unknown exception in runConcurrentAudioClock");
                UT_ASSERT_TRUE(false);
            }

            for (auto& t : audioPrimeThreads)
            {
                if (t.joinable())
                    t.join();
            }
    }

    void runConcurrentVideoClock(testConfiguration_t& testConfig, bool secureDecode, bool isTunnel)
    {
        try {
            vector<thread> videoPrimeThreads;
            vector<unique_ptr<AVClockPrimeVideo>> videoPrimeTests;

            for(clockid = 0; clockid < mAVClockList.size(); clockid++)
            {
                if(mAVClockList.size() == 1)
                {
                    UT_LOG_INFO("Platform has once instace of Clock it is same as sequential skipping concurrent Tests");
                    continue;
                }

                auto result = getTupleElements(mVideoDecoderList, clockid, 0, 1, 2, 3);
                if (result)
                {
                    auto& [videoDecId, videoDecHandle, videoDecControlListener, videoDecEventListener] = *result;
                }

                auto result = getTupleElements(mVideoSinkList, clockid, 0, 1, 2, 3);
                if (result)
                {
                    auto& [videoSinkId, videoSinkHandle, videoSinkControlListener, videoSinkEventListener] = *result;
                }

                auto videoprimetest = make_unique<AVClockPrimeVideo>(_clockId, clockHandle, clockMode, videoDecId, videoDecHandle, videoDecControlListener,
                            videoDecEventListener, videoSinkId, videoSinkHandle, videoSinkControlListener, videoSinkEventListener,
                            mAVBufferManager, mAVbufferHelper, isSecure, isTunnel, vdecConfig, sinkConfig);
                videoPrimeThreads.emplace_back(&AVClockPrimeVideo::run, videoprimetest.get(), true);
                videoprimetest.push_back(move(videoprimetest));
                         
            }
        } catch (const std::exception& ex) {
            UT_LOG_ERROR("Exception in runConcurrentVdieoClock: %s", ex.what());
            UT_ASSERT_TRUE(false);
        } catch (...) {
            UT_LOG_ERROR("Unknown exception in runConcurrentVideoClock");
            UT_ASSERT_TRUE(false);
        }

        for (auto& t : videoPrimeThreads)
        {
            if (t.joinable())
            t.join();
        }
    }

    void runConcurrentPCRClock(long pcrValue, avclock::ClockMode clockMode)
    {
        try {
            vector<thread> pcrPrimeThreads;
            vector<unique_ptr<AVClockPrimePcr>> pcrPrimeTests;

            for(clockid = 0; clockid < mAVClockList.size(); clockid++)
            {
                if(mAVClockList.size() == 1)
                {
                    UT_LOG_INFO("Platform has once instace of Clock it is same as sequential skipping concurrent Tests");
                    continue;
                }

                auto pcrprimetest = make_unique<AVClockPrimePcr>(_clockId, clockHandle, clockMode, pcrValue);
                pcrPrimeThreads.emplace_back(&AVClockPrimePcr::run, pcrprimetest.get(), true);
                pcrPrimeTests.push_back(move(pcrprimetest));
                         
            }
        } catch (const std::exception& ex) {
            UT_LOG_ERROR("Exception in runConcurrentPCRClock: %s", ex.what());
            UT_ASSERT_TRUE(false);
        } catch (...) {
            UT_LOG_ERROR("Unknown exception in runConcurrentPCRClock");
            UT_ASSERT_TRUE(false);
        }

        for (auto& t : pcrPrimeThreads)
        {
            if (t.joinable())
            t.join();
        }
    }

    void runAudioPrimeTest(const char *testName, bool isSecureMode, bool isTunnel, bool isConcurrentMode, avclock::ClockMode clockMode, contentprimetype primetype)
    {
        UT_LOG_INFO("Started Thread : %s", __func__);

        vector<testConfiguration_t> testConfiguration;

        vts_audiodecoder_parse_testconfig(gAudioSinkTestConfiguration, gAudioModuleName, gTestGroup, testName, testConfiguration);

        if(isConcurrentMode)
        {
            for(auto& test: testConfiguration)
            {
                runConcurrentAudioClock(test, isSecureMode, isTunnel, clockMode, primetype);
            }
        }
        else
        {
            for(auto& test: testConfiguration)
            {
                runSequentialAudioClock(test, isSecureMode, isTunnel, clockMode, primetype);
            }
        }
    }

    void runVideoPrimeTest(const char *testName, bool isSecureMode, bool isTunnel, bool isConcurrentMode, avclock::ClockMode clockMode, contentprimetype primetype)
    {
        UT_LOG_INFO("Started Thread : %s", __func__);

        vector<videoDecoderTestConfiguration_t> decoderConfigs;
        vector<videoTestConfiguration_t> sinkConfigs;

        vts_videodecoder_parse_testconfig(gVideoSinkTestConfiguration, VIDEODECODERMGR, gTestGroup, testName, decoderConfigs);

        vts_videosink_parse_testconfig(gVideoSinkTestConfiguration, VIDEODECODERMGR, gTestGroup, testName, sinkConfigs);

        for (size_t i = 0; i < decoderConfigs.size(); ++i)
        {
            auto& decoderConfig = decoderConfigs[i];
            auto& sinkConfig = sinkConfigs[i];

            if (concurrentMode)
            {
                runConcurrentVideoClock(decoderConfig, sinkConfig, secureMode, isTunnel);
            } else
            {
                runSequentialVideoClock(decoderConfig, sinkConfig, secureMode, isTunnel);
            }
        }
    }

    void runPcrPrimeTest(const char *testName, bool isConcurrentMode, avclock::ClockMode clockMode)
    {
        UT_LOG_INFO("Started Thread : %s", __func__);

        if(isConcurrentMode)
        {
            for(auto& _pcrValue: pcrValues)
            {
                runConcurrentPCRClock(_pcrValue, clockMode);
            }
        }
        else
        {
            for(auto& _pcrValue: pcrValues)
            {
                runSequentialAudioClock(_pcrValue, clockMode);
            }
        }
    }

    void runAVClockPrimeTest(const char *testName, bool isSecureMode, bool isTunnel, bool isConcurrentMode, avclock::ClockMode clockMode, contentprimetype primetype)
    {
        if(primetype == primeaudio)
        {
            std::thread runaudioprimethread(runAudioPimeTest, testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
        } else if(primetype == primevideo)
        {
            std::thread runvideoprimethread(runAudioPimeTest, testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
        } else if(pirmetype == primeaudiobeforevideo)
        {
            std::thread runaudioprimethread(runAudioPimeTest, testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
            while(!threadStarted)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                UT_LOG_ERROR("Unable to Start Thread got Timed out");
                return;
            }
            std::thread runvideoprimethread(runAudioPimeTest, testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
        } else if(primetype == primevideobeforeaudio)
        {
            std::thread runvideoprimethread(runAudioPimeTest, testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
            while(!threadStarted)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                UT_LOG_ERROR("Unable to Start Thread got Timed out");
                return;
            }
            std::thread runaudioprimethread(runAudioPimeTest, testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
        } else if(primetype == primepcr)
        {
            std::thread runaudioprimethread(runAudioPrimeTest, testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
            while(!threadStarted)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                UT_LOG_ERROR("Unable to Start Thread got Timed out");
                return;
            }
            std::thread runvideoprimethread(runAudioPrimeTest, testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
            std::thread runpcrprimethread(runPcrPrimeTest, testName, isConcurrentMode, clockMode);
        }

        if (runaudioprimethread.joinable())
                runaudioprimethread.join();

        if (runvideoprimethread.joinable())
                runvideoprimethread.join();

        if (runpcrprimethread.joinable())
                runpcrprimethread.join();

    }

    void TearDown() override
    {
        for (const auto& [_avClockId, avClockHandle, controlListener, eventListener] : mAVClockList)
        {
            /* Un-register the events */
            AVCLOCK_UNREGISTER_EVENT_SUCCESS(avClockHandle, eventListener);
        }
        mAVClockList.clear();
        //TODO: check right place to call joinThreadPool
        //android::IPCThreadState::self()->joinThreadPool();
    }

/**
* @brief Test the AVClock functionality (sequential, non-tunnel, non-secure)
*
* This test verifies the basic AVClock functionality for Audio Stream in Auto clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 001@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Sequential_NonTunnel_NonSecure_AutoAudioOnly)
{
    const char *testName = "PrimeClock_AutoAudioOnly";
    bool isSecureMode = false;
    bool isConcurrentMode = false;
    bool isTunnel = false;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (sequential, non-tunnel, non-secure)
*
* This test verifies the basic AVClock functionality for Video Stream in Auto clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 002@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Sequential_NonTunnel_NonSecure_AutoVideoOnly)
{
    const char *testName = "PrimeClock_AutoVideoOnly";
    bool isSecureMode = false;
    bool isConcurrentMode = false;
    bool isTunnel = false;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (sequential, non-tunnel, non-secure)
*
* This test verifies the basic AVClock functionality for Audio Stream in AudioMaster clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 003@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Sequential_NonTunnel_NonSecure_AudioMasterAudioOnly)
{
    const char *testName = "PrimeClock_AudioMasterAudioOnly";
    bool isSecureMode = false;
    bool isConcurrentMode = false;
    bool isTunnel = false;
    avclock::ClockMode clockMode = avclock::ClockMode::AUDIO_MASTER;
    contentprimetype primetype = primeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (sequential, non-tunnel, non-secure)
*
* This test verifies the basic AVClock functionality for Video Stream in VideoMaster clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 004@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Sequential_NonTunnel_NonSecure_VideoMasterVideoOnly)
{
    const char *testName = "PrimeClock_VideoMasterVideoOnly";
    bool isSecureMode = false;
    bool isConcurrentMode = false;
    bool isTunnel = false;
    avclock::ClockMode clockMode = avclock::ClockMode::VIDEO_MASTER;
    contentprimetype primetype = primevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (sequential, non-tunnel, non-secure)
*
* This test verifies the basic AVClock functionality for Video Stream in AudioMaster clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 005@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Sequential_NonTunnel_NonSecure_AudioMasterVideoOnly)
{
    const char *testName = "PrimeClock_AudioMasterVideoOnly";
    bool isSecureMode = false;
    bool isConcurrentMode = false;
    bool isTunnel = false;
    avclock::ClockMode clockMode = avclock::ClockMode::AUDIO_MASTER;
    contentprimetype primetype = primevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (sequential, non-tunnel, non-secure)
*
* This test verifies the basic AVClock functionality for Audio Stream in VideoMaster clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 006@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Sequential_NonTunnel_NonSecure_VideoMasterAudioOnly)
{
    const char *testName = "PrimeClock_VideoMasterAudioOnly";
    bool isSecureMode = false;
    bool isConcurrentMode = false;
    bool isTunnel = false;
    avclock::ClockMode clockMode = avclock::ClockMode::VIDEO_MASTER;
    contentprimetype primetype = primeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (sequential, non-tunnel, non-secure)
*
* This test verifies the basic AVClock functionality for PCR Stream in PCR clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 007@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Sequential_NonTunnel_NonSecure_PCRMode)
{
    const char *testName = "PrimeClock_PCRMode";
    bool isSecureMode = false;
    bool isConcurrentMode = false;
    bool isTunnel = false;
    avclock::ClockMode clockMode = avclock::ClockMode::PCR;
    contentprimetype primetype = primepcr; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (sequential, non-tunnel, non-secure)
*
* This test verifies the basic AVClock functionality for Audio, Video Streams in Auto clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 008@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Sequential_NonTunnel_NonSecure_AutoAudioBeforeVideo)
{
    const char *testName = "PrimeClock_AutoAudioBeforeVideo";
    bool isSecureMode = false;
    bool isConcurrentMode = false;
    bool isTunnel = false;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primeaudiobeforevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (sequential, non-tunnel, non-secure)
*
* This test verifies the basic AVClock functionality for Video, Audio Streams in Auto clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 009@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Sequential_NonTunnel_NonSecure_AutoVideoBeforeAudio)
{
    const char *testName = "PrimeClock_AutoVideoBeforeAudio";
    bool isSecureMode = false;
    bool isConcurrentMode = false;
    bool isTunnel = false;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primevideobeforeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (sequential, non-tunnel, non-secure)
*
* This test verifies the basic AVClock Playback Speed functionality for Audio Streams in Auto clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 010@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Sequential_NonTunnel_NonSecure_AutoSetAudioPlayRateNormalSpeed)
{
    const char *testName = "PrimeClock_AutoSetAudioPlayRateNormalSpeed";
    bool isSecureMode = false;
    bool isConcurrentMode = false;
    bool isTunnel = false;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (sequential, non-tunnel, non-secure)
*
* This test verifies the basic AVClock Playback Pause functionality for Audio Streams in Auto clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 011@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Sequential_NonTunnel_NonSecure_AutoSetAudioPlayRatePause)
{
    const char *testName = "PrimeClock_AutoSetAudioPlayRatePause";
    bool isSecureMode = false;
    bool isConcurrentMode = false;
    bool isTunnel = false;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (sequential, non-tunnel, non-secure)
*
* This test verifies the basic AVClock Playback FastForward functionality for Audio Streams in Auto clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 012@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Sequential_NonTunnel_NonSecure_AutoSetAudioPlayRateFastForward)
{
    const char *testName = "PrimeClock_AutoSetAudioPlayRateFastForward";
    bool isSecureMode = false;
    bool isConcurrentMode = false;
    bool isTunnel = false;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (sequential, non-tunnel, non-secure)
*
* This test verifies the basic AVClock Playback Speed functionality for Video Streams in Auto clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 013@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Sequential_NonTunnel_NonSecure_AutoSetVideoPlayRateNormalSpeed)
{
    const char *testName = "PrimeClock_AutoSetVideoPlayRateNormalSpeed";
    bool isSecureMode = false;
    bool isConcurrentMode = false;
    bool isTunnel = false;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (sequential, non-tunnel, non-secure)
*
* This test verifies the basic AVClock Playback Pause functionality for Video Streams in Auto clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 014@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Sequential_NonTunnel_NonSecure_AutoSetVideoPlayRatePause)
{
    const char *testName = "PrimeClock_AutoSetVideoPlayRatePause";
    bool isSecureMode = false;
    bool isConcurrentMode = false;
    bool isTunnel = false;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (sequential, non-tunnel, non-secure)
*
* This test verifies the basic AVClock Playback FastForward functionality for Video Streams in Auto clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 015@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Sequential_NonTunnel_NonSecure_AutoSetVideoPlayRateFastForward)
{
    const char *testName = "PrimeClock_AutoSetVideoPlayRateFastForward";
    bool isSecureMode = false;
    bool isConcurrentMode = false;
    bool isTunnel = false;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (sequential, non-tunnel, secure)
*
* This test verifies the basic AVClock functionality for Audio Stream in Auto clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 0016@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Sequential_NonTunnel_Secure_AutoAudioOnly)
{
    const char *testName = "PrimeClock_AutoAudioOnly";
    bool isSecureMode = true;
    bool isConcurrentMode = false;
    bool isTunnel = false;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (sequential, non-tunnel, secure)
*
* This test verifies the basic AVClock functionality for Video Stream in Auto clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 017@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Sequential_NonTunnel_Secure_AutoVideoOnly)
{
    const char *testName = "PrimeClock_AutoVideoOnly";
    bool isSecureMode = true;
    bool isConcurrentMode = false;
    bool isTunnel = false;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (sequential, non-tunnel, secure)
*
* This test verifies the basic AVClock functionality for Audio Stream in AudioMaster clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 018@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Sequential_NonTunnel_Secure_AudioMasterAudioOnly)
{
    const char *testName = "PrimeClock_AudioMasterAudioOnly";
    bool isSecureMode = true;
    bool isConcurrentMode = false;
    bool isTunnel = false;
    avclock::ClockMode clockMode = avclock::ClockMode::AUDIO_MASTER;
    contentprimetype primetype = primeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (sequential, non-tunnel, secure)
*
* This test verifies the basic AVClock functionality for Video Stream in VideoMaster clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 019@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Sequential_NonTunnel_Secure_VideoMasterVideoOnly)
{
    const char *testName = "PrimeClock_VideoMasterVideoOnly";
    bool isSecureMode = true;
    bool isConcurrentMode = false;
    bool isTunnel = false;
    avclock::ClockMode clockMode = avclock::ClockMode::VIDEO_MASTER;
    contentprimetype primetype = primevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (sequential, non-tunnel, secure)
*
* This test verifies the basic AVClock functionality for Video Stream in AudioMaster clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 020@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Sequential_NonTunnel_Secure_AudioMasterVideoOnly)
{
    const char *testName = "PrimeClock_AudioMasterVideoOnly";
    bool isSecureMode = true;
    bool isConcurrentMode = false;
    bool isTunnel = false;
    avclock::ClockMode clockMode = avclock::ClockMode::AUDIO_MASTER;
    contentprimetype primetype = primevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (sequential, non-tunnel, secure)
*
* This test verifies the basic AVClock functionality for Audio Stream in VideoMaster clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 021@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Sequential_NonTunnel_Secure_VideoMasterAudioOnly)
{
    const char *testName = "PrimeClock_VideoMasterAudioOnly";
    bool isSecureMode = true;
    bool isConcurrentMode = false;
    bool isTunnel = false;
    avclock::ClockMode clockMode = avclock::ClockMode::VIDEO_MASTER;
    contentprimetype primetype = primeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (sequential, non-tunnel, secure)
*
* This test verifies the basic AVClock functionality for PCR Stream in PCR clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 022@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Sequential_NonTunnel_Secure_PCRMode)
{
    const char *testName = "PrimeClock_PCRMode";
    bool isSecureMode = true;
    bool isConcurrentMode = false;
    bool isTunnel = false;
    avclock::ClockMode clockMode = avclock::ClockMode::PCR;
    contentprimetype primetype = primepcr; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (sequential, non-tunnel, secure)
*
* This test verifies the basic AVClock functionality for Audio, Video Streams in Auto clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 023@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Sequential_NonTunnel_Secure_AutoAudioBeforeVideo)
{
    const char *testName = "PrimeClock_AutoAudioBeforeVideo";
    bool isSecureMode = true;
    bool isConcurrentMode = false;
    bool isTunnel = false;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primeaudiobeforevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (sequential, non-tunnel, secure)
*
* This test verifies the basic AVClock functionality for Video, Audio Streams in Auto clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 024@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Sequential_NonTunnel_Secure_AutoVideoBeforeAudio)
{
    const char *testName = "PrimeClock_AutoVideoBeforeAudio";
    bool isSecureMode = true;
    bool isConcurrentMode = false;
    bool isTunnel = false;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primevideobeforeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (sequential, non-tunnel, secure)
*
* This test verifies the basic AVClock Playback Speed functionality for Audio Streams in Auto clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 025@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Sequential_NonTunnel_Secure_AutoSetAudioPlayRateNormalSpeed)
{
    const char *testName = "PrimeClock_AutoSetAudioPlayRateNormalSpeed";
    bool isSecureMode = true;
    bool isConcurrentMode = false;
    bool isTunnel = false;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (sequential, non-tunnel, secure)
*
* This test verifies the basic AVClock Playback Pause functionality for Audio Streams in Auto clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 026@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Sequential_NonTunnel_Secure_AutoSetAudioPlayRatePause)
{
    const char *testName = "PrimeClock_AutoSetAudioPlayRatePause";
    bool isSecureMode = true;
    bool isConcurrentMode = false;
    bool isTunnel = false;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (sequential, non-tunnel, secure)
*
* This test verifies the basic AVClock Playback FastForward functionality for Audio Streams in Auto clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 027@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Sequential_NonTunnel_Secure_AutoSetAudioPlayRateFastForward)
{
    const char *testName = "PrimeClock_AutoSetAudioPlayRateFastForward";
    bool isSecureMode = true;
    bool isConcurrentMode = false;
    bool isTunnel = false;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (sequential, non-tunnel, secure)
*
* This test verifies the basic AVClock Playback Speed functionality for Video Streams in Auto clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 028@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Sequential_NonTunnel_Secure_AutoSetVideoPlayRateNormalSpeed)
{
    const char *testName = "PrimeClock_AutoSetVideoPlayRateNormalSpeed";
    bool isSecureMode = true;
    bool isConcurrentMode = false;
    bool isTunnel = false;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (sequential, non-tunnel, secure)
*
* This test verifies the basic AVClock Playback Pause functionality for Video Streams in Auto clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 029@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Sequential_NonTunnel_Secure_AutoSetVideoPlayRatePause)
{
    const char *testName = "PrimeClock_AutoSetVideoPlayRatePause";
    bool isSecureMode = true;
    bool isConcurrentMode = false;
    bool isTunnel = false;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (sequential, non-tunnel, secure)
*
* This test verifies the basic AVClock Playback FastForward functionality for Video Streams in Auto clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 030@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Sequential_NonTunnel_Secure_AutoSetVideoPlayRateFastForward)
{
    const char *testName = "PrimeClock_AutoSetVideoPlayRateFastForward";
    bool isSecureMode = true;
    bool isConcurrentMode = false;
    bool isTunnel = false;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (sequential, tunnel, non-secure)
*
* This test verifies the basic AVClock functionality for Audio Stream in Auto clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 031@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Sequential_Tunnel_NonSecure_AutoAudioOnly)
{
    const char *testName = "PrimeClock_AutoAudioOnly";
    bool isSecureMode = false;
    bool isConcurrentMode = false;
    bool isTunnel = true;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (sequential, tunnel, non-secure)
*
* This test verifies the basic AVClock functionality for Video Stream in Auto clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 032@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Sequential_Tunnel_NonSecure_AutoVideoOnly)
{
    const char *testName = "PrimeClock_AutoVideoOnly";
    bool isSecureMode = false;
    bool isConcurrentMode = false;
    bool isTunnel = true;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (sequential, tunnel, non-secure)
*
* This test verifies the basic AVClock functionality for Audio Stream in AudioMaster clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 033@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Sequential_Tunnel_NonSecure_AudioMasterAudioOnly)
{
    const char *testName = "PrimeClock_AudioMasterAudioOnly";
    bool isSecureMode = false;
    bool isConcurrentMode = false;
    bool isTunnel = true;
    avclock::ClockMode clockMode = avclock::ClockMode::AUDIO_MASTER;
    contentprimetype primetype = primeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (sequential, tunnel, non-secure)
*
* This test verifies the basic AVClock functionality for Video Stream in VideoMaster clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 034@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Sequential_Tunnel_NonSecure_VideoMasterVideoOnly)
{
    const char *testName = "PrimeClock_VideoMasterVideoOnly";
    bool isSecureMode = false;
    bool isConcurrentMode = false;
    bool isTunnel = true;
    avclock::ClockMode clockMode = avclock::ClockMode::VIDEO_MASTER;
    contentprimetype primetype = primevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (sequential, tunnel, non-secure)
*
* This test verifies the basic AVClock functionality for Video Stream in AudioMaster clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 035@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Sequential_Tunnel_NonSecure_AudioMasterVideoOnly)
{
    const char *testName = "PrimeClock_AudioMasterVideoOnly";
    bool isSecureMode = false;
    bool isConcurrentMode = false;
    bool isTunnel = true;
    avclock::ClockMode clockMode = avclock::ClockMode::AUDIO_MASTER;
    contentprimetype primetype = primevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (sequential, tunnel, non-secure)
*
* This test verifies the basic AVClock functionality for Audio Stream in VideoMaster clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 036@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Sequential_Tunnel_NonSecure_VideoMasterAudioOnly)
{
    const char *testName = "PrimeClock_VideoMasterAudioOnly";
    bool isSecureMode = false;
    bool isConcurrentMode = false;
    bool isTunnel = true;
    avclock::ClockMode clockMode = avclock::ClockMode::VIDEO_MASTER;
    contentprimetype primetype = primeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (sequential, tunnel, non-secure)
*
* This test verifies the basic AVClock functionality for PCR Stream in PCR clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 037@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Sequential_Tunnel_NonSecure_PCRMode)
{
    const char *testName = "PrimeClock_PCRMode";
    bool isSecureMode = false;
    bool isConcurrentMode = false;
    bool isTunnel = true;
    avclock::ClockMode clockMode = avclock::ClockMode::PCR;
    contentprimetype primetype = primepcr; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (sequential, tunnel, non-secure)
*
* This test verifies the basic AVClock functionality for Audio, Video Streams in Auto clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 0038@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Sequential_Tunnel_NonSecure_AutoAudioBeforeVideo)
{
    const char *testName = "PrimeClock_AutoAudioBeforeVideo";
    bool isSecureMode = false;
    bool isConcurrentMode = false;
    bool isTunnel = true;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primeaudiobeforevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (sequential, tunnel, non-secure)
*
* This test verifies the basic AVClock functionality for Video, Audio Streams in Auto clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 0039@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Sequential_Tunnel_NonSecure_AutoVideoBeforeAudio)
{
    const char *testName = "PrimeClock_AutoVideoBeforeAudio";
    bool isSecureMode = false;
    bool isConcurrentMode = false;
    bool isTunnel = true;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primevideobeforeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (sequential, tunnel, non-secure)
*
* This test verifies the basic AVClock Playback Speed functionality for Audio Streams in Auto clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 040@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Sequential_Tunnel_NonSecure_AutoSetAudioPlayRateNormalSpeed)
{
    const char *testName = "PrimeClock_AutoSetAudioPlayRateNormalSpeed";
    bool isSecureMode = false;
    bool isConcurrentMode = false;
    bool isTunnel = true;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (sequential, tunnel, non-secure)
*
* This test verifies the basic AVClock Playback Pause functionality for Audio Streams in Auto clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 041@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Sequential_Tunnel_NonSecure_AutoSetAudioPlayRatePause)
{
    const char *testName = "PrimeClock_AutoSetAudioPlayRatePause";
    bool isSecureMode = false;
    bool isConcurrentMode = false;
    bool isTunnel = true;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (sequential, tunnel, non-secure)
*
* This test verifies the basic AVClock Playback FastForward functionality for Audio Streams in Auto clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 042@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Sequential_Tunnel_NonSecure_AutoSetAudioPlayRateFastForward)
{
    const char *testName = "PrimeClock_AutoSetAudioPlayRateFastForward";
    bool isSecureMode = false;
    bool isConcurrentMode = false;
    bool isTunnel = true;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (sequential, tunnel, non-secure)
*
* This test verifies the basic AVClock Playback Speed functionality for Video Streams in Auto clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 043@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Sequential_Tunnel_NonSecure_AutoSetVideoPlayRateNormalSpeed)
{
    const char *testName = "PrimeClock_AutoSetVideoPlayRateNormalSpeed";
    bool isSecureMode = false;
    bool isConcurrentMode = false;
    bool isTunnel = true;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (sequential, tunnel, non-secure)
*
* This test verifies the basic AVClock Playback Pause functionality for Video Streams in Auto clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 044@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Sequential_Tunnel_NonSecure_AutoSetVideoPlayRatePause)
{
    const char *testName = "PrimeClock_AutoSetVideoPlayRatePause";
    bool isSecureMode = false;
    bool isConcurrentMode = false;
    bool isTunnel = true;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (sequential, tunnel, non-secure)
*
* This test verifies the basic AVClock Playback FastForward functionality for Video Streams in Auto clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 045@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Sequential_Tunnel_NonSecure_AutoSetVideoPlayRateFastForward)
{
    const char *testName = "PrimeClock_AutoSetVideoPlayRateFastForward";
    bool isSecureMode = false;
    bool isConcurrentMode = false;
    bool isTunnel = true;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (sequential, tunnel, secure)
*
* This test verifies the basic AVClock functionality for Audio Stream in Auto clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 046@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Sequential_Tunnel_Secure_AutoAudioOnly)
{
    const char *testName = "PrimeClock_AutoAudioOnly";
    bool isSecureMode = true;
    bool isConcurrentMode = false;
    bool isTunnel = true;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (sequential, tunnel, secure)
*
* This test verifies the basic AVClock functionality for Video Stream in Auto clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 047@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Sequential_Tunnel_Secure_AutoVideoOnly)
{
    const char *testName = "PrimeClock_AutoVideoOnly";
    bool isSecureMode = true;
    bool isConcurrentMode = false;
    bool isTunnel = true;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (sequential, tunnel, secure)
*
* This test verifies the basic AVClock functionality for Audio Stream in AudioMaster clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 048@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Sequential_Tunnel_Secure_AudioMasterAudioOnly)
{
    const char *testName = "PrimeClock_AudioMasterAudioOnly";
    bool isSecureMode = true;
    bool isConcurrentMode = false;
    bool isTunnel = true;
    avclock::ClockMode clockMode = avclock::ClockMode::AUDIO_MASTER;
    contentprimetype primetype = primeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (sequential, tunnel, secure)
*
* This test verifies the basic AVClock functionality for Video Stream in VideoMaster clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 049@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Sequential_Tunnel_Secure_VideoMasterVideoOnly)
{
    const char *testName = "PrimeClock_VideoMasterVideoOnly";
    bool isSecureMode = true;
    bool isConcurrentMode = false;
    bool isTunnel = true;
    avclock::ClockMode clockMode = avclock::ClockMode::VIDEO_MASTER;
    contentprimetype primetype = primevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (sequential, tunnel, secure)
*
* This test verifies the basic AVClock functionality for Video Stream in AudioMaster clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 050@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Sequential_Tunnel_Secure_AudioMasterVideoOnly)
{
    const char *testName = "PrimeClock_AudioMasterVideoOnly";
    bool isSecureMode = true;
    bool isConcurrentMode = false;
    bool isTunnel = true;
    avclock::ClockMode clockMode = avclock::ClockMode::AUDIO_MASTER;
    contentprimetype primetype = primevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (sequential, tunnel, secure)
*
* This test verifies the basic AVClock functionality for Audio Stream in VideoMaster clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 051@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Sequential_Tunnel_Secure_VideoMasterAudioOnly)
{
    const char *testName = "PrimeClock_VideoMasterAudioOnly";
    bool isSecureMode = true;
    bool isConcurrentMode = false;
    bool isTunnel = true;
    avclock::ClockMode clockMode = avclock::ClockMode::VIDEO_MASTER;
    contentprimetype primetype = primeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (sequential, tunnel, secure)
*
* This test verifies the basic AVClock functionality for PCR Stream in PCR clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 052@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Sequential_Tunnel_Secure_PCRMode)
{
    const char *testName = "PrimeClock_PCRMode";
    bool isSecureMode = true;
    bool isConcurrentMode = false;
    bool isTunnel = true;
    avclock::ClockMode clockMode = avclock::ClockMode::PCR;
    contentprimetype primetype = primepcr; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (sequential, tunnel, secure)
*
* This test verifies the basic AVClock functionality for Audio, Video Streams in Auto clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 053@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Sequential_Tunnel_Secure_AutoAudioBeforeVideo)
{
    const char *testName = "PrimeClock_AutoAudioBeforeVideo";
    bool isSecureMode = true;
    bool isConcurrentMode = false;
    bool isTunnel = true;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primeaudiobeforevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (sequential, tunnel, secure)
*
* This test verifies the basic AVClock functionality for Video, Audio Streams in Auto clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 054@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Sequential_Tunnel_Secure_AutoVideoBeforeAudio)
{
    const char *testName = "PrimeClock_AutoVideoBeforeAudio";
    bool isSecureMode = true;
    bool isConcurrentMode = false;
    bool isTunnel = true;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primevideobeforeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (sequential, tunnel, secure)
*
* This test verifies the basic AVClock Playback Speed functionality for Audio Streams in Auto clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 055@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Sequential_Tunnel_Secure_AutoSetAudioPlayRateNormalSpeed)
{
    const char *testName = "PrimeClock_AutoSetAudioPlayRateNormalSpeed";
    bool isSecureMode = true;
    bool isConcurrentMode = false;
    bool isTunnel = true;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (sequential, tunnel, secure)
*
* This test verifies the basic AVClock Playback Pause functionality for Audio Streams in Auto clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 056@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Sequential_Tunnel_Secure_AutoSetAudioPlayRatePause)
{
    const char *testName = "PrimeClock_AutoSetAudioPlayRatePause";
    bool isSecureMode = true;
    bool isConcurrentMode = false;
    bool isTunnel = true;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (sequential, tunnel, secure)
*
* This test verifies the basic AVClock Playback FastForward functionality for Audio Streams in Auto clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 057@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Sequential_Tunnel_Secure_AutoSetAudioPlayRateFastForward)
{
    const char *testName = "PrimeClock_AutoSetAudioPlayRateFastForward";
    bool isSecureMode = true;
    bool isConcurrentMode = false;
    bool isTunnel = true;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (sequential, tunnel, secure)
*
* This test verifies the basic AVClock Playback Speed functionality for Video Streams in Auto clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 058@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Sequential_Tunnel_Secure_AutoSetVideoPlayRateNormalSpeed)
{
    const char *testName = "PrimeClock_AutoSetVideoPlayRateNormalSpeed";
    bool isSecureMode = true;
    bool isConcurrentMode = false;
    bool isTunnel = true;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (sequential, tunnel, secure)
*
* This test verifies the basic AVClock Playback Pause functionality for Video Streams in Auto clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 059@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Sequential_Tunnel_Secure_AutoSetVideoPlayRatePause)
{
    const char *testName = "PrimeClock_AutoSetVideoPlayRatePause";
    bool isSecureMode = true;
    bool isConcurrentMode = false;
    bool isTunnel = true;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (sequential, tunnel, secure)
*
* This test verifies the basic AVClock Playback FastForward functionality for Video Streams in Auto clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 060@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Sequential_Tunnel_Secure_AutoSetVideoPlayRateFastForward)
{
    const char *testName = "PrimeClock_AutoSetVideoPlayRateFastForward";
    bool isSecureMode = true;
    bool isConcurrentMode = false;
    bool isTunnel = true;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (concurrent, non-tunnel, non-secure)
*
* This test verifies the basic AVClock functionality for Audio Stream in Auto clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 061@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Concurrent_NonTunnel_NonSecure_AutoAudioOnly)
{
    const char *testName = "PrimeClock_AutoAudioOnly";
    bool isSecureMode = false;
    bool isConcurrentMode = true;
    bool isTunnel = false;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (concurrent, non-tunnel, non-secure)
*
* This test verifies the basic AVClock functionality for Video Stream in Auto clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 062@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Concurrent_NonTunnel_NonSecure_AutoVideoOnly)
{
    const char *testName = "PrimeClock_AutoVideoOnly";
    bool isSecureMode = false;
    bool isConcurrentMode = true;
    bool isTunnel = false;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (concurrent, non-tunnel, non-secure)
*
* This test verifies the basic AVClock functionality for Audio Stream in AudioMaster clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 063@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Concurrent_NonTunnel_NonSecure_AudioMasterAudioOnly)
{
    const char *testName = "PrimeClock_AudioMasterAudioOnly";
    bool isSecureMode = false;
    bool isConcurrentMode = true;
    bool isTunnel = false;
    avclock::ClockMode clockMode = avclock::ClockMode::AUDIO_MASTER;
    contentprimetype primetype = primeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (concurrent, non-tunnel, non-secure)
*
* This test verifies the basic AVClock functionality for Video Stream in VideoMaster clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 064@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Concurrent_NonTunnel_NonSecure_VideoMasterVideoOnly)
{
    const char *testName = "PrimeClock_VideoMasterVideoOnly";
    bool isSecureMode = false;
    bool isConcurrentMode = true;
    bool isTunnel = false;
    avclock::ClockMode clockMode = avclock::ClockMode::VIDEO_MASTER;
    contentprimetype primetype = primevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (concurrent, non-tunnel, non-secure)
*
* This test verifies the basic AVClock functionality for Video Stream in AudioMaster clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 065@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Concurrent_NonTunnel_NonSecure_AudioMasterVideoOnly)
{
    const char *testName = "PrimeClock_AudioMasterVideoOnly";
    bool isSecureMode = false;
    bool isConcurrentMode = true;
    bool isTunnel = false;
    avclock::ClockMode clockMode = avclock::ClockMode::AUDIO_MASTER;
    contentprimetype primetype = primevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (concurrent, non-tunnel, non-secure)
*
* This test verifies the basic AVClock functionality for Audio Stream in VideoMaster clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 066@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Concurrent_NonTunnel_NonSecure_VideoMasterAudioOnly)
{
    const char *testName = "PrimeClock_VideoMasterAudioOnly";
    bool isSecureMode = false;
    bool isConcurrentMode = true;
    bool isTunnel = false;
    avclock::ClockMode clockMode = avclock::ClockMode::VIDEO_MASTER;
    contentprimetype primetype = primeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (concurrent, non-tunnel, non-secure)
*
* This test verifies the basic AVClock functionality for PCR Stream in PCR clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 067@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Concurrent_NonTunnel_NonSecure_PCRMode)
{
    const char *testName = "PrimeClock_PCRMode";
    bool isSecureMode = false;
    bool isConcurrentMode = true;
    bool isTunnel = false;
    avclock::ClockMode clockMode = avclock::ClockMode::PCR;
    contentprimetype primetype = primepcr; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (concurrent, non-tunnel, non-secure)
*
* This test verifies the basic AVClock functionality for Audio, Video Streams in Auto clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 068@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Concurrent_NonTunnel_NonSecure_AutoAudioBeforeVideo)
{
    const char *testName = "PrimeClock_AutoAudioBeforeVideo";
    bool isSecureMode = false;
    bool isConcurrentMode = true;
    bool isTunnel = false;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primeaudiobeforevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (concurrent, non-tunnel, non-secure)
*
* This test verifies the basic AVClock functionality for Video, Audio Streams in Auto clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 069@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Concurrent_NonTunnel_NonSecure_AutoVideoBeforeAudio)
{
    const char *testName = "PrimeClock_AutoVideoBeforeAudio";
    bool isSecureMode = false;
    bool isConcurrentMode = true;
    bool isTunnel = false;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primevideobeforeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (concurrent, non-tunnel, non-secure)
*
* This test verifies the basic AVClock Playback Speed functionality for Audio Streams in Auto clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 070@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Concurrent_NonTunnel_NonSecure_AutoSetAudioPlayRateNormalSpeed)
{
    const char *testName = "PrimeClock_AutoSetAudioPlayRateNormalSpeed";
    bool isSecureMode = false;
    bool isConcurrentMode = true;
    bool isTunnel = false;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (concurrent, non-tunnel, non-secure)
*
* This test verifies the basic AVClock Playback Pause functionality for Audio Streams in Auto clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 071@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Concurrent_NonTunnel_NonSecure_AutoSetAudioPlayRatePause)
{
    const char *testName = "PrimeClock_AutoSetAudioPlayRatePause";
    bool isSecureMode = false;
    bool isConcurrentMode = true;
    bool isTunnel = false;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (concurrent, non-tunnel, non-secure)
*
* This test verifies the basic AVClock Playback FastForward functionality for Audio Streams in Auto clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 072@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Concurrent_NonTunnel_NonSecure_AutoSetAudioPlayRateFastForward)
{
    const char *testName = "PrimeClock_AutoSetAudioPlayRateFastForward";
    bool isSecureMode = false;
    bool isConcurrentMode = true;
    bool isTunnel = false;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (concurrent, non-tunnel, non-secure)
*
* This test verifies the basic AVClock Playback Speed functionality for Video Streams in Auto clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 073@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Concurrent_NonTunnel_NonSecure_AutoSetVideoPlayRateNormalSpeed)
{
    const char *testName = "PrimeClock_AutoSetVideoPlayRateNormalSpeed";
    bool isSecureMode = false;
    bool isConcurrentMode = true;
    bool isTunnel = false;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (concurrent, non-tunnel, non-secure)
*
* This test verifies the basic AVClock Playback Pause functionality for Video Streams in Auto clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 074@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Concurrent_NonTunnel_NonSecure_AutoSetVideoPlayRatePause)
{
    const char *testName = "PrimeClock_AutoSetVideoPlayRatePause";
    bool isSecureMode = false;
    bool isConcurrentMode = true;
    bool isTunnel = false;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (concurrent, non-tunnel, non-secure)
*
* This test verifies the basic AVClock Playback FastForward functionality for Video Streams in Auto clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 075@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Concurrent_NonTunnel_NonSecure_AutoSetVideoPlayRateFastForward)
{
    const char *testName = "PrimeClock_AutoSetVideoPlayRateFastForward";
    bool isSecureMode = false;
    bool isConcurrentMode = true;
    bool isTunnel = false;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (concurrent, non-tunnel, secure)
*
* This test verifies the basic AVClock functionality for Audio Stream in Auto clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 076@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Concurrent_NonTunnel_Secure_AutoAudioOnly)
{
    const char *testName = "PrimeClock_AutoAudioOnly";
    bool isSecureMode = true;
    bool isConcurrentMode = true;
    bool isTunnel = false;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (concurrent, non-tunnel, secure)
*
* This test verifies the basic AVClock functionality for Video Stream in Auto clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 077@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Concurrent_NonTunnel_Secure_AutoVideoOnly)
{
    const char *testName = "PrimeClock_AutoVideoOnly";
    bool isSecureMode = true;
    bool isConcurrentMode = true;
    bool isTunnel = false;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (concurrent, non-tunnel, secure)
*
* This test verifies the basic AVClock functionality for Audio Stream in AudioMaster clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 078@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Concurrent_NonTunnel_Secure_AudioMasterAudioOnly)
{
    const char *testName = "PrimeClock_AudioMasterAudioOnly";
    bool isSecureMode = true;
    bool isConcurrentMode = true;
    bool isTunnel = false;
    avclock::ClockMode clockMode = avclock::ClockMode::AUDIO_MASTER;
    contentprimetype primetype = primeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (concurrent, non-tunnel, secure)
*
* This test verifies the basic AVClock functionality for Video Stream in VideoMaster clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 079@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Concurrent_NonTunnel_Secure_VideoMasterVideoOnly)
{
    const char *testName = "PrimeClock_VideoMasterVideoOnly";
    bool isSecureMode = true;
    bool isConcurrentMode = true;
    bool isTunnel = false;
    avclock::ClockMode clockMode = avclock::ClockMode::VIDEO_MASTER;
    contentprimetype primetype = primevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (concurrent, non-tunnel, secure)
*
* This test verifies the basic AVClock functionality for Video Stream in AudioMaster clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 080@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Concurrent_NonTunnel_Secure_AudioMasterVideoOnly)
{
    const char *testName = "PrimeClock_AudioMasterVideoOnly";
    bool isSecureMode = true;
    bool isConcurrentMode = true;
    bool isTunnel = false;
    avclock::ClockMode clockMode = avclock::ClockMode::AUDIO_MASTER;
    contentprimetype primetype = primevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (concurrent, non-tunnel, secure)
*
* This test verifies the basic AVClock functionality for Audio Stream in VideoMaster clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 081@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Concurrent_NonTunnel_Secure_VideoMasterAudioOnly)
{
    const char *testName = "PrimeClock_VideoMasterAudioOnly";
    bool isSecureMode = true;
    bool isConcurrentMode = true;
    bool isTunnel = false;
    avclock::ClockMode clockMode = avclock::ClockMode::VIDEO_MASTER;
    contentprimetype primetype = primeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (concurrent, non-tunnel, secure)
*
* This test verifies the basic AVClock functionality for PCR Stream in PCR clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 082@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Concurrent_NonTunnel_Secure_PCRMode)
{
    const char *testName = "PrimeClock_PCRMode";
    bool isSecureMode = true;
    bool isConcurrentMode = true;
    bool isTunnel = false;
    avclock::ClockMode clockMode = avclock::ClockMode::PCR;
    contentprimetype primetype = primepcr; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (concurrent, non-tunnel, secure)
*
* This test verifies the basic AVClock functionality for Audio, Video Streams in Auto clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 083@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Concurrent_NonTunnel_Secure_AutoAudioBeforeVideo)
{
    const char *testName = "PrimeClock_AutoAudioBeforeVideo";
    bool isSecureMode = true;
    bool isConcurrentMode = true;
    bool isTunnel = false;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primeaudiobeforevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (concurrent, non-tunnel, secure)
*
* This test verifies the basic AVClock functionality for Video, Audio Streams in Auto clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 084@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Concurrent_NonTunnel_Secure_AutoVideoBeforeAudio)
{
    const char *testName = "PrimeClock_AutoVideoBeforeAudio";
    bool isSecureMode = true;
    bool isConcurrentMode = true;
    bool isTunnel = false;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primevideobeforeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (concurrent, non-tunnel, secure)
*
* This test verifies the basic AVClock Playback Speed functionality for Audio Streams in Auto clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 085@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Concurrent_NonTunnel_Secure_AutoSetAudioPlayRateNormalSpeed)
{
    const char *testName = "PrimeClock_AutoSetAudioPlayRateNormalSpeed";
    bool isSecureMode = true;
    bool isConcurrentMode = true;
    bool isTunnel = false;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (concurrent, non-tunnel, secure)
*
* This test verifies the basic AVClock Playback Pause functionality for Audio Streams in Auto clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 086@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Concurrent_NonTunnel_Secure_AutoSetAudioPlayRatePause)
{
    const char *testName = "PrimeClock_AutoSetAudioPlayRatePause";
    bool isSecureMode = true;
    bool isConcurrentMode = true;
    bool isTunnel = false;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (concurrent, non-tunnel, secure)
*
* This test verifies the basic AVClock Playback FastForward functionality for Audio Streams in Auto clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 087@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Concurrent_NonTunnel_Secure_AutoSetAudioPlayRateFastForward)
{
    const char *testName = "PrimeClock_AutoSetAudioPlayRateFastForward";
    bool isSecureMode = true;
    bool isConcurrentMode = true;
    bool isTunnel = false;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (concurrent, non-tunnel, secure)
*
* This test verifies the basic AVClock Playback Speed functionality for Video Streams in Auto clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 088@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Concurrent_NonTunnel_Secure_AutoSetVideoPlayRateNormalSpeed)
{
    const char *testName = "PrimeClock_AutoSetVideoPlayRateNormalSpeed";
    bool isSecureMode = true;
    bool isConcurrentMode = true;
    bool isTunnel = false;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (concurrent, non-tunnel, secure)
*
* This test verifies the basic AVClock Playback Pause functionality for Video Streams in Auto clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 089@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Concurrent_NonTunnel_Secure_AutoSetVideoPlayRatePause)
{
    const char *testName = "PrimeClock_AutoSetVideoPlayRatePause";
    bool isSecureMode = true;
    bool isConcurrentMode = true;
    bool isTunnel = false;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (concurrent, non-tunnel, secure)
*
* This test verifies the basic AVClock Playback FastForward functionality for Video Streams in Auto clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 090@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Concurrent_NonTunnel_Secure_AutoSetVideoPlayRateFastForward)
{
    const char *testName = "PrimeClock_AutoSetVideoPlayRateFastForward";
    bool isSecureMode = true;
    bool isConcurrentMode = true;
    bool isTunnel = false;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (concurrent, tunnel, non-secure)
*
* This test verifies the basic AVClock functionality for Audio Stream in Auto clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 091@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Concurrent_Tunnel_NonSecure_AutoAudioOnly)
{
    const char *testName = "PrimeClock_AutoAudioOnly";
    bool isSecureMode = false;
    bool isConcurrentMode = true;
    bool isTunnel = true;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (concurrent, tunnel, non-secure)
*
* This test verifies the basic AVClock functionality for Video Stream in Auto clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 092@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Concurrent_Tunnel_NonSecure_AutoVideoOnly)
{
    const char *testName = "PrimeClock_AutoVideoOnly";
    bool isSecureMode = false;
    bool isConcurrentMode = true;
    bool isTunnel = true;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (concurrent, tunnel, non-secure)
*
* This test verifies the basic AVClock functionality for Audio Stream in AudioMaster clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 093@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Concurrent_Tunnel_NonSecure_AudioMasterAudioOnly)
{
    const char *testName = "PrimeClock_AudioMasterAudioOnly";
    bool isSecureMode = false;
    bool isConcurrentMode = true;
    bool isTunnel = true;
    avclock::ClockMode clockMode = avclock::ClockMode::AUDIO_MASTER;
    contentprimetype primetype = primeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (concurrent, tunnel, non-secure)
*
* This test verifies the basic AVClock functionality for Video Stream in VideoMaster clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 094@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Concurrent_Tunnel_NonSecure_VideoMasterVideoOnly)
{
    const char *testName = "PrimeClock_VideoMasterVideoOnly";
    bool isSecureMode = false;
    bool isConcurrentMode = true;
    bool isTunnel = true;
    avclock::ClockMode clockMode = avclock::ClockMode::VIDEO_MASTER;
    contentprimetype primetype = primevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (concurrent, tunnel, non-secure)
*
* This test verifies the basic AVClock functionality for Video Stream in AudioMaster clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 095@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Concurrent_Tunnel_NonSecure_AudioMasterVideoOnly)
{
    const char *testName = "PrimeClock_AudioMasterVideoOnly";
    bool isSecureMode = false;
    bool isConcurrentMode = true;
    bool isTunnel = true;
    avclock::ClockMode clockMode = avclock::ClockMode::AUDIO_MASTER;
    contentprimetype primetype = primevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (concurrent, tunnel, non-secure)
*
* This test verifies the basic AVClock functionality for Audio Stream in VideoMaster clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 096@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Concurrent_Tunnel_NonSecure_VideoMasterAudioOnly)
{
    const char *testName = "PrimeClock_VideoMasterAudioOnly";
    bool isSecureMode = false;
    bool isConcurrentMode = true;
    bool isTunnel = true;
    avclock::ClockMode clockMode = avclock::ClockMode::VIDEO_MASTER;
    contentprimetype primetype = primeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (concurrent, tunnel, non-secure)
*
* This test verifies the basic AVClock functionality for PCR Stream in PCR clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 097@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Concurrent_Tunnel_NonSecure_PCRMode)
{
    const char *testName = "PrimeClock_PCRMode";
    bool isSecureMode = false;
    bool isConcurrentMode = true;
    bool isTunnel = true;
    avclock::ClockMode clockMode = avclock::ClockMode::PCR;
    contentprimetype primetype = primepcr; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (concurrent, tunnel, non-secure)
*
* This test verifies the basic AVClock functionality for Audio, Video Streams in Auto clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 0098@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Concurrent_Tunnel_NonSecure_AutoAudioBeforeVideo)
{
    const char *testName = "PrimeClock_AutoAudioBeforeVideo";
    bool isSecureMode = false;
    bool isConcurrentMode = true;
    bool isTunnel = true;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primeaudiobeforevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (concurrent, tunnel, non-secure)
*
* This test verifies the basic AVClock functionality for Video, Audio Streams in Auto clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 0099@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Concurrent_Tunnel_NonSecure_AutoVideoBeforeAudio)
{
    const char *testName = "PrimeClock_AutoVideoBeforeAudio";
    bool isSecureMode = false;
    bool isConcurrentMode = true;
    bool isTunnel = true;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primevideobeforeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (concurrent, tunnel, non-secure)
*
* This test verifies the basic AVClock Playback Speed functionality for Audio Streams in Auto clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 0100@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Concurrent_Tunnel_NonSecure_AutoSetAudioPlayRateNormalSpeed)
{
    const char *testName = "PrimeClock_AutoSetAudioPlayRateNormalSpeed";
    bool isSecureMode = false;
    bool isConcurrentMode = true;
    bool isTunnel = true;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (concurrent, tunnel, non-secure)
*
* This test verifies the basic AVClock Playback Pause functionality for Audio Streams in Auto clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 101@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Concurrent_Tunnel_NonSecure_AutoSetAudioPlayRatePause)
{
    const char *testName = "PrimeClock_AutoSetAudioPlayRatePause";
    bool isSecureMode = false;
    bool isConcurrentMode = true;
    bool isTunnel = true;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (concurrent, tunnel, non-secure)
*
* This test verifies the basic AVClock Playback FastForward functionality for Audio Streams in Auto clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 102@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Concurrent_Tunnel_NonSecure_AutoSetAudioPlayRateFastForward)
{
    const char *testName = "PrimeClock_AutoSetAudioPlayRateFastForward";
    bool isSecureMode = false;
    bool isConcurrentMode = true;
    bool isTunnel = true;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (concurrent, tunnel, non-secure)
*
* This test verifies the basic AVClock Playback Speed functionality for Video Streams in Auto clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 103@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, concurrent_Tunnel_NonSecure_AutoSetVideoPlayRateNormalSpeed)
{
    const char *testName = "PrimeClock_AutoSetVideoPlayRateNormalSpeed";
    bool isSecureMode = false;
    bool isConcurrentMode = true;
    bool isTunnel = true;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (concurrent, tunnel, non-secure)
*
* This test verifies the basic AVClock Playback Pause functionality for Video Streams in Auto clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 104@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Concurrent_Tunnel_NonSecure_AutoSetVideoPlayRatePause)
{
    const char *testName = "PrimeClock_AutoSetVideoPlayRatePause";
    bool isSecureMode = false;
    bool isConcurrentMode = true;
    bool isTunnel = true;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (concurrent, tunnel, non-secure)
*
* This test verifies the basic AVClock Playback FastForward functionality for Video Streams in Auto clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 105@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Concurrent_Tunnel_NonSecure_AutoSetVideoPlayRateFastForward)
{
    const char *testName = "PrimeClock_AutoSetVideoPlayRateFastForward";
    bool isSecureMode = false;
    bool isConcurrentMode = true;
    bool isTunnel = true;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (concurrent, tunnel, secure)
*
* This test verifies the basic AVClock functionality for Audio Stream in Auto clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 106@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Concurrent_Tunnel_Secure_AutoAudioOnly)
{
    const char *testName = "PrimeClock_AutoAudioOnly";
    bool isSecureMode = true;
    bool isConcurrentMode = true;
    bool isTunnel = true;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (concurrent, tunnel, secure)
*
* This test verifies the basic AVClock functionality for Video Stream in Auto clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 107@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Concurrent_Tunnel_Secure_AutoVideoOnly)
{
    const char *testName = "PrimeClock_AutoVideoOnly";
    bool isSecureMode = true;
    bool isConcurrentMode = true;
    bool isTunnel = true;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (concurrent, tunnel, secure)
*
* This test verifies the basic AVClock functionality for Audio Stream in AudioMaster clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 108@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Concurrent_Tunnel_Secure_AudioMasterAudioOnly)
{
    const char *testName = "PrimeClock_AudioMasterAudioOnly";
    bool isSecureMode = true;
    bool isConcurrentMode = true;
    bool isTunnel = true;
    avclock::ClockMode clockMode = avclock::ClockMode::AUDIO_MASTER;
    contentprimetype primetype = primeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (concurrent, tunnel, secure)
*
* This test verifies the basic AVClock functionality for Video Stream in VideoMaster clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 109@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Concurrent_Tunnel_Secure_VideoMasterVideoOnly)
{
    const char *testName = "PrimeClock_VideoMasterVideoOnly";
    bool isSecureMode = true;
    bool isConcurrentMode = true;
    bool isTunnel = true;
    avclock::ClockMode clockMode = avclock::ClockMode::VIDEO_MASTER;
    contentprimetype primetype = primevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (concurrent, tunnel, secure)
*
* This test verifies the basic AVClock functionality for Video Stream in AudioMaster clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 110@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Concurrent_Tunnel_Secure_AudioMasterVideoOnly)
{
    const char *testName = "PrimeClock_AudioMasterVideoOnly";
    bool isSecureMode = true;
    bool isConcurrentMode = true;
    bool isTunnel = true;
    avclock::ClockMode clockMode = avclock::ClockMode::AUDIO_MASTER;
    contentprimetype primetype = primevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (concurrent, tunnel, secure)
*
* This test verifies the basic AVClock functionality for Audio Stream in VideoMaster clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 111@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Concurrent_Tunnel_Secure_VideoMasterAudioOnly)
{
    const char *testName = "PrimeClock_VideoMasterAudioOnly";
    bool isSecureMode = true;
    bool isConcurrentMode = true;
    bool isTunnel = true;
    avclock::ClockMode clockMode = avclock::ClockMode::VIDEO_MASTER;
    contentprimetype primetype = primeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (concurrent, tunnel, secure)
*
* This test verifies the basic AVClock functionality for PCR Stream in PCR clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 112@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Concurrent_Tunnel_Secure_PCRMode)
{
    const char *testName = "PrimeClock_PCRMode";
    bool isSecureMode = true;
    bool isConcurrentMode = true;
    bool isTunnel = true;
    avclock::ClockMode clockMode = avclock::ClockMode::PCR;
    contentprimetype primetype = primepcr; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (concurrent, tunnel, secure)
*
* This test verifies the basic AVClock functionality for Audio, Video Streams in Auto clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 113@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Concurrent_Tunnel_Secure_AutoAudioBeforeVideo)
{
    const char *testName = "PrimeClock_AutoAudioBeforeVideo";
    bool isSecureMode = true;
    bool isConcurrentMode = true;
    bool isTunnel = true;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primeaudiobeforevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (concurrent, tunnel, secure)
*
* This test verifies the basic AVClock functionality for Video, Audio Streams in Auto clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 114@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Concurrent_Tunnel_Secure_AutoVideoBeforeAudio)
{
    const char *testName = "PrimeClock_AutoVideoBeforeAudio";
    bool isSecureMode = true;
    bool isConcurrentMode = true;
    bool isTunnel = true;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primevideobeforeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (concurrent, tunnel, secure)
*
* This test verifies the basic AVClock Playback Speed functionality for Audio Streams in Auto clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 115@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Concurrent_Tunnel_Secure_AutoSetAudioPlayRateNormalSpeed)
{
    const char *testName = "PrimeClock_AutoSetAudioPlayRateNormalSpeed";
    bool isSecureMode = true;
    bool isConcurrentMode = true;
    bool isTunnel = true;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (concurrent, tunnel, secure)
*
* This test verifies the basic AVClock Playback Pause functionality for Audio Streams in Auto clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 116@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Concurrent_Tunnel_Secure_AutoSetAudioPlayRatePause)
{
    const char *testName = "PrimeClock_AutoSetAudioPlayRatePause";
    bool isSecureMode = true;
    bool isConcurrentMode = true;
    bool isTunnel = true;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (concurrent, tunnel, secure)
*
* This test verifies the basic AVClock Playback FastForward functionality for Audio Streams in Auto clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 117@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Concurrent_Tunnel_Secure_AutoSetAudioPlayRateFastForward)
{
    const char *testName = "PrimeClock_AutoSetAudioPlayRateFastForward";
    bool isSecureMode = true;
    bool isConcurrentMode = true;
    bool isTunnel = true;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (concurrent, tunnel, secure)
*
* This test verifies the basic AVClock Playback Speed functionality for Video Streams in Auto clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 118@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Concurrent_Tunnel_Secure_AutoSetVideoPlayRateNormalSpeed)
{
    const char *testName = "PrimeClock_AutoSetVideoPlayRateNormalSpeed";
    bool isSecureMode = true;
    bool isConcurrentMode = true;
    bool isTunnel = true;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (concurrent, tunnel, secure)
*
* This test verifies the basic AVClock Playback Pause functionality for Video Streams in Auto clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 119@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Concurrent_Tunnel_Secure_AutoSetVideoPlayRatePause)
{
    const char *testName = "PrimeClock_AutoSetVideoPlayRatePause";
    bool isSecureMode = true;
    bool isConcurrentMode = true;
    bool isTunnel = true;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}

/**
* @brief Test the AVClock functionality (concurrent, tunnel, secure)
*
* This test verifies the basic AVClock Playback FastForward functionality for Video Streams in Auto clock Mode.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 120@n
*
* **Test Procedure:**
* Refer to UT specification documentation [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L3_AVClock, Concurrent_Tunnel_Secure_AutoSetVideoPlayRateFastForward)
{
    const char *testName = "PrimeClock_AutoSetVideoPlayRateFastForward";
    bool isSecureMode = true;
    bool isConcurrentMode = true;
    bool isTunnel = true;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype);
}
/** @} */ // End of AUDIO_SINK_HALTEST L3 File
/** @} */ // End of AUDIO_SINK_HALTEST
/** @} */ // End of AUDIO_SINK Module
/** @} */ // End of HPK
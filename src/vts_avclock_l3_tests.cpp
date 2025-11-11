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
#include "vts_avbuffer.h"

#include "vts_videosink_helper_functions.h"
#include "vts_videosink_parse_hfp.h"
#include "vts_videosink_parse_testconfig.h"

#include "vts_avclock_parse_testconfig.h"
#include "vts_avclock_parse_hfp.h"

#include "vts_avclock.h"

extern const char *gAudioModuleName;

static const char *gModuleName = "IAVClockManager";
static const char *gAudioSinkModuleName = "IAudioSinkManager";
static const char *gVideoDecoderModuleName = "IVideoDecoderManager";
static const char *gVideoSinkModuleName = "IVideoSinkManager";
static const char *gTestGroup = "L3";

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

class VTS_L3_AVClock : public UTCore
{
  public:
    static std::vector<std::tuple<IAVClock::Id, sp<IAVClock>, sp<AVClockControllerListener>, sp<AVClockEventListener>>> mAVClockList;
    static std::vector<std::tuple<IAudioDecoder::Id, sp<IAudioDecoder>, sp<AudioDecoderControllerListener>, sp<AudioDecoderEventListener>, audiodecoder::Capabilities*>> mAudioDecoderList;
    static std::vector<std::tuple<IVideoDecoder::Id, sp<IVideoDecoder>, sp<VideoDecoderControllerListener>, sp<VideoDecoderEventListener>, videodecoder::Capabilities*>> mVideoDecoderList;
    static std::vector<std::tuple<IAudioSink::Id, sp<IAudioSink>, sp<AudioSinkControllerListener>, sp<AudioSinkEventListener>>> mAudioSinkList;
    static std::vector<std::tuple<IVideoSink::Id, sp<IVideoSink>, sp<VideoSinkControllerListener>, sp<VideoSinkEventListener>>> mVideoSinkList;
    static sp<IAVBuffer> mAVBufferManager;
    static IAVBufferHelper* mAVbufferHelper;

    VTS_L3_AVClock() : UTCore() {};

    ~VTS_L3_AVClock() override = default;

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

            controlListener->setDecoderID(_audioDecoderId.value);
            eventListener->setDecoderID(_audioDecoderId.value);

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
            UT_LOG_INFO("Added decoder ID %d to test list",  _videoDecoderId.value);
        }

        for(const auto& _videoSinkId: videoSinkIds)
        {
            sp<IVideoSink> videoSink;

            sp<VideoSinkEventListener> videoSinkEventListener = new VideoSinkEventListener();

            sp<VideoSinkControllerListener> videoSinkControlListener = new VideoSinkControllerListener();

            videoSinkControlListener->setSinkID(_videoSinkId.value);
            videoSinkEventListener->setSinkID(_videoSinkId.value);

            /* Get Video Sink handle */
            VIDEOSINK_MANAGER_GETVIDEOSINK(_videoSinkId, videoSink);

            /* Register events */
            VIDEOSINK_REGISTER_EVENT_SUCCESS(videoSink, videoSinkEventListener);

            //TODO: VideoSink Get capabilities are yet to implement in the video sink

            mVideoSinkList.push_back({_videoSinkId, videoSink, videoSinkControlListener, videoSinkEventListener});
        }
    }
    
    void TearDown() override
    {
        for (const auto& [_avClockId, avClockHandle, controlListener, eventListener] : mAVClockList)
        {
            /* Un-register the events */
            AVCLOCK_UNREGISTER_EVENT_SUCCESS(avClockHandle, eventListener);
        }
        mAVClockList.clear();
        mAudioDecoderList.clear();
        mVideoDecoderList.clear();
        mAudioSinkList.clear();
        mVideoSinkList.clear();
        //TODO: check right place to call joinThreadPool
        //android::IPCThreadState::self()->joinThreadPool();
    }
};

std::vector<std::tuple<IAVClock::Id, sp<IAVClock>, sp<AVClockControllerListener>, sp<AVClockEventListener>>> VTS_L3_AVClock::mAVClockList;
std::vector<std::tuple<IAudioDecoder::Id, sp<IAudioDecoder>, sp<AudioDecoderControllerListener>, sp<AudioDecoderEventListener>, audiodecoder::Capabilities*>> VTS_L3_AVClock::mAudioDecoderList;
std::vector<std::tuple<IVideoDecoder::Id, sp<IVideoDecoder>, sp<VideoDecoderControllerListener>, sp<VideoDecoderEventListener>, videodecoder::Capabilities*>> VTS_L3_AVClock::mVideoDecoderList;
std::vector<std::tuple<IAudioSink::Id, sp<IAudioSink>, sp<AudioSinkControllerListener>, sp<AudioSinkEventListener>>> VTS_L3_AVClock::mAudioSinkList;
std::vector<std::tuple<IVideoSink::Id, sp<IVideoSink>, sp<VideoSinkControllerListener>, sp<VideoSinkEventListener>>> VTS_L3_AVClock::mVideoSinkList;
sp<IAVBuffer> VTS_L3_AVClock::mAVBufferManager;
IAVBufferHelper* VTS_L3_AVClock::mAVbufferHelper;

std::atomic<bool> threadStarted(false);

/* Test class registration with UT_Core */
UT_ADD_TEST_TO_GROUP(VTS_L3_AVClock, UT_TESTS_L3)

class AVClockBufferSpaceListener : public BnAVBufferSpaceListener
{
public:

    AVClockBufferSpaceListener() {};
    virtual ~AVClockBufferSpaceListener() = default;

    Status onSpaceAvailable() override
    {
        return Status::ok();
    }
};
    
void checkCompleteStateTransition(sp<IAVClock> avClock, sp<AVClockEventListener> eventListener,
                                      sp<AVClockControllerListener> controlListener, State oldState,
                                      State intermediateState, State newState)
    {
        ::com::rdk::hal::State currentState;

        UT_ASSERT_TRUE(eventListener->checkStateTransition(oldState, intermediateState));
        UT_ASSERT_TRUE(controlListener->checkStateTransition(oldState, intermediateState));

        eventListener->waitForState(newState, std::chrono::milliseconds(MAX_FINAL_STATE_WAIT_MS));
        controlListener->waitForState(newState, std::chrono::milliseconds(MAX_FINAL_STATE_WAIT_MS));

        AVCLOCK_GET_STATE(avClock, currentState);

        UT_ASSERT_EQUAL(currentState, newState);

        UT_ASSERT_TRUE(eventListener->checkStateTransition(intermediateState, newState));
        UT_ASSERT_TRUE(controlListener->checkStateTransition(intermediateState, newState));

        eventListener->resetStateChanges();
        controlListener->resetStateChanges();
    }

   /* bool isClockUsable(const IAVClock::Id& clockId)
    {
        for (const auto& item : gAVClock_hfp)
        {
            if (std::find(item.begin(), item.availableIds.end(), clockId.value) != item.availableIds.end())
            {
                return true;
            }
        }
        return false;
    } */

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
                     bool audioPrimeSecDecode, bool isTunnel, testConfiguration_t audioPrimeconfig, double playbackRate)
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
        mplaybackRate = playbackRate;
    }
    void run()
    {
        audioPrimeClockTest();
    }

private:
        IAVClock::Id maudioPrimeClockId;
        sp<IAVClock> maudioPrimeClockHandle;
        avclock::ClockMode maudioPrimeClockMode;
        sp<AVClockControllerListener> maudioPrimeClckController;
        IAudioDecoder::Id maudioDecId;
        sp<IAudioDecoder> maudioDecHandle;
        sp<AudioDecoderControllerListener> maudioDecControlListener;
        sp<AudioDecoderEventListener> maudioDecEventListener;
        IAudioSink::Id maudioSinkId;
        sp<IAudioSink> maudioSinkHandle;
        sp<AudioSinkControllerListener> maudioSinkControlListener;
        sp<AudioSinkEventListener> maudioSinkEventListener;
        bool maudioPrimeSecureDecode;
        testConfiguration_t maudioPrimeTestConfig;
        bool maudioTunnel;
        sp<IAVBuffer> mAVBufferManager;
        IAVBufferHelper* mAVbufferHelper;
        double mplaybackRate;

    void audioPrimeClockTest()
    {
        try {
                sp<com::rdk::hal::avclock::IAVClockController> avClockController;
                bool foundDecoder = false;
                
                /* Open the AVClock */
                AVCLOCK_OPEN_SUCCESS(maudioPrimeClockHandle, maudioPrimeClckController, avClockController, true);

                /* Set the AVClock Mode */
                AVCLOCK_SET_CLOCK_MODE_SUCCESS(maudioPrimeClockHandle, avClockController, maudioPrimeClockMode, true);
     
                /* Start the AVClock */
                AVCLOCK_START_SUCCESS(maudioPrimeClockHandle, avClockController, true);

                /*Set the Sink*/
                AVCLOCK_SET_AUDIO_SINK_SUCCESS(maudioPrimeClockHandle, avClockController, maudioSinkId, true);

                /*set the Play Back Rate*/
                AVCLOCK_SET_PLAYBACK_RATE_SUCCESS(maudioPrimeClockHandle, avClockController, mplaybackRate, true);


                if (!isClkSinkUsable(maudioSinkId))
                {
                    UT_LOG_INFO("[SinkID: %d] is not usable for this test configuration, skipping.", maudioSinkId.value);
                    return;
                }

                if (isClkDecoderUsable(maudioDecId))
                {
                    foundDecoder = true;
                }

                if (!foundDecoder)
                {
                    UT_LOG_WARNING("No usable decoder found for Sink ID %d, skipping.", maudioSinkId);
                    return;
                }
                
                com::rdk::hal::avclock::ClockTime clockBeforeDecoder , clockAfterDecoder, clockAfterSink;

                AVCLOCK_GET_CURRENT_CLOCK_TIME_SUCCESS(maudioPrimeClockHandle, avClockController, clockBeforeDecoder);

                HeapMetrics beforeHeapMetrics, afterHeapMetrics;
                AVBUFFER_GETMETRICS(mAVBufferManager, maudioPrimeSecureDecode, beforeHeapMetrics);

                AudioDecoderTest decodeTest(maudioDecId, maudioDecHandle, maudioDecControlListener,
                                    maudioDecEventListener, mAVBufferManager, mAVbufferHelper,  maudioPrimeSecureDecode, maudioPrimeTestConfig);

                AVCLOCK_GET_CURRENT_CLOCK_TIME_SUCCESS(maudioPrimeClockHandle, avClockController, clockAfterDecoder);

                std::thread decodeThread(&AudioDecoderTest::run, &decodeTest, false);

                AudioSinkTest audioSinkTest(maudioSinkId, maudioSinkHandle, maudioSinkControlListener, maudioSinkEventListener,  maudioDecControlListener, mAVBufferManager, maudioDecId, maudioPrimeSecureDecode, maudioTunnel, maudioPrimeTestConfig);

                std::this_thread::sleep_for(std::chrono::milliseconds(500));
                audioSinkTest.run();

                AVCLOCK_GET_CURRENT_CLOCK_TIME_SUCCESS(maudioPrimeClockHandle, avClockController, clockAfterSink);

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
                     sp<AVClockControllerListener> videoPrimeClkController, IVideoDecoder::Id videoDecId, sp<IVideoDecoder> videoDecHandle,
                     sp<VideoDecoderControllerListener> videoDecControlListener,
                     sp<VideoDecoderEventListener> videoDecEventListener,IVideoSink::Id videoSinkId, 
                     sp<IVideoSink> videoSinkHandle,
                     sp<VideoSinkControllerListener> videoSinkControlListener,
                     sp<VideoSinkEventListener> videoSinkEventListener,
                     sp<IAVBuffer> AVBufferManager, IAVBufferHelper* avbufferHelper,
                     bool videoPrimeSecDecode, bool isTunnel, videoDecoderTestConfiguration_t videoDecTestconfig, videoTestConfiguration_t VideoSinkConfig, double playbackRate)
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
        mvideoDecTestConfig = videoDecTestconfig;
        mvideoSinkConfig = VideoSinkConfig;
        mplaybackRate = playbackRate;
    }
    void run()
    {
        videoPrimeClockTest();
    }

private:
        IAVClock::Id mvideoPrimeClockId;
        sp<IAVClock> mvideoPrimeClockHandle;
        avclock::ClockMode mvideoPrimeClockMode;
        sp<AVClockControllerListener> mvideoPrimeClckController;
        IVideoDecoder::Id mvideoDecId;
        sp<IVideoDecoder> mvideoDecHandle;
        sp<VideoDecoderControllerListener> mvideoDecControlListener;
        sp<VideoDecoderEventListener> mvideoDecEventListener;
        IVideoSink::Id mvideoSinkId;
        sp<IVideoSink> mvideoSinkHandle;
        sp<VideoSinkControllerListener> mvideoSinkControlListener;
        sp<VideoSinkEventListener> mvideoSinkEventListener;
        bool mvideoPrimeSecureDecode;
        bool mvideoTunnel;
        sp<IAVBuffer> mAVBufferManager;
        IAVBufferHelper* mAVbufferHelper;
        videoDecoderTestConfiguration_t mvideoDecTestConfig;
        videoTestConfiguration_t mvideoSinkConfig;
        double mplaybackRate;

    void videoPrimeClockTest()
    {
        try {
                android::sp<com::rdk::hal::avclock::IAVClockController> avClockController;
                bool foundDecoder = false;

                /* Open the AVClock */
                AVCLOCK_OPEN_SUCCESS(mvideoPrimeClockHandle, mvideoPrimeClckController, avClockController, true);

                /* Set the AVClock Mode */
                AVCLOCK_SET_CLOCK_MODE_SUCCESS(mvideoPrimeClockHandle, avClockController, mvideoPrimeClockMode, true);
     
                /* Start the AVClock */
                AVCLOCK_START_SUCCESS(mvideoPrimeClockHandle, avClockController, true);

                /*Set the Sink*/
                AVCLOCK_SET_VIDEO_SINK_SUCCESS(mvideoPrimeClockHandle, avClockController, mvideoSinkId, true);

                /*set the Play Back Rate*/
                AVCLOCK_SET_PLAYBACK_RATE_SUCCESS(mvideoPrimeClockHandle, avClockController, mplaybackRate, true);

                if (!isClkVideoSinkUsable(mvideoSinkId))
                {
                    UT_LOG_INFO("[SinkID: %d] is not usable for this test configuration, skipping.", mvideoSinkId.value);
                    return;
                }

                if (isClkVideoDecoderUsable(mvideoDecId))
                {
                    foundDecoder = true;
                }

                if (!foundDecoder)
                {
                    UT_LOG_WARNING("No usable decoder found for Decoder ID %d, skipping.", mvideoDecId.value);
                    return;
                }

                com::rdk::hal::avclock::ClockTime videoClockBeforeDecoder , videoClockAfterDecoder, videoClockAfterSink;

                AVCLOCK_GET_CURRENT_CLOCK_TIME_SUCCESS(maudioPrimeClockHandle, avClockController, videoClockBeforeDecoder);

                HeapMetrics beforeHeapMetrics, afterHeapMetrics;
                AVBUFFER_GETMETRICS(mAVBufferManager, mvideoPrimeSecureDecode, beforeHeapMetrics);

                videoDecoderTest decodeTest(mvideoDecId, mvideoDecHandle, mvideoDecControlListener,
                                    mvideoDecEventListener, mAVBufferManager, mvideoPrimeSecureDecode, 
                                    mvideoTunnel, mvideoDecTestConfig, mAVbufferHelper);

                std::thread decodeThread(&videoDecoderTest::run, &decodeTest, false);

                AVCLOCK_GET_CURRENT_CLOCK_TIME_SUCCESS(maudioPrimeClockHandle, avClockController, videoClockAfterDecoder);

                SinkTest *sinkTest = new SinkTest(mvideoSinkId, mvideoSinkHandle, mvideoSinkControlListener, mvideoSinkEventListener,
                                                mvideoDecControlListener, mvideoDecTestConfig, mvideoSinkConfig);

                std::this_thread::sleep_for(std::chrono::milliseconds(500));
                sinkTest->run(mvideoTunnel);

                AVCLOCK_GET_CURRENT_CLOCK_TIME_SUCCESS(maudioPrimeClockHandle, avClockController, videoClockAfterSink);

                if (decodeThread.joinable())
                    decodeThread.join();

                AVBUFFER_GETMETRICS(mAVBufferManager, mvideoPrimeSecureDecode, afterHeapMetrics);

                // Check if the decoder freed all the buffer pools
                UT_ASSERT_TRUE(beforeHeapMetrics.secure == afterHeapMetrics.secure);
                UT_ASSERT_TRUE(beforeHeapMetrics.bytesUsed == afterHeapMetrics.bytesUsed);
                UT_ASSERT_TRUE(beforeHeapMetrics.bytesTotal == afterHeapMetrics.bytesTotal);

                /*Stop the AVClock*/
                AVCLOCK_STOP_SUCCESS(mvideoPrimeClockHandle, avClockController, true);

                /*Close the AVClock*/
                AVCLOCK_CLOSE_SUCCESS(mvideoPrimeClockHandle, avClockController, true);
                
        } catch (const std::exception& ex) {
            UT_LOG_ERROR("Exception in runSequentialSink: %s", ex.what());
            UT_ASSERT_TRUE(false);
        } catch (...) {
            UT_LOG_ERROR("Unknown exception in runSequentialSink");
            UT_ASSERT_TRUE(false);
        }
    }
};

class AVClockPrimePcr
{
public:
    AVClockPrimePcr(IAVClock::Id pcrPrimeClockId , sp<IAVClock> pcrPrimeClkHandle, avclock::ClockMode pcrPrimeClockMode, sp<AVClockControllerListener> pcrPrimeClkController, int64_t pcrValues)
    {
        
        mpcrPrimeClockId = pcrPrimeClockId;
        mpcrPrimeClockHandle = pcrPrimeClkHandle;
        mpcrPrimeClockMode = pcrPrimeClockMode;
        mpcrPrimeClckController = pcrPrimeClkController;
        mpcrValue = pcrValues;
    }

    long getMonotonicRawTimestamp() 
    {
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
        return static_cast<int64_t>(ts.tv_sec) * 1'000'000'000LL + ts.tv_nsec;
    }

    void run()
    {
        pcrPrimeClockTest();
    }

    private:
        IAVClock::Id mpcrPrimeClockId;
        sp<IAVClock> mpcrPrimeClockHandle;
        avclock::ClockMode mpcrPrimeClockMode;
        sp<AVClockControllerListener> mpcrPrimeClckController;
        int64_t mpcrValue;

    void pcrPrimeClockTest()
    {
        try {
                android::sp<com::rdk::hal::avclock::IAVClockController> avClockController;
                ClockTime pcrClock;

                /* Open the AVClock */
                AVCLOCK_OPEN_SUCCESS(mpcrPrimeClockHandle, mpcrPrimeClckController, avClockController, true);

                /* Set the AVClock Mode */
                AVCLOCK_SET_CLOCK_MODE_SUCCESS(mpcrPrimeClockHandle, avClockController, mpcrPrimeClockMode, true);
     
                /* Start the AVClock */
                AVCLOCK_START_SUCCESS(mpcrPrimeClockHandle, avClockController, true);

                // Get current timestamp
                int64_t sampleTimestampNs = getMonotonicRawTimestamp();
                AVCLOCK_NOTIFY_PCR_SAMPLE_SUCCESS(mpcrPrimeClockHandle, avClockController, mpcrValue, sampleTimestampNs, true);
                
                std::this_thread::sleep_for(std::chrono::milliseconds(500));

                AVCLOCK_GET_CURRENT_CLOCK_TIME_SUCCESS(mpcrPrimeClockHandle, avClockController, pcrClock);

                /*Stop the AVClock*/
                AVCLOCK_STOP_SUCCESS(mpcrPrimeClockHandle, avClockController, true);

                /*Close the AVClock*/
                AVCLOCK_CLOSE_SUCCESS(mpcrPrimeClockHandle, avClockController, true);

        } catch (const std::exception& ex) {
            UT_LOG_ERROR("Exception in runpcrPrimeClockTest: %s", ex.what());
            UT_ASSERT_TRUE(false);
        } catch (...) {
            UT_LOG_ERROR("Unknown exception in runpcrPrimeClockTest");
            UT_ASSERT_TRUE(false);
        }
    }
};

void runSequentialAudioClock(testConfiguration_t& testConfig, bool isSecure, bool isTunnel, avclock::ClockMode clockMode, double playbackRate)
{
    try {
            auto& avClockList = VTS_L3_AVClock::mAVClockList;
            auto& audioDecoderList = VTS_L3_AVClock::mAudioDecoderList;
            auto& audioSinkList = VTS_L3_AVClock::mAudioSinkList;
            auto& avBufferManager = VTS_L3_AVClock::mAVBufferManager;
            auto& avBufferHelper = VTS_L3_AVClock::mAVbufferHelper;

            for (const auto& [_clockId, clockHandle, controlListener, eventListener] : avClockList)
             {  
                IAudioDecoder::Id audioDecId;
                sp<IAudioDecoder> audioDecHandle;
                sp<AudioDecoderControllerListener> audioDecControlListener;
                sp<AudioDecoderEventListener> audioDecEventListener;
                audiodecoder::Capabilities *audioDecCapabilites;
                IAudioSink::Id audioSinkId;
                sp<IAudioSink> audioSinkHandle;
                sp<AudioSinkControllerListener> audioSinkControlListener;
                sp<AudioSinkEventListener> audioSinkEventListener;

               /* if (!isClockUsable(_clockId))
                {
                    UT_LOG_INFO("[ClockID: %d] is not usable for this test configuration, skipping.", _clockId.value);
                    continue;
                }*/

                if (!audioDecoderList.empty()) 
                {
                    auto& [audioDecId, audioDecHandle, audioDecControlListener, audioDecEventListener, audioDecCapabilites] = audioDecoderList[0];
                } else {
                    UT_LOG_INFO(" Audio Decoder List is empty Skipping Sequential Test");
                    break;
                }

                if (!audioSinkList.empty()) 
                {
                    auto& [audioSinkId, audioSinkHandle, audioSinkControlListener, audioSinkEventListener] = audioSinkList[0];
                } else {
                    UT_LOG_INFO("Audio Sink List is empty Skipping Sequential Test");
                    break;
                }

                AVClockPrimeAudio audioprimetest(_clockId, clockHandle, clockMode, controlListener, audioDecId, audioDecHandle, audioDecControlListener,
                            audioDecEventListener, audioSinkId, audioSinkHandle, audioSinkControlListener,audioSinkEventListener,
                            avBufferManager, avBufferHelper, isSecure, isTunnel, testConfig, playbackRate);
                std::thread primeaudiothread(&AVClockPrimeAudio::run, &audioprimetest); 

                if (primeaudiothread.joinable())
                        primeaudiothread.join();
            }
        } catch (const std::exception& ex) {
                UT_LOG_ERROR("Exception in runSequentialAudioClock: %s", ex.what());
                UT_ASSERT_TRUE(false);
        } catch (...) {
                UT_LOG_ERROR("Unknown exception in runSequentialAudioClock");
                UT_ASSERT_TRUE(false);
        }
    }

    void runSequentialVideoClock(videoDecoderTestConfiguration_t vdecConfig, videoTestConfiguration_t sinkConfig, bool isSecure, bool isTunnel, avclock::ClockMode clockMode, double playbackRate)
    {
       try {
            auto& avClockList = VTS_L3_AVClock::mAVClockList;
            auto& videoDecoderList = VTS_L3_AVClock::mVideoDecoderList;
            auto& videoSinkList = VTS_L3_AVClock::mVideoSinkList;
            auto& avBufferManager = VTS_L3_AVClock::mAVBufferManager;
            auto& avBufferHelper = VTS_L3_AVClock::mAVbufferHelper;

            for (const auto& [_clockId, clockHandle, controlListener, eventListener] : avClockList)
             {  
                IVideoDecoder::Id videoDecId;
                sp<IVideoDecoder> videoDecHandle;
                sp<VideoDecoderControllerListener> videoDecControlListener;
                sp<VideoDecoderEventListener> videoDecEventListener;
                videodecoder::Capabilities *videoDecCapabilites;
                IVideoSink::Id videoSinkId;
                sp<IVideoSink> videoSinkHandle;
                sp<VideoSinkControllerListener> videoSinkControlListener;
                sp<VideoSinkEventListener> videoSinkEventListener;

                /*if (!isClockUsable(_clockId))
                {
                    UT_LOG_INFO("[ClockID: %d] is not usable for this test configuration, skipping.", _clockId.value);
                    continue;
                }*/

                if (!videoDecoderList.empty()) 
                {
                    auto& [videoDecId, videoDecHandle, videoDecControlListener, videoDecEventListener, videoDecCapabilites] = videoDecoderList[0];
                } else {
                    UT_LOG_INFO(" Video Decoder List is empty Skipping Sequential Test");
                    break;
                }

                if (!videoSinkList.empty()) 
                {
                    auto& [videoSinkId, videoSinkHandle, videoSinkControlListener, videoSinkEventListener] = videoSinkList[0];
                } else {
                    UT_LOG_ERROR("Video Sink List is empty Skipping Sequential Test");
                    break;
                }
        
                AVClockPrimeVideo videoprimetest(_clockId, clockHandle, clockMode, controlListener, videoDecId, videoDecHandle, videoDecControlListener,
                        videoDecEventListener, videoSinkId, videoSinkHandle, videoSinkControlListener, videoSinkEventListener,
                        avBufferManager, avBufferHelper, isSecure, isTunnel, vdecConfig, sinkConfig, playbackRate);
                std::thread primevideothread(&AVClockPrimeVideo::run, &videoprimetest);  
                
                if (primevideothread.joinable())
                        primevideothread.join();
            }
        } catch (const std::exception& ex) {
                UT_LOG_ERROR("Exception in runSequentialVideoClock: %s", ex.what());
                UT_ASSERT_TRUE(false);
        } catch (...) {
                UT_LOG_ERROR("Unknown exception in runSequentialVideoClock");
                UT_ASSERT_TRUE(false);
        }
}


void runSequentialPCRClock(long pcrValue, avclock::ClockMode clockMode)
{
    try {
            auto& avClockList = VTS_L3_AVClock::mAVClockList;

            for (const auto& [_clockId, clockHandle, controlListener, eventListener] : avClockList)
             {  

                /*if (!isClockUsable(_clockId))
                {
                    UT_LOG_INFO("[ClockID: %d] is not usable for this test configuration, skipping.", _clockId.value);
                    continue;
                }*/
        
                AVClockPrimePcr pcrprimetest(_clockId, clockHandle, clockMode, controlListener, pcrValue);
                std::thread primepcrthread(&AVClockPrimePcr::run, &pcrprimetest);  
                
                if (primepcrthread.joinable())
                        primepcrthread.join();
             }    
        } catch (const std::exception& ex) {
            UT_LOG_ERROR("Exception in runSequentialPCRClock: %s", ex.what());
            UT_ASSERT_TRUE(false);
        } catch (...) {
            UT_LOG_ERROR("Unknown exception in runSequentialPCRClock");
            UT_ASSERT_TRUE(false);
        }
}

void runConcurrentAudioClock(testConfiguration_t& testConfig, bool isSecure, bool isTunnel, avclock::ClockMode clkMode, double playbackRate)
{      
        try {
            auto& avClockList = VTS_L3_AVClock::mAVClockList;
            auto& audioDecoderList = VTS_L3_AVClock::mAudioDecoderList;
            auto& audioSinkList = VTS_L3_AVClock::mAudioSinkList;
            auto& avBufferManager = VTS_L3_AVClock::mAVBufferManager;
            auto& avBufferHelper = VTS_L3_AVClock::mAVbufferHelper;

            vector<thread> audioPrimeThreads;
            vector<unique_ptr<AVClockPrimeAudio>> audioPrimeTests;

                for(int32_t clockid = 0; clockid < avClockList.size(); clockid++)
                {   
                    IAudioDecoder::Id audioDecId;
                    sp<IAudioDecoder> audioDecHandle;
                    sp<AudioDecoderControllerListener> audioDecControlListener;
                    sp<AudioDecoderEventListener> audioDecEventListener;
                    audiodecoder::Capabilities *audioDecCapabilites;
                    IAudioSink::Id audioSinkId;
                    sp<IAudioSink> audioSinkHandle;
                    sp<AudioSinkControllerListener> audioSinkControlListener;
                    sp<AudioSinkEventListener> audioSinkEventListener;
                    IAVClock::Id clockId;
                    sp<IAVClock> clkHandle; 
                    sp<AVClockControllerListener> clkControllerListener;
                    sp<AVClockEventListener> clkEventListener;

                    if(avClockList.size() == 1)
                    {
                        UT_LOG_INFO("Platform has once instance of Clock it is same as sequential skipping concurrent Tests");
                        continue;
                    }
     
                    auto resultClk = getTupleElements(avClockList, clockid, std::index_sequence<0, 1, 2, 3>{});
                    if (resultClk)
                    {
                        auto& [clockId, clkHandle, clkControllerListener, clkEventListener] = *resultClk;
                    }

                    auto resultDec = getTupleElements(audioDecoderList, clockid, std::index_sequence<0, 1, 2, 3>{});
                    if (resultDec)
                    {
                        auto& [audioDecId, audioDecHandle, audioDecControlListener, audioDecEventListener] = *resultDec;
                    }

                    auto resultSink = getTupleElements(audioSinkList, clockid, std::index_sequence<0, 1, 2, 3>{});
                    if (resultSink)
                    {
                        auto& [audioSinkId, audioSinkHandle, audioSinkControlListener, audioSinkEventListener] = *resultSink;
                    }
  
                    auto audioPrimetest = make_unique<AVClockPrimeAudio>(clockId, clkHandle, clkMode, clkControllerListener, audioDecId, audioDecHandle, audioDecControlListener,
                                audioDecEventListener, audioSinkId, audioSinkHandle, audioSinkControlListener,audioSinkEventListener,
                                avBufferManager, avBufferHelper, isSecure, isTunnel, testConfig, playbackRate);
                    audioPrimeThreads.emplace_back(&AVClockPrimeAudio::run, audioPrimetest.get()); 
                    audioPrimeTests.push_back(move(audioPrimetest));
                }

                for (auto& t : audioPrimeThreads)
                {
                    if (t.joinable())
                    t.join();
                }
            } catch (const std::exception& ex) {
                UT_LOG_ERROR("Exception in runConcurrentAudioClock: %s", ex.what());
                UT_ASSERT_TRUE(false);
            } catch (...) {
                UT_LOG_ERROR("Unknown exception in runConcurrentAudioClock");
                UT_ASSERT_TRUE(false);
            }

}

void runConcurrentVideoClock(videoDecoderTestConfiguration_t &vdecConfig, videoTestConfiguration_t &sinkConfig, bool isSecure, bool isTunnel, avclock::ClockMode clkMode, double playbackRate)
{
    
        try {
            auto& avClockList = VTS_L3_AVClock::mAVClockList;
            auto& videoDecoderList = VTS_L3_AVClock::mVideoDecoderList;
            auto& videoSinkList = VTS_L3_AVClock::mVideoSinkList;
            auto& avBufferManager = VTS_L3_AVClock::mAVBufferManager;
            auto& avBufferHelper = VTS_L3_AVClock::mAVbufferHelper;

            vector<thread> videoPrimeThreads;
            vector<unique_ptr<AVClockPrimeVideo>> videoPrimeTests;

            for(int32_t clockid = 0; clockid < avClockList.size(); clockid++)
            {
                IVideoDecoder::Id videoDecId;
                sp<IVideoDecoder> videoDecHandle;
                sp<VideoDecoderControllerListener> videoDecControlListener;
                sp<VideoDecoderEventListener> videoDecEventListener;
                videodecoder::Capabilities *videoDecCapabilites;
                IVideoSink::Id videoSinkId;
                sp<IVideoSink> videoSinkHandle;
                sp<VideoSinkControllerListener> videoSinkControlListener;
                sp<VideoSinkEventListener> videoSinkEventListener;
                IAVClock::Id clockId;
                sp<IAVClock> clkHandle; 
                sp<AVClockControllerListener> clkControllerListener;
                sp<AVClockEventListener> clkEventListener;

                if(avClockList.size() == 1)
                {
                    UT_LOG_INFO("Platform has once instace of Clock it is same as sequential skipping concurrent Tests");
                    continue;
                }

                auto resultClk = getTupleElements(avClockList, clockid, std::index_sequence<0, 1, 2, 3>{});
                if (resultClk)
                {
                    auto& [clockId, clkHandle, clkControllerListener, clkEventListener] = *resultClk;
                }

                auto resultDec = getTupleElements(videoDecoderList, clockid, std::index_sequence<0, 1, 2, 3>{});
                if (resultDec)
                {
                    auto& [videoDecId, videoDecHandle, videoDecControlListener, videoDecEventListener] = *resultDec;
                }

                auto resultSink = getTupleElements(videoSinkList, clockid, std::index_sequence<0, 1, 2, 3>{});
                if (resultSink)
                {
                    auto& [videoSinkId, videoSinkHandle, videoSinkControlListener, videoSinkEventListener] = *resultSink;
                }

                auto videoprimetest = make_unique<AVClockPrimeVideo>(clockId, clkHandle, clkMode, clkControllerListener, videoDecId, videoDecHandle, videoDecControlListener,
                            videoDecEventListener, videoSinkId, videoSinkHandle, videoSinkControlListener, videoSinkEventListener,
                            avBufferManager, avBufferHelper, isSecure, isTunnel, vdecConfig, sinkConfig, playbackRate);
                videoPrimeThreads.emplace_back(&AVClockPrimeVideo::run, videoprimetest.get());
                videoPrimeTests.push_back(move(videoprimetest));
            }

            for (auto& t : videoPrimeThreads)
            {
                if (t.joinable())
                t.join();
            }
        } catch (const std::exception& ex) {
            UT_LOG_ERROR("Exception in runConcurrentVdieoClock: %s", ex.what());
            UT_ASSERT_TRUE(false);
        } catch (...) {
            UT_LOG_ERROR("Unknown exception in runConcurrentVideoClock");
            UT_ASSERT_TRUE(false);
        }
}

void runConcurrentPCRClock(int64_t pcrValue, avclock::ClockMode clockMode)
{
        try {
            auto& avClockList = VTS_L3_AVClock::mAVClockList;
            vector<thread> pcrPrimeThreads;
            vector<unique_ptr<AVClockPrimePcr>> pcrPrimeTests;

            if(avClockList.size() == 1)
            {
                UT_LOG_INFO("Platform has once instace of Clock it is same as sequential skipping concurrent Tests");
                return;
            }

            for(int32_t clockid = 0; clockid < avClockList.size(); clockid++)
            {
                IAVClock::Id clockId;
                sp<IAVClock> clkHandle; 
                sp<AVClockControllerListener> clkControllerListener;
                sp<AVClockEventListener> clkEventListener;

                auto result = getTupleElements(avClockList, clockid, std::index_sequence<0, 1, 2, 3>{});
                if (result)
                {
                    auto& [clockId, clkHandle, clkControllerListener, clkEventListener] = *result;
                }

                auto pcrprimetest = make_unique<AVClockPrimePcr>(clockId, clkHandle, clockMode, clkControllerListener, pcrValue);
                pcrPrimeThreads.emplace_back(&AVClockPrimePcr::run, pcrprimetest.get());
                pcrPrimeTests.push_back(move(pcrprimetest));
                         
            }
   
            for (auto& t : pcrPrimeThreads)
            {
                if (t.joinable())
                t.join();
            }
        } catch (const std::exception& ex) {
            UT_LOG_ERROR("Exception in runConcurrentPCRClock: %s", ex.what());
            UT_ASSERT_TRUE(false);
        } catch (...) {
            UT_LOG_ERROR("Unknown exception in runConcurrentPCRClock");
            UT_ASSERT_TRUE(false);
        }
    }

void runAudioPrimeTest(const char *testName, bool isSecureMode, bool isTunnel, bool isConcurrentMode, avclock::ClockMode clockMode, contentprimetype primetype, double playbackRate)
{
        UT_LOG_INFO("Started Thread : %s", __func__);

        vector<testConfiguration_t> testConfiguration;
        const char *DecodeStreams = "Decode";

        vts_audiodecoder_parse_testconfig(gAudioSinkTestConfiguration, gAudioModuleName, gTestGroup, DecodeStreams, testConfiguration);

        if(isConcurrentMode)
        {
            for(auto& test: testConfiguration)
            {
                runConcurrentAudioClock(test, isSecureMode, isTunnel, clockMode, playbackRate);
            }
        }
        else
        {
            for(auto& test: testConfiguration)
            {
                runSequentialAudioClock(test, isSecureMode, isTunnel, clockMode, playbackRate);
            }
        }
    }

    void runVideoPrimeTest(const char *testName, bool isSecureMode, bool isTunnel, bool isConcurrentMode, avclock::ClockMode clockMode, contentprimetype primetype, double playbackRate)
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

            if (isConcurrentMode)
            {
                runConcurrentVideoClock(decoderConfig, sinkConfig, isSecureMode, isTunnel, clockMode, playbackRate);
            } else
            {
                runSequentialVideoClock(decoderConfig, sinkConfig, isSecureMode, isTunnel, clockMode, playbackRate);
            }
        }
    }

    void runPcrPrimeTest(const char *testName, bool isConcurrentMode, avclock::ClockMode clockMode)
    {
        UT_LOG_INFO("Started Thread : %s", __func__);
        std::vector<int64_t> pcrValues;

        vts_avclock_parse_testconfig(gAVClockTestConfiguration, gModuleName, gTestGroup, testName, pcrValues);

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
                runSequentialPCRClock(_pcrValue, clockMode);
            }
        }
    }

    void runAVClockPrimeTest(const char *testName, bool isSecureMode, bool isTunnel, bool isConcurrentMode, avclock::ClockMode clockMode, contentprimetype primetype, double playbackRate)
    {
        std::thread runaudioprimethread;
        std::thread runvideoprimethread;
        std::thread runpcrprimethread;

        if(primetype == primeaudio)
        {
            runaudioprimethread = std::thread(runAudioPrimeTest, testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackRate);
        } else if(primetype == primevideo)
        {
            runvideoprimethread = std::thread(runVideoPrimeTest, testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackRate);
        } else if(primetype == primeaudiobeforevideo)
        {
            runaudioprimethread = std::thread(runAudioPrimeTest, testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackRate);
            auto start = std::chrono::steady_clock::now();
            while(!threadStarted)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                auto now = std::chrono::steady_clock::now();
                if (std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count() > 1000)
                {
                        UT_LOG_ERROR("Unable to Start Thread: Timed out after 1 second");
                        return;
                }
            }
            runvideoprimethread = std::thread(runVideoPrimeTest, testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackRate);
        } else if(primetype == primevideobeforeaudio)
        {
            runvideoprimethread = std::thread(runVideoPrimeTest, testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackRate);
            auto start = std::chrono::steady_clock::now();
            while(!threadStarted)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                auto now = std::chrono::steady_clock::now();
                if (std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count() > 1000)
                {
                        UT_LOG_ERROR("Unable to Start Thread: Timed out after 1 second");
                        return;
                }
            }
            runaudioprimethread = std::thread(runAudioPrimeTest, testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackRate);
        } else if(primetype == primepcr)
        {
            runaudioprimethread = std::thread(runAudioPrimeTest, testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackRate);
            auto start = std::chrono::steady_clock::now();
            while(!threadStarted)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                auto now = std::chrono::steady_clock::now();
                if (std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count() > 1000)
                {
                    UT_LOG_ERROR("Unable to Start Thread: Timed out after 1 second");
                    return;
                }
            runvideoprimethread = std::thread(runVideoPrimeTest, testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackRate);
            runpcrprimethread = std::thread(runPcrPrimeTest, testName, isConcurrentMode, clockMode);
            }
        }   

        if (runaudioprimethread.joinable())
                runaudioprimethread.join();

        if (runvideoprimethread.joinable())
                runvideoprimethread.join();

        if (runpcrprimethread.joinable())
                runpcrprimethread.join();
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
    UT_LOG_INFO("entered in test case %s", __func__);
    const char *testName = "PrimeClock_AutoAudioOnly";
    bool isSecureMode = false;
    bool isConcurrentMode = false;
    bool isTunnel = false;
    double playbackrate = 1.0;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
    UT_LOG_INFO("after avclockprimetest in test case %s", __func__);
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
    double playbackrate = 1.0;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 1.0;
    avclock::ClockMode clockMode = avclock::ClockMode::AUDIO_MASTER;
    contentprimetype primetype = primeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 1.0;
    avclock::ClockMode clockMode = avclock::ClockMode::VIDEO_MASTER;
    contentprimetype primetype = primevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 1.0;
    avclock::ClockMode clockMode = avclock::ClockMode::AUDIO_MASTER;
    contentprimetype primetype = primevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 1.0;
    avclock::ClockMode clockMode = avclock::ClockMode::VIDEO_MASTER;
    contentprimetype primetype = primeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 1.0;
    avclock::ClockMode clockMode = avclock::ClockMode::PCR;
    contentprimetype primetype = primepcr; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 1.0;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primeaudiobeforevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 1.0;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primevideobeforeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 1.0;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 0.0;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 2.0;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 1.0;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 0.0;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 2.0;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 1.0;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 1.0;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 1.0;
    avclock::ClockMode clockMode = avclock::ClockMode::AUDIO_MASTER;
    contentprimetype primetype = primeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 1.0;
    avclock::ClockMode clockMode = avclock::ClockMode::VIDEO_MASTER;
    contentprimetype primetype = primevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 1.0;
    avclock::ClockMode clockMode = avclock::ClockMode::AUDIO_MASTER;
    contentprimetype primetype = primevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 1.0;
    avclock::ClockMode clockMode = avclock::ClockMode::VIDEO_MASTER;
    contentprimetype primetype = primeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 1.0;
    avclock::ClockMode clockMode = avclock::ClockMode::PCR;
    contentprimetype primetype = primepcr; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 1.0;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primeaudiobeforevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 1.0;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primevideobeforeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 1.0;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 0.0;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 2.0;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 1.0;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 0.0;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 2.0;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 1.0;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 1.0;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 1.0;
    avclock::ClockMode clockMode = avclock::ClockMode::AUDIO_MASTER;
    contentprimetype primetype = primeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 1.0;
    avclock::ClockMode clockMode = avclock::ClockMode::VIDEO_MASTER;
    contentprimetype primetype = primevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 1.0;
    avclock::ClockMode clockMode = avclock::ClockMode::AUDIO_MASTER;
    contentprimetype primetype = primevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 1.0;
    avclock::ClockMode clockMode = avclock::ClockMode::VIDEO_MASTER;
    contentprimetype primetype = primeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 1.0;
    avclock::ClockMode clockMode = avclock::ClockMode::PCR;
    contentprimetype primetype = primepcr; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 1.0;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primeaudiobeforevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 1.0;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primevideobeforeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 1.0;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 0.0;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 2.0;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 1.0;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 0.0;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 1.0;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 1.0;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 1.0;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 1.0;
    avclock::ClockMode clockMode = avclock::ClockMode::AUDIO_MASTER;
    contentprimetype primetype = primeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 1.0;
    avclock::ClockMode clockMode = avclock::ClockMode::VIDEO_MASTER;
    contentprimetype primetype = primevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 1.0;
    avclock::ClockMode clockMode = avclock::ClockMode::AUDIO_MASTER;
    contentprimetype primetype = primevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 1.0;
    avclock::ClockMode clockMode = avclock::ClockMode::VIDEO_MASTER;
    contentprimetype primetype = primeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 1.0;
    avclock::ClockMode clockMode = avclock::ClockMode::PCR;
    contentprimetype primetype = primepcr; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 1.0;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primeaudiobeforevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 1.0;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primevideobeforeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 1.0;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 0.0;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 2.0;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 1.0;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 0.0;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 2.0;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 1.0;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 1.0;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 1.0;
    avclock::ClockMode clockMode = avclock::ClockMode::AUDIO_MASTER;
    contentprimetype primetype = primeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 1.0;
    avclock::ClockMode clockMode = avclock::ClockMode::VIDEO_MASTER;
    contentprimetype primetype = primevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 1.0;
    avclock::ClockMode clockMode = avclock::ClockMode::AUDIO_MASTER;
    contentprimetype primetype = primevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 1.0;
    avclock::ClockMode clockMode = avclock::ClockMode::VIDEO_MASTER;
    contentprimetype primetype = primeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 1.0;
    avclock::ClockMode clockMode = avclock::ClockMode::PCR;
    contentprimetype primetype = primepcr; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 1.0;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primeaudiobeforevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 1.0;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primevideobeforeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 1.0;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 0.0;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 2.0;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 1.0;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 0.0;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 2.0;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 1.0;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 1.0;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 1.0;
    avclock::ClockMode clockMode = avclock::ClockMode::AUDIO_MASTER;
    contentprimetype primetype = primeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 1.0;
    avclock::ClockMode clockMode = avclock::ClockMode::VIDEO_MASTER;
    contentprimetype primetype = primevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 1.0;
    avclock::ClockMode clockMode = avclock::ClockMode::AUDIO_MASTER;
    contentprimetype primetype = primevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 1.0;
    avclock::ClockMode clockMode = avclock::ClockMode::VIDEO_MASTER;
    contentprimetype primetype = primeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 1.0;
    avclock::ClockMode clockMode = avclock::ClockMode::PCR;
    contentprimetype primetype = primepcr; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 1.0;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primeaudiobeforevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 1.0;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primevideobeforeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 1.0;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 0.0;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 1.0;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 1.0;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 0.0;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 2.0;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 1.0;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 1.0;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 1.0;
    avclock::ClockMode clockMode = avclock::ClockMode::AUDIO_MASTER;
    contentprimetype primetype = primeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 1.0;
    avclock::ClockMode clockMode = avclock::ClockMode::VIDEO_MASTER;
    contentprimetype primetype = primevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 1.0;
    avclock::ClockMode clockMode = avclock::ClockMode::AUDIO_MASTER;
    contentprimetype primetype = primevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 1.0;
    avclock::ClockMode clockMode = avclock::ClockMode::VIDEO_MASTER;
    contentprimetype primetype = primeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 1.0;
    avclock::ClockMode clockMode = avclock::ClockMode::PCR;
    contentprimetype primetype = primepcr; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 1.0;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primeaudiobeforevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 1.0;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primevideobeforeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 1.0;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 0.0;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 2.0;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 2.0;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 0.0;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 2.0;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 1.0;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 1.0;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 1.0;
    avclock::ClockMode clockMode = avclock::ClockMode::AUDIO_MASTER;
    contentprimetype primetype = primeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 1.0;
    avclock::ClockMode clockMode = avclock::ClockMode::VIDEO_MASTER;
    contentprimetype primetype = primevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 1.0;
    avclock::ClockMode clockMode = avclock::ClockMode::AUDIO_MASTER;
    contentprimetype primetype = primevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 1.0;
    avclock::ClockMode clockMode = avclock::ClockMode::VIDEO_MASTER;
    contentprimetype primetype = primeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 1.0;
    avclock::ClockMode clockMode = avclock::ClockMode::PCR;
    contentprimetype primetype = primepcr; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 1.0;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primeaudiobeforevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 1.0;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primevideobeforeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 1.0;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 0.0;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 2.0;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primeaudio; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 1.0;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 0.0;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
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
    double playbackrate = 2.0;
    avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
    contentprimetype primetype = primevideo; 
    
    runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode, clockMode, primetype, playbackrate);
}
/** @} */ // End of AUDIO_SINK_HALTEST L3 File
/** @} */ // End of AUDIO_SINK_HALTEST
/** @} */ // End of AUDIO_SINK Module
/** @} */ // End of HPK
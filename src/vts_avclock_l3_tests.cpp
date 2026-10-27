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
 * This module includes Level 3 functional tests (success and failure
 * scenarios). This is to ensure that the API meets the operational requirements
 * of the module across all vendors.
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

#include <algorithm>
#include <atomic>
#include <binder/IPCThreadState.h>
#include <binder/IServiceManager.h>
#include <binder/ProcessState.h>
#include <binder/Status.h>
#include <chrono>
#include <condition_variable>
#include <deque>
#include <mutex>
#include <optional>
#include <thread>
#include <tuple>
#include <unordered_set>
#include <ut.h>
#include <ut_control_plane.h>
#include <ut_kvp_profile.h>
#include <ut_log.h>
#include <utils/String16.h>
#include <vector>

#include <map>

#include "com/rdk/hal/avbuffer/BnAVBufferSpaceListener.h"
#include "com/rdk/hal/avbuffer/IAVBuffer.h"
#include "com/rdk/hal/audiosink/IAudioSink.h"

#include "vts_audiodecoder.h"
#include "vts_audiodecoder_parse_testconfig.h"
#include "vts_audiosink_parse_hfp.h"
#include "vts_videodecoder.h"
#include "vts_audiosink.h"

#include "vts_audiosink_helper_functions.h"
#include "vts_avbuffer.h"

#include "vts_videosink_helper_functions.h"
#include "vts_videosink_parse_hfp.h"
#include "vts_videosink.h"
#include "vts_videosink_test.h"


#include "vts_avclock_parse_hfp.h"
#include "vts_avclock_parse_testconfig.h"

#include "vts_avclock.h"

extern const char *gAudioModuleName;

static const char *gModuleName = "IAVClockManager";
static const char *gAudioSinkModuleName = "IAudioSinkManager";
static const char *gVideoDecoderModuleName = "IVideoDecoderManager";
static const char *gVideoSinkModuleName = "IVideoSinkManager";
static const char *gTestGroup = "L3";

// static IAVBufferHelper* gAVbufferHelper;
static sp<IAVBuffer> vts_avbuffer_getService() {
  return android::interface_cast<IAVBuffer>(
      android::defaultServiceManager()->getService(
          android::String16(IAVBuffer::serviceName().c_str())));
}

class VTS_L3_AVClock : public UTCore {
public:
  static std::vector<
      std::tuple<IAVClock::Id, sp<IAVClock>, sp<AVClockControllerListener>,
                 sp<AVClockEventListener>>>
      mAVClockList;
  static std::vector<std::tuple<
      IAudioDecoder::Id, sp<IAudioDecoder>, sp<AudioDecoderControllerListener>,
      sp<AudioDecoderEventListener>, audiodecoder::Capabilities *>>
      mAudioDecoderList;
  static std::vector<std::tuple<
      IVideoDecoder::Id, sp<IVideoDecoder>, sp<VideoDecoderControllerListener>,
      sp<VideoDecoderEventListener>, videodecoder::Capabilities *>>
      mVideoDecoderList;
  static std::vector<
      std::tuple<IAudioSink::Id, sp<IAudioSink>,
                 sp<AudioSinkControllerListener>, sp<AudioSinkEventListener>>>
      mAudioSinkList;
  static std::vector<
      std::tuple<IVideoSink::Id, sp<IVideoSink>,
                 sp<VideoSinkControllerListener>, sp<VideoSinkEventListener>>>
      mVideoSinkList;
  static sp<IAVBuffer> mAVBufferManager;
  static IAVBufferHelper *mAVbufferHelper;

  VTS_L3_AVClock() : UTCore(){};

  ~VTS_L3_AVClock() override = default;

  void SetUp() override {
    std::vector<IAVClock::Id> avClockIds;
    std::vector<IAudioSink::Id> audioSinkIds;
    std::vector<IVideoSink::Id> videoSinkIds;
    std::vector<IAudioDecoder::Id> audioDecoderIds;
    std::vector<IVideoDecoder::Id> videoDecoderIds;

    if (gAVClockManagerService == NULL) {
      UT_FAIL_FATAL("AVClock Service not available");
    }

    if (gAudioDecoderManager == NULL) {
      UT_FAIL_FATAL("AudioDecoder Service not available");
    }

    if (gAudioSinkManager == NULL) {
      UT_FAIL_FATAL("AudioSink Service not available");
    }

    if (gVideoDecoderManager == NULL) {
      UT_FAIL_FATAL("VideoDecoder Service not available");
    }

    if (gVideoSinkManager == NULL) {
      UT_FAIL_FATAL("VideoSink Service not available");
    }

    mAVBufferManager = vts_avbuffer_getService();
    if (mAVBufferManager == NULL) {
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

    for (const auto &_avClockId : avClockIds) {
      sp<IAVClock> avClock;

      sp<AVClockEventListener> eventListener =
          new AVClockEventListener(_avClockId.value);

      sp<AVClockControllerListener> controlListener =
          new AVClockControllerListener(_avClockId.value);

      /* Get AVClock handle */
      AVCLOCK_MANAGER_GETAVCLOCK_SUCCESS(_avClockId, avClock);

      /* Register events */
      AVCLOCK_REGISTER_EVENT_LISTENER_SUCCESS(avClock, eventListener);

      /* Get Capabilities of AVClock */
      // TODO: Uncomment when capabilities are implemented in the HAL
      // AVCLOCK_GETCAPABILITIES(avClock, capabilities);

      mAVClockList.push_back(
          {_avClockId, std::move(avClock), std::move(controlListener), std::move(eventListener)});
    }

    for (const auto &_audioSinkId : audioSinkIds) {
      sp<IAudioSink> audioSink;

      sp<AudioSinkEventListener> eventListener =
          new AudioSinkEventListener(_audioSinkId.value);

      sp<AudioSinkControllerListener> controlListener =
          new AudioSinkControllerListener(_audioSinkId.value);

      /* Get Audio decoder handle */
      AUDIOSINK_MANAGER_GETAUDIOSINK(_audioSinkId, audioSink);

      /* Register events */
      AUDIOSINK_REGISTER_EVENT_SUCCESS(audioSink, eventListener);

      /* Get Capabilities of Audio Sink */
      // TODO: Uncomment when capabilities are implemented in the HAL
      // AUDIOSINK_GETCAPABILITIES(audioSink, capabilities);

      mAudioSinkList.push_back(
          {_audioSinkId, std::move(audioSink), std::move(controlListener), std::move(eventListener)});
    }

    for (const auto &_audioDecoderId : audioDecoderIds) {
      sp<IAudioDecoder> audioDecoder;

      std::unique_ptr<audiodecoder::Capabilities> capabilitiesOwner(new audiodecoder::Capabilities());
      audiodecoder::Capabilities *capabilities = capabilitiesOwner.get();

      sp<AudioDecoderEventListener> eventListener =
          new AudioDecoderEventListener();

      sp<AudioDecoderControllerListener> controlListener =
          new AudioDecoderControllerListener();

      controlListener->setDecoderID(_audioDecoderId.value);
      eventListener->setDecoderID(_audioDecoderId.value);

      /* Get Audio decoder handle */
      AUDIODECODER_MANAGER_GETAUDIODECODER(_audioDecoderId, audioDecoder);

      /* Register events */
      AUDIODECODER_REGISTER_EVENT_SUCCESS(audioDecoder, eventListener);

      /* Get Capabilities of Audio Decoder */
      AUDIODECODER_GETCAPABILITIES(audioDecoder, capabilities);
      UT_LOG_DEBUG(
          "Result getAudioDecoder(IN:_decoderId:[%d], OUT:audioDecoder[0x%X]), "
          "IN:audioDeccontrollerListener[%p], audioDecEventListener[%p] ",
          _audioDecoderId.value, audioDecoder.get(), controlListener.get(),
          eventListener.get());

      mAudioDecoderList.push_back({_audioDecoderId, std::move(audioDecoder),
                                   std::move(controlListener), std::move(eventListener),
                                   capabilitiesOwner.release()});
    }

    for (const auto &_videoDecoderId : videoDecoderIds) {
      sp<IVideoDecoder> videoDecoder;
      std::unique_ptr<videodecoder::Capabilities> capabilitiesOwner(new videodecoder::Capabilities());
      videodecoder::Capabilities *capabilities = capabilitiesOwner.get();
      sp<VideoDecoderEventListener> eventListener =
          new VideoDecoderEventListener();
      sp<VideoDecoderControllerListener> controlListener =
          new VideoDecoderControllerListener();

      VIDEODECODER_MANAGER_GETVIDEODECODER(_videoDecoderId, videoDecoder);

      VIDEODECODER_REGISTER_EVENT_SUCCESS(videoDecoder, eventListener);

      VIDEODECODER_GETCAPABILITIES(videoDecoder, capabilities);

      mVideoDecoderList.push_back({_videoDecoderId, std::move(videoDecoder),
                                   std::move(controlListener), std::move(eventListener),
                                   capabilitiesOwner.release()});
      UT_LOG_INFO("Added decoder ID %d to test list", _videoDecoderId.value);
    }

    for (const auto &_videoSinkId : videoSinkIds) {
      sp<IVideoSink> videoSink;

      sp<VideoSinkEventListener> videoSinkEventListener =
          new VideoSinkEventListener();

      sp<VideoSinkControllerListener> videoSinkControlListener =
          new VideoSinkControllerListener();

      videoSinkControlListener->setSinkID(_videoSinkId.value);
      videoSinkEventListener->setSinkID(_videoSinkId.value);

      /* Get Video Sink handle */
      VIDEOSINK_MANAGER_GETVIDEOSINK(_videoSinkId, videoSink);

      /* Register events */
      VIDEOSINK_REGISTER_EVENT_SUCCESS(videoSink, videoSinkEventListener);

      // TODO: VideoSink Get capabilities are yet to implement in the video sink

      mVideoSinkList.push_back({_videoSinkId, std::move(videoSink),
                                std::move(videoSinkControlListener),
                                std::move(videoSinkEventListener)});
    }
  }

  void TearDown() override {
    mAVClockList.clear();
    mAudioDecoderList.clear();
    mVideoDecoderList.clear();
    mAudioSinkList.clear();
    mVideoSinkList.clear();
    // TODO: check right place to call joinThreadPool
    // android::IPCThreadState::self()->joinThreadPool();
  }
};

std::vector<std::tuple<IAVClock::Id, sp<IAVClock>,
                       sp<AVClockControllerListener>, sp<AVClockEventListener>>>
    VTS_L3_AVClock::mAVClockList;
std::vector<std::tuple<
    IAudioDecoder::Id, sp<IAudioDecoder>, sp<AudioDecoderControllerListener>,
    sp<AudioDecoderEventListener>, audiodecoder::Capabilities *>>
    VTS_L3_AVClock::mAudioDecoderList;
std::vector<std::tuple<
    IVideoDecoder::Id, sp<IVideoDecoder>, sp<VideoDecoderControllerListener>,
    sp<VideoDecoderEventListener>, videodecoder::Capabilities *>>
    VTS_L3_AVClock::mVideoDecoderList;
std::vector<
    std::tuple<IAudioSink::Id, sp<IAudioSink>, sp<AudioSinkControllerListener>,
               sp<AudioSinkEventListener>>>
    VTS_L3_AVClock::mAudioSinkList;
std::vector<
    std::tuple<IVideoSink::Id, sp<IVideoSink>, sp<VideoSinkControllerListener>,
               sp<VideoSinkEventListener>>>
    VTS_L3_AVClock::mVideoSinkList;
sp<IAVBuffer> VTS_L3_AVClock::mAVBufferManager;
IAVBufferHelper *VTS_L3_AVClock::mAVbufferHelper;

std::atomic<bool> audioThreadStarted{false};
std::atomic<bool> videoThreadStarted{false};
std::atomic<bool> pcrThreadStarted{false};

/* Test class registration with UT_Core */
UT_ADD_TEST_TO_GROUP(VTS_L3_AVClock, UT_TESTS_L3)

class AVClockBufferSpaceListener : public BnAVBufferSpaceListener {
public:
  AVClockBufferSpaceListener(){};
  virtual ~AVClockBufferSpaceListener() = default;

  Status onSpaceAvailable() override { return Status::ok(); }
};

void checkCompleteStateTransition(sp<IAVClock> avClock,
                                  sp<AVClockEventListener> eventListener,
                                  sp<AVClockControllerListener> controlListener,
                                  com::rdk::hal::avclock::State oldState,
                                  com::rdk::hal::avclock::State intermediateState,
                                  com::rdk::hal::avclock::State newState) {
  ::com::rdk::hal::avclock::State currentState;

  UT_ASSERT_TRUE(
      eventListener->checkStateTransition(oldState, intermediateState));
  UT_ASSERT_TRUE(
      controlListener->checkStateTransition(oldState, intermediateState));

  eventListener->waitForState(
      newState, std::chrono::milliseconds(MAX_FINAL_STATE_WAIT_MS));
  controlListener->waitForState(
      newState, std::chrono::milliseconds(MAX_FINAL_STATE_WAIT_MS));

  AVCLOCK_GET_STATE(avClock, currentState);

  UT_ASSERT_EQUAL(currentState, newState);

  UT_ASSERT_TRUE(
      eventListener->checkStateTransition(intermediateState, newState));
  UT_ASSERT_TRUE(
      controlListener->checkStateTransition(intermediateState, newState));

  eventListener->resetStateChanges();
  controlListener->resetStateChanges();
}

android::sp<IAVClockController> AVClockPrimeTest::setupAndStartClock() {
  android::sp<IAVClockController> clockController;
  try {
    // Open the AVClock
    AVCLOCK_OPEN_SUCCESS(mClockHandle, mClockController, clockController, true);

    // Set the AVClock Mode
    AVCLOCK_SET_CLOCK_MODE_SUCCESS(mClockHandle, clockController, mClockMode,
                                   true);

    // Start the AVClock
    AVCLOCK_START_SUCCESS(mClockHandle, clockController, true);

    // Set the Playback Rate
    if(mClockMode != avclock::ClockMode::PCR)
    {
        AVCLOCK_SET_PLAYBACK_RATE_SUCCESS(mClockHandle, clockController,
                                                    mPlaybackRate, true);
    }

    UT_LOG_INFO("AVClock setup and start completed successfully.");
  } catch (const std::exception &ex) {
    UT_LOG_ERROR("Exception in setupAndStartClock: %s", ex.what());
    UT_ASSERT_TRUE(false);
  } catch (...) {
    UT_LOG_ERROR("Unknown exception in setupAndStartClock");
    UT_ASSERT_TRUE(false);
  }
  return clockController;
}

void AVClockPrimeTest::shutdownClock(
    android::sp<IAVClockController> clockcontroller) {
  try {
    // Stop the AVClock
    AVCLOCK_STOP_SUCCESS(mClockHandle, clockcontroller, true);

    // Close the AVClock
    AVCLOCK_CLOSE_SUCCESS(mClockHandle, clockcontroller, true);

    UT_LOG_INFO("AVClock shutdown completed successfully.");
  } catch (const std::exception &ex) {
    UT_LOG_ERROR("Exception during shutdownClock: %s", ex.what());
    UT_ASSERT_TRUE(false);
  } catch (...) {
    UT_LOG_ERROR("Unknown exception during shutdownClock");
    UT_ASSERT_TRUE(false);
  }
}

/* Check AVClock Has more than one instance to support concurrent test cases*/
static void isAVClockSupportsConcurrency() {
    bool supported = VTS_L3_AVClock::mAVClockList.size() > 1;

    UT_LOG_DEBUG("AVClock concurrent support: %s", supported ? "true" : "false");

    if(!supported)
    {
       UT_IGNORE_TEST()<< "[  SKIPPED ] Clock Supports only one instance doesn't support Concurrency\n";
    } else {
       UT_LOG_INFO("Platform Supports Concurrency");
    }
}

void AVClockPrimeTest::validateClockTime(std::shared_ptr<AVClockPrimeTest>& clockTest,
    sp<IAVClock> clockHandle,
    sp<IAVClockController>& avClockController,
    double expectedRate,
    contentprimetype primetype,
    sp<AVClockControllerListener>& clockControlListener,
    threadtype threadType)
{
    UT_LOG_INFO("%s Called", __FUNCTION__);

    std::optional<com::rdk::hal::avclock::ClockTime> t1;
    std::optional<com::rdk::hal::avclock::ClockTime> t2;

    double mNormalPlay = 1.0, mFastForward = 2.0, mPause = 0.0;

    // Decide which thread should wait based on clock mode ownership.
    bool shouldWait = false;
    const char* expectedSource = "unknown";

    const char* threadName = (threadType == threadtype::audio) ? "audio"
                 : (threadType == threadtype::video) ? "video" : "pcr";
    const char* modeName = (mClockMode == avclock::ClockMode::AUTO) ? "AUTO"
               : (mClockMode == avclock::ClockMode::AUDIO_MASTER) ? "AUDIO_MASTER"
               : (mClockMode == avclock::ClockMode::VIDEO_MASTER) ? "VIDEO_MASTER"
               : "PCR";
    const char* primetypeName = (primetype == contentprimetype::primeaudio) ? "primeaudio"
                  : (primetype == contentprimetype::primevideo) ? "primevideo"
                  : (primetype == contentprimetype::primeaudiobeforevideo) ? "primeaudiobeforevideo"
                  : (primetype == contentprimetype::primevideobeforeaudio) ? "primevideobeforeaudio"
                  : "primepcr";

    if (mClockMode == avclock::ClockMode::AUDIO_MASTER) {
      expectedSource = "audio";
      shouldWait = (threadType == threadtype::audio);
    } else if (mClockMode == avclock::ClockMode::VIDEO_MASTER) {
      expectedSource = "video";
      shouldWait = (threadType == threadtype::video);
    } else if (mClockMode == avclock::ClockMode::PCR) {
      expectedSource = "pcr";
      shouldWait = (threadType == threadtype::pcr);
    } else {
      // AUTO mode: primetype selects the expected first prime source.
      if (primetype == contentprimetype::primeaudio ||
        primetype == contentprimetype::primeaudiobeforevideo) {
        expectedSource = "audio";
        shouldWait = (threadType == threadtype::audio);
      } else if (primetype == contentprimetype::primevideo ||
             primetype == contentprimetype::primevideobeforeaudio) {
        expectedSource = "video";
        shouldWait = (threadType == threadtype::video);
      } else {
        expectedSource = "pcr";
        shouldWait = (threadType == threadtype::pcr);
      }
    }

        UT_LOG_INFO("Prime wait decision: mode=%s(%d), primetype=%s, expectedSource=%s, thread=%s, shouldWait=%s",
          modeName, static_cast<int>(mClockMode), primetypeName, expectedSource, threadName,
          shouldWait ? "true" : "false");

    if (shouldWait)
    {
      UT_LOG_INFO("Waiting up to 5 seconds for mOnPrimeEventAvailable");
    }
    else
    {
        UT_LOG_INFO("Thread [%s] is not the prime source for mode=%s primetype=%s; "
                    "clock state is validated after its first-frame callback",
                    threadName, modeName, primetypeName);
        return;
    }

      if (clockControlListener->waitForPrimedEvent(std::chrono::seconds(5), threadType))
    {
        UT_LOG_INFO("PRIMED event received");
        // Capture t1 immediately — audio stream is short, no delay allowed here.
        AVCLOCK_GET_CURRENT_CLOCK_TIME_SUCCESS(
            clockHandle, avClockController, &t1
        );

        if (!t1.has_value()) {
            UT_LOG_WARNING("getCurrentClockTime() returned null at t1 — stream ended before measurement; skipping rate check");
            return;
        }

        const auto& firstClockTime = t1.value();
        UT_LOG_INFO("t1 clock: %lld at system time: %lld",
              firstClockTime.clockTimeNs,
              firstClockTime.sampleTimestampNs);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        AVCLOCK_GET_CURRENT_CLOCK_TIME_SUCCESS(
            clockHandle, avClockController, &t2
        );

        if (!t2.has_value()) {
            UT_LOG_WARNING("getCurrentClockTime() returned null at t2 — stream ended during measurement window; skipping rate check");
            UT_ASSERT_TRUE(false);
            return;
        }

        const auto& secondClockTime = t2.value();
        UT_LOG_INFO("t2 clock: %lld at system time: %lld",
              secondClockTime.clockTimeNs,
              secondClockTime.sampleTimestampNs);

        double clockDelta =
          static_cast<double>(secondClockTime.clockTimeNs - firstClockTime.clockTimeNs);

        double timeDelta =
          static_cast<double>(secondClockTime.sampleTimestampNs - firstClockTime.sampleTimestampNs);

        double actualRate = clockDelta / timeDelta;

        UT_LOG_INFO("ClockDelta: %f TimeDelta: %f Rate: %f",
                    clockDelta, timeDelta, actualRate);

        const double tolerance = 0.15;
        double diff = std::fabs(actualRate - expectedRate);

        UT_LOG_INFO("Expected: %f Actual: %f Diff: %f",
                    expectedRate, actualRate, diff);

        UT_ASSERT_TRUE(diff < tolerance);

        bool isPause  = (actualRate < 0.1);
        bool isFast   = (actualRate > 1.5);
        bool isNormal = (!isPause && !isFast);

        if (isPause) {
            UT_LOG_INFO("Detected PAUSE mode");
        } else if (isFast) {
            UT_LOG_INFO("Detected FAST FORWARD");
        } else {
            UT_LOG_INFO("Detected NORMAL playback");
        }

        bool expectedPause  = (std::fabs(expectedRate - mPause) < tolerance);
        bool expectedFast   = (std::fabs(expectedRate - mFastForward) < tolerance);
        bool expectedNormal = (std::fabs(expectedRate - mNormalPlay) < tolerance);

        UT_ASSERT_TRUE(
            (isPause && expectedPause) ||
            (isFast && expectedFast) ||
            (isNormal && expectedNormal)
        );
    }
    else
    {
      if(threadName == "audio")
      {
         auto& audDecCtrlListener = clockTest->getDecCtrlListener();
         if (!audDecCtrlListener)
         {
           UT_LOG_ERROR("Audio config not available");
           UT_ASSERT_TRUE(false);
         } else {
           FrameOutputMode outputMode = audDecCtrlListener->getFrameOutputMode();
           UT_LOG_INFO("Audio frame output mode: %d", static_cast<int>(outputMode));
           if((!(outputMode == FrameOutputMode::TUNNEL)) && (clockTest->getTunnelMode()))
           {
             UT_LOG_ERROR("Skipping Test: Audio frame output mode is not TUNNEL while tunnel mode is enabled");
             return;
           }
         }
      }
      UT_LOG_ERROR("Timed out waiting for mOnPrimeEventAvailable");
        UT_LOG_ERROR("Expected prime source [%s] did not produce a prime event", expectedSource);
      UT_ASSERT_TRUE(false);
    }
}

// Audio Test
void AVClockPrimeTest::runAudioTest(const AudioConfig& audioCfg,
                                    sp<IAVClockController> &avClockController) {
  try {
    if (!isClkSinkUsable(audioCfg.sinkId))
      return;
    if (!isClkDecoderUsable(audioCfg.decId))
      return;

    bool isAudioSinkAttached = (audioCfg.sinkHandle != nullptr);
    bool isClockInUse = true;

    UT_LOG_INFO("AudioDecoder Thread about to start");
    AudioDecoderTest decodeTest(
        audioCfg.decId, audioCfg.decHandle, audioCfg.decCtrlListener,
        audioCfg.decEventListener, mBufferManager, mBufferHelper,
        audioCfg.secureDecode, audioCfg.testConfig, isAudioSinkAttached);
    std::thread decodeThread(&AudioDecoderTest::run, &decodeTest, true);

    UT_LOG_INFO("AudioSink Thread about to start");
    AudioSinkTest sinkTest(
        audioCfg.sinkId, audioCfg.sinkHandle, audioCfg.sinkCtrlListener,
        audioCfg.sinkEventListener, audioCfg.decCtrlListener, mBufferManager,
        audioCfg.decId, mClockId, audioCfg.secureDecode, audioCfg.tunnelMode,
        audioCfg.testConfig, isClockInUse);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    std::thread sinkThread(&AudioSinkTest::run, &sinkTest);

    if (sinkThread.joinable())
      sinkThread.join();

    if (decodeThread.joinable())
      decodeThread.join();

  } catch (...) {
    UT_LOG_ERROR("Exception in runAudioTest");
    UT_ASSERT_TRUE(false);
  }
}

// Video Test
void AVClockPrimeTest::runVideoTest(const VideoConfig& videoCfg,
                                    sp<IAVClockController> &avClockController) {
  try {
    UT_LOG_INFO("%s", __FUNCTION__);

    if (!isClkVideoSinkUsable(videoCfg.sinkId))
    return;

    if (!isClkVideoDecoderUsable(videoCfg.decId))
    return;

    bool isVideoSinkAttached = (videoCfg.sinkHandle != nullptr);
    bool isClockInUse = true;

    UT_LOG_INFO("Resetting video decoder listener state for the next prime phase");
    videoCfg.decCtrlListener->resetQueues();
    videoCfg.decEventListener->resetQueues();

    UT_LOG_INFO("VideoDecoder Thread about to start");
    videoDecoderTest decodeTest(
        videoCfg.decId, videoCfg.decHandle, videoCfg.decCtrlListener,
        videoCfg.decEventListener, mBufferManager, videoCfg.secureDecode,
        videoCfg.tunnelMode, videoCfg.decTestConfig, mBufferHelper,
        isVideoSinkAttached);
    std::thread decodeThread(&videoDecoderTest::run, &decodeTest, true);

    UT_LOG_INFO("VideoSink Thread about to start");
    VideoSinkTest sinkTest(videoCfg.sinkId, videoCfg.sinkHandle,
                 videoCfg.sinkCtrlListener,
                 videoCfg.sinkEventListener,
                 videoCfg.decCtrlListener, mBufferManager,
                 videoCfg.decId, mClockId, videoCfg.sinkTestConfig, isClockInUse);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    std::thread sinkThread(&VideoSinkTest::run, &sinkTest, videoCfg.tunnelMode);

    if (sinkThread.joinable())
      sinkThread.join();

    if (decodeThread.joinable()) {
      decodeThread.join();
      decodeTest.closeDecoderSession();
    }

  } catch (...) {
    UT_LOG_ERROR("Exception in runVideoTest");
    UT_ASSERT_TRUE(false);
  }
}

int64_t AVClockPrimeTest::getMonotonicRawTimestamp() {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
  return static_cast<int64_t>(ts.tv_sec) * 1'000'000'000LL + ts.tv_nsec;
}

// PCR Test
void AVClockPrimeTest::runPcrTest(
    const PcrConfig& pcrCfg,
    android::sp<IAVClockController> &avClockController) {
  try {
    int64_t ts = getMonotonicRawTimestamp();
    AVCLOCK_NOTIFY_PCR_SAMPLE_SUCCESS(mClockHandle, avClockController,
                                      pcrCfg.pcrValue, ts, true);

  } catch (...) {
    UT_LOG_ERROR("Exception in runPcrTest");
    UT_ASSERT_TRUE(false);
  }
}

static const char *primeStreamName(threadtype streamType) {
  return streamType == threadtype::audio ? "audio"
       : streamType == threadtype::video ? "video"
                                         : "pcr";
}

static const char *primeTypeName(contentprimetype primetype) {
  return primetype == primeaudio ? "primeaudio"
       : primetype == primevideo ? "primevideo"
       : primetype == primeaudiobeforevideo ? "primeaudiobeforevideo"
       : primetype == primevideobeforeaudio ? "primevideobeforeaudio"
                                            : "primepcr";
}

PrimePhaseSynchronizer::PrimePhaseSynchronizer(
    sp<IAVClock> clockHandle, sp<IAVClockController> clockController,
    sp<AVClockControllerListener> controlListener, size_t participantCount)
    : mClockHandle(std::move(clockHandle)),
      mClockController(std::move(clockController)),
      mControlListener(std::move(controlListener)),
      mParticipantCount(participantCount) {
  UT_LOG_INFO("Prime phase synchronizer created: participants=%zu",
              mParticipantCount);
}

void PrimePhaseSynchronizer::beginConfiguration(
    threadtype streamType, contentprimetype primetype) {
  UT_LOG_INFO("Prime phase begin requested: stream=%s primetype=%s",
              primeStreamName(streamType), primeTypeName(primetype));
  arriveAtStart(false, streamType);
  UT_LOG_INFO("Prime phase start barrier released: stream=%s primetype=%s",
              primeStreamName(streamType), primeTypeName(primetype));

  bool waitForLeader = false;
  threadtype leaderType = streamType;
  if (primetype == primeaudiobeforevideo && streamType == threadtype::video) {
    waitForLeader = true;
    leaderType = threadtype::audio;
  } else if (primetype == primevideobeforeaudio &&
             streamType == threadtype::audio) {
    waitForLeader = true;
    leaderType = threadtype::video;
  } else if (primetype == primepcr && streamType != threadtype::pcr) {
    waitForLeader = true;
    leaderType = threadtype::pcr;
  }

  const bool leaderActive = !waitForLeader || isStreamActive(leaderType);
  if (waitForLeader && leaderActive) {
    UT_LOG_INFO("Prime phase follower waiting: stream=%s leader=%s",
                primeStreamName(streamType), primeStreamName(leaderType));
  }

  if (waitForLeader && leaderActive &&
      !mControlListener->waitForPrimedEvent(std::chrono::seconds(5),
                                            leaderType)) {
    UT_LOG_ERROR("Timed out waiting for the leading stream to prime");
    UT_ASSERT_TRUE(false);
  } else if (waitForLeader && leaderActive) {
    UT_LOG_INFO("Prime phase follower released: stream=%s leader=%s",
                primeStreamName(streamType), primeStreamName(leaderType));
  } else if (waitForLeader) {
    UT_LOG_INFO("Prime phase leader inactive; follower continues: stream=%s leader=%s",
                primeStreamName(streamType), primeStreamName(leaderType));
  }
}

void PrimePhaseSynchronizer::drop(threadtype streamType) {
  UT_LOG_INFO("Prime phase participant dropping: stream=%s",
              primeStreamName(streamType));
  arriveAtStart(true, streamType);
  UT_LOG_INFO("Prime phase participant dropped: stream=%s",
              primeStreamName(streamType));
}

void PrimePhaseSynchronizer::finishConfiguration() {
  std::unique_lock<std::mutex> lock(mMutex);
  const size_t generation = mFinishGeneration;

  ++mFinishArrivals;
  UT_LOG_INFO("Prime phase finish arrival: generation=%zu arrivals=%zu participants=%zu",
              generation, mFinishArrivals, mParticipantCount);

  if (mFinishArrivals == mParticipantCount) {
    UT_LOG_INFO("Prime phase complete; restarting clock: generation=%zu participants=%zu",
                generation, mParticipantCount);
    AVCLOCK_STOP_SUCCESS(mClockHandle, mClockController, true);
    AVCLOCK_START_SUCCESS(mClockHandle, mClockController, true);
    mFinishArrivals = 0;
    ++mFinishGeneration;
    UT_LOG_INFO("Prime phase clock restarted: completedGeneration=%zu nextGeneration=%zu",
                generation, mFinishGeneration);
    lock.unlock();
    mCV.notify_all();
    return;
  }

  UT_LOG_INFO("Prime phase finish waiting: generation=%zu arrivals=%zu participants=%zu",
              generation, mFinishArrivals, mParticipantCount);
  mCV.wait(lock, [&] { return mFinishGeneration != generation; });
  UT_LOG_INFO("Prime phase finish released: completedGeneration=%zu nextGeneration=%zu",
              generation, mFinishGeneration);
}

void PrimePhaseSynchronizer::arriveAtStart(bool isDropping,
                                           threadtype streamType) {
  std::unique_lock<std::mutex> lock(mMutex);
  const size_t generation = mStartGeneration;

  // A drop is an arrival for this generation and a withdrawal from later ones.
  ++mStartArrivals;
  if (isDropping) {
    ++mDrops;
    if (streamType == threadtype::audio) {
      mAudioActive = false;
    } else if (streamType == threadtype::video) {
      mVideoActive = false;
    } else {
      mPcrActive = false;
    }
  } else {
    mPhaseHasWork = true;
  }

  UT_LOG_INFO("Prime phase start arrival: generation=%zu stream=%s dropping=%s arrivals=%zu participants=%zu pendingDrops=%zu",
              generation, primeStreamName(streamType),
              isDropping ? "true" : "false", mStartArrivals,
              mParticipantCount, mDrops);

  if (mStartArrivals == mParticipantCount) {
    if (mPhaseHasWork) {
      UT_LOG_INFO("Prime phase resetting primed event: generation=%zu",
                  generation);
      mControlListener->resetPrimedEvent();
    }
    mParticipantCount -= mDrops;
    mStartArrivals = 0;
    mDrops = 0;
    mPhaseHasWork = false;
    ++mStartGeneration;
    UT_LOG_INFO("Prime phase start released: completedGeneration=%zu nextGeneration=%zu remainingParticipants=%zu",
                generation, mStartGeneration, mParticipantCount);
    lock.unlock();
    mCV.notify_all();
    return;
  }

  UT_LOG_INFO("Prime phase start waiting: generation=%zu stream=%s arrivals=%zu participants=%zu",
              generation, primeStreamName(streamType), mStartArrivals,
              mParticipantCount);
  mCV.wait(lock, [&] { return mStartGeneration != generation; });
  UT_LOG_INFO("Prime phase start wait released: completedGeneration=%zu stream=%s nextGeneration=%zu",
              generation, primeStreamName(streamType), mStartGeneration);
}

bool PrimePhaseSynchronizer::isStreamActive(threadtype streamType) {
  std::lock_guard<std::mutex> lock(mMutex);
  if (streamType == threadtype::audio) {
    return mAudioActive;
  }
  if (streamType == threadtype::video) {
    return mVideoActive;
  }
  return mPcrActive;
}

void runAudioClockTest(std::shared_ptr<AVClockPrimeTest> clockTest,
                       const testConfiguration_t& testConfig, bool isSecure,
                       bool isTunnel, bool isConcurrentMode, contentprimetype primetype,
                       sp<IAVClockController> &avClockController, FrameOutputMode& isAudioTunnelMode, bool& shouldSkipAudio) {
  try {
    std::optional<AudioConfig> mAudioList;

    auto &audioDecoderList = VTS_L3_AVClock::mAudioDecoderList;
    auto &audioSinkList = VTS_L3_AVClock::mAudioSinkList;

    // std::lock_guard<std::mutex> lock(AVClockPrimeTest::mMutex);
    // UT_LOG_INFO("Running audio test for clockId: %d", mClockId.value)

    if (audioDecoderList.empty() || audioSinkList.empty()) {
      UT_LOG_WARNING("Audio Decoder or Sink list empty. Skipping test.");
      return;
    }

    if (isConcurrentMode) {
      // Concurrent Mode: Get decoder and sink using clockId
      auto resultDec =
          getTupleElements(audioDecoderList, clockTest->getClockId(),
                           std::index_sequence<0, 1, 2, 3, 4>{});
      auto resultSink = getTupleElements(audioSinkList, clockTest->getClockId(),
                                         std::index_sequence<0, 1, 2, 3>{});

      if (resultDec && resultSink) {
        auto &[audioDecId, audioDecHandle, audioDecControlListener,
               audioDecEventListener, audioDecCapabilities] = *resultDec;
        auto &[audioSinkId, audioSinkHandle, audioSinkControlListener,
               audioSinkEventListener] = *resultSink;

        mAudioList = AudioConfig{audioDecId,
                                 audioDecHandle,
                                 audioDecControlListener,
                                 audioDecEventListener,
                                 audioDecCapabilities,
                                 audioSinkId,
                                 audioSinkHandle,
                                 audioSinkControlListener,
                                 audioSinkEventListener,
                                 isSecure,
                                 isTunnel,
                                 testConfig};

        UT_LOG_INFO(
            "Concurrent mode: Using decoder %d and sink %d for clockId %d",
            audioDecId, audioSinkId, clockTest->getClockId());
      } else {
        UT_LOG_WARNING(
            "Concurrent mode: Decoder or Sink not found for clockId %d",
            clockTest->getClockId());
        return;
      }
    } else {
      // Sequential Mode: Use first decoder and sink
      const auto &[audioDecId, audioDecHandle, audioDecControlListener,
                   audioDecEventListener,
                   audioDecCapabilities] = audioDecoderList.front();
      const auto &[audioSinkId, audioSinkHandle, audioSinkControlListener,
                   audioSinkEventListener] = audioSinkList.front();

      mAudioList = AudioConfig{audioDecId,
                               audioDecHandle,
                               audioDecControlListener,
                               audioDecEventListener,
                               audioDecCapabilities,
                               audioSinkId,
                               audioSinkHandle,
                               audioSinkControlListener,
                               audioSinkEventListener,
                               isSecure,
                               isTunnel,
                               testConfig};

      UT_LOG_INFO(
          "Sequential mode: Using decoder %d and sink %d for clockId %d",
          audioDecId, audioSinkId, clockTest->getClockId());
    }

    if (mAudioList.has_value())
    {
        if(isSecure && !(mAudioList->decCaps->supportsSecure))
        {
          UT_LOG_WARNING("Audio Decoder don't support Secure \n");
          return;
        }
    }

    if (mAudioList) {
      UT_LOG_INFO("Prime Audio Thread about to start");

      sp<IAVClock> clockHandle = clockTest->getClockHandle();
      double playbackRate = clockTest->getPlaybackRate();
      auto clockControllerListener = clockTest->getControlListener();
      clockTest->setAudioProperties(mAudioList->decCtrlListener, isTunnel);

      UT_LOG_INFO("Prime Audio Validation Thread about to start");
      std::thread validatepts(
          [clockTest, clockHandle, avClockController, playbackRate, primetype, clockControllerListener]() mutable {
            clockTest->validateClockTime(clockTest, clockHandle, avClockController,
                                         playbackRate, primetype, clockControllerListener, threadtype::audio);
          });

      std::thread primeAudioThread(
          [clockTest, audioCfg = *mAudioList, avClockController]() mutable {
            clockTest->runAudioTest(audioCfg, avClockController);
          });

      if (validatepts.joinable()) {
        validatepts.join();
      }

      if (primeAudioThread.joinable()) {

          if((isAudioTunnelMode == FrameOutputMode::UNKNOWN) && (isTunnel))
          {
              FrameOutputMode outPutMode = mAudioList->decCtrlListener->getFrameOutputMode();
              UT_LOG_INFO("Detected frame output mode: %d", outPutMode);

              isAudioTunnelMode = outPutMode;

              if((outPutMode == FrameOutputMode::TUNNEL) && (isTunnel) )
              {
                  UT_LOG_INFO("Detected tunnel mode for audio output");
                  shouldSkipAudio = false;
              } else {
                  UT_LOG_INFO("Detected non-tunnel mode for audio output");
                  shouldSkipAudio = true;
              }
          }

        UT_LOG_INFO("Before join mode=%d",
            mAudioList->decCtrlListener->getFrameOutputMode());

        primeAudioThread.join();

        UT_LOG_INFO("After join mode=%d",
            mAudioList->decCtrlListener->getFrameOutputMode());
      }

      UT_LOG_INFO("Audio Clock Test completed.");
    } else {
      UT_LOG_WARNING("AudioConfig could not be initialized.");
    }

  } catch (const std::exception &ex) {
    UT_LOG_ERROR("Exception in runAudioClockTest: %s", ex.what());
  }
}

void runVideoClockTest(std::shared_ptr<AVClockPrimeTest> clockTest,
                       const videoDecoderTestConfiguration_t& sinkTestConfig,
                       bool isSecure,
                       bool isTunnel, bool isConcurrentMode, contentprimetype primetype,
                       sp<IAVClockController> &avClockController) {
  try {
    std::optional<VideoConfig> mVideoList;

    auto &videoDecoderList = VTS_L3_AVClock::mVideoDecoderList;
    auto &videoSinkList = VTS_L3_AVClock::mVideoSinkList;

    if (videoDecoderList.empty() || videoSinkList.empty()) {
      UT_LOG_WARNING("Video Decoder or Sink list empty. Skipping test.");
      return;
    }

    if (isConcurrentMode) {
      //  Concurrent Mode: Fetch decoder and sink by clockId

      auto resultDec =
          getTupleElements(videoDecoderList, clockTest->getClockId(),
                           std::index_sequence<0, 1, 2, 3, 4>{});
      auto resultSink = getTupleElements(videoSinkList, clockTest->getClockId(),
                                         std::index_sequence<0, 1, 2, 3>{});

      if (resultDec && resultSink) {
        auto &[videoDecId, videoDecHandle, videoDecControlListener,
               videoDecEventListener, videoDecCapabilities] = *resultDec;
        auto &[videoSinkId, videoSinkHandle, videoSinkControlListener,
               videoSinkEventListener] = *resultSink;

        mVideoList = VideoConfig{videoDecId,
                                 videoDecHandle,
                                 videoDecControlListener,
                                 videoDecEventListener,
                                 videoDecCapabilities,
                                 videoSinkId,
                                 videoSinkHandle,
                                 videoSinkControlListener,
                                 videoSinkEventListener,
                                 isSecure,
                                 isTunnel,
                                 sinkTestConfig,
                                 sinkTestConfig};

        UT_LOG_INFO("Concurrent mode: Using video decoder %d and sink %d for "
                    "clockId %d",
                    videoDecId, videoSinkId, clockTest->getClockId());
      } else {
        UT_LOG_WARNING(
            "Concurrent mode: Decoder or Sink not found for clockId %d",
            clockTest->getClockId());
        return;
      }
    } else {
      // Sequential Mode: Use first decoder and sink
      const auto &[videoDecId, videoDecHandle, videoDecControlListener,
                   videoDecEventListener,
                   videoDecCapabilities] = videoDecoderList.front();
      const auto &[videoSinkId, videoSinkHandle, videoSinkControlListener,
                   videoSinkEventListener] = videoSinkList.front();

      mVideoList = VideoConfig{videoDecId,
                               videoDecHandle,
                               videoDecControlListener,
                               videoDecEventListener,
                               videoDecCapabilities,
                               videoSinkId,
                               videoSinkHandle,
                               videoSinkControlListener,
                               videoSinkEventListener,
                               isSecure,
                               isTunnel,
                               sinkTestConfig,
                               sinkTestConfig};

      UT_LOG_INFO(
          "Sequential mode: Using video decoder %d and sink %d for clockId %d",
          videoDecId, videoSinkId, clockTest->getClockId());
    }

    if (mVideoList.has_value())
    {
        if(isSecure && !(mVideoList->decCaps->supportsSecure))
        {
          UT_LOG_WARNING("Video Decoder don't support Secure \n");
          return;
        }
    }

    if (mVideoList) {
      UT_LOG_INFO("Prime Video Thread about to start");

      sp<IAVClock> clockHandle = clockTest->getClockHandle();
      double playbackRate = clockTest->getPlaybackRate();
      auto clockControllerListener = clockTest->getControlListener();

      UT_LOG_INFO("Prime Video Validation Thread about to start");

      std::thread validatepts(
          [clockTest, clockHandle, avClockController, playbackRate, primetype, clockControllerListener]() mutable {
            clockTest->validateClockTime(clockTest, clockHandle, avClockController,
                                         playbackRate, primetype, clockControllerListener, threadtype::video);
          });
      std::thread primeVideoThread(
          [clockTest, videoCfg = *mVideoList, avClockController]() mutable {
            clockTest->runVideoTest(videoCfg, avClockController);
          });

      if (validatepts.joinable()) {
        validatepts.join();
      }

      if (primeVideoThread.joinable()) {
        primeVideoThread.join();
      }

      UT_LOG_INFO("Video Clock Test completed.");
    } else {
      UT_LOG_WARNING("VideoConfig could not be initialized.");
    }

  } catch (const std::exception &ex) {
    UT_LOG_ERROR("Exception in runVideoClockTest: %s", ex.what());
  }
}

void runPCRClockTest(std::shared_ptr<AVClockPrimeTest> clockTest,
                     const std::vector<int64_t>& pcrValues,
                     contentprimetype primetype, sp<IAVClockController> &avClockController) {
  try {
    // Launch PCR test in a thread using the existing pcrPrimeTest instance
    UT_LOG_INFO("Prime PCR Thread about to start");

    sp<IAVClock> clockHandle = clockTest->getClockHandle();
    double playbackRate = clockTest->getPlaybackRate();
    auto clockControllerListener = clockTest->getControlListener();

    UT_LOG_INFO("Prime PCR Validation Thread about to start");
    std::thread validatepts(
        [clockTest, clockHandle, avClockController, playbackRate, primetype, clockControllerListener]() mutable {
          clockTest->validateClockTime(clockTest,clockHandle, avClockController,
                                       playbackRate, primetype, clockControllerListener, threadtype::pcr);
        });

    std::thread primePcrThread(
        [clockTest, pcrValues, avClockController]() mutable {
          try {
            const int64_t firstPcrValue = pcrValues.front();
            int64_t previousPcrValue = firstPcrValue;
            const auto streamStart = std::chrono::steady_clock::now();
            for (const auto pcrValue : pcrValues) {
              if (pcrValue < firstPcrValue) {
                UT_LOG_ERROR("PCR values must be monotonic: first=%lld current=%lld",
                             static_cast<long long>(previousPcrValue),
                             static_cast<long long>(pcrValue));
                UT_ASSERT_TRUE(false);
                return;
              }
              previousPcrValue = pcrValue;

              const auto targetTime = streamStart +
                  std::chrono::nanoseconds(pcrValue - firstPcrValue);
              std::this_thread::sleep_until(targetTime);

              PcrConfig config{pcrValue};
              clockTest->runPcrTest(config, avClockController);
            }
          } catch (const std::exception &ex) {
            UT_LOG_ERROR("Exception in PCR test thread: %s", ex.what());
          }
        });

    if (validatepts.joinable()) {
      UT_LOG_DEBUG("Joining validation thread");
      validatepts.join();
    }

    if (primePcrThread.joinable()) {
      UT_LOG_DEBUG("Joining PCR Thread");
      primePcrThread.join();
    }
    UT_LOG_INFO("Sequential PCR Clock Test completed successfully.");
  } catch (const std::exception &ex) {
    UT_LOG_ERROR("Exception in runSequentialPCRClock: %s", ex.what());
    UT_ASSERT_TRUE(false);
  } catch (...) {
    UT_LOG_ERROR("Unknown exception in runSequentialPCRClock");
    UT_ASSERT_TRUE(false);
  }
}

static bool isAudioSecureModeSupported(size_t clockIndex) {
  const auto decoder = getTupleElements(
      VTS_L3_AVClock::mAudioDecoderList, clockIndex,
      std::index_sequence<0, 1, 2, 3, 4>{});
  return decoder.has_value() && std::get<4>(*decoder) != nullptr &&
         std::get<4>(*decoder)->supportsSecure;
}

static bool isVideoSecureModeSupported(size_t clockIndex) {
  const auto decoder = getTupleElements(
      VTS_L3_AVClock::mVideoDecoderList, clockIndex,
      std::index_sequence<0, 1, 2, 3, 4>{});
  return decoder.has_value() && std::get<4>(*decoder) != nullptr &&
         std::get<4>(*decoder)->supportsSecure;
}

static bool isAVClockTunnelModeSupported() {
  std::vector<videodecoder::OperationalMode> supportedModes;
  const android::binder::Status status =
      gVideoDecoderManager->getSupportedOperationalModes(&supportedModes);

  if (!status.isOk()) {
    UT_LOG_WARNING("Unable to query platform tunnel-mode capability");
    return false;
  }

  return std::find(supportedModes.begin(), supportedModes.end(),
                   videodecoder::OperationalMode::TUNNELLED) !=
         supportedModes.end();
}

void runAudioPrimeTest(std::shared_ptr<AVClockPrimeTest> clockTest,
                       bool isSecureMode, bool isTunnel, bool isConcurrentMode,
                       contentprimetype primetype, android::sp<IAVClockController> &clockController,
                       const std::shared_ptr<PrimePhaseSynchronizer>& phaseSynchronizer) {
  UT_LOG_INFO("Started Thread : %s", __func__);

  const size_t clockIndex = static_cast<size_t>(clockTest->getClockId());

  if (isSecureMode && !isAudioSecureModeSupported(clockIndex)) {
    UT_IGNORE_TEST() << "Skipping audio prime case: AVClock instance " << clockIndex
                 << " does not support secure audio decoding";
    return;
  }

  vector<testConfiguration_t> testConfiguration;
  const char *DecodeStreams = "PrimeAudio";
  FrameOutputMode isAudioTunnelMode = FrameOutputMode::UNKNOWN;
  bool shouldSkipAudio = false;

  vts_audiodecoder_parse_testconfig(gAudioPrimeTestConfiguration,
                                    gAudioModuleName, gTestGroup, DecodeStreams,
                                    testConfiguration);

  for (auto &test : testConfiguration) {

    if (shouldSkipAudio) {
      UT_IGNORE_TEST() << "Skipping audio prime case due to platform don't support Tunnel mode";
      continue;
    }
    phaseSynchronizer->beginConfiguration(threadtype::audio, primetype);
    std::thread audioclockthread([&, test = std::move(test)] {
      runAudioClockTest(clockTest, test, isSecureMode, isTunnel,
                        isConcurrentMode, primetype, clockController, isAudioTunnelMode, shouldSkipAudio);
    });

    // Join immediately after starting
    if (audioclockthread.joinable())
      audioclockthread.join();

    phaseSynchronizer->finishConfiguration();

    UT_LOG_INFO("runAudioPrimeTest Completed");
  }
  phaseSynchronizer->drop(threadtype::audio);
}

void runVideoPrimeTest(std::shared_ptr<AVClockPrimeTest> clockTest,
                       bool isSecureMode, bool isTunnel, bool isConcurrentMode,
                       contentprimetype primetype, android::sp<IAVClockController> &clockController,
                       const std::shared_ptr<PrimePhaseSynchronizer>& phaseSynchronizer) {
  UT_LOG_INFO("Started Thread : %s", __func__);

  const size_t clockIndex = static_cast<size_t>(clockTest->getClockId());

  if (isSecureMode && !isVideoSecureModeSupported(clockIndex)) {
    UT_IGNORE_TEST() << "Skipping video prime case: AVClock instance " << clockIndex
                 << " does not support secure video decoding";
  }
  if (isTunnel && !isAVClockTunnelModeSupported()) {
    UT_IGNORE_TEST() << "Platform does not support video tunnel mode";
  }

  vector<videoDecoderTestConfiguration_t> sinkConfigs;

  const char *DecodeStreams = "PrimeVideo";

  vts_videodecoder_parse_testconfig(gVideoPrimeTestConfiguration, VIDEOSINKMGR,
                                    gTestGroup, DecodeStreams, sinkConfigs);
  if (sinkConfigs.empty()) {
    phaseSynchronizer->drop(threadtype::video);
    std::string errorMsg = std::string("No valid test configurations found - missing test streams or invalid configuration for test: ") + DecodeStreams;
    UT_FAIL_FATAL(errorMsg.c_str());
    return;
  }

  for (size_t i = 0; i < sinkConfigs.size(); ++i) {
    auto &sinkConfig = sinkConfigs[i];

    phaseSynchronizer->beginConfiguration(threadtype::video, primetype);
    std::thread videoclockthread([&, sinkConfig] {
      runVideoClockTest(clockTest, sinkConfig, isSecureMode,
                        isTunnel, isConcurrentMode, primetype,
                        clockController);
    });

    // Join immediately after starting
    if (videoclockthread.joinable())
      videoclockthread.join();

    phaseSynchronizer->finishConfiguration();

    UT_LOG_INFO("runVideoPrimeTest Completed");
  }
  phaseSynchronizer->drop(threadtype::video);
}

void runPcrPrimeTest(std::shared_ptr<AVClockPrimeTest> clockTest, bool isSecureMode,
                     bool isTunnel, bool isConcurrentMode, contentprimetype primetype,
                     android::sp<IAVClockController> &clockController,
                     const std::shared_ptr<PrimePhaseSynchronizer>& phaseSynchronizer) {
  UT_LOG_INFO("Started Thread : %s", __func__);

  const size_t clockIndex = static_cast<size_t>(clockTest->getClockId());

  if (isSecureMode && (!isAudioSecureModeSupported(clockIndex) ||
                       !isVideoSecureModeSupported(clockIndex))) {
    UT_IGNORE_TEST() << "Skipping PCR prime case: AVClock instance " << clockIndex
                 << " does not support secure audio and video decoding";
    return;
  }
  if (isTunnel && !isAVClockTunnelModeSupported()) {
    UT_IGNORE_TEST() << "Skipping PCR prime case: platform does not support tunnel mode";
    return;
  }

  std::vector<int64_t> pcrValues;
  const char *ParsePCR = "ParsePCR";

  vts_avclock_parse_testconfig(gAVClockTestConfiguration, gModuleName,
                               gTestGroup, ParsePCR, pcrValues);

  if (pcrValues.empty()) {
    UT_LOG_ERROR("No PCR samples found for %s", ParsePCR);
    phaseSynchronizer->drop(threadtype::pcr);
    return;
  }

  phaseSynchronizer->beginConfiguration(threadtype::pcr, primetype);
  std::thread pcrclockthread([&, pcrValues] {
    runPCRClockTest(clockTest, pcrValues, primetype, clockController);
  });

  if (pcrclockthread.joinable())
    pcrclockthread.join();

  phaseSynchronizer->finishConfiguration();

  UT_LOG_INFO("runPcrPrimeTest Completed");
  phaseSynchronizer->drop(threadtype::pcr);
}

static size_t getPrimePhaseParticipantCount(contentprimetype primetype) {
  if (primetype == primeaudiobeforevideo ||
      primetype == primevideobeforeaudio) {
    return 2;
  }
  if (primetype == primepcr) {
    return 3;
  }
  return 1;
}

void runAVClockPrimeTest(const char *testName, bool isSecureMode, bool isTunnel,
                         bool isConcurrentMode, avclock::ClockMode clockMode,
                         contentprimetype primetype, double playbackRate) {
  vector<shared_ptr<AVClockPrimeTest>> clockTests;
  vector<std::thread> primeThreads;
  std::vector<sp<IAVClockController>> clockControllerList;
  std::thread runaudioprimethread;
  std::thread runvideoprimethread;
  std::thread runpcrprimethread;

  std::mutex mtx;
  std::condition_variable cv;
  bool audioReady = false, videoReady = false, pcrReady = false;

  auto cleanupAndReturn = [&](std::thread &t1, std::thread &t2,
                              std::thread &t3) {
    if (t1.joinable())
      t1.detach();
    if (t2.joinable())
      t2.detach();
    if (t3.joinable())
      t3.detach();
  };

  auto waitForReady = [&](bool &flag, const char *name) {
    std::unique_lock<std::mutex> lock(mtx);
    if (!cv.wait_for(lock, std::chrono::milliseconds(200),
                     [&] { return flag; })) {
      UT_LOG_ERROR(
          "Timeout %s thread: Timed out",
          name);
      return false;
    }
    return true;
  };

  auto &avClockList = VTS_L3_AVClock::mAVClockList;
  auto &avBufferManager = VTS_L3_AVClock::mAVBufferManager;
  auto &avBufferHelper = VTS_L3_AVClock::mAVbufferHelper;

  if (isConcurrentMode) {
    for (const auto &[_clockId, clockHandle, controlListener, eventListener] :
         avClockList) {
      if (avClockList.size() == 1) {
        UT_IGNORE_TEST() << "Platform Supports One Instance Skipping Concurrent Case "
                     << "same as Sequential";
      }
      auto clockTest = std::make_shared<AVClockPrimeTest>(
          _clockId, clockHandle, clockMode, controlListener, avBufferManager,
          avBufferHelper, playbackRate);

      // Store in vector
      clockTests.push_back(clockTest);
      auto storedClockTest = clockTests.back();

      // Setup and start clock
      auto clockController = storedClockTest->setupAndStartClock();
      clockControllerList.push_back(clockController);
        auto phaseSynchronizer = std::make_shared<PrimePhaseSynchronizer>(
          clockHandle, clockController, controlListener,
          getPrimePhaseParticipantCount(primetype));

      // Prime Audio Only
      if (primetype == primeaudio) {
        runaudioprimethread = std::thread([&, storedClockTest, phaseSynchronizer, ctrl = clockControllerList.back()]() mutable {
          {
            std::lock_guard<std::mutex> lock(mtx);
            audioReady = true;
          }
          cv.notify_all();
          runAudioPrimeTest(storedClockTest, isSecureMode, isTunnel,
                            isConcurrentMode, primetype, ctrl, phaseSynchronizer);
        });

        if (!waitForReady(audioReady, "Audio")) {
          cleanupAndReturn(runaudioprimethread, runvideoprimethread,
                           runpcrprimethread);
          return;
        }
        primeThreads.push_back(std::move(runaudioprimethread));
      }

      // Prime Video Only
      else if (primetype == primevideo) {
        runvideoprimethread = std::thread([&, storedClockTest, phaseSynchronizer, ctrl = clockControllerList.back()]() mutable {
          {
            std::lock_guard<std::mutex> lock(mtx);
            videoReady = true;
          }
          runVideoPrimeTest(storedClockTest, isSecureMode, isTunnel,
                            isConcurrentMode, primetype, ctrl, phaseSynchronizer);
          cv.notify_all();
        });

        if (!waitForReady(videoReady, "Video")) {
          cleanupAndReturn(runaudioprimethread, runvideoprimethread,
                           runpcrprimethread);
          return;
        }
        primeThreads.push_back(std::move(runvideoprimethread));
      }

      // Audio Before Video
      else if (primetype == primeaudiobeforevideo) {
        runaudioprimethread = std::thread([&, storedClockTest, phaseSynchronizer, ctrl = clockControllerList.back()]() mutable {
          {
            std::lock_guard<std::mutex> lock(mtx);
            audioReady = true;
          }
          runAudioPrimeTest(storedClockTest, isSecureMode, isTunnel,
                            isConcurrentMode, primetype, ctrl, phaseSynchronizer);
          cv.notify_all();
        });

        if (!waitForReady(audioReady, "Audio")) {
          cleanupAndReturn(runaudioprimethread, runvideoprimethread,
                           runpcrprimethread);
          return;
        }

        primeThreads.push_back(std::move(runaudioprimethread));

        runvideoprimethread = std::thread([&, storedClockTest, phaseSynchronizer, ctrl = clockControllerList.back()]() mutable {
          {
            std::lock_guard<std::mutex> lock(mtx);
            videoReady = true;
          }
          runVideoPrimeTest(storedClockTest, isSecureMode, isTunnel,
                            isConcurrentMode, primetype, ctrl, phaseSynchronizer);
          cv.notify_all();
        });

        if (!waitForReady(videoReady, "Video")) {
          cleanupAndReturn(runaudioprimethread, runvideoprimethread,
                           runpcrprimethread);
          return;
        }
        primeThreads.push_back(std::move(runvideoprimethread));
      }

      // Video Before Audio
      else if (primetype == primevideobeforeaudio) {
        runvideoprimethread = std::thread([&, storedClockTest, phaseSynchronizer, ctrl = clockControllerList.back()]() mutable {
          {
            std::lock_guard<std::mutex> lock(mtx);
            videoReady = true;
          }
          runVideoPrimeTest(storedClockTest, isSecureMode, isTunnel,
                            isConcurrentMode, primetype, ctrl, phaseSynchronizer);
          cv.notify_all();
        });

        if (!waitForReady(videoReady, "Video")) {
          cleanupAndReturn(runaudioprimethread, runvideoprimethread,
                           runpcrprimethread);
          return;
        }

        primeThreads.push_back(std::move(runvideoprimethread));

        runaudioprimethread = std::thread([&, storedClockTest, phaseSynchronizer, ctrl = clockControllerList.back()]() mutable {
          {
            std::lock_guard<std::mutex> lock(mtx);
            audioReady = true;
          }
          runAudioPrimeTest(storedClockTest, isSecureMode, isTunnel,
                            isConcurrentMode, primetype, ctrl, phaseSynchronizer);
          cv.notify_all();
        });

        if (!waitForReady(audioReady, "Audio")) {
          cleanupAndReturn(runaudioprimethread, runvideoprimethread,
                           runpcrprimethread);
          return;
        }

        primeThreads.push_back(std::move(runaudioprimethread));
      }

      // PCR Case
      else if (primetype == primepcr) {
        runaudioprimethread = std::thread([&, storedClockTest, phaseSynchronizer, ctrl = clockControllerList.back()]() mutable {
          {
            std::lock_guard<std::mutex> lock(mtx);
             audioReady = true;
          }
          runAudioPrimeTest(storedClockTest, isSecureMode, isTunnel,
                            isConcurrentMode, primetype, ctrl, phaseSynchronizer);
          cv.notify_all();
        });

        if (!waitForReady(audioReady, "Audio")) {
          cleanupAndReturn(runaudioprimethread, runvideoprimethread,
                           runpcrprimethread);
          return;
        }

        primeThreads.push_back(std::move(runaudioprimethread));

        runvideoprimethread = std::thread([&, storedClockTest, phaseSynchronizer, ctrl = clockControllerList.back()]() mutable {
          {
            std::lock_guard<std::mutex> lock(mtx);
            videoReady = true;
          }
          runVideoPrimeTest(storedClockTest, isSecureMode, isTunnel,
                            isConcurrentMode, primetype, ctrl, phaseSynchronizer);
          cv.notify_all();
        });

        if (!waitForReady(videoReady, "Video")) {
          cleanupAndReturn(runaudioprimethread, runvideoprimethread,
                           runpcrprimethread);
          return;
        }

        primeThreads.push_back(std::move(runvideoprimethread));

        runpcrprimethread = std::thread([&, storedClockTest, phaseSynchronizer, ctrl = clockControllerList.back()]() mutable {
          {
            std::lock_guard<std::mutex> lock(mtx);
            pcrReady = true;
          }
          runPcrPrimeTest(storedClockTest, isSecureMode, isTunnel, isConcurrentMode, primetype,
                          ctrl, phaseSynchronizer);
          cv.notify_all();
        });

        if (!waitForReady(pcrReady, "PCR")) {
          cleanupAndReturn(runaudioprimethread, runvideoprimethread,
                           runpcrprimethread);
          return;
        }

        primeThreads.push_back(std::move(runpcrprimethread));
      }
    }
    // Final cleanup: join all threads
    for (auto &t : primeThreads) {
      if (t.joinable())
        t.join();
    }

    for (size_t i = 0; i < clockTests.size(); ++i) {
      auto &_clockTest = clockTests[i];
      auto &_clockController = clockControllerList[i];
      sp<AVClockEventListener> _eventlistener = std::get<3>(VTS_L3_AVClock::mAVClockList[i]);
      AVCLOCK_UNREGISTER_EVENT_SUCCESS(_clockTest->getClockHandle(), _eventlistener);
      _clockTest->shutdownClock(_clockController);
    }
  } else {
    for (const auto &[_clockId, clockHandle, controlListener, eventListener] :
         avClockList) {
      auto clockTest = std::make_shared<AVClockPrimeTest>(
          _clockId, clockHandle, clockMode, controlListener, avBufferManager,
          avBufferHelper, playbackRate);

      // Call the method on the instance
      auto avclockController = clockTest->setupAndStartClock();
        auto phaseSynchronizer = std::make_shared<PrimePhaseSynchronizer>(
          clockHandle, avclockController, controlListener,
          getPrimePhaseParticipantCount(primetype));

      // Prime Audio Only
      if (primetype == primeaudio) {
        runaudioprimethread = std::thread([&, clockTest, phaseSynchronizer] {
          {
            std::lock_guard<std::mutex> lock(mtx);
            audioReady = true;
          }
          runAudioPrimeTest(clockTest, isSecureMode, isTunnel, isConcurrentMode, primetype,
                            avclockController, phaseSynchronizer);
          cv.notify_all();
        });

        if (!waitForReady(audioReady, "Audio")) {
          cleanupAndReturn(runaudioprimethread, runvideoprimethread,
                           runpcrprimethread);
          return;
        }
      }

      // Prime Video Only
      else if (primetype == primevideo) {
        runvideoprimethread = std::thread([&, clockTest, phaseSynchronizer] {
          {
            std::lock_guard<std::mutex> lock(mtx);
            videoReady = true;
          }
          runVideoPrimeTest(clockTest, isSecureMode, isTunnel, isConcurrentMode, primetype,
                            avclockController, phaseSynchronizer);
          cv.notify_all();
        });

        if (!waitForReady(videoReady, "Video")) {
          cleanupAndReturn(runaudioprimethread, runvideoprimethread,
                           runpcrprimethread);
          return;
        }
      }

      // Audio Before Video
      else if (primetype == primeaudiobeforevideo) {
        runaudioprimethread = std::thread([&, clockTest, phaseSynchronizer] {
          {
            std::lock_guard<std::mutex> lock(mtx);
            audioReady = true;
          }
          runAudioPrimeTest(clockTest, isSecureMode, isTunnel, isConcurrentMode, primetype,
                            avclockController, phaseSynchronizer);
          cv.notify_all();
        });

        if (!waitForReady(audioReady, "Audio")) {
          cleanupAndReturn(runaudioprimethread, runvideoprimethread,
                           runpcrprimethread);
          return;
        }

        runvideoprimethread = std::thread([&, clockTest, phaseSynchronizer] {
          {
            std::lock_guard<std::mutex> lock(mtx);
            videoReady = true;
          }
          runVideoPrimeTest(clockTest, isSecureMode, isTunnel, isConcurrentMode, primetype,
                            avclockController, phaseSynchronizer);
          cv.notify_all();
        });

        if (!waitForReady(videoReady, "Video")) {
          cleanupAndReturn(runaudioprimethread, runvideoprimethread,
                           runpcrprimethread);
          return;
        }
      }

      // Video Before Audio
      else if (primetype == primevideobeforeaudio) {
        runvideoprimethread = std::thread([&, clockTest, phaseSynchronizer] {
          {
            std::lock_guard<std::mutex> lock(mtx);
            videoReady = true;
          }
          runVideoPrimeTest(clockTest, isSecureMode, isTunnel, isConcurrentMode, primetype,
                            avclockController, phaseSynchronizer);
          cv.notify_all();
        });

        if (!waitForReady(videoReady, "Video")) {
          cleanupAndReturn(runaudioprimethread, runvideoprimethread,
                           runpcrprimethread);
          return;
        }

        runaudioprimethread = std::thread([&, clockTest, phaseSynchronizer] {
          {
            std::lock_guard<std::mutex> lock(mtx);
            audioReady = true;
          }
          runAudioPrimeTest(clockTest, isSecureMode, isTunnel, isConcurrentMode, primetype,
                            avclockController, phaseSynchronizer);
          cv.notify_all();
        });

        if (!waitForReady(audioReady, "Audio")) {
          cleanupAndReturn(runaudioprimethread, runvideoprimethread,
                           runpcrprimethread);
          return;
        }
      }

      // PCR Case
      else if (primetype == primepcr) {
        runaudioprimethread = std::thread([&, clockTest, phaseSynchronizer] {
          {
            std::lock_guard<std::mutex> lock(mtx);
            audioReady = true;
          }
          runAudioPrimeTest(clockTest, isSecureMode, isTunnel, isConcurrentMode, primetype,
                            avclockController, phaseSynchronizer);
          cv.notify_all();
        });

        if (!waitForReady(audioReady, "Audio")) {
          cleanupAndReturn(runaudioprimethread, runvideoprimethread,
                           runpcrprimethread);
          return;
        }

        runvideoprimethread = std::thread([&, clockTest, phaseSynchronizer] {
          {
            std::lock_guard<std::mutex> lock(mtx);
            videoReady = true;
          }
          runVideoPrimeTest(clockTest, isSecureMode, isTunnel, isConcurrentMode, primetype,
                            avclockController, phaseSynchronizer);
          cv.notify_all();
        });

        if (!waitForReady(videoReady, "Video")) {
          cleanupAndReturn(runaudioprimethread, runvideoprimethread,
                           runpcrprimethread);
          return;
        }

        runpcrprimethread = std::thread([&, clockTest, phaseSynchronizer] {
          {
            std::lock_guard<std::mutex> lock(mtx);
            pcrReady = true;
          }
          runPcrPrimeTest(clockTest, isSecureMode, isTunnel, isConcurrentMode, primetype, avclockController,
                          phaseSynchronizer);
          cv.notify_all();
        });

        if (!waitForReady(pcrReady, "PCR")) {
          cleanupAndReturn(runaudioprimethread, runvideoprimethread,
                           runpcrprimethread);
          return;
        }
      }

      // Final cleanup: join all threads
      if (runaudioprimethread.joinable())
        runaudioprimethread.join();
      if (runvideoprimethread.joinable())
        runvideoprimethread.join();
      if (runpcrprimethread.joinable())
        runpcrprimethread.join();

      // Unregistering Event handlers
      AVCLOCK_UNREGISTER_EVENT_SUCCESS(clockHandle, eventListener);

      // Shutdown the clock after tests
      UT_LOG_DEBUG("Entering to shutdownClock");
      clockTest->shutdownClock(avclockController);
      UT_LOG_DEBUG("Exiting to shutdownClock");
    }
  }
}

/**
 * @brief Test the AVClock functionality (sequential, non-tunnel, non-secure)
 *
 * This test verifies the basic AVClock functionality for Audio Stream in Auto
 * clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 001@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, seq_nontun_nonsec_AutoAudioOnly) {
  const char *testName = "PrimeClock_AutoAudioOnly";
  bool isSecureMode = false;
  bool isConcurrentMode = false;
  bool isTunnel = false;
  double playbackrate = 1.0;
  avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
  contentprimetype primetype = primeaudio;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (sequential, non-tunnel, non-secure)
 *
 * This test verifies the basic AVClock functionality for Video Stream in Auto
 * clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 002@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, seq_nontun_nonsec_AutoVideoOnly) {
  const char *testName = "PrimeClock_AutoVideoOnly";
  bool isSecureMode = false;
  bool isConcurrentMode = false;
  bool isTunnel = false;
  double playbackrate = 1.0;
  avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
  contentprimetype primetype = primevideo;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (sequential, non-tunnel, non-secure)
 *
 * This test verifies the basic AVClock functionality for Audio Stream in
 * AudioMaster clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 003@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, seq_nontun_nonsec_AudioMasterAudioOnly) {
  const char *testName = "PrimeClock_AudioMasterAudioOnly";
  bool isSecureMode = false;
  bool isConcurrentMode = false;
  bool isTunnel = false;
  double playbackrate = 1.0;
  avclock::ClockMode clockMode = avclock::ClockMode::AUDIO_MASTER;
  contentprimetype primetype = primeaudio;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (sequential, non-tunnel, non-secure)
 *
 * This test verifies the basic AVClock functionality for Video Stream in
 * VideoMaster clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 004@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, seq_nontun_nonsec_VideoMasterVideoOnly) {
  const char *testName = "PrimeClock_VideoMasterVideoOnly";
  bool isSecureMode = false;
  bool isConcurrentMode = false;
  bool isTunnel = false;
  double playbackrate = 1.0;
  avclock::ClockMode clockMode = avclock::ClockMode::VIDEO_MASTER;
  contentprimetype primetype = primevideo;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (sequential, non-tunnel, non-secure)
 *
 * This test verifies the basic AVClock functionality for Video Stream in
 * AudioMaster clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 005@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, seq_nontun_nonsec_AudioMasterVideoOnly) {
  const char *testName = "PrimeClock_AudioMasterVideoOnly";
  bool isSecureMode = false;
  bool isConcurrentMode = false;
  bool isTunnel = false;
  double playbackrate = 1.0;
  avclock::ClockMode clockMode = avclock::ClockMode::AUDIO_MASTER;
  contentprimetype primetype = primevideo;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (sequential, non-tunnel, non-secure)
 *
 * This test verifies the basic AVClock functionality for Audio Stream in
 * VideoMaster clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 006@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, seq_nontun_nonsec_VideoMasterAudioOnly) {
  const char *testName = "PrimeClock_VideoMasterAudioOnly";
  bool isSecureMode = false;
  bool isConcurrentMode = false;
  bool isTunnel = false;
  double playbackrate = 1.0;
  avclock::ClockMode clockMode = avclock::ClockMode::VIDEO_MASTER;
  contentprimetype primetype = primeaudio;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (sequential, non-tunnel, non-secure)
 *
 * This test verifies the basic AVClock functionality for PCR Stream in PCR
 * clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 007@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, seq_nontun_nonsec_PCRMode) {
  const char *testName = "PrimeClock_PCRMode";
  bool isSecureMode = false;
  bool isConcurrentMode = false;
  bool isTunnel = false;
  double playbackrate = 1.0;
  avclock::ClockMode clockMode = avclock::ClockMode::PCR;
  contentprimetype primetype = primepcr;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (sequential, non-tunnel, non-secure)
 *
 * This test verifies the basic AVClock functionality for Audio, Video Streams
 * in Auto clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 008@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, seq_nontun_nonsec_AutoAudioBeforeVideo) {
  const char *testName = "PrimeClock_AutoAudioBeforeVideo";
  bool isSecureMode = false;
  bool isConcurrentMode = false;
  bool isTunnel = false;
  double playbackrate = 1.0;
  avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
  contentprimetype primetype = primeaudiobeforevideo;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (sequential, non-tunnel, non-secure)
 *
 * This test verifies the basic AVClock functionality for Video, Audio Streams
 * in Auto clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 009@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, seq_nontun_nonsec_AutoVideoBeforeAudio) {
  const char *testName = "PrimeClock_AutoVideoBeforeAudio";
  bool isSecureMode = false;
  bool isConcurrentMode = false;
  bool isTunnel = false;
  double playbackrate = 1.0;
  avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
  contentprimetype primetype = primevideobeforeaudio;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (sequential, non-tunnel, non-secure)
 *
 * This test verifies the basic AVClock Playback Speed functionality for Audio
 * Streams in Auto clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 010@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, seq_nontun_nonsec_AutoSetAudioPlayRateNormalSpeed) {
  const char *testName = "PrimeClock_AutoSetAudioPlayRateNormalSpeed";
  bool isSecureMode = false;
  bool isConcurrentMode = false;
  bool isTunnel = false;
  double playbackrate = 1.0;
  avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
  contentprimetype primetype = primeaudio;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (sequential, non-tunnel, non-secure)
 *
 * This test verifies the basic AVClock Playback Pause functionality for Audio
 * Streams in Auto clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 011@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, seq_nontun_nonsec_AutoSetAudioPlayRatePause) {
  const char *testName = "PrimeClock_AutoSetAudioPlayRatePause";
  bool isSecureMode = false;
  bool isConcurrentMode = false;
  bool isTunnel = false;
  double playbackrate = 0.0;
  avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
  contentprimetype primetype = primeaudio;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (sequential, non-tunnel, non-secure)
 *
 * This test verifies the basic AVClock Playback FastForward functionality for
 * Audio Streams in Auto clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 012@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, seq_nontun_nonsec_AutoSetAudioPlayRateFastForward) {
  const char *testName = "PrimeClock_AutoSetAudioPlayRateFastForward";
  bool isSecureMode = false;
  bool isConcurrentMode = false;
  bool isTunnel = false;
  double playbackrate = 2.0;
  avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
  contentprimetype primetype = primeaudio;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (sequential, non-tunnel, non-secure)
 *
 * This test verifies the basic AVClock Playback Speed functionality for Video
 * Streams in Auto clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 013@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, seq_nontun_nonsec_AutoSetVideoPlayRateNormalSpeed) {
  const char *testName = "PrimeClock_AutoSetVideoPlayRateNormalSpeed";
  bool isSecureMode = false;
  bool isConcurrentMode = false;
  bool isTunnel = false;
  double playbackrate = 1.0;
  avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
  contentprimetype primetype = primevideo;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (sequential, non-tunnel, non-secure)
 *
 * This test verifies the basic AVClock Playback Pause functionality for Video
 * Streams in Auto clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 014@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, seq_nontun_nonsec_AutoSetVideoPlayRatePause) {
  const char *testName = "PrimeClock_AutoSetVideoPlayRatePause";
  bool isSecureMode = false;
  bool isConcurrentMode = false;
  bool isTunnel = false;
  double playbackrate = 0.0;
  avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
  contentprimetype primetype = primevideo;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (sequential, non-tunnel, non-secure)
 *
 * This test verifies the basic AVClock Playback FastForward functionality for
 * Video Streams in Auto clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 015@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, seq_nontun_nonsec_AutoSetVideoPlayRateFastForward) {
  const char *testName = "PrimeClock_AutoSetVideoPlayRateFastForward";
  bool isSecureMode = false;
  bool isConcurrentMode = false;
  bool isTunnel = false;
  double playbackrate = 2.0;
  avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
  contentprimetype primetype = primevideo;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (sequential, non-tunnel, secure)
 *
 * This test verifies the basic AVClock functionality for Audio Stream in Auto
 * clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 0016@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, seq_nontun_sec_AutoAudioOnly) {
  const char *testName = "PrimeClock_AutoAudioOnly";
  bool isSecureMode = true;
  bool isConcurrentMode = false;
  bool isTunnel = false;
  double playbackrate = 1.0;
  avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
  contentprimetype primetype = primeaudio;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (sequential, non-tunnel, secure)
 *
 * This test verifies the basic AVClock functionality for Video Stream in Auto
 * clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 017@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, seq_nontun_sec_AutoVideoOnly) {
  const char *testName = "PrimeClock_AutoVideoOnly";
  bool isSecureMode = true;
  bool isConcurrentMode = false;
  bool isTunnel = false;
  double playbackrate = 1.0;
  avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
  contentprimetype primetype = primevideo;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (sequential, non-tunnel, secure)
 *
 * This test verifies the basic AVClock functionality for Audio Stream in
 * AudioMaster clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 018@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, seq_nontun_sec_AudioMasterAudioOnly) {
  const char *testName = "PrimeClock_AudioMasterAudioOnly";
  bool isSecureMode = true;
  bool isConcurrentMode = false;
  bool isTunnel = false;
  double playbackrate = 1.0;
  avclock::ClockMode clockMode = avclock::ClockMode::AUDIO_MASTER;
  contentprimetype primetype = primeaudio;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (sequential, non-tunnel, secure)
 *
 * This test verifies the basic AVClock functionality for Video Stream in
 * VideoMaster clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 019@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, seq_nontun_sec_VideoMasterVideoOnly) {
  const char *testName = "PrimeClock_VideoMasterVideoOnly";
  bool isSecureMode = true;
  bool isConcurrentMode = false;
  bool isTunnel = false;
  double playbackrate = 1.0;
  avclock::ClockMode clockMode = avclock::ClockMode::VIDEO_MASTER;
  contentprimetype primetype = primevideo;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (sequential, non-tunnel, secure)
 *
 * This test verifies the basic AVClock functionality for Video Stream in
 * AudioMaster clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 020@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, seq_nontun_sec_AudioMasterVideoOnly) {
  const char *testName = "PrimeClock_AudioMasterVideoOnly";
  bool isSecureMode = true;
  bool isConcurrentMode = false;
  bool isTunnel = false;
  double playbackrate = 1.0;
  avclock::ClockMode clockMode = avclock::ClockMode::AUDIO_MASTER;
  contentprimetype primetype = primevideo;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (sequential, non-tunnel, secure)
 *
 * This test verifies the basic AVClock functionality for Audio Stream in
 * VideoMaster clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 021@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, seq_nontun_sec_VideoMasterAudioOnly) {
  const char *testName = "PrimeClock_VideoMasterAudioOnly";
  bool isSecureMode = true;
  bool isConcurrentMode = false;
  bool isTunnel = false;
  double playbackrate = 1.0;
  avclock::ClockMode clockMode = avclock::ClockMode::VIDEO_MASTER;
  contentprimetype primetype = primeaudio;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (sequential, non-tunnel, secure)
 *
 * This test verifies the basic AVClock functionality for PCR Stream in PCR
 * clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 022@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, seq_nontun_sec_PCRMode) {
  const char *testName = "PrimeClock_PCRMode";
  bool isSecureMode = true;
  bool isConcurrentMode = false;
  bool isTunnel = false;
  double playbackrate = 1.0;
  avclock::ClockMode clockMode = avclock::ClockMode::PCR;
  contentprimetype primetype = primepcr;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (sequential, non-tunnel, secure)
 *
 * This test verifies the basic AVClock functionality for Audio, Video Streams
 * in Auto clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 023@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, seq_nontun_sec_AutoAudioBeforeVideo) {
  const char *testName = "PrimeClock_AutoAudioBeforeVideo";
  bool isSecureMode = true;
  bool isConcurrentMode = false;
  bool isTunnel = false;
  double playbackrate = 1.0;
  avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
  contentprimetype primetype = primeaudiobeforevideo;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (sequential, non-tunnel, secure)
 *
 * This test verifies the basic AVClock functionality for Video, Audio Streams
 * in Auto clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 024@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, seq_nontun_sec_AutoVideoBeforeAudio) {
  const char *testName = "PrimeClock_AutoVideoBeforeAudio";
  bool isSecureMode = true;
  bool isConcurrentMode = false;
  bool isTunnel = false;
  double playbackrate = 1.0;
  avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
  contentprimetype primetype = primevideobeforeaudio;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (sequential, non-tunnel, secure)
 *
 * This test verifies the basic AVClock Playback Speed functionality for Audio
 * Streams in Auto clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 025@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, seq_nontun_sec_AutoSetAudioPlayRateNormalSpeed) {
  const char *testName = "PrimeClock_AutoSetAudioPlayRateNormalSpeed";
  bool isSecureMode = true;
  bool isConcurrentMode = false;
  bool isTunnel = false;
  double playbackrate = 1.0;
  avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
  contentprimetype primetype = primeaudio;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (sequential, non-tunnel, secure)
 *
 * This test verifies the basic AVClock Playback Pause functionality for Audio
 * Streams in Auto clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 026@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, seq_nontun_sec_AutoSetAudioPlayRatePause) {
  const char *testName = "PrimeClock_AutoSetAudioPlayRatePause";
  bool isSecureMode = true;
  bool isConcurrentMode = false;
  bool isTunnel = false;
  double playbackrate = 0.0;
  avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
  contentprimetype primetype = primeaudio;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (sequential, non-tunnel, secure)
 *
 * This test verifies the basic AVClock Playback FastForward functionality for
 * Audio Streams in Auto clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 027@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, seq_nontun_sec_AutoSetAudioPlayRateFastForward) {
  const char *testName = "PrimeClock_AutoSetAudioPlayRateFastForward";
  bool isSecureMode = true;
  bool isConcurrentMode = false;
  bool isTunnel = false;
  double playbackrate = 2.0;
  avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
  contentprimetype primetype = primeaudio;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (sequential, non-tunnel, secure)
 *
 * This test verifies the basic AVClock Playback Speed functionality for Video
 * Streams in Auto clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 028@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, seq_nontun_sec_AutoSetVideoPlayRateNormalSpeed) {
  const char *testName = "PrimeClock_AutoSetVideoPlayRateNormalSpeed";
  bool isSecureMode = true;
  bool isConcurrentMode = false;
  bool isTunnel = false;
  double playbackrate = 1.0;
  avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
  contentprimetype primetype = primevideo;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (sequential, non-tunnel, secure)
 *
 * This test verifies the basic AVClock Playback Pause functionality for Video
 * Streams in Auto clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 029@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, seq_nontun_sec_AutoSetVideoPlayRatePause) {
  const char *testName = "PrimeClock_AutoSetVideoPlayRatePause";
  bool isSecureMode = true;
  bool isConcurrentMode = false;
  bool isTunnel = false;
  double playbackrate = 0.0;
  avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
  contentprimetype primetype = primevideo;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (sequential, non-tunnel, secure)
 *
 * This test verifies the basic AVClock Playback FastForward functionality for
 * Video Streams in Auto clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 030@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, seq_nontun_sec_AutoSetVideoPlayRateFastForward) {
  const char *testName = "PrimeClock_AutoSetVideoPlayRateFastForward";
  bool isSecureMode = true;
  bool isConcurrentMode = false;
  bool isTunnel = false;
  double playbackrate = 2.0;
  avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
  contentprimetype primetype = primevideo;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (sequential, tunnel, non-secure)
 *
 * This test verifies the basic AVClock functionality for Audio Stream in Auto
 * clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 031@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, seq_tun_nonsec_AutoAudioOnly) {
  const char *testName = "PrimeClock_AutoAudioOnly";
  bool isSecureMode = false;
  bool isConcurrentMode = false;
  bool isTunnel = true;
  double playbackrate = 1.0;
  avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
  contentprimetype primetype = primeaudio;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (sequential, tunnel, non-secure)
 *
 * This test verifies the basic AVClock functionality for Video Stream in Auto
 * clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 032@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, seq_tun_nonsec_AutoVideoOnly) {
  const char *testName = "PrimeClock_AutoVideoOnly";
  bool isSecureMode = false;
  bool isConcurrentMode = false;
  bool isTunnel = true;
  double playbackrate = 1.0;
  avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
  contentprimetype primetype = primevideo;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (sequential, tunnel, non-secure)
 *
 * This test verifies the basic AVClock functionality for Audio Stream in
 * AudioMaster clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 033@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, seq_tun_nonsec_AudioMasterAudioOnly) {
  const char *testName = "PrimeClock_AudioMasterAudioOnly";
  bool isSecureMode = false;
  bool isConcurrentMode = false;
  bool isTunnel = true;
  double playbackrate = 1.0;
  avclock::ClockMode clockMode = avclock::ClockMode::AUDIO_MASTER;
  contentprimetype primetype = primeaudio;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (sequential, tunnel, non-secure)
 *
 * This test verifies the basic AVClock functionality for Video Stream in
 * VideoMaster clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 034@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, seq_tun_nonsec_VideoMasterVideoOnly) {
  const char *testName = "PrimeClock_VideoMasterVideoOnly";
  bool isSecureMode = false;
  bool isConcurrentMode = false;
  bool isTunnel = true;
  double playbackrate = 1.0;
  avclock::ClockMode clockMode = avclock::ClockMode::VIDEO_MASTER;
  contentprimetype primetype = primevideo;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (sequential, tunnel, non-secure)
 *
 * This test verifies the basic AVClock functionality for Video Stream in
 * AudioMaster clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 035@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, seq_tun_nonsec_AudioMasterVideoOnly) {
  const char *testName = "PrimeClock_AudioMasterVideoOnly";
  bool isSecureMode = false;
  bool isConcurrentMode = false;
  bool isTunnel = true;
  double playbackrate = 1.0;
  avclock::ClockMode clockMode = avclock::ClockMode::AUDIO_MASTER;
  contentprimetype primetype = primevideo;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (sequential, tunnel, non-secure)
 *
 * This test verifies the basic AVClock functionality for Audio Stream in
 * VideoMaster clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 036@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, seq_tun_nonsec_VideoMasterAudioOnly) {
  const char *testName = "PrimeClock_VideoMasterAudioOnly";
  bool isSecureMode = false;
  bool isConcurrentMode = false;
  bool isTunnel = true;
  double playbackrate = 1.0;
  avclock::ClockMode clockMode = avclock::ClockMode::VIDEO_MASTER;
  contentprimetype primetype = primeaudio;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (sequential, tunnel, non-secure)
 *
 * This test verifies the basic AVClock functionality for PCR Stream in PCR
 * clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 037@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, seq_tun_nonsec_PCRMode) {
  const char *testName = "PrimeClock_PCRMode";
  bool isSecureMode = false;
  bool isConcurrentMode = false;
  bool isTunnel = true;
  double playbackrate = 1.0;
  avclock::ClockMode clockMode = avclock::ClockMode::PCR;
  contentprimetype primetype = primepcr;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (sequential, tunnel, non-secure)
 *
 * This test verifies the basic AVClock functionality for Audio, Video Streams
 * in Auto clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 0038@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, seq_tun_nonsec_AutoAudioBeforeVideo) {
  const char *testName = "PrimeClock_AutoAudioBeforeVideo";
  bool isSecureMode = false;
  bool isConcurrentMode = false;
  bool isTunnel = true;
  double playbackrate = 1.0;
  avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
  contentprimetype primetype = primeaudiobeforevideo;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (sequential, tunnel, non-secure)
 *
 * This test verifies the basic AVClock functionality for Video, Audio Streams
 * in Auto clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 0039@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, seq_tun_nonsec_AutoVideoBeforeAudio) {
  const char *testName = "PrimeClock_AutoVideoBeforeAudio";
  bool isSecureMode = false;
  bool isConcurrentMode = false;
  bool isTunnel = true;
  double playbackrate = 1.0;
  avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
  contentprimetype primetype = primevideobeforeaudio;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (sequential, tunnel, non-secure)
 *
 * This test verifies the basic AVClock Playback Speed functionality for Audio
 * Streams in Auto clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 040@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, seq_tun_nonsec_AutoSetAudioPlayRateNormalSpeed) {
  const char *testName = "PrimeClock_AutoSetAudioPlayRateNormalSpeed";
  bool isSecureMode = false;
  bool isConcurrentMode = false;
  bool isTunnel = true;
  double playbackrate = 1.0;
  avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
  contentprimetype primetype = primeaudio;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (sequential, tunnel, non-secure)
 *
 * This test verifies the basic AVClock Playback Pause functionality for Audio
 * Streams in Auto clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 041@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, seq_tun_nonsec_AutoSetAudioPlayRatePause) {
  const char *testName = "PrimeClock_AutoSetAudioPlayRatePause";
  bool isSecureMode = false;
  bool isConcurrentMode = false;
  bool isTunnel = true;
  double playbackrate = 0.0;
  avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
  contentprimetype primetype = primeaudio;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (sequential, tunnel, non-secure)
 *
 * This test verifies the basic AVClock Playback FastForward functionality for
 * Audio Streams in Auto clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 042@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, seq_tun_nonsec_AutoSetAudioPlayRateFastForward) {
  const char *testName = "PrimeClock_AutoSetAudioPlayRateFastForward";
  bool isSecureMode = false;
  bool isConcurrentMode = false;
  bool isTunnel = true;
  double playbackrate = 2.0;
  avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
  contentprimetype primetype = primeaudio;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (sequential, tunnel, non-secure)
 *
 * This test verifies the basic AVClock Playback Speed functionality for Video
 * Streams in Auto clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 043@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, seq_tun_nonsec_AutoSetVideoPlayRateNormalSpeed) {
  const char *testName = "PrimeClock_AutoSetVideoPlayRateNormalSpeed";
  bool isSecureMode = false;
  bool isConcurrentMode = false;
  bool isTunnel = true;
  double playbackrate = 1.0;
  avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
  contentprimetype primetype = primevideo;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (sequential, tunnel, non-secure)
 *
 * This test verifies the basic AVClock Playback Pause functionality for Video
 * Streams in Auto clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 044@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, seq_tun_nonsec_AutoSetVideoPlayRatePause) {
  const char *testName = "PrimeClock_AutoSetVideoPlayRatePause";
  bool isSecureMode = false;
  bool isConcurrentMode = false;
  bool isTunnel = true;
  double playbackrate = 0.0;
  avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
  contentprimetype primetype = primevideo;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (sequential, tunnel, non-secure)
 *
 * This test verifies the basic AVClock Playback FastForward functionality for
 * Video Streams in Auto clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 045@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, seq_tun_nonsec_AutoSetVideoPlayRateFastForward) {
  const char *testName = "PrimeClock_AutoSetVideoPlayRateFastForward";
  bool isSecureMode = false;
  bool isConcurrentMode = false;
  bool isTunnel = true;
  double playbackrate = 1.0;
  avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
  contentprimetype primetype = primevideo;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (sequential, tunnel, secure)
 *
 * This test verifies the basic AVClock functionality for Audio Stream in Auto
 * clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 046@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, seq_tun_sec_AutoAudioOnly) {
  const char *testName = "PrimeClock_AutoAudioOnly";
  bool isSecureMode = true;
  bool isConcurrentMode = false;
  bool isTunnel = true;
  double playbackrate = 1.0;
  avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
  contentprimetype primetype = primeaudio;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (sequential, tunnel, secure)
 *
 * This test verifies the basic AVClock functionality for Video Stream in Auto
 * clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 047@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, seq_tun_sec_AutoVideoOnly) {
  const char *testName = "PrimeClock_AutoVideoOnly";
  bool isSecureMode = true;
  bool isConcurrentMode = false;
  bool isTunnel = true;
  double playbackrate = 1.0;
  avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
  contentprimetype primetype = primevideo;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (sequential, tunnel, secure)
 *
 * This test verifies the basic AVClock functionality for Audio Stream in
 * AudioMaster clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 048@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, seq_tun_sec_AudioMasterAudioOnly) {
  const char *testName = "PrimeClock_AudioMasterAudioOnly";
  bool isSecureMode = true;
  bool isConcurrentMode = false;
  bool isTunnel = true;
  double playbackrate = 1.0;
  avclock::ClockMode clockMode = avclock::ClockMode::AUDIO_MASTER;
  contentprimetype primetype = primeaudio;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (sequential, tunnel, secure)
 *
 * This test verifies the basic AVClock functionality for Video Stream in
 * VideoMaster clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 049@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, seq_tun_sec_VideoMasterVideoOnly) {
  const char *testName = "PrimeClock_VideoMasterVideoOnly";
  bool isSecureMode = true;
  bool isConcurrentMode = false;
  bool isTunnel = true;
  double playbackrate = 1.0;
  avclock::ClockMode clockMode = avclock::ClockMode::VIDEO_MASTER;
  contentprimetype primetype = primevideo;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (sequential, tunnel, secure)
 *
 * This test verifies the basic AVClock functionality for Video Stream in
 * AudioMaster clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 050@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, seq_tun_sec_AudioMasterVideoOnly) {
  const char *testName = "PrimeClock_AudioMasterVideoOnly";
  bool isSecureMode = true;
  bool isConcurrentMode = false;
  bool isTunnel = true;
  double playbackrate = 1.0;
  avclock::ClockMode clockMode = avclock::ClockMode::AUDIO_MASTER;
  contentprimetype primetype = primevideo;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (sequential, tunnel, secure)
 *
 * This test verifies the basic AVClock functionality for Audio Stream in
 * VideoMaster clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 051@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, seq_tun_sec_VideoMasterAudioOnly) {
  const char *testName = "PrimeClock_VideoMasterAudioOnly";
  bool isSecureMode = true;
  bool isConcurrentMode = false;
  bool isTunnel = true;
  double playbackrate = 1.0;
  avclock::ClockMode clockMode = avclock::ClockMode::VIDEO_MASTER;
  contentprimetype primetype = primeaudio;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (sequential, tunnel, secure)
 *
 * This test verifies the basic AVClock functionality for PCR Stream in PCR
 * clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 052@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, seq_tun_sec_PCRMode) {
  const char *testName = "PrimeClock_PCRMode";
  bool isSecureMode = true;
  bool isConcurrentMode = false;
  bool isTunnel = true;
  double playbackrate = 1.0;
  avclock::ClockMode clockMode = avclock::ClockMode::PCR;
  contentprimetype primetype = primepcr;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (sequential, tunnel, secure)
 *
 * This test verifies the basic AVClock functionality for Audio, Video Streams
 * in Auto clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 053@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, seq_tun_sec_AutoAudioBeforeVideo) {
  const char *testName = "PrimeClock_AutoAudioBeforeVideo";
  bool isSecureMode = true;
  bool isConcurrentMode = false;
  bool isTunnel = true;
  double playbackrate = 1.0;
  avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
  contentprimetype primetype = primeaudiobeforevideo;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (sequential, tunnel, secure)
 *
 * This test verifies the basic AVClock functionality for Video, Audio Streams
 * in Auto clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 054@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, seq_tun_sec_AutoVideoBeforeAudio) {
  const char *testName = "PrimeClock_AutoVideoBeforeAudio";
  bool isSecureMode = true;
  bool isConcurrentMode = false;
  bool isTunnel = true;
  double playbackrate = 1.0;
  avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
  contentprimetype primetype = primevideobeforeaudio;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (sequential, tunnel, secure)
 *
 * This test verifies the basic AVClock Playback Speed functionality for Audio
 * Streams in Auto clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 055@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, seq_tun_sec_AutoSetAudioPlayRateNormalSpeed) {
  const char *testName = "PrimeClock_AutoSetAudioPlayRateNormalSpeed";
  bool isSecureMode = true;
  bool isConcurrentMode = false;
  bool isTunnel = true;
  double playbackrate = 1.0;
  avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
  contentprimetype primetype = primeaudio;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (sequential, tunnel, secure)
 *
 * This test verifies the basic AVClock Playback Pause functionality for Audio
 * Streams in Auto clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 056@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, seq_tun_sec_AutoSetAudioPlayRatePause) {
  const char *testName = "PrimeClock_AutoSetAudioPlayRatePause";
  bool isSecureMode = true;
  bool isConcurrentMode = false;
  bool isTunnel = true;
  double playbackrate = 0.0;
  avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
  contentprimetype primetype = primeaudio;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (sequential, tunnel, secure)
 *
 * This test verifies the basic AVClock Playback FastForward functionality for
 * Audio Streams in Auto clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 057@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, seq_tun_sec_AutoSetAudioPlayRateFastForward) {
  const char *testName = "PrimeClock_AutoSetAudioPlayRateFastForward";
  bool isSecureMode = true;
  bool isConcurrentMode = false;
  bool isTunnel = true;
  double playbackrate = 2.0;
  avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
  contentprimetype primetype = primeaudio;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (sequential, tunnel, secure)
 *
 * This test verifies the basic AVClock Playback Speed functionality for Video
 * Streams in Auto clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 058@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, seq_tun_sec_AutoSetVideoPlayRateNormalSpeed) {
  const char *testName = "PrimeClock_AutoSetVideoPlayRateNormalSpeed";
  bool isSecureMode = true;
  bool isConcurrentMode = false;
  bool isTunnel = true;
  double playbackrate = 1.0;
  avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
  contentprimetype primetype = primevideo;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (sequential, tunnel, secure)
 *
 * This test verifies the basic AVClock Playback Pause functionality for Video
 * Streams in Auto clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 059@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, seq_tun_sec_AutoSetVideoPlayRatePause) {
  const char *testName = "PrimeClock_AutoSetVideoPlayRatePause";
  bool isSecureMode = true;
  bool isConcurrentMode = false;
  bool isTunnel = true;
  double playbackrate = 0.0;
  avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
  contentprimetype primetype = primevideo;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (sequential, tunnel, secure)
 *
 * This test verifies the basic AVClock Playback FastForward functionality for
 * Video Streams in Auto clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 060@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, seq_tun_sec_AutoSetVideoPlayRateFastForward) {
  const char *testName = "PrimeClock_AutoSetVideoPlayRateFastForward";
  bool isSecureMode = true;
  bool isConcurrentMode = false;
  bool isTunnel = true;
  double playbackrate = 2.0;
  avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
  contentprimetype primetype = primevideo;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (concurrent, non-tunnel, non-secure)
 *
 * This test verifies the basic AVClock functionality for Audio Stream in Auto
 * clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 061@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, con_ntun_nonsec_AutoAudioOnly) {

  isAVClockSupportsConcurrency();

  const char *testName = "PrimeClock_AutoAudioOnly";
  bool isSecureMode = false;
  bool isConcurrentMode = true;
  bool isTunnel = false;
  double playbackrate = 1.0;
  avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
  contentprimetype primetype = primeaudio;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (concurrent, non-tunnel, non-secure)
 *
 * This test verifies the basic AVClock functionality for Video Stream in Auto
 * clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 062@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, con_ntun_nonsec_AutoVideoOnly) {

  isAVClockSupportsConcurrency();

  const char *testName = "PrimeClock_AutoVideoOnly";
  bool isSecureMode = false;
  bool isConcurrentMode = true;
  bool isTunnel = false;
  double playbackrate = 1.0;
  avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
  contentprimetype primetype = primevideo;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (concurrent, non-tunnel, non-secure)
 *
 * This test verifies the basic AVClock functionality for Audio Stream in
 * AudioMaster clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 063@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, con_ntun_nonsec_AudioMasterAudioOnly) {

  isAVClockSupportsConcurrency();

  const char *testName = "PrimeClock_AudioMasterAudioOnly";
  bool isSecureMode = false;
  bool isConcurrentMode = true;
  bool isTunnel = false;
  double playbackrate = 1.0;
  avclock::ClockMode clockMode = avclock::ClockMode::AUDIO_MASTER;
  contentprimetype primetype = primeaudio;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (concurrent, non-tunnel, non-secure)
 *
 * This test verifies the basic AVClock functionality for Video Stream in
 * VideoMaster clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 064@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, con_ntun_nonsec_VideoMasterVideoOnly) {

  isAVClockSupportsConcurrency();

  const char *testName = "PrimeClock_VideoMasterVideoOnly";
  bool isSecureMode = false;
  bool isConcurrentMode = true;
  bool isTunnel = false;
  double playbackrate = 1.0;
  avclock::ClockMode clockMode = avclock::ClockMode::VIDEO_MASTER;
  contentprimetype primetype = primevideo;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (concurrent, non-tunnel, non-secure)
 *
 * This test verifies the basic AVClock functionality for Video Stream in
 * AudioMaster clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 065@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, con_ntun_nonsec_AudioMasterVideoOnly) {

  isAVClockSupportsConcurrency();

  const char *testName = "PrimeClock_AudioMasterVideoOnly";
  bool isSecureMode = false;
  bool isConcurrentMode = true;
  bool isTunnel = false;
  double playbackrate = 1.0;
  avclock::ClockMode clockMode = avclock::ClockMode::AUDIO_MASTER;
  contentprimetype primetype = primevideo;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (concurrent, non-tunnel, non-secure)
 *
 * This test verifies the basic AVClock functionality for Audio Stream in
 * VideoMaster clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 066@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, con_ntun_nonsec_VideoMasterAudioOnly) {

  isAVClockSupportsConcurrency();

  const char *testName = "PrimeClock_VideoMasterAudioOnly";
  bool isSecureMode = false;
  bool isConcurrentMode = true;
  bool isTunnel = false;
  double playbackrate = 1.0;
  avclock::ClockMode clockMode = avclock::ClockMode::VIDEO_MASTER;
  contentprimetype primetype = primeaudio;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (concurrent, non-tunnel, non-secure)
 *
 * This test verifies the basic AVClock functionality for PCR Stream in PCR
 * clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 067@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, con_ntun_nonsec_PCRMode) {

  isAVClockSupportsConcurrency();

  const char *testName = "PrimeClock_PCRMode";
  bool isSecureMode = false;
  bool isConcurrentMode = true;
  bool isTunnel = false;
  double playbackrate = 1.0;
  avclock::ClockMode clockMode = avclock::ClockMode::PCR;
  contentprimetype primetype = primepcr;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (concurrent, non-tunnel, non-secure)
 *
 * This test verifies the basic AVClock functionality for Audio, Video Streams
 * in Auto clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 068@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, con_ntun_nonsec_AutoAudioBeforeVideo) {

  isAVClockSupportsConcurrency();

  const char *testName = "PrimeClock_AutoAudioBeforeVideo";
  bool isSecureMode = false;
  bool isConcurrentMode = true;
  bool isTunnel = false;
  double playbackrate = 1.0;
  avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
  contentprimetype primetype = primeaudiobeforevideo;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (concurrent, non-tunnel, non-secure)
 *
 * This test verifies the basic AVClock functionality for Video, Audio Streams
 * in Auto clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 069@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, con_ntun_nonsec_AutoVideoBeforeAudio) {

  isAVClockSupportsConcurrency();

  const char *testName = "PrimeClock_AutoVideoBeforeAudio";
  bool isSecureMode = false;
  bool isConcurrentMode = true;
  bool isTunnel = false;
  double playbackrate = 1.0;
  avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
  contentprimetype primetype = primevideobeforeaudio;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (concurrent, non-tunnel, non-secure)
 *
 * This test verifies the basic AVClock Playback Speed functionality for Audio
 * Streams in Auto clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 070@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, con_ntun_nonsec_AutoSetAudioPlayRateNormalSpeed) {

  isAVClockSupportsConcurrency();

  const char *testName = "PrimeClock_AutoSetAudioPlayRateNormalSpeed";
  bool isSecureMode = false;
  bool isConcurrentMode = true;
  bool isTunnel = false;
  double playbackrate = 1.0;
  avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
  contentprimetype primetype = primeaudio;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (concurrent, non-tunnel, non-secure)
 *
 * This test verifies the basic AVClock Playback Pause functionality for Audio
 * Streams in Auto clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 071@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, con_ntun_nonsec_AutoSetAudioPlayRatePause) {

  isAVClockSupportsConcurrency();

  const char *testName = "PrimeClock_AutoSetAudioPlayRatePause";
  bool isSecureMode = false;
  bool isConcurrentMode = true;
  bool isTunnel = false;
  double playbackrate = 0.0;
  avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
  contentprimetype primetype = primeaudio;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (concurrent, non-tunnel, non-secure)
 *
 * This test verifies the basic AVClock Playback FastForward functionality for
 * Audio Streams in Auto clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 072@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, con_ntun_nonsec_AutoSetAudioPlayRateFastForward) {

  isAVClockSupportsConcurrency();

  const char *testName = "PrimeClock_AutoSetAudioPlayRateFastForward";
  bool isSecureMode = false;
  bool isConcurrentMode = true;
  bool isTunnel = false;
  double playbackrate = 2.0;
  avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
  contentprimetype primetype = primeaudio;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (concurrent, non-tunnel, non-secure)
 *
 * This test verifies the basic AVClock Playback Speed functionality for Video
 * Streams in Auto clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 073@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, con_ntun_nonsec_AutoSetVideoPlayRateNormalSpeed) {

  isAVClockSupportsConcurrency();

  const char *testName = "PrimeClock_AutoSetVideoPlayRateNormalSpeed";
  bool isSecureMode = false;
  bool isConcurrentMode = true;
  bool isTunnel = false;
  double playbackrate = 1.0;
  avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
  contentprimetype primetype = primevideo;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (concurrent, non-tunnel, non-secure)
 *
 * This test verifies the basic AVClock Playback Pause functionality for Video
 * Streams in Auto clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 074@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, con_ntun_nonsec_AutoSetVideoPlayRatePause) {

  isAVClockSupportsConcurrency();

  const char *testName = "PrimeClock_AutoSetVideoPlayRatePause";
  bool isSecureMode = false;
  bool isConcurrentMode = true;
  bool isTunnel = false;
  double playbackrate = 0.0;
  avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
  contentprimetype primetype = primevideo;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (concurrent, non-tunnel, non-secure)
 *
 * This test verifies the basic AVClock Playback FastForward functionality for
 * Video Streams in Auto clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 075@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, con_ntun_nonsec_AutoSetVideoPlayRateFastForward) {

  isAVClockSupportsConcurrency();

  const char *testName = "PrimeClock_AutoSetVideoPlayRateFastForward";
  bool isSecureMode = false;
  bool isConcurrentMode = true;
  bool isTunnel = false;
  double playbackrate = 2.0;
  avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
  contentprimetype primetype = primevideo;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (concurrent, non-tunnel, secure)
 *
 * This test verifies the basic AVClock functionality for Audio Stream in Auto
 * clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 076@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, con_ntun_sec_AutoAudioOnly) {

  isAVClockSupportsConcurrency();

  const char *testName = "PrimeClock_AutoAudioOnly";
  bool isSecureMode = true;
  bool isConcurrentMode = true;
  bool isTunnel = false;
  double playbackrate = 1.0;
  avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
  contentprimetype primetype = primeaudio;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (concurrent, non-tunnel, secure)
 *
 * This test verifies the basic AVClock functionality for Video Stream in Auto
 * clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 077@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, con_ntun_sec_AutoVideoOnly) {

  isAVClockSupportsConcurrency();

  const char *testName = "PrimeClock_AutoVideoOnly";
  bool isSecureMode = true;
  bool isConcurrentMode = true;
  bool isTunnel = false;
  double playbackrate = 1.0;
  avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
  contentprimetype primetype = primevideo;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (concurrent, non-tunnel, secure)
 *
 * This test verifies the basic AVClock functionality for Audio Stream in
 * AudioMaster clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 078@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, con_ntun_sec_AudioMasterAudioOnly) {

  isAVClockSupportsConcurrency();

  const char *testName = "PrimeClock_AudioMasterAudioOnly";
  bool isSecureMode = true;
  bool isConcurrentMode = true;
  bool isTunnel = false;
  double playbackrate = 1.0;
  avclock::ClockMode clockMode = avclock::ClockMode::AUDIO_MASTER;
  contentprimetype primetype = primeaudio;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (concurrent, non-tunnel, secure)
 *
 * This test verifies the basic AVClock functionality for Video Stream in
 * VideoMaster clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 079@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, con_ntun_sec_VideoMasterVideoOnly) {

  isAVClockSupportsConcurrency();

  const char *testName = "PrimeClock_VideoMasterVideoOnly";
  bool isSecureMode = true;
  bool isConcurrentMode = true;
  bool isTunnel = false;
  double playbackrate = 1.0;
  avclock::ClockMode clockMode = avclock::ClockMode::VIDEO_MASTER;
  contentprimetype primetype = primevideo;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (concurrent, non-tunnel, secure)
 *
 * This test verifies the basic AVClock functionality for Video Stream in
 * AudioMaster clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 080@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, con_ntun_sec_AudioMasterVideoOnly) {

  isAVClockSupportsConcurrency();

  const char *testName = "PrimeClock_AudioMasterVideoOnly";
  bool isSecureMode = true;
  bool isConcurrentMode = true;
  bool isTunnel = false;
  double playbackrate = 1.0;
  avclock::ClockMode clockMode = avclock::ClockMode::AUDIO_MASTER;
  contentprimetype primetype = primevideo;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (concurrent, non-tunnel, secure)
 *
 * This test verifies the basic AVClock functionality for Audio Stream in
 * VideoMaster clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 081@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, con_ntun_sec_VideoMasterAudioOnly) {

  isAVClockSupportsConcurrency();

  const char *testName = "PrimeClock_VideoMasterAudioOnly";
  bool isSecureMode = true;
  bool isConcurrentMode = true;
  bool isTunnel = false;
  double playbackrate = 1.0;
  avclock::ClockMode clockMode = avclock::ClockMode::VIDEO_MASTER;
  contentprimetype primetype = primeaudio;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (concurrent, non-tunnel, secure)
 *
 * This test verifies the basic AVClock functionality for PCR Stream in PCR
 * clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 082@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, con_ntun_sec_PCRMode) {

  isAVClockSupportsConcurrency();

  const char *testName = "PrimeClock_PCRMode";
  bool isSecureMode = true;
  bool isConcurrentMode = true;
  bool isTunnel = false;
  double playbackrate = 1.0;
  avclock::ClockMode clockMode = avclock::ClockMode::PCR;
  contentprimetype primetype = primepcr;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (concurrent, non-tunnel, secure)
 *
 * This test verifies the basic AVClock functionality for Audio, Video Streams
 * in Auto clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 083@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, con_ntun_sec_AutoAudioBeforeVideo) {

  isAVClockSupportsConcurrency();

  const char *testName = "PrimeClock_AutoAudioBeforeVideo";
  bool isSecureMode = true;
  bool isConcurrentMode = true;
  bool isTunnel = false;
  double playbackrate = 1.0;
  avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
  contentprimetype primetype = primeaudiobeforevideo;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (concurrent, non-tunnel, secure)
 *
 * This test verifies the basic AVClock functionality for Video, Audio Streams
 * in Auto clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 084@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, con_ntun_sec_AutoVideoBeforeAudio) {

  isAVClockSupportsConcurrency();

  const char *testName = "PrimeClock_AutoVideoBeforeAudio";
  bool isSecureMode = true;
  bool isConcurrentMode = true;
  bool isTunnel = false;
  double playbackrate = 1.0;
  avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
  contentprimetype primetype = primevideobeforeaudio;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (concurrent, non-tunnel, secure)
 *
 * This test verifies the basic AVClock Playback Speed functionality for Audio
 * Streams in Auto clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 085@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, con_ntun_sec_AutoSetAudioPlayRateNormalSpeed) {

  isAVClockSupportsConcurrency();

  const char *testName = "PrimeClock_AutoSetAudioPlayRateNormalSpeed";
  bool isSecureMode = true;
  bool isConcurrentMode = true;
  bool isTunnel = false;
  double playbackrate = 1.0;
  avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
  contentprimetype primetype = primeaudio;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (concurrent, non-tunnel, secure)
 *
 * This test verifies the basic AVClock Playback Pause functionality for Audio
 * Streams in Auto clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 086@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, con_ntun_sec_AutoSetAudioPlayRatePause) {

  isAVClockSupportsConcurrency();

  const char *testName = "PrimeClock_AutoSetAudioPlayRatePause";
  bool isSecureMode = true;
  bool isConcurrentMode = true;
  bool isTunnel = false;
  double playbackrate = 0.0;
  avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
  contentprimetype primetype = primeaudio;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (concurrent, non-tunnel, secure)
 *
 * This test verifies the basic AVClock Playback FastForward functionality for
 * Audio Streams in Auto clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 087@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, con_ntun_sec_AutoSetAudioPlayRateFastForward) {

  isAVClockSupportsConcurrency();

  const char *testName = "PrimeClock_AutoSetAudioPlayRateFastForward";
  bool isSecureMode = true;
  bool isConcurrentMode = true;
  bool isTunnel = false;
  double playbackrate = 1.0;
  avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
  contentprimetype primetype = primeaudio;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (concurrent, non-tunnel, secure)
 *
 * This test verifies the basic AVClock Playback Speed functionality for Video
 * Streams in Auto clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 088@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, con_ntun_sec_AutoSetVideoPlayRateNormalSpeed) {

  isAVClockSupportsConcurrency();

  const char *testName = "PrimeClock_AutoSetVideoPlayRateNormalSpeed";
  bool isSecureMode = true;
  bool isConcurrentMode = true;
  bool isTunnel = false;
  double playbackrate = 1.0;
  avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
  contentprimetype primetype = primevideo;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (concurrent, non-tunnel, secure)
 *
 * This test verifies the basic AVClock Playback Pause functionality for Video
 * Streams in Auto clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 089@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, con_ntun_sec_AutoSetVideoPlayRatePause) {

  isAVClockSupportsConcurrency();

  const char *testName = "PrimeClock_AutoSetVideoPlayRatePause";
  bool isSecureMode = true;
  bool isConcurrentMode = true;
  bool isTunnel = false;
  double playbackrate = 0.0;
  avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
  contentprimetype primetype = primevideo;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (concurrent, non-tunnel, secure)
 *
 * This test verifies the basic AVClock Playback FastForward functionality for
 * Video Streams in Auto clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 090@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, con_ntun_sec_AutoSetVideoPlayRateFastForward) {

  isAVClockSupportsConcurrency();

  const char *testName = "PrimeClock_AutoSetVideoPlayRateFastForward";
  bool isSecureMode = true;
  bool isConcurrentMode = true;
  bool isTunnel = false;
  double playbackrate = 2.0;
  avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
  contentprimetype primetype = primevideo;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (concurrent, tunnel, non-secure)
 *
 * This test verifies the basic AVClock functionality for Audio Stream in Auto
 * clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 091@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, con_tun_nonsec_AutoAudioOnly) {

  isAVClockSupportsConcurrency();

  const char *testName = "PrimeClock_AutoAudioOnly";
  bool isSecureMode = false;
  bool isConcurrentMode = true;
  bool isTunnel = true;
  double playbackrate = 1.0;
  avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
  contentprimetype primetype = primeaudio;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (concurrent, tunnel, non-secure)
 *
 * This test verifies the basic AVClock functionality for Video Stream in Auto
 * clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 092@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, con_tun_nonsec_AutoVideoOnly) {

  isAVClockSupportsConcurrency();

  const char *testName = "PrimeClock_AutoVideoOnly";
  bool isSecureMode = false;
  bool isConcurrentMode = true;
  bool isTunnel = true;
  double playbackrate = 1.0;
  avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
  contentprimetype primetype = primevideo;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (concurrent, tunnel, non-secure)
 *
 * This test verifies the basic AVClock functionality for Audio Stream in
 * AudioMaster clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 093@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, con_tun_nonsec_AudioMasterAudioOnly) {

  isAVClockSupportsConcurrency();

  const char *testName = "PrimeClock_AudioMasterAudioOnly";
  bool isSecureMode = false;
  bool isConcurrentMode = true;
  bool isTunnel = true;
  double playbackrate = 1.0;
  avclock::ClockMode clockMode = avclock::ClockMode::AUDIO_MASTER;
  contentprimetype primetype = primeaudio;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (concurrent, tunnel, non-secure)
 *
 * This test verifies the basic AVClock functionality for Video Stream in
 * VideoMaster clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 094@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, con_tun_nonsec_VideoMasterVideoOnly) {

  isAVClockSupportsConcurrency();

  const char *testName = "PrimeClock_VideoMasterVideoOnly";
  bool isSecureMode = false;
  bool isConcurrentMode = true;
  bool isTunnel = true;
  double playbackrate = 1.0;
  avclock::ClockMode clockMode = avclock::ClockMode::VIDEO_MASTER;
  contentprimetype primetype = primevideo;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (concurrent, tunnel, non-secure)
 *
 * This test verifies the basic AVClock functionality for Video Stream in
 * AudioMaster clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 095@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, con_tun_nonsec_AudioMasterVideoOnly) {

  isAVClockSupportsConcurrency();

  const char *testName = "PrimeClock_AudioMasterVideoOnly";
  bool isSecureMode = false;
  bool isConcurrentMode = true;
  bool isTunnel = true;
  double playbackrate = 1.0;
  avclock::ClockMode clockMode = avclock::ClockMode::AUDIO_MASTER;
  contentprimetype primetype = primevideo;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (concurrent, tunnel, non-secure)
 *
 * This test verifies the basic AVClock functionality for Audio Stream in
 * VideoMaster clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 096@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, con_tun_nonsec_VideoMasterAudioOnly) {

  isAVClockSupportsConcurrency();

  const char *testName = "PrimeClock_VideoMasterAudioOnly";
  bool isSecureMode = false;
  bool isConcurrentMode = true;
  bool isTunnel = true;
  double playbackrate = 1.0;
  avclock::ClockMode clockMode = avclock::ClockMode::VIDEO_MASTER;
  contentprimetype primetype = primeaudio;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (concurrent, tunnel, non-secure)
 *
 * This test verifies the basic AVClock functionality for PCR Stream in PCR
 * clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 097@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, con_tun_nonsec_PCRMode) {

  isAVClockSupportsConcurrency();

  const char *testName = "PrimeClock_PCRMode";
  bool isSecureMode = false;
  bool isConcurrentMode = true;
  bool isTunnel = true;
  double playbackrate = 1.0;
  avclock::ClockMode clockMode = avclock::ClockMode::PCR;
  contentprimetype primetype = primepcr;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (concurrent, tunnel, non-secure)
 *
 * This test verifies the basic AVClock functionality for Audio, Video Streams
 * in Auto clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 0098@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, con_tun_nonsec_AutoAudioBeforeVideo) {

  isAVClockSupportsConcurrency();

  const char *testName = "PrimeClock_AutoAudioBeforeVideo";
  bool isSecureMode = false;
  bool isConcurrentMode = true;
  bool isTunnel = true;
  double playbackrate = 1.0;
  avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
  contentprimetype primetype = primeaudiobeforevideo;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (concurrent, tunnel, non-secure)
 *
 * This test verifies the basic AVClock functionality for Video, Audio Streams
 * in Auto clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 0099@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, con_tun_nonsec_AutoVideoBeforeAudio) {

  isAVClockSupportsConcurrency();

  const char *testName = "PrimeClock_AutoVideoBeforeAudio";
  bool isSecureMode = false;
  bool isConcurrentMode = true;
  bool isTunnel = true;
  double playbackrate = 1.0;
  avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
  contentprimetype primetype = primevideobeforeaudio;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (concurrent, tunnel, non-secure)
 *
 * This test verifies the basic AVClock Playback Speed functionality for Audio
 * Streams in Auto clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 0100@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, con_tun_nonsec_AutoSetAudioPlayRateNormalSpeed) {

  isAVClockSupportsConcurrency();

  const char *testName = "PrimeClock_AutoSetAudioPlayRateNormalSpeed";
  bool isSecureMode = false;
  bool isConcurrentMode = true;
  bool isTunnel = true;
  double playbackrate = 1.0;
  avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
  contentprimetype primetype = primeaudio;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (concurrent, tunnel, non-secure)
 *
 * This test verifies the basic AVClock Playback Pause functionality for Audio
 * Streams in Auto clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 101@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, con_tun_nonsec_AutoSetAudioPlayRatePause) {

  isAVClockSupportsConcurrency();

  const char *testName = "PrimeClock_AutoSetAudioPlayRatePause";
  bool isSecureMode = false;
  bool isConcurrentMode = true;
  bool isTunnel = true;
  double playbackrate = 0.0;
  avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
  contentprimetype primetype = primeaudio;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (concurrent, tunnel, non-secure)
 *
 * This test verifies the basic AVClock Playback FastForward functionality for
 * Audio Streams in Auto clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 102@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, con_tun_nonsec_AutoSetAudioPlayRateFastForward) {

  isAVClockSupportsConcurrency();

  const char *testName = "PrimeClock_AutoSetAudioPlayRateFastForward";
  bool isSecureMode = false;
  bool isConcurrentMode = true;
  bool isTunnel = true;
  double playbackrate = 2.0;
  avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
  contentprimetype primetype = primeaudio;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (concurrent, tunnel, non-secure)
 *
 * This test verifies the basic AVClock Playback Speed functionality for Video
 * Streams in Auto clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 103@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, con_tun_nonsec_AutoSetVideoPlayRateNormalSpeed) {

  isAVClockSupportsConcurrency();

  const char *testName = "PrimeClock_AutoSetVideoPlayRateNormalSpeed";
  bool isSecureMode = false;
  bool isConcurrentMode = true;
  bool isTunnel = true;
  double playbackrate = 2.0;
  avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
  contentprimetype primetype = primevideo;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (concurrent, tunnel, non-secure)
 *
 * This test verifies the basic AVClock Playback Pause functionality for Video
 * Streams in Auto clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 104@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, con_tun_nonsec_AutoSetVideoPlayRatePause) {

  isAVClockSupportsConcurrency();

  const char *testName = "PrimeClock_AutoSetVideoPlayRatePause";
  bool isSecureMode = false;
  bool isConcurrentMode = true;
  bool isTunnel = true;
  double playbackrate = 0.0;
  avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
  contentprimetype primetype = primevideo;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (concurrent, tunnel, non-secure)
 *
 * This test verifies the basic AVClock Playback FastForward functionality for
 * Video Streams in Auto clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 105@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, con_tun_nonsec_AutoSetVideoPlayRateFastForward) {

  isAVClockSupportsConcurrency();

  const char *testName = "PrimeClock_AutoSetVideoPlayRateFastForward";
  bool isSecureMode = false;
  bool isConcurrentMode = true;
  bool isTunnel = true;
  double playbackrate = 2.0;
  avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
  contentprimetype primetype = primevideo;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (concurrent, tunnel, secure)
 *
 * This test verifies the basic AVClock functionality for Audio Stream in Auto
 * clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 106@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, con_tun_sec_AutoAudioOnly) {

  isAVClockSupportsConcurrency();

  const char *testName = "PrimeClock_AutoAudioOnly";
  bool isSecureMode = true;
  bool isConcurrentMode = true;
  bool isTunnel = true;
  double playbackrate = 1.0;
  avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
  contentprimetype primetype = primeaudio;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (concurrent, tunnel, secure)
 *
 * This test verifies the basic AVClock functionality for Video Stream in Auto
 * clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 107@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, con_tun_sec_AutoVideoOnly) {

  isAVClockSupportsConcurrency();

  const char *testName = "PrimeClock_AutoVideoOnly";
  bool isSecureMode = true;
  bool isConcurrentMode = true;
  bool isTunnel = true;
  double playbackrate = 1.0;
  avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
  contentprimetype primetype = primevideo;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (concurrent, tunnel, secure)
 *
 * This test verifies the basic AVClock functionality for Audio Stream in
 * AudioMaster clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 108@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, con_tun_sec_AudioMasterAudioOnly) {

  isAVClockSupportsConcurrency();

  const char *testName = "PrimeClock_AudioMasterAudioOnly";
  bool isSecureMode = true;
  bool isConcurrentMode = true;
  bool isTunnel = true;
  double playbackrate = 1.0;
  avclock::ClockMode clockMode = avclock::ClockMode::AUDIO_MASTER;
  contentprimetype primetype = primeaudio;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (concurrent, tunnel, secure)
 *
 * This test verifies the basic AVClock functionality for Video Stream in
 * VideoMaster clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 109@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, con_tun_sec_VideoMasterVideoOnly) {

  isAVClockSupportsConcurrency();

  const char *testName = "PrimeClock_VideoMasterVideoOnly";
  bool isSecureMode = true;
  bool isConcurrentMode = true;
  bool isTunnel = true;
  double playbackrate = 1.0;
  avclock::ClockMode clockMode = avclock::ClockMode::VIDEO_MASTER;
  contentprimetype primetype = primevideo;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (concurrent, tunnel, secure)
 *
 * This test verifies the basic AVClock functionality for Video Stream in
 * AudioMaster clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 110@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, con_tun_sec_AudioMasterVideoOnly) {

  isAVClockSupportsConcurrency();

  const char *testName = "PrimeClock_AudioMasterVideoOnly";
  bool isSecureMode = true;
  bool isConcurrentMode = true;
  bool isTunnel = true;
  double playbackrate = 1.0;
  avclock::ClockMode clockMode = avclock::ClockMode::AUDIO_MASTER;
  contentprimetype primetype = primevideo;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (concurrent, tunnel, secure)
 *
 * This test verifies the basic AVClock functionality for Audio Stream in
 * VideoMaster clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 111@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, con_tun_sec_VideoMasterAudioOnly) {

  isAVClockSupportsConcurrency();

  const char *testName = "PrimeClock_VideoMasterAudioOnly";
  bool isSecureMode = true;
  bool isConcurrentMode = true;
  bool isTunnel = true;
  double playbackrate = 1.0;
  avclock::ClockMode clockMode = avclock::ClockMode::VIDEO_MASTER;
  contentprimetype primetype = primeaudio;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (concurrent, tunnel, secure)
 *
 * This test verifies the basic AVClock functionality for PCR Stream in PCR
 * clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 112@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, con_tun_sec_PCRMode) {

  isAVClockSupportsConcurrency();

  const char *testName = "PrimeClock_PCRMode";
  bool isSecureMode = true;
  bool isConcurrentMode = true;
  bool isTunnel = true;
  double playbackrate = 1.0;
  avclock::ClockMode clockMode = avclock::ClockMode::PCR;
  contentprimetype primetype = primepcr;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (concurrent, tunnel, secure)
 *
 * This test verifies the basic AVClock functionality for Audio, Video Streams
 * in Auto clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 113@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, con_tun_sec_AutoAudioBeforeVideo) {

  isAVClockSupportsConcurrency();

  const char *testName = "PrimeClock_AutoAudioBeforeVideo";
  bool isSecureMode = true;
  bool isConcurrentMode = true;
  bool isTunnel = true;
  double playbackrate = 1.0;
  avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
  contentprimetype primetype = primeaudiobeforevideo;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (concurrent, tunnel, secure)
 *
 * This test verifies the basic AVClock functionality for Video, Audio Streams
 * in Auto clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 114@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, con_tun_sec_AutoVideoBeforeAudio) {

  isAVClockSupportsConcurrency();

  const char *testName = "PrimeClock_AutoVideoBeforeAudio";
  bool isSecureMode = true;
  bool isConcurrentMode = true;
  bool isTunnel = true;
  double playbackrate = 1.0;
  avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
  contentprimetype primetype = primevideobeforeaudio;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (concurrent, tunnel, secure)
 *
 * This test verifies the basic AVClock Playback Speed functionality for Audio
 * Streams in Auto clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 115@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, con_tun_sec_AutoSetAudioPlayRateNormalSpeed) {

  isAVClockSupportsConcurrency();

  const char *testName = "PrimeClock_AutoSetAudioPlayRateNormalSpeed";
  bool isSecureMode = true;
  bool isConcurrentMode = true;
  bool isTunnel = true;
  double playbackrate = 1.0;
  avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
  contentprimetype primetype = primeaudio;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (concurrent, tunnel, secure)
 *
 * This test verifies the basic AVClock Playback Pause functionality for Audio
 * Streams in Auto clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 116@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, con_tun_sec_AutoSetAudioPlayRatePause) {

  isAVClockSupportsConcurrency();

  const char *testName = "PrimeClock_AutoSetAudioPlayRatePause";
  bool isSecureMode = true;
  bool isConcurrentMode = true;
  bool isTunnel = true;
  double playbackrate = 0.0;
  avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
  contentprimetype primetype = primeaudio;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (concurrent, tunnel, secure)
 *
 * This test verifies the basic AVClock Playback FastForward functionality for
 * Audio Streams in Auto clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 117@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, con_tun_sec_AutoSetAudioPlayRateFastForward) {

  isAVClockSupportsConcurrency();

  const char *testName = "PrimeClock_AutoSetAudioPlayRateFastForward";
  bool isSecureMode = true;
  bool isConcurrentMode = true;
  bool isTunnel = true;
  double playbackrate = 2.0;
  avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
  contentprimetype primetype = primeaudio;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (concurrent, tunnel, secure)
 *
 * This test verifies the basic AVClock Playback Speed functionality for Video
 * Streams in Auto clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 118@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, con_tun_sec_AutoSetVideoPlayRateNormalSpeed) {

  isAVClockSupportsConcurrency();

  const char *testName = "PrimeClock_AutoSetVideoPlayRateNormalSpeed";
  bool isSecureMode = true;
  bool isConcurrentMode = true;
  bool isTunnel = true;
  double playbackrate = 1.0;
  avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
  contentprimetype primetype = primevideo;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (concurrent, tunnel, secure)
 *
 * This test verifies the basic AVClock Playback Pause functionality for Video
 * Streams in Auto clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 119@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, con_tun_sec_AutoSetVideoPlayRatePause) {

  isAVClockSupportsConcurrency();

  const char *testName = "PrimeClock_AutoSetVideoPlayRatePause";
  bool isSecureMode = true;
  bool isConcurrentMode = true;
  bool isTunnel = true;
  double playbackrate = 0.0;
  avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
  contentprimetype primetype = primevideo;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/**
 * @brief Test the AVClock functionality (concurrent, tunnel, secure)
 *
 * This test verifies the basic AVClock Playback FastForward functionality for
 * Video Streams in Auto clock Mode.
 *
 * **Test Group ID:** 03@n
 * **Test Case ID:** 120@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation
 * [avClock_High-Level-2_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_AVClock, con_tun_sec_AutoSetVideoPlayRateFastForward) {

  isAVClockSupportsConcurrency();

  const char *testName = "PrimeClock_AutoSetVideoPlayRateFastForward";
  bool isSecureMode = true;
  bool isConcurrentMode = true;
  bool isTunnel = true;
  double playbackrate = 2.0;
  avclock::ClockMode clockMode = avclock::ClockMode::AUTO;
  contentprimetype primetype = primevideo;

  runAVClockPrimeTest(testName, isSecureMode, isTunnel, isConcurrentMode,
                      clockMode, primetype, playbackrate);
}

/** @} */ // End of AUDIO_SINK_HALTEST L3 File
/** @} */ // End of AUDIO_SINK_HALTEST
/** @} */ // End of AUDIO_SINK Module
/** @} */ // End of HPK

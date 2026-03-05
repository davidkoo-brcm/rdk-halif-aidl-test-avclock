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

#include "vts_audiodecoder.h"
#include "vts_audiodecoder_parse_testconfig.h"
#include "vts_audiosink.h"
#include "vts_videodecoder.h"

#include "vts_audiosink_helper_functions.h"
#include "vts_avbuffer.h"

#include "vts_videosink_helper_functions.h"
#include "vts_videosink_parse_hfp.h"
#include "vts_videosink_parse_testconfig.h"

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
          {_avClockId, avClock, controlListener, eventListener});
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
          {_audioSinkId, audioSink, controlListener, eventListener});
    }

    for (const auto &_audioDecoderId : audioDecoderIds) {
      sp<IAudioDecoder> audioDecoder;

      audiodecoder::Capabilities *capabilities =
          new audiodecoder::Capabilities();

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

      mAudioDecoderList.push_back({_audioDecoderId, audioDecoder,
                                   controlListener, eventListener,
                                   capabilities});
    }

    for (const auto &_videoDecoderId : videoDecoderIds) {
      sp<IVideoDecoder> videoDecoder;
      videodecoder::Capabilities *capabilities =
          new videodecoder::Capabilities();
      sp<VideoDecoderEventListener> eventListener =
          new VideoDecoderEventListener();
      sp<VideoDecoderControllerListener> controlListener =
          new VideoDecoderControllerListener();

      VIDEODECODER_MANAGER_GETVIDEODECODER(_videoDecoderId, videoDecoder);

      VIDEODECODER_REGISTER_EVENT_SUCCESS(videoDecoder, eventListener);

      VIDEODECODER_GETCAPABILITIES(videoDecoder, capabilities);

      mVideoDecoderList.push_back({_videoDecoderId, videoDecoder,
                                   controlListener, eventListener,
                                   capabilities});
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

      mVideoSinkList.push_back({_videoSinkId, videoSink,
                                videoSinkControlListener,
                                videoSinkEventListener});
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
                                  com::rdk::hal::State oldState,
                                  com::rdk::hal::State intermediateState,
                                  com::rdk::hal::State newState) {
  ::com::rdk::hal::State currentState;

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
       GTEST_SKIP()<< "[  SKIPPED ] Clock Supports only one instance doesn't support Concurrency\n";
    } else {
       UT_LOG_INFO("Platform Supports Concurrency");
    }
}



void AVClockPrimeTest::validateClockTime(
    sp<IAVClock> clockHandle, sp<IAVClockController> &avClockController,
    double playbackrate) {
  double mNormalPlay = 1.0, mFastForward = 2.0, mPause = 0.0;
  ClockTime mFramePTSTime;

  std::lock_guard<std::mutex> lock(mMutex);
  UT_LOG_INFO("Validating clock time for playback rate: %f", playbackrate);

  if (std::fabs(playbackrate - mNormalPlay) < 1e-6) {
    AVCLOCK_GET_CURRENT_CLOCK_TIME_SUCCESS(clockHandle, avClockController,
                                           mFramePTSTime);
  } else if (std::fabs(playbackrate - mFastForward) < 1e-6) {
    AVCLOCK_GET_CURRENT_CLOCK_TIME_SUCCESS(clockHandle, avClockController,
                                           mFramePTSTime);
  } else if (std::fabs(playbackrate - mPause) < 1e-6) {
    AVCLOCK_GET_CURRENT_CLOCK_TIME_SUCCESS(clockHandle, avClockController,
                                           mFramePTSTime);
  }
}

// Audio Test
void AVClockPrimeTest::runAudioTest(const AudioConfig audioCfg,
                                    sp<IAVClockController> &avClockController) {
  try {
    if (!isClkSinkUsable(audioCfg.sinkId))
      return;
    if (!isClkDecoderUsable(audioCfg.decId))
      return;

    bool isAudioSinkAttached = (audioCfg.sinkHandle != nullptr);

    // Set the Audio Sink
    AVCLOCK_SET_AUDIO_SINK_SUCCESS(mClockHandle, avClockController,
                                   audioCfg.sinkId, true);

    AudioDecoderTest decodeTest(
        audioCfg.decId, audioCfg.decHandle, audioCfg.decCtrlListener,
        audioCfg.decEventListener, mBufferManager, mBufferHelper,
        audioCfg.secureDecode, audioCfg.testConfig, isAudioSinkAttached);
    std::thread decodeThread(&AudioDecoderTest::run, &decodeTest, false);

    AudioSinkTest sinkTest(
        audioCfg.sinkId, audioCfg.sinkHandle, audioCfg.sinkCtrlListener,
        audioCfg.sinkEventListener, audioCfg.decCtrlListener, mBufferManager,
        audioCfg.decId, audioCfg.secureDecode, audioCfg.tunnelMode,
        audioCfg.testConfig);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    sinkTest.run();

    if (decodeThread.joinable())
      decodeThread.join();

  } catch (...) {
    UT_LOG_ERROR("Exception in runAudioTest");
    UT_ASSERT_TRUE(false);
  }
}

// Video Test
void AVClockPrimeTest::runVideoTest(const VideoConfig videoCfg,
                                    sp<IAVClockController> &avClockController) {
  try {
    if (!isClkVideoSinkUsable(videoCfg.sinkId))
      return;
    if (!isClkVideoDecoderUsable(videoCfg.decId))
      return;

    bool isVideoSinkAttached = (videoCfg.sinkHandle != nullptr);

    // Set the Audio Sink
    AVCLOCK_SET_VIDEO_SINK_SUCCESS(mClockHandle, avClockController,
                                   videoCfg.sinkId, true);

    videoDecoderTest decodeTest(
        videoCfg.decId, videoCfg.decHandle, videoCfg.decCtrlListener,
        videoCfg.decEventListener, mBufferManager, videoCfg.secureDecode,
        videoCfg.tunnelMode, videoCfg.decTestConfig, mBufferHelper,
        isVideoSinkAttached);
    std::thread decodeThread(&videoDecoderTest::run, &decodeTest, false);

    auto decConfig = videoCfg.decTestConfig;
    auto sinkConfig = videoCfg.sinkTestConfig;

    SinkTest sinkTest(videoCfg.sinkId, videoCfg.sinkHandle,
                      videoCfg.sinkCtrlListener, videoCfg.sinkEventListener,
                      videoCfg.decCtrlListener, decConfig, sinkConfig);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    sinkTest.run(videoCfg.tunnelMode);

    if (decodeThread.joinable())
      decodeThread.join();

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
    const PcrConfig pcrCfg,
    android::sp<IAVClockController> &avClockController) {
  try {
    int64_t ts = getMonotonicRawTimestamp();
    AVCLOCK_NOTIFY_PCR_SAMPLE_SUCCESS(mClockHandle, avClockController,
                                      pcrCfg.pcrValue, ts, true);

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

  } catch (...) {
    UT_LOG_ERROR("Exception in runPcrTest");
    UT_ASSERT_TRUE(false);
  }
}

void runAudioClockTest(std::shared_ptr<AVClockPrimeTest> clockTest,
                       const testConfiguration_t testConfig, bool isSecure,
                       bool isTunnel, bool isConcurrentMode,
                       sp<IAVClockController> &avClockController) {
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
      std::thread primeAudioThread(
          [clockTest, audioCfg = *mAudioList, avClockController]() mutable {
            clockTest->runAudioTest(audioCfg, avClockController);
          });

      sp<IAVClock> clockHandle = clockTest->getClockHandle();
      double playbackRate = clockTest->getPlaybackRate();

      // TBD will below sleep with wait event after spec update.
      std::this_thread::sleep_for(std::chrono::milliseconds(500));

      std::thread validatepts(
          [clockTest, clockHandle, avClockController, playbackRate]() mutable {
            clockTest->validateClockTime(clockHandle, avClockController,
                                         playbackRate);
          });

      if (validatepts.joinable()) {
        validatepts.join();
      }

      if (primeAudioThread.joinable()) {
        primeAudioThread.join();
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
                       videoDecoderTestConfiguration_t decTestConfig,
                       videoTestConfiguration_t sinkTestConfig, bool isSecure,
                       bool isTunnel, bool isConcurrentMode,
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
                                 decTestConfig,
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
                               decTestConfig,
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
      std::thread primeVideoThread(
          [clockTest, videoCfg = *mVideoList, avClockController]() mutable {
            clockTest->runVideoTest(videoCfg, avClockController);
          });

      sp<IAVClock> clockHandle = clockTest->getClockHandle();
      double playbackRate = clockTest->getPlaybackRate();

      // TBD will below sleep with wait event after spec update.
      std::this_thread::sleep_for(std::chrono::milliseconds(500));

      std::thread validatepts(
          [clockTest, clockHandle, avClockController, playbackRate]() mutable {
            clockTest->validateClockTime(clockHandle, avClockController,
                                         playbackRate);
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

void runPCRClockTest(std::shared_ptr<AVClockPrimeTest> clockTest, long pcrValue,
                     sp<IAVClockController> &avClockController) {
  PcrConfig config{pcrValue};
  try {
    // Launch PCR test in a thread using the existing pcrPrimeTest instance
    std::thread primePcrThread(
        [clockTest, config, avClockController]() mutable {
          try {
            clockTest->runPcrTest(config, avClockController);
          } catch (const std::exception &ex) {
            UT_LOG_ERROR("Exception in PCR test thread: %s", ex.what());
          }
        });

    sp<IAVClock> clockHandle = clockTest->getClockHandle();
    double playbackRate = clockTest->getPlaybackRate();

    // TBD will below sleep with wait event after spec update.
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    std::thread validatepts(
        [clockTest, clockHandle, avClockController, playbackRate]() mutable {
          clockTest->validateClockTime(clockHandle, avClockController,
                                       playbackRate);
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

void runAudioPrimeTest(std::shared_ptr<AVClockPrimeTest> clockTest,
                       bool isSecureMode, bool isTunnel, bool isConcurrentMode,
                       android::sp<IAVClockController> &clockController) {
  UT_LOG_INFO("Started Thread : %s", __func__);

  vector<testConfiguration_t> testConfiguration;
  const char *DecodeStreams = "PrimeAudio";

  vts_audiodecoder_parse_testconfig(gAudioPrimeTestConfiguration,
                                    gAudioModuleName, gTestGroup, DecodeStreams,
                                    testConfiguration);

  for (auto &test : testConfiguration) {
    std::thread audioclockthread([&, test] {
      UT_LOG_INFO("Input string %s", test.input);
      runAudioClockTest(clockTest, test, isSecureMode, isTunnel,
                        isConcurrentMode, clockController);
    });

    // Join immediately after starting
    if (audioclockthread.joinable())
      audioclockthread.join();

    UT_LOG_INFO("runAudioPrimeTest Completed");
  }
}

void runVideoPrimeTest(std::shared_ptr<AVClockPrimeTest> clockTest,
                       bool isSecureMode, bool isTunnel, bool isConcurrentMode,
                       android::sp<IAVClockController> &clockController) {
  UT_LOG_INFO("Started Thread : %s", __func__);

  vector<videoDecoderTestConfiguration_t> decoderConfigs;
  vector<videoTestConfiguration_t> sinkConfigs;

  const char *DecodeStreams = "PrimeVideo";

  vts_videodecoder_parse_testconfig(gVideoPrimeTestConfiguration,
                                    VIDEODECODERMGR, gTestGroup, DecodeStreams,
                                    decoderConfigs);

  vts_videosink_parse_testconfig(gVideoPrimeTestConfiguration, VIDEODECODERMGR,
                                 gTestGroup, DecodeStreams, sinkConfigs);

  for (size_t i = 0; i < decoderConfigs.size(); ++i) {
    auto &decoderConfig = decoderConfigs[i];
    auto &sinkConfig = sinkConfigs[i];

    std::thread videoclockthread([&, sinkConfig] {
      UT_LOG_INFO("Video Input string %s", sinkConfig.input);
      runVideoClockTest(clockTest, decoderConfig, sinkConfig, isSecureMode,
                        isTunnel, isConcurrentMode, clockController);
    });

    // Join immediately after starting
    if (videoclockthread.joinable())
      videoclockthread.join();

    UT_LOG_INFO("runVideoPrimeTest Completed");
  }
}

void runPcrPrimeTest(std::shared_ptr<AVClockPrimeTest> clockTest,
                     bool isConcurrentMode,
                     android::sp<IAVClockController> &clockController) {
  UT_LOG_INFO("Started Thread : %s", __func__);
  std::vector<int64_t> pcrValues;
  const char *ParsePCR = "ParsePCR";

  vts_avclock_parse_testconfig(gAVClockTestConfiguration, gModuleName,
                               gTestGroup, ParsePCR, pcrValues);

  for (auto &_pcrValue : pcrValues) {
    std::thread pcrclockthread([&, _pcrValue] {
      UT_LOG_INFO(" PCR Input Value %d", _pcrValue);
      runPCRClockTest(clockTest, _pcrValue, clockController);
    });

    // Join immediately after starting
    if (pcrclockthread.joinable())
      pcrclockthread.join();

    UT_LOG_INFO("runPcrPrimeTest Completed");
  }
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
        UT_LOG_INFO("Platform Supports One Instacne Skipping Concurrent Case "
                    "same as Sequential");
        break;
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

      // Prime Audio Only
      if (primetype == primeaudio) {
        runaudioprimethread = std::thread([&, storedClockTest]() mutable {
          {
            std::lock_guard<std::mutex> lock(mtx);
            audioReady = true;
          }
          cv.notify_all();
          runAudioPrimeTest(storedClockTest, isSecureMode, isTunnel,
                            isConcurrentMode, clockControllerList.back());
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
        runvideoprimethread = std::thread([&, storedClockTest] {
          {
            std::lock_guard<std::mutex> lock(mtx);
            videoReady = true;
          }
          runVideoPrimeTest(storedClockTest, isSecureMode, isTunnel,
                            isConcurrentMode, clockControllerList.back());
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
        runaudioprimethread = std::thread([&, storedClockTest] {
          {
            std::lock_guard<std::mutex> lock(mtx);
            audioReady = true;
          }
          runAudioPrimeTest(storedClockTest, isSecureMode, isTunnel,
                            isConcurrentMode, clockControllerList.back());
          cv.notify_all();
        });

        if (!waitForReady(audioReady, "Audio")) {
          cleanupAndReturn(runaudioprimethread, runvideoprimethread,
                           runpcrprimethread);
          return;
        }

        primeThreads.push_back(std::move(runaudioprimethread));

        runvideoprimethread = std::thread([&, storedClockTest] {
          {
            std::lock_guard<std::mutex> lock(mtx);
            videoReady = true;
          }
          runVideoPrimeTest(storedClockTest, isSecureMode, isTunnel,
                            isConcurrentMode, clockControllerList.back());
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
        runvideoprimethread = std::thread([&, storedClockTest] {
          {
            std::lock_guard<std::mutex> lock(mtx);
            videoReady = true;
          }
          runVideoPrimeTest(storedClockTest, isSecureMode, isTunnel,
                            isConcurrentMode, clockControllerList.back());
          cv.notify_all();
        });

        if (!waitForReady(videoReady, "Video")) {
          cleanupAndReturn(runaudioprimethread, runvideoprimethread,
                           runpcrprimethread);
          return;
        }

        primeThreads.push_back(std::move(runvideoprimethread));

        runaudioprimethread = std::thread([&, storedClockTest] {
          {
            std::lock_guard<std::mutex> lock(mtx);
            audioReady = true;
          }
          runAudioPrimeTest(storedClockTest, isSecureMode, isTunnel,
                            isConcurrentMode, clockControllerList.back());
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
        runaudioprimethread = std::thread([&, storedClockTest] {
          {
            std::lock_guard<std::mutex> lock(mtx);
            audioReady = true;
          }
          runAudioPrimeTest(storedClockTest, isSecureMode, isTunnel,
                            isConcurrentMode, clockControllerList.back());
          cv.notify_all();
        });

        if (!waitForReady(audioReady, "Audio")) {
          cleanupAndReturn(runaudioprimethread, runvideoprimethread,
                           runpcrprimethread);
          return;
        }

        primeThreads.push_back(std::move(runaudioprimethread));

        runvideoprimethread = std::thread([&, storedClockTest] {
          {
            std::lock_guard<std::mutex> lock(mtx);
            videoReady = true;
          }
          runVideoPrimeTest(storedClockTest, isSecureMode, isTunnel,
                            isConcurrentMode, clockControllerList.back());
          cv.notify_all();
        });

        if (!waitForReady(videoReady, "Video")) {
          cleanupAndReturn(runaudioprimethread, runvideoprimethread,
                           runpcrprimethread);
          return;
        }

        primeThreads.push_back(std::move(runvideoprimethread));

        runpcrprimethread = std::thread([&, storedClockTest] {
          {
            std::lock_guard<std::mutex> lock(mtx);
            audioReady = true;
          }
          runPcrPrimeTest(storedClockTest, isConcurrentMode,
                          clockControllerList.back());
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
      _clockTest->shutdownClock(_clockController.get());
    }
  } else {
    for (const auto &[_clockId, clockHandle, controlListener, eventListener] :
         avClockList) {
      auto clockTest = std::make_shared<AVClockPrimeTest>(
          _clockId, clockHandle, clockMode, controlListener, avBufferManager,
          avBufferHelper, playbackRate);

      // Call the method on the instance
      auto avclockController = clockTest->setupAndStartClock();

      // Prime Audio Only
      if (primetype == primeaudio) {
        runaudioprimethread = std::thread([&, clockTest] {
          {
            std::lock_guard<std::mutex> lock(mtx);
            audioReady = true;
          }
          runAudioPrimeTest(clockTest, isSecureMode, isTunnel, isConcurrentMode,
                            avclockController);
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
        runvideoprimethread = std::thread([&, clockTest] {
          {
            std::lock_guard<std::mutex> lock(mtx);
            videoReady = true;
          }
          runVideoPrimeTest(clockTest, isSecureMode, isTunnel, isConcurrentMode,
                            avclockController);
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
        runaudioprimethread = std::thread([&, clockTest] {
          {
            std::lock_guard<std::mutex> lock(mtx);
            audioReady = true;
          }
          runAudioPrimeTest(clockTest, isSecureMode, isTunnel, isConcurrentMode,
                            avclockController);
          cv.notify_all();
        });

        if (!waitForReady(audioReady, "Audio")) {
          cleanupAndReturn(runaudioprimethread, runvideoprimethread,
                           runpcrprimethread);
          return;
        }

        runvideoprimethread = std::thread([&, clockTest] {
          {
            std::lock_guard<std::mutex> lock(mtx);
            videoReady = true;
          }
          runVideoPrimeTest(clockTest, isSecureMode, isTunnel, isConcurrentMode,
                            avclockController);
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
        runvideoprimethread = std::thread([&, clockTest] {
          {
            std::lock_guard<std::mutex> lock(mtx);
            videoReady = true;
          }
          runVideoPrimeTest(clockTest, isSecureMode, isTunnel, isConcurrentMode,
                            avclockController);
          cv.notify_all();
        });

        if (!waitForReady(videoReady, "Video")) {
          cleanupAndReturn(runaudioprimethread, runvideoprimethread,
                           runpcrprimethread);
          return;
        }

        runaudioprimethread = std::thread([&, clockTest] {
          {
            std::lock_guard<std::mutex> lock(mtx);
            audioReady = true;
          }
          runAudioPrimeTest(clockTest, isSecureMode, isTunnel, isConcurrentMode,
                            avclockController);
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
        runaudioprimethread = std::thread([&, clockTest] {
          {
            std::lock_guard<std::mutex> lock(mtx);
            audioReady = true;
          }
          runAudioPrimeTest(clockTest, isSecureMode, isTunnel, isConcurrentMode,
                            avclockController);
          cv.notify_all();
        });

        if (!waitForReady(audioReady, "Audio")) {
          cleanupAndReturn(runaudioprimethread, runvideoprimethread,
                           runpcrprimethread);
          return;
        }

        runvideoprimethread = std::thread([&, clockTest] {
          {
            std::lock_guard<std::mutex> lock(mtx);
            videoReady = true;
          }
          runVideoPrimeTest(clockTest, isSecureMode, isTunnel, isConcurrentMode,
                            avclockController);
          cv.notify_all();
        });

        if (!waitForReady(videoReady, "Video")) {
          cleanupAndReturn(runaudioprimethread, runvideoprimethread,
                           runpcrprimethread);
          return;
        }

        runpcrprimethread = std::thread([&, clockTest] {
          {
            std::lock_guard<std::mutex> lock(mtx);
            pcrReady = true;
          }
          runPcrPrimeTest(clockTest, isConcurrentMode, avclockController);
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
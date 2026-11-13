# AVClock VTS Change Review

## Latest test results

Platform: Broadcom Xione. Profile: `profiles/hfp-unified_xione.yaml`. Run date: 2026-11-12.

| Level | Total | PASS | FAIL | SKIP | Duration (s) |
| --- | ---: | ---: | ---: | ---: | ---: |
| L1 | 32 | 32 | 0 | 0 | 2.334 |
| L2 | 5 | 5 | 0 | 0 | 0.447 |
| L3 | 120 | 21 | 0 | 99 | 1421.244 |
| Total | 157 | 58 | 0 | 99 | 1424.025 |

All executed cases pass. These tests verify clock priming, mode transitions and progression. PASS does not establish simultaneous audible playback or pitch-corrected trick audio.

## Prepare audio routing before decoder start

Reset stream listeners, prepare the decoder, prepare its sink/STC, and then launch media threads. The fixed 500 ms startup delay is removed; the 500 ms measurement interval remains.

[`src/vts_avclock_l3_tests.cpp:691`](../../src/vts_avclock_l3_tests.cpp#L691)

```cpp
    // Open/configure the decoder, then connect the sink/STC while READY.
    // run() starts the decoder only after that STC binding is in place.
    if (!decodeTest.prepare()) return;

    UT_LOG_INFO("Preparing AudioSink before submitting decoder input");
    AudioSinkTest sinkTest(
        audioCfg.sinkId, audioCfg.sinkHandle, audioCfg.sinkCtrlListener,
        audioCfg.sinkEventListener, audioCfg.decCtrlListener, mBufferManager,
        audioCfg.decId, mClockId, audioCfg.secureDecode, audioCfg.tunnelMode,
        audioCfg.testConfig, isClockInUse);
    if (!sinkTest.prepare()) return;
    // Both run() methods reuse the prepared session. No fixed delay and no
    // input submission before sink/STC attachment.
    std::thread sinkThread(&AudioSinkTest::runKeepingSession, &sinkTest);
    std::thread decodeThread(&AudioDecoderTest::run, &decodeTest, true);
```

## Retain audio resources until measurement completes

Use a packaged validation task and shared completion future. Playback waits before closing the sink, and the caller joins both threads before surfacing validation exceptions. Output-mode classification is read after producer completion so UNKNOWN does not prematurely reject a running tunnel session.

[`src/vts_avclock_l3_tests.cpp:713`](../../src/vts_avclock_l3_tests.cpp#L713)

```cpp
    // Sink run() may finish before the 500ms rate sample. Keep its route and
    // the attached decoder alive until validation returns on every path.
    validationDone.wait();
    sinkTest.close();
```

## Connect video before starting its producer

Open the shared decoder session, prepare the video sink, and only then start decoding. Sink preparation initializes listener state once and avoids a second reset that could erase early callbacks.

[`src/vts_avclock_l3_tests.cpp:795`](../../src/vts_avclock_l3_tests.cpp#L795)

```cpp
    // Register the decoder and connect the sink before decoder start can
    // create a headless route. prepare() does not reset listeners twice.
    if (!decodeTest.openDecoderSession()) {
      UT_ASSERT_TRUE(false) << "Failed to open video decoder session";
      return;
    }
    if (!sinkTest.prepare()) {
      decodeTest.closeDecoderSession();
      return;
    }
    std::thread decodeThread(&videoDecoderTest::run, &decodeTest, true);
    std::thread sinkThread(&VideoSinkTest::run, &sinkTest, videoCfg.tunnelMode);
```

## Feed PCR across all media phases

Keep PCR participating until both media producers withdraw. Restarted clock phases receive fresh PCR samples. Validate PCR values against the previous value, not only the first value. Video PTS and enabled control points are shifted together to the current PCR plus two seconds, with overflow checks; disabled negative controls stay disabled.

[`src/vts_avclock_l3_tests.cpp:1535`](../../src/vts_avclock_l3_tests.cpp#L1535)

```cpp
  // Keep PCR as a participant until both media producers withdraw. Each
  // finish barrier restarts the clock, so every new phase needs new PCRs.
  while (phaseSynchronizer->beginPcrConfiguration(primetype)) {
    runPCRClockTest(clockTest, pcrValues, primetype, clockController);
    phaseSynchronizer->finishConfiguration();
  }

  phaseSynchronizer->drop(threadtype::pcr);
}
```

## Filter unsupported video modes before barriers

Reject unsupported video operational modes before constructing phase participants. Secure/capability exits withdraw their participant. Select supported streams for the actual decoder, including PIP capability differences. Audio output-mode checks preserve their own path.

[`src/vts_avclock_l3_tests.cpp:1559`](../../src/vts_avclock_l3_tests.cpp#L1559)

```cpp
  // Filter the whole case before creating phase participants. Returning a
  // single stream early can otherwise strand peers at the phase barrier.
  const bool needsVideo = primetype != primeaudio;
  if (needsVideo && !isAVClockVideoModeSupported(isTunnel)) {
    UT_IGNORE_TEST() << "Requested video operational mode is unsupported";
    return;
  }

  vector<shared_ptr<AVClockPrimeTest>> clockTests;
```

## Make rate failures explicit

A primed clock returning no t1 or t2 is an assertion failure. Keep the 0.15 absolute rate tolerance and report raw interval and clock interval in a failed rate assertion. PRIMED is followed immediately by t1, a 500 ms wait and t2; progression uses the returned sample timestamps.

[`src/vts_avclock_l3_tests.cpp:617`](../../src/vts_avclock_l3_tests.cpp#L617)

```cpp
        UT_ASSERT_TRUE(diff < tolerance)
            << "expectedRate=" << expectedRate << " actualRate=" << actualRate
            << " clockDeltaNs=" << clockDelta << " rawDeltaNs=" << timeDelta
            << " t1=" << firstClockTime.clockTimeNs
            << " t2=" << secondClockTime.clockTimeNs;
```

## Release fixture listeners once

Assign video listener decoder IDs during setup. Unregister all clock, decoder and sink event listeners in fixture teardown and release capability objects before clearing lists. Per-case clock shutdown no longer unregisters those listeners a second time.

[`src/vts_avclock_l3_tests.cpp:333`](../../src/vts_avclock_l3_tests.cpp#L333)

```cpp
  void TearDown() override {
    for (const auto& [id, clock, controlListener, eventListener] : mAVClockList) {
      AVCLOCK_UNREGISTER_EVENT_LISTENER_SUCCESS(clock, eventListener);
    }
    for (const auto& [id, decoder, controlListener, eventListener, capabilities] : mAudioDecoderList) {
      AUDIODECODER_UNREGISTER_EVENT_SUCCESS(decoder, eventListener);
      delete capabilities;
    }
    for (const auto& [id, decoder, controlListener, eventListener, capabilities] : mVideoDecoderList) {
      VIDEODECODER_UNREGISTER_EVENT_SUCCESS(decoder, eventListener);
      delete capabilities;
    }
    for (const auto& [id, sink, controlListener, eventListener] : mAudioSinkList) {
      AUDIOSINK_UNREGISTER_EVENT_SUCCESS(sink, eventListener);
    }
    for (const auto& [id, sink, controlListener, eventListener] : mVideoSinkList) {
      VIDEOSINK_UNREGISTER_EVENT_SUCCESS(sink, eventListener);
    }
    mAVClockList.clear();
    mAudioDecoderList.clear();
    mVideoDecoderList.clear();
    mAudioSinkList.clear();
    mVideoSinkList.clear();
```

.PHONY: all


AVCLOCK_AIDL_LIB_NAME := avclockaidl
AUDIOSINK_AIDL_LIB_NAME := audiosinkaidl
AUDIODECODER_AIDL_LIB_NAME := audiodecoderaidl
VIDEOSINK_AIDL_LIB_NAME := videosinkaidl
VIDEODECODER_AIDL_LIB_NAME := videodecoderaidl

EMULATE_SERVICE_BIN := EmulateAVClockService
AVCLOCK_AIDL_LIB := lib$(AVCLOCK_AIDL_LIB_NAME).so
AUDIOSINK_AIDL_LIB := lib$(AUDIOSINK_AIDL_LIB_NAME).so
AUDIODECODER_AIDL_LIB := lib$(AUDIODECODER_AIDL_LIB_NAME).so
VIDEOSINK_AIDL_LIB := lib$(VIDEOSINK_AIDL_LIB_NAME).so
VIDEODECODER_AIDL_LIB := lib$(VIDEODECODER_AIDL_LIB_NAME).so

EMULATE_SERVICE_DIR ?= $(CURDIR)/src

BUILD_DIR ?= $(CURDIR)/../build

AIDL_GEN_DIR ?= $(BUILD_DIR)/avclock_gen/current

COMM_AIDL_LIB_SRC += $(AIDL_GEN_DIR)/cpp/com/rdk/hal/AVSource.cpp
COMM_AIDL_LIB_SRC += $(AIDL_GEN_DIR)/cpp/com/rdk/hal/HALError.cpp
COMM_AIDL_LIB_SRC += $(AIDL_GEN_DIR)/cpp/com/rdk/hal/PropertyValue.cpp
COMM_AIDL_LIB_SRC += $(AIDL_GEN_DIR)/cpp/com/rdk/hal/State.cpp

AVCLOCK_AIDL_LIB_SRC += $(AIDL_GEN_DIR)/cpp/com/rdk/hal/avclock/Capabilities.cpp
AVCLOCK_AIDL_LIB_SRC += $(AIDL_GEN_DIR)/cpp/com/rdk/hal/avclock/ClockMode.cpp
AVCLOCK_AIDL_LIB_SRC += $(AIDL_GEN_DIR)/cpp/com/rdk/hal/avclock/ClockTime.cpp
AVCLOCK_AIDL_LIB_SRC += $(AIDL_GEN_DIR)/cpp/com/rdk/hal/avclock/IAVClockController.cpp
AVCLOCK_AIDL_LIB_SRC += $(AIDL_GEN_DIR)/cpp/com/rdk/hal/avclock/IAVClockControllerListener.cpp
AVCLOCK_AIDL_LIB_SRC += $(AIDL_GEN_DIR)/cpp/com/rdk/hal/avclock/IAVClock.cpp
AVCLOCK_AIDL_LIB_SRC += $(AIDL_GEN_DIR)/cpp/com/rdk/hal/avclock/IAVClockEventListener.cpp
AVCLOCK_AIDL_LIB_SRC += $(AIDL_GEN_DIR)/cpp/com/rdk/hal/avclock/IAVClockManager.cpp
AVCLOCK_AIDL_LIB_SRC += $(AIDL_GEN_DIR)/cpp/com/rdk/hal/avclock/Property.cpp
AVCLOCK_AIDL_LIB_SRC += $(COMM_AIDL_LIB_SRC)

AUDIODECODER_AIDL_LIB_SRC += $(AIDL_GEN_DIR)/cpp/com/rdk/hal/audiodecoder/Capabilities.cpp
AUDIODECODER_AIDL_LIB_SRC += $(AIDL_GEN_DIR)/cpp/com/rdk/hal/audiodecoder/ChannelType.cpp
AUDIODECODER_AIDL_LIB_SRC += $(AIDL_GEN_DIR)/cpp/com/rdk/hal/audiodecoder/Codec.cpp
AUDIODECODER_AIDL_LIB_SRC += $(AIDL_GEN_DIR)/cpp/com/rdk/hal/audiodecoder/CSDAudioFormat.cpp
AUDIODECODER_AIDL_LIB_SRC += $(AIDL_GEN_DIR)/cpp/com/rdk/hal/audiodecoder/ErrorCode.cpp
AUDIODECODER_AIDL_LIB_SRC += $(AIDL_GEN_DIR)/cpp/com/rdk/hal/audiodecoder/FrameMetadata.cpp
AUDIODECODER_AIDL_LIB_SRC += $(AIDL_GEN_DIR)/cpp/com/rdk/hal/audiodecoder/IAudioDecoderController.cpp
AUDIODECODER_AIDL_LIB_SRC += $(AIDL_GEN_DIR)/cpp/com/rdk/hal/audiodecoder/IAudioDecoderControllerListener.cpp
AUDIODECODER_AIDL_LIB_SRC += $(AIDL_GEN_DIR)/cpp/com/rdk/hal/audiodecoder/IAudioDecoder.cpp
AUDIODECODER_AIDL_LIB_SRC += $(AIDL_GEN_DIR)/cpp/com/rdk/hal/audiodecoder/IAudioDecoderEventListener.cpp
AUDIODECODER_AIDL_LIB_SRC += $(AIDL_GEN_DIR)/cpp/com/rdk/hal/audiodecoder/IAudioDecoderManager.cpp
AUDIODECODER_AIDL_LIB_SRC += $(AIDL_GEN_DIR)/cpp/com/rdk/hal/audiodecoder/PCMFormat.cpp
AUDIODECODER_AIDL_LIB_SRC += $(AIDL_GEN_DIR)/cpp/com/rdk/hal/audiodecoder/Property.cpp
AUDIODECODER_AIDL_LIB_SRC_LIB_SRC += $(COMM_AIDL_LIB_SRC)

AUDIOSINK_AIDL_LIB_SRC += $(AIDL_GEN_DIR)/cpp/com/rdk/hal/audiosink/Capabilities.cpp
AUDIOSINK_AIDL_LIB_SRC += $(AIDL_GEN_DIR)/cpp/com/rdk/hal/audiosink/ErrorCode.cpp
AUDIOSINK_AIDL_LIB_SRC += $(AIDL_GEN_DIR)/cpp/com/rdk/hal/audiosink/IAudioSinkController.cpp
AUDIOSINK_AIDL_LIB_SRC += $(AIDL_GEN_DIR)/cpp/com/rdk/hal/audiosink/IAudioSinkControllerListener.cpp
AUDIOSINK_AIDL_LIB_SRC += $(AIDL_GEN_DIR)/cpp/com/rdk/hal/audiosink/IAudioSink.cpp
AUDIOSINK_AIDL_LIB_SRC += $(AIDL_GEN_DIR)/cpp/com/rdk/hal/audiosink/IAudioSinkEventListener.cpp
AUDIOSINK_AIDL_LIB_SRC += $(AIDL_GEN_DIR)/cpp/com/rdk/hal/audiosink/IAudioSinkManager.cpp
AUDIOSINK_AIDL_LIB_SRC += $(AIDL_GEN_DIR)/cpp/com/rdk/hal/audiosink/PlatformCapabilities.cpp
AUDIOSINK_AIDL_LIB_SRC += $(AIDL_GEN_DIR)/cpp/com/rdk/hal/audiosink/Property.cpp
AUDIOSINK_AIDL_LIB_SRC += $(AIDL_GEN_DIR)/cpp/com/rdk/hal/audiosink/Volume.cpp
AUDIOSINK_AIDL_LIB_SRC += $(AIDL_GEN_DIR)/cpp/com/rdk/hal/audiosink/VolumeRamp.cpp
AUDIOSINK_AIDL_LIB_SRC += $(COMM_AIDL_LIB_SRC)

VIDEODECODER_AIDL_LIB_SRC += $(AIDL_GEN_DIR)/cpp/com/rdk/hal/videodecoder/Capabilities.cpp
VIDEODECODER_AIDL_LIB_SRC += $(AIDL_GEN_DIR)/cpp/com/rdk/hal/videodecoder/CodecCapabilities.cpp
VIDEODECODER_AIDL_LIB_SRC += $(AIDL_GEN_DIR)/cpp/com/rdk/hal/videodecoder/Codec.cpp
VIDEODECODER_AIDL_LIB_SRC += $(AIDL_GEN_DIR)/cpp/com/rdk/hal/videodecoder/CSDVideoFormat.cpp
VIDEODECODER_AIDL_LIB_SRC += $(AIDL_GEN_DIR)/cpp/com/rdk/hal/videodecoder/DynamicRange.cpp
VIDEODECODER_AIDL_LIB_SRC += $(AIDL_GEN_DIR)/cpp/com/rdk/hal/videodecoder/ErrorCode.cpp
VIDEODECODER_AIDL_LIB_SRC += $(AIDL_GEN_DIR)/cpp/com/rdk/hal/videodecoder/FrameMetadata.cpp
VIDEODECODER_AIDL_LIB_SRC += $(AIDL_GEN_DIR)/cpp/com/rdk/hal/videodecoder/IVideoDecoderController.cpp
VIDEODECODER_AIDL_LIB_SRC += $(AIDL_GEN_DIR)/cpp/com/rdk/hal/videodecoder/IVideoDecoderControllerListener.cpp
VIDEODECODER_AIDL_LIB_SRC += $(AIDL_GEN_DIR)/cpp/com/rdk/hal/videodecoder/IVideoDecoder.cpp
VIDEODECODER_AIDL_LIB_SRC += $(AIDL_GEN_DIR)/cpp/com/rdk/hal/videodecoder/IVideoDecoderEventListener.cpp
VIDEODECODER_AIDL_LIB_SRC += $(AIDL_GEN_DIR)/cpp/com/rdk/hal/videodecoder/IVideoDecoderManager.cpp
VIDEODECODER_AIDL_LIB_SRC += $(AIDL_GEN_DIR)/cpp/com/rdk/hal/videodecoder/OperationalMode.cpp
VIDEODECODER_AIDL_LIB_SRC += $(AIDL_GEN_DIR)/cpp/com/rdk/hal/videodecoder/PixelFormat.cpp
VIDEODECODER_AIDL_LIB_SRC += $(AIDL_GEN_DIR)/cpp/com/rdk/hal/videodecoder/Property.cpp
VIDEODECODER_AIDL_LIB_SRC += $(AIDL_GEN_DIR)/cpp/com/rdk/hal/videodecoder/PropertyKVPair.cpp
VIDEODECODER_AIDL_LIB_SRC += $(AIDL_GEN_DIR)/cpp/com/rdk/hal/videodecoder/ScanType.cpp
VIDEODECODER_AIDL_LIB_SRC += $(AIDL_GEN_DIR)/cpp/com/rdk/hal/videosink/Capabilities.cpp
VIDEODECODER_AIDL_LIB_SRC += $(COMM_AIDL_LIB_SRC)

VIDEOSINK_AIDL_LIB_SRC += $(AIDL_GEN_DIR)/cpp/com/rdk/hal/videosink/IVideoSinkController.cpp
VIDEOSINK_AIDL_LIB_SRC += $(AIDL_GEN_DIR)/cpp/com/rdk/hal/videosink/IVideoSinkControllerListener.cpp
VIDEOSINK_AIDL_LIB_SRC += $(AIDL_GEN_DIR)/cpp/com/rdk/hal/videosink/IVideoSink.cpp
VIDEOSINK_AIDL_LIB_SRC += $(AIDL_GEN_DIR)/cpp/com/rdk/hal/videosink/IVideoSinkEventListener.cpp
VIDEOSINK_AIDL_LIB_SRC += $(AIDL_GEN_DIR)/cpp/com/rdk/hal/videosink/IVideoSinkManager.cpp
VIDEOSINK_AIDL_LIB_SRC += $(AIDL_GEN_DIR)/cpp/com/rdk/hal/videosink/Property.cpp
VIDEOSINK_AIDL_LIB_SRC += $(COMM_AIDL_LIB_SRC)

EMULATE_SERVICE_BIN_SRC := $(EMULATE_SERVICE_DIR)/AVClockService.cpp
EMULATE_SERVICE_BIN_SRC += $(EMULATE_SERVICE_DIR)/AVClock.cpp

EMULATE_SERVICE_INC_FLAGS := -I$(AIDL_GEN_DIR)/h
EMULATE_SERVICE_INC_FLAGS += -I$(BUILD_DIR)/usr/include
EMULATE_SERVICE_INC_FLAGS += -I$(EMULATE_SERVICE_DIR)

EMULATE_SERVICE_LD_LIBS := -lbinder -lutils

EMULATE_SERVICE_LDFLAGS := -L$(BUILD_DIR)/usr/lib
EMULATE_SERVICE_LDFLAGS += -Wl,-rpath-link=$(BUILD_DIR)/usr/lib

EMULATE_SERVICE_CFLAGS := -Wall -Wextra -std=c++17 -ggdb

VIDEODECODER_AIDL_LIB := $(BUILD_DIR)/usr/lib/$(VIDEODECODER_AIDL_LIB)
AUDIODECODER_AIDL_LIB := $(BUILD_DIR)/usr/lib/$(AUDIODECODER_AIDL_LIB)
AUDIOSINK_AIDL_LIB := $(BUILD_DIR)/usr/lib/$(AUDIOSINK_AIDL_LIB)
VIDEOSINK_AIDL_LIB := $(BUILD_DIR)/usr/lib/$(VIDEOSINK_AIDL_LIB)
AVCLOCK_AIDL_LIB := $(BUILD_DIR)/usr/lib/$(AVCLOCK_AIDL_LIB)
EMULATE_SERVICE_BIN := $(BUILD_DIR)/usr/lib/$(EMULATE_SERVICE_BIN)

all: $(EMULATE_SERVICE_BIN) $(AVCLOCK_AIDL_LIB)

$(VIDEODECODER_AIDL_LIB): $(VIDEODECODER_AIDL_LIB_SRC:%.cpp=%.o)
	g++ -shared $(EMULATE_SERVICE_LDFLAGS) $(filter %.o, $^) -o $@ $(EMULATE_SERVICE_LD_LIBS)

$(AUDIODECODER_AIDL_LIB): $(AUDIODECODER_AIDL_LIB_SRC:%.cpp=%.o)
	g++ -shared $(EMULATE_SERVICE_LDFLAGS) $(filter %.o, $^) -o $@ $(EMULATE_SERVICE_LD_LIBS)

$(VIDEOSINK_AIDL_LIB): $(VIDEODECODER_AIDL_LIB)
$(VIDEOSINK_AIDL_LIB): $(VIDEOSINK_AIDL_LIB_SRC:%.cpp=%.o)
	g++ -shared $(EMULATE_SERVICE_LDFLAGS) $(filter %.o, $^) -o $@ $(EMULATE_SERVICE_LD_LIBS) \
		-l$(VIDEODECODER_AIDL_LIB_NAME)

$(AUDIOSINK_AIDL_LIB): $(AUDIODECODER_AIDL_LIB)
$(AUDIOSINK_AIDL_LIB): $(AUDIOSINK_AIDL_LIB_SRC:%.cpp=%.o)
	g++ -shared $(EMULATE_SERVICE_LDFLAGS) $(filter %.o, $^) -o $@ $(EMULATE_SERVICE_LD_LIBS) \
		-l$(AUDIODECODER_AIDL_LIB_NAME)

$(AVCLOCK_AIDL_LIB): $(AUDIOSINK_AIDL_LIB) $(VIDEOSINK_AIDL_LIB)
$(AVCLOCK_AIDL_LIB): $(AVCLOCK_AIDL_LIB_SRC:%.cpp=%.o)
	g++ -shared $(EMULATE_SERVICE_LDFLAGS) $(filter %.o, $^) -o $@ $(EMULATE_SERVICE_LD_LIBS) \
		-l$(AUDIOSINK_AIDL_LIB_NAME) \
		-l$(VIDEOSINK_AIDL_LIB_NAME)

$(EMULATE_SERVICE_BIN): $(AVCLOCK_AIDL_LIB)
$(EMULATE_SERVICE_BIN): $(EMULATE_SERVICE_BIN_SRC:%.cpp=%.o)
	g++ $(EMULATE_SERVICE_LDFLAGS) \
		$(filter %.o, $^) -o $@ \
		$(EMULATE_SERVICE_LD_LIBS) \
		-lpthread \
		-l$(AVCLOCK_AIDL_LIB_NAME)

# TODO: to be compile out of source tree
%.o: %.cpp
	g++ $(EMULATE_SERVICE_CFLAGS) $(EMULATE_SERVICE_INC_FLAGS) -fPIC -c $< -o $@

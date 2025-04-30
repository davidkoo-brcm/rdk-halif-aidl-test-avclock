# Testing Suite For AVClock HAL

## Build Steps

### 1. Build linux binder

The test suite depends on the Linux binder with `AIDL` support. Follow these steps to build it:

For detail build instruction, please refer to [Linux Binder](https://github.com/rdk-e/linux_binder_idl/tree/main)

Here outline the simplest steps are

```bash
git clone -b develop 'ssh://git@github.com/rdk-e/linux_binder_idl.git'

cd linux_binder_idl

./build-linux-binder-aidl.sh 64
```

### 2. Build this test

Clone and build the test suite:

```bash
git clone git@github.com:rdkcentral/rdk-halif-binder-test-avbuffer.git
```

```bash
cd rdk-halif-binder-test-avbuffer
```

Addition environment variables can be used:

- AIDL_BIN: Path to the `AIDL` compiler (e.g., /path/to/linux_binder_idl/out/aidl).

- LINUX_BINDER_OUT_DIR: Path to the Linux binder output directory (e.g., /path/to/linux_binder_idl/local).

- UT_CORE_VERSION: Specific version of the UT-Core framework (e.g., 4.6.0). Defaults to the latest tagged version.

Example:

```bash
export UT_CORE_VERSION=4.6.0
export AIDL_BIN=/path/linux_binder_idl/out/aidl
export LINUX_BINDER_OUT_DIR=/path//linux_binder_idl/local/
```

Run the build script:

```bash
./build.sh
```

The build.sh script:

- Clones the rdk-halif-aidl repository and UT-Core framework.

- Compiles the test suite.

- Generates binaries in the build/bin directory.

Example directory structure :

```bash
build/bin/
├── aidl
├── EmulateAVClockService
├── lib
├── libaudiodecoderaidl.so
├── libaudiosinkaidl.so
├── libavclockaidl.so
├── libbase.so
├── libbinder.so
├── libcutils.so
├── libcutils_sockets.so
├── liblog.so
├── libut_control.so
├── libutils.so
├── libvideodecoderaidl.so
├── libvideosinkaidl.so
├── profiles
│   └── hfp-avclock.yaml
├── run.sh
├── servicemanager
└── vts_avclock

```

## Run test

- Package the Binaries:

```bash
cd build
tar -zcf avclock_vts.tar.gz bin
```

- Transfer to Target Device:

Copy the tarball to the target device (e.g., using scp):

```bash
scp avclock_vts.tar.gz root@target-device:/home/root/

```

- Extract on Target Device:

Log into the target device and extract the tarball:

```bash
ssh root@target-device
cd /home/root
tar -zxf avclock_vts.tar.gz
cd bin
```

- Run the Test:
Execute the run.sh script as root:

```bash
sudo ./run.sh -p profiles/hfp-avclock.yaml
```

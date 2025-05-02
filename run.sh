#!/bin/bash
#* ******************************************************************************
#*  If not stated otherwise in this file or this component's LICENSE
#*  file the following copyright and licenses apply:
#*
#*  Copyright 2025 RDK Management
#*
#*  Licensed under the Apache License, Version 2.0 (the License);
#*  you may not use this file except in compliance with the License.
#*  You may obtain a copy of the License at
#*
#*  http://www.apache.org/licenses/LICENSE-2.0
#*
#*  Unless required by applicable law or agreed to in writing, software
#*  distributed under the License is distributed on an AS IS BASIS,
#*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#*  See the License for the specific language governing permissions and
#*  limitations under the License.
#*
#* ******************************************************************************

DEV_BINDER='/dev/binder'
RUN_DIR=$(realpath "$(dirname "$0")")

# Check if the script has write permission to the binder device
if [ ! -w "$DEV_BINDER" ]; then
    echo "No permission to access $DEV_BINDER, consider executing this as root"
    exit 1
fi

# Set LD_LIBRARY_PATH to include the script's directory
export LD_LIBRARY_PATH="$RUN_DIR:$LD_LIBRARY_PATH"

# Check if servicemanager is already running
if pidof servicemanager > /dev/null; then
    echo "Service 'servicemanager' is already running"
    servicemanager_pid=$(pidof servicemanager)
else
    echo "Executing [servicemanager]"
    "${RUN_DIR}/servicemanager" &
    servicemanager_pid=$!
    # Wait for the service to be ready
    sleep 2
    # Verify if servicemanager started successfully
    if ! ps -p "$servicemanager_pid" > /dev/null; then
        echo "Failed to start servicemanager"
        exit 1
    fi
fi

# Prompt user to start EmulateAVClockService
echo "Do you want to start the mock service [EmulateAVClockService]? (y/n)"
read -r user_choice
case "$user_choice" in
    [Yy]*)
        echo "Executing [EmulateAVClockService]"
        "${RUN_DIR}/EmulateAVClockService" &
        AVClockService_pid=$!
        # Wait for the service to be ready
        sleep 2
        # Verify if EmulateAVClockService started successfully
        if ! ps -p "$AVClockService_pid" > /dev/null; then
            echo "Failed to start EmulateAVClockService"
            # Clean up servicemanager if it was started by this script
            if [ -n "$servicemanager_pid" ] && ps -p "$servicemanager_pid" > /dev/null; then
                kill -9 "$servicemanager_pid"
            fi
            exit 1
        fi
        ;;
    [Nn]*)
        echo "Skipping [EmulateAVClockService]"
        AVClockService_pid=""
        ;;
    *)
        echo "Invalid input. Please enter 'y' or 'n'. Skipping [EmulateAVClockService]"
        AVClockService_pid=""
        ;;
esac

# Execute vts_avclock with the specified profile
echo "Executing [vts_avclock]"
${RUN_DIR}/vts_avclock $@

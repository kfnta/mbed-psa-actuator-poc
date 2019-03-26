#!/bin/bash -x
cp -r  actuator_control mbed-os/components/TARGET_PSA/services
python mbed-os/tools/psa/generate_partition_code.py
python mbed-os/tools/psa/release.py -m CY8CKIT_062_WIFI_BT_M0_PSA -d

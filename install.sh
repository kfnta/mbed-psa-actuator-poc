#!/bin/bash -x
cp -r  actuator_control mbed-os/components/TARGET_PSA/services
python mbed-os/tools/psa/generate_partition_code.py

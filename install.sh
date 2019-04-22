#!/bin/bash -x
rm -rf BUILD
rm -rf mbed-os/BUILD
cp -r  actuator_control mbed-os/components/TARGET_PSA/services
python mbed-os/tools/psa/generate_partition_code.py
python mbed-os/tools/psa/release.py -m CY8CKIT_062_WIFI_BT_M0_PSA -d --skip-tests -x --app-config mbed_app.json
if [ $? -ne 0 ]
then
    mbed compile -m CY8CKIT_062_WIFI_BT_M0_PSA -t GCC_ARM --profile debug --artifact-name psa_release_1.0 --app-config mbed_app.json -v
fi

/* Copyright (c) 2017-2019 ARM Limited
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/***********************************************************************************************************************
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 * THIS FILE IS AN AUTO-GENERATED FILE - DO NOT MODIFY IT.
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 * Template Version 1.0
 * Generated by tools/spm/generate_partition_code.py Version 1.0
 **********************************************************************************************************************/

#include "cmsis.h"
#include "mbed_toolchain.h" /* For using MBED_ALIGN macro */
#include "rtx_os.h"
#include "spm_panic.h"
#include "spm_internal.h"
#include "psa_actuator_control_srv_partition.h"
#include "psa_manifest/sid.h"


/* Threads stacks */
MBED_ALIGN(8) uint8_t actuator_control_srv_thread_stack[1024] = {0};

/* Threads control blocks */
osRtxThread_t actuator_control_srv_thread_cb = {0};

/* Thread attributes - for thread initialization */
osThreadAttr_t actuator_control_srv_thread_attr = {
    .name = "actuator_control_srv",
    .attr_bits = 0,
    .cb_mem = &actuator_control_srv_thread_cb,
    .cb_size = sizeof(actuator_control_srv_thread_cb),
    .stack_mem = actuator_control_srv_thread_stack,
    .stack_size = 1024,
    .priority = osPriorityNormal,
    .tz_module = 0,
    .reserved = 0
};

spm_rot_service_t actuator_control_srv_rot_services[ACTUATOR_CONTROL_SRV_ROT_SRV_COUNT] = {
    {
        .sid = ACTUATOR_CONTROL_PROCESS_PACKET,
        .mask = ACTUATOR_CONTROL_PROCESS_PACKET_MSK,
        .partition = NULL,
        .min_version = 1,
        .min_version_policy = PSA_MINOR_VERSION_POLICY_RELAXED,
        .allow_nspe = true,
        .queue = {
            .head = NULL,
            .tail = NULL
        }
    },
};


static osRtxMutex_t actuator_control_srv_mutex = {0};
static const osMutexAttr_t actuator_control_srv_mutex_attr = {
    .name = "actuator_control_srv_mutex",
    .attr_bits = osMutexRecursive | osMutexPrioInherit | osMutexRobust,
    .cb_mem = &actuator_control_srv_mutex,
    .cb_size = sizeof(actuator_control_srv_mutex),
};


extern void actuator_control_entry_point(void *ptr);

void actuator_control_srv_init(spm_partition_t *partition)
{
    if (NULL == partition) {
        SPM_PANIC("partition is NULL!\n");
    }

    partition->mutex = osMutexNew(&actuator_control_srv_mutex_attr);
    if (NULL == partition->mutex) {
        SPM_PANIC("Failed to create mutex for secure partition actuator_control_srv!\n");
    }

    for (uint32_t i = 0; i < ACTUATOR_CONTROL_SRV_ROT_SRV_COUNT; ++i) {
        actuator_control_srv_rot_services[i].partition = partition;
    }
    partition->rot_services = actuator_control_srv_rot_services;

    partition->thread_id = osThreadNew(actuator_control_entry_point, NULL, &actuator_control_srv_thread_attr);
    if (NULL == partition->thread_id) {
        SPM_PANIC("Failed to create start main thread of partition actuator_control_srv!\n");
    }
}

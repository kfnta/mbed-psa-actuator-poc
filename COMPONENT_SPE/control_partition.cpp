/* Copyright (c) 2019 ARM Limited
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

#include "psa_august_control_srv_partition.h"
#include "psa/service.h"
#include "mbed.h"

#if !defined(PACKET_SIZE)
#define PACKET_SIZE 512
#endif

using namespace mbed;

/* Mail */
typedef struct {
    uint8_t packet[PACKET_SIZE];
    size_t packet_size;
} mail_t;

Mail<mail_t, 4> mail_box;

typedef psa_status_t (*SignalHandler)(psa_msg_t *);

void august_business_logic(void)
{
    osEvent evt = mail_box.get();
        if (evt.status == osEventMail) {
            mail_t *mail = (mail_t*)evt.value.p;
            // TODO handle packet
        }
}


static psa_status_t august_process_packet(psa_msg_t *msg)
{

    if (msg->in_size[0] <= sizeof(PACKET_SIZE)) {
        return PSA_DROP_CONNECTION;
    }
    
    mail_t *mail = mail_box.alloc();

    if (NULL == mail) {
        return PSA_ERROR_CONNECTION_BUSY;
    }

    mail->packet_size = msg->in_size[0];

    psa_read(msg->handle, 0, mail->packet, PACKET_SIZE);
    
    mail_box.put(mail);
    
    return PSA_SUCCESS;
}

static void message_handler(psa_msg_t *msg, SignalHandler handler)
{
    psa_status_t status = PSA_SUCCESS;
    switch (msg->type) {
        case PSA_IPC_CONNECT: //fallthrough
        case PSA_IPC_DISCONNECT: {
            break;
        }
        case PSA_IPC_CALL: //fallthrough
        default: {
            status = handler(msg);
            break;
        }
    }
    psa_reply(msg->handle, status);
}

extern "C" void august_control_entry_point(void *ptr)
{
    psa_signal_t asserted_signals = 0;
    psa_msg_t msg = {0};
    
    Thread thread;
    thread.start(callback(august_business_logic)); //spin-out business logic thread

    while (1) {
        asserted_signals = psa_wait(AUGUST_CONTROL_SRV_WAIT_ANY_SID_MSK, PSA_BLOCK);
        if ((asserted_signals & AUGUST_CONTROL_PROCESS_PACKET_MSK) != 0) {
            if (PSA_SUCCESS != psa_get(AUGUST_CONTROL_PROCESS_PACKET_MSK, &msg)) {
                continue;
            }
            message_handler(&msg, august_process_packet);
        }
    }
}

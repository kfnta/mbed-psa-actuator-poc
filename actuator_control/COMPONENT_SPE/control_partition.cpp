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

#include "psa_actuator_control_srv_partition.h"
#include "psa/crypto.h"
#include "psa/service.h"
#include "mbed.h"

using namespace mbed;

/* Mail */
typedef struct {
    uint8_t ciphertext[45];
    size_t ciphertext_size;
    uint8_t additional_data[32];
    size_t additional_data_size;
    uint8_t nonce[16];
    size_t nonce_size;

} mail_t;

Mail<mail_t, 4> mail_box;
const psa_key_id_t g_key_id = 100500;


typedef psa_status_t (*SignalHandler)(psa_msg_t *);

void lazy_init()
{
    static int is_initialized = 0;

    if (is_initialized) {
        return;
    }

    if (PSA_SUCCESS != psa_crypto_init()) {  //make sure crypto partition is initialized
        error("Cannot initialize crypto");
    }

    // provisioning use case
    {
        psa_key_handle_t key_handle;

        psa_status_t status = psa_create_key(PSA_KEY_LIFETIME_PERSISTENT, g_key_id, &key_handle);
        if (PSA_SUCCESS != status) {
            if (PSA_ERROR_ALREADY_EXISTS != status) { // already provisioned
                error("Failed to provision a key");
            }
        }

        psa_key_policy_t policy = psa_key_policy_init();
        psa_key_policy_set_usage(&policy, PSA_KEY_USAGE_DECRYPT, PSA_ALG_GCM);
        psa_set_key_policy(key_handle, &policy);

        uint8_t key_data[] = {
            0x3d, 0xe0, 0x98, 0x74, 0xb3, 0x88, 0xe6, 0x49,
            0x19, 0x88, 0xd0, 0xc3, 0x60, 0x7e, 0xae, 0x1f
        };

        psa_import_key(key_handle, PSA_KEY_TYPE_AES, key_data, sizeof(key_data));
        psa_close_key(key_handle);
    }

    is_initialized = 1;
}

extern "C" void actuator_business_logic(void *ptr)
{
    psa_key_handle_t key_handle;
    lazy_init();
    psa_status_t status = psa_open_key(PSA_KEY_LIFETIME_PERSISTENT, g_key_id, &key_handle);
    if (PSA_SUCCESS != status) {
        error("can't load key");
    }
    while(1){
        osEvent evt = mail_box.get();
        if (evt.status == osEventMail) {
            mail_t *mail = (mail_t*)evt.value.p;

            uint8_t plaintext[30] = { 0 };
            size_t plaintext_actual_size = 0;
            status = psa_aead_decrypt(
                key_handle,
                PSA_ALG_GCM,
                mail->nonce, mail->nonce_size,
                mail->additional_data, mail->additional_data_size,
                mail->ciphertext, mail->ciphertext_size,
                plaintext, sizeof(plaintext),
                &plaintext_actual_size
            );

            mail_box.free(mail);

            if (PSA_SUCCESS != status){
                continue;
            }

            // analyze plaintext and action upon it
        }
    }
}

#define member_size(type, member) sizeof(((type *)0)->member)

static psa_status_t actuator_process_packet(psa_msg_t *msg)
{

    if ((msg->in_size[0] == 0) || (msg->in_size[0] > member_size(mail_t, ciphertext))      ||
        (msg->in_size[1] == 0) || (msg->in_size[1] > member_size(mail_t, additional_data)) ||
        (msg->in_size[2] == 0) || (msg->in_size[2] > member_size(mail_t, nonce))
    ) {
        return PSA_DROP_CONNECTION;
    }

    mail_t *mail = mail_box.alloc();

    if (NULL == mail) {
        return PSA_ERROR_CONNECTION_BUSY;
    }

    mail->ciphertext_size = psa_read(msg->handle, 0, mail->ciphertext, sizeof(mail->ciphertext));
    mail->additional_data_size = psa_read(msg->handle, 1, mail->additional_data, sizeof(mail->additional_data));
    mail->nonce_size = psa_read(msg->handle, 2, mail->nonce, sizeof(mail->nonce));

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
            //our operation is context-less
        }
        case PSA_IPC_CALL: //fallthrough
        default: {
            status = handler(msg);
            break;
        }
    }
    psa_reply(msg->handle, status);
}



extern "C" void actuator_control_entry_point(void *ptr)
{
    psa_signal_t asserted_signals = 0;
    psa_msg_t msg = {0};

    while (1) {
        asserted_signals = psa_wait(ACTUATOR_CONTROL_SRV_WAIT_ANY_SID_MSK, PSA_BLOCK);

        if ((asserted_signals & ACTUATOR_CONTROL_PROCESS_PACKET_MSK) != 0) {
            if (PSA_SUCCESS != psa_get(ACTUATOR_CONTROL_PROCESS_PACKET_MSK, &msg)) {
                continue;
            }
            message_handler(&msg, actuator_process_packet);
        }
    }
}

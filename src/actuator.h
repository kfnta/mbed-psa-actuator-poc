#ifndef __ACTUATOR_H__
#define __ACTUATOR_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "psa/error.h"


psa_status_t actuator_process_packet(
    const uint8_t *ciphertext_packet, size_t ciphertext_packet_size,
    const uint8_t *additional_data, size_t additional_data_size,
    const uint8_t *nonce, size_t nonce_size
);


#ifdef __cplusplus
}
#endif




#endif  // __ACTUATOR_H__





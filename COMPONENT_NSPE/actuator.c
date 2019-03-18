#include "actuator.h"
#include "psa/client.h"
#include "psa_manifest/sid.h"


psa_status_t actuator_process_packet(
    const uint8_t *ciphertext_packet, size_t ciphertext_packet_size,
    const uint8_t *additional_data, size_t additional_data_size,
    const uint8_t *nonce, size_t nonce_size
)
{
    psa_handle_t conn = psa_connect(ACTUATOR_CONTROL_PROCESS_PACKET, 1);
    if (conn <= PSA_NULL_HANDLE) {
        return (psa_status_t)conn;
    }

    psa_invec request[3] = {
        {ciphertext_packet, ciphertext_packet_size},
        {additional_data, additional_data_size},
        {nonce, nonce_size}
    };

    psa_status_t status = psa_call(
        conn, 
        request, sizeof(request)/sizeof(request[0]), 
        NULL, 0
    );

    psa_close(conn);

    return status;
}
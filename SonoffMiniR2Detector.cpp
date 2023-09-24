#include "SonoffMiniR2Detector.h"
extern "C" {
    #include "tcpclient.h"
}

#include <stdexcept>
#include "pico/stdlib.h"

const uint16_t port = 8081;
const std::string request = "POST /zeroconf/info HTTP/1.1\r\nContent-Length: 25\r\nConnection: close\r\n\r\n{\"deviceid\":\"\",\"data\":{}}";

SonoffMiniR2Detector::SonoffMiniR2Detector(const std::string& ipAddress) :
    m_ipAddress(ipAddress)
{}

bool SonoffMiniR2Detector::Detect(uint8_t timeout) {
        TCP_CLIENT_T *state = tcp_client_init(m_ipAddress.c_str());
        if (!state) {
            throw std::runtime_error("Failed to initialize TCP client");
        }

        if (!tcp_client_open(state, port, timeout)) {
            free(state);
            throw std::runtime_error("Failed to initiate connection");
        }

        while(!state->complete) {
            sleep_ms(500);
        }

        free(state);
        return state->connected;
}
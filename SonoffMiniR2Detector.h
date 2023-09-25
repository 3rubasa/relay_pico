#pragma once

#include <string> 

class SonoffMiniR2Detector {
public:
    SonoffMiniR2Detector(const std::string& ipAddress);
    bool Detect(uint8_t timeout);
private:
    const std::string m_ipAddress;
    bool m_lastState;
};
#pragma once

#include "lwip/apps/httpd.h"
#include "pico/cyw43_arch.h"
#include "hardware/adc.h"
#include <memory>

u16_t ssi_handler(int iIndex, char *pcInsert, int iInsertLen, u16_t part, u16_t* next_part);
// Initialise the SSI handler
void ssi_init(bool* state);
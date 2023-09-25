#include "ssi.h"

#include "lwip/apps/httpd.h"
#include "pico/cyw43_arch.h"
#include "hardware/adc.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

bool* g_pState = NULL;
std::string g_output;

// SSI tags - tag length limited to 8 bytes by default
const char * ssi_tags[] = {"output"};

u16_t ssi_handler(int iIndex, char *pcInsert, int iInsertLen, u16_t part, u16_t* next_part) {
    //printf("Handler called\n");
    //printf("part=%d\n", part);
    //printf("nextpart=%d\n", *next_part);

    if (part == 0) {
        g_output = "{\"state\":";
        if (*g_pState) {
            g_output += "true";
        } else {
            g_output += "false";
        }
        g_output += "}";
    }
    
  char toPrint[192];

  if (g_output.length() > 191+part*191) {
    *next_part = part+1;
    g_output.copy(toPrint, 191, part*191);
    toPrint[191]=0;
  } else {
    g_output.copy(toPrint, g_output.length()-part*191, part*191);
    toPrint[g_output.length()-part*191] =0;
  }

  size_t printed = snprintf(pcInsert, iInsertLen, toPrint);
  
  //printf("IsertLen=%d\n", iInsertLen);
  return (u16_t)printed;
}

// Initialise the SSI handler
void ssi_init(bool* state) {
    g_pState=state;
    http_set_ssi_handler(ssi_handler, ssi_tags, LWIP_ARRAYSIZE(ssi_tags));
}
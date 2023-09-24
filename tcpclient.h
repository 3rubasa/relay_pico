
#ifndef TCP_CLIENT_H_INCLUDED
#define TCP_CLIENT_H_INCLUDED

#include "lwip/pbuf.h"
#include "lwip/tcp.h"

typedef struct TCP_CLIENT_T_ {
    struct tcp_pcb *tcp_pcb;
    ip_addr_t remote_addr;
    bool complete;
    bool connected;
    bool timeout;
    bool error;
} TCP_CLIENT_T;


TCP_CLIENT_T* tcp_client_init(const char* ip_address);
bool tcp_client_open(void *arg, u16_t port, u8_t timeout);

err_t tcp_client_close(void *arg);
err_t tcp_result(void *arg);

err_t tcp_client_connected(void *arg, struct tcp_pcb *tpcb, err_t err);
err_t tcp_client_poll(void *arg, struct tcp_pcb *tpcb);
void tcp_client_err(void *arg, err_t err);

#endif
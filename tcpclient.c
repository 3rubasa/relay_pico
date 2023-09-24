#include "tcpclient.h"

#include <string.h>
#include <time.h>

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

TCP_CLIENT_T* tcp_client_init(const char* ip_address) {
    TCP_CLIENT_T *state = calloc(1, sizeof(TCP_CLIENT_T));
    if (!state) {
        printf("failed to allocate state\n");
        return NULL;
    }

    state->connected = false;
    state->complete = false;
    state->error = false;
    state->timeout = false;

    ip4addr_aton(ip_address, &state->remote_addr);
    return state;
}

bool tcp_client_open(void *arg, u16_t port, u8_t timeout) {
    TCP_CLIENT_T *state = (TCP_CLIENT_T*)arg;
    state->tcp_pcb = tcp_new_ip_type(IP_GET_TYPE(&state->remote_addr));
    if (!state->tcp_pcb) {
        printf("failed to create pcb\n");
        return false;
    }

    tcp_arg(state->tcp_pcb, state);
    tcp_poll(state->tcp_pcb, tcp_client_poll, timeout * 2);
    tcp_sent(state->tcp_pcb, NULL);
    tcp_recv(state->tcp_pcb, NULL);
    tcp_err(state->tcp_pcb, tcp_client_err);

    // cyw43_arch_lwip_begin/end should be used around calls into lwIP to ensure correct locking.
    // You can omit them if you are in a callback from lwIP. Note that when using pico_cyw_arch_poll
    // these calls are a no-op and can be omitted, but it is a good practice to use them in
    // case you switch the cyw43_arch type later.
    cyw43_arch_lwip_begin();
    err_t err = tcp_connect(state->tcp_pcb, &state->remote_addr, port, tcp_client_connected);
    cyw43_arch_lwip_end();

    return err == ERR_OK;
}

err_t tcp_result(void *arg) {
    TCP_CLIENT_T *state = (TCP_CLIENT_T*)arg;
    err_t e = tcp_client_close(arg);
    state->complete = true;
    return e;
}

err_t tcp_client_close(void *arg) {
    TCP_CLIENT_T *state = (TCP_CLIENT_T*)arg;
    err_t err = ERR_OK;
    if (state->tcp_pcb != NULL) {
        tcp_arg(state->tcp_pcb, NULL);
        tcp_poll(state->tcp_pcb, NULL, 0);
        tcp_sent(state->tcp_pcb, NULL);
        tcp_recv(state->tcp_pcb, NULL);
        tcp_err(state->tcp_pcb, NULL);
        err = tcp_close(state->tcp_pcb);
        if (err != ERR_OK) {
            printf("close failed %d, calling abort\n", err);
            tcp_abort(state->tcp_pcb);
            err = ERR_ABRT;
        }
        state->tcp_pcb = NULL;
    }
    return err;
}

err_t tcp_client_connected(void *arg, struct tcp_pcb *tpcb, err_t err) {
    TCP_CLIENT_T *state = (TCP_CLIENT_T*)arg;
    if (err != ERR_OK) {
        printf("connect failed %d\n", err);
        state->error = true;
        return tcp_result(arg);
    }
    state->connected = true;

    return tcp_result(arg);
}

err_t tcp_client_poll(void *arg, struct tcp_pcb *tpcb) {
    TCP_CLIENT_T *state = (TCP_CLIENT_T*)arg;
    state->timeout=true;
    err_t res = tcp_result(arg);
    return res;
}

void tcp_client_err(void *arg, err_t err) {
    if (err != ERR_ABRT) {
        TCP_CLIENT_T *state = (TCP_CLIENT_T*)arg;
        state->error = true;
        state->complete = true;
    }
}
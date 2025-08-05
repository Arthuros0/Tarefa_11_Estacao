#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <stdio.h>
#include "pico/stdlib.h"

#include "pico/cyw43_arch.h"       
#include "lwip/pbuf.h"           
#include "lwip/tcp.h"            
#include "lwip/netif.h" 

#include "config/wifi_config.h"

#define LED_PIN CYW43_WL_GPIO_LED_PIN



void user_request(char *html, size_t html_size);
static err_t tcp_server_accept(void *arg, struct tcp_pcb *newpcb, err_t err);
err_t tcp_server_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
void server_init(void);
bool handle_http_request(const char *request, size_t request_size, char *response, size_t response_size);

#endif
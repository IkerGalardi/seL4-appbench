#include <stdbool.h>
#include <stdint.h>
#include <os/sddf.h>
#include <sddf/serial/queue.h>
#include <sddf/serial/config.h>
#include <sddf/network/queue.h>
#include <sddf/network/config.h>
#include <sddf/network/lib_sddf_lwip.h>
#include <sddf/timer/client.h>
#include <sddf/timer/config.h>
#include <lwip/pbuf.h>
#include <lwip/apps/httpd.h>

__attribute__((__section__(".serial_client_config")))
serial_client_config_t serial_config;

__attribute__((__section__(".net_client_config")))
net_client_config_t net_config;

__attribute__((__section__(".lib_sddf_lwip_config")))
lib_sddf_lwip_config_t lwip_config;

__attribute__((__section__(".timer_client_config")))
timer_client_config_t timer_config;

serial_queue_handle_t serial_tx_queue_handle;

net_queue_handle_t net_rx_handle;
net_queue_handle_t net_tx_handle;

#define LWIP_TICK_MS 100

struct pbuf *head;
struct pbuf *tail;

char *strchr(const char *s, int c)
{
	while (*s != '\0') {
		if (*s == c)
			return (char *)s;
		s++;
	}
	return 0;
}

// Linear Congruential Generator
int rand(void) {
    static int next = 389029785;

    next = next * 1103515245 + 12345;

    return (unsigned int)(next / 65536) % 32768;
}

static void netif_status_callback(char *ip_addr)
{
    sddf_printf("ECHOSERVER: DHCP request finished, got IP %s\n", ip_addr);
}

static void set_timeout(void)
{
    sddf_timer_set_timeout(timer_config.driver_id, LWIP_TICK_MS * NS_IN_MS);
}

void init()
{
    assert(serial_config_check_magic(&serial_config));
    assert(net_config_check_magic(&net_config));
    assert(timer_config_check_magic(&timer_config));

    serial_queue_init(&serial_tx_queue_handle,
                      serial_config.tx.queue.vaddr,
                      serial_config.tx.data.size,
                      serial_config.tx.data.vaddr);
    serial_putchar_init(serial_config.tx.id, &serial_tx_queue_handle);

    net_queue_init(&net_rx_handle, net_config.rx.free_queue.vaddr, net_config.rx.active_queue.vaddr,
                   net_config.rx.num_buffers);
    net_queue_init(&net_tx_handle, net_config.tx.free_queue.vaddr, net_config.tx.active_queue.vaddr,
                   net_config.tx.num_buffers);
    net_buffers_init(&net_tx_handle, 0);

    sddf_lwip_init(&lwip_config, &net_config, &timer_config, net_rx_handle, net_tx_handle, NULL,
                   NULL, netif_status_callback, NULL, NULL, NULL);
    set_timeout();

    tcpecho_raw_init(80);

    sddf_lwip_maybe_notify();

    sddf_printf("ECHOSERVER: initialized\n");

}

void notified(microkit_channel channel)
{
    if (channel == net_config.rx.id) {
        sddf_lwip_process_rx();
    } else if (channel == timer_config.driver_id) {
        sddf_lwip_process_timeout();
        set_timeout();
    }

    sddf_lwip_maybe_notify();
}

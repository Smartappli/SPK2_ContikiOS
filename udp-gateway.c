#include "contiki.h"
//#include "contiki-lib.h"
//#include "contiki-net.h"
#include "os/net/netstack.h"
#include "os/net/routing/routing.h"
#include "os/net/ipv6/simple-udp.h"

#include "os/sys/log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include "cfs/cfs.h"
#include "parameters.h"
#include "HNF.c"
#include <ctype.h>

//#define DEBUG DEBUG_PRINT
//#include "os/net/ipv6/uip-debug.h"

// a voir #define UIP_IP_BUF   ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])

//#define UIP_LLH_LEN 0
/*---------------------------------------------------------------------------*/
PROCESS(udp_gateway_process, "UDP gateway process");
AUTOSTART_PROCESSES(&udp_gateway_process);
/*---------------------------------------------------------------------------*/

static struct simple_udp_connection udp_conn;
static struct lattice public_lattice;


void create_lattice(){

    public_lattice.metadata.dim_n = VECTOR_SIZE;
    public_lattice.metadata.dim_m = NBR_VECTORS;
    public_lattice.metadata.max = MODULO_LATTICE;

    int i;
    for (i = 0; i<public_lattice.metadata.dim_n; i++) {
        public_lattice.vectors[i] = rand()%public_lattice.metadata.max;
        //for (j = 0; j < public_lattice.dim_m; j++) {
            //public_lattice.vectors[i][j] = rand()%public_lattice.max;
        //}
    }

    printf("Lattice created\n");

    for (i = 0; i < public_lattice.metadata.dim_n; i++){
        printf("%d ", public_lattice.vectors[i]);
	if((i+1)%30==0)
	   printf("\n");
        //for (j = 0; j < public_lattice.dim_m; j++) {
        //    printf("%d ", public_lattice.vectors[i][j]);
        //}
    }
    printf("\n");
}
/*---------------------------------------------------------------------------*/

void get_lattice(){
    if(public_lattice.metadata.max==(int) NULL) {
	create_lattice();
    }
}
/*---------------------------------------------------------------------------*/
static void
tcpip_handler(struct simple_udp_connection *c,
         const uip_ipaddr_t *sender_addr,
         uint16_t sender_port,
         const uip_ipaddr_t *receiver_addr,
         uint16_t receiver_port,
         const void *data,
         uint16_t datalen)
{
  char *appdata;

  if(data) {
    appdata = (char*)data;
    appdata[uip_datalen()] = 0;
    int request_id = strtol(appdata, &appdata, 10);
    printf("Request received : %d\n", request_id);

    if(request_id == 0) {

		printf("Request for new Lattice received from ");
	    	printf("%d",
		   UIP_IP_BUF->srcipaddr.u8[sizeof(UIP_IP_BUF->srcipaddr.u8) - 1]);
	    	printf("\n");

		get_lattice();

		printf("Sending...\n");
     }

     struct udp_lattice temp_lattice;
     temp_lattice.metadata = public_lattice.metadata;
     if(request_id >= VECTOR_SIZE/DATA_PER_PACKET){ 
	     temp_lattice.metadata.id = -1;
	     printf("Sending last element '%d' to ", request_id);
             printf("%d", UIP_IP_BUF->srcipaddr.u8[sizeof(UIP_IP_BUF->srcipaddr.u8) - 1]);
	     printf("\n");
	     memcpy(temp_lattice.vectors, &public_lattice.vectors[request_id*DATA_PER_PACKET], (VECTOR_SIZE-request_id*DATA_PER_PACKET)*sizeof(*public_lattice.vectors)); //The last elements are never instanciated but never read so its nothing I believe
     } else {
	     temp_lattice.metadata.id = request_id;
	     printf("Sending element '%d' to ", request_id);
             printf("%d", UIP_IP_BUF->srcipaddr.u8[sizeof(UIP_IP_BUF->srcipaddr.u8) - 1]);
	     printf("\n");
	     memcpy(temp_lattice.vectors, &public_lattice.vectors[(request_id*DATA_PER_PACKET)%VECTOR_SIZE], DATA_PER_PACKET*sizeof(*public_lattice.vectors));
     }
     //uip_ipaddr_copy(&server_conn->ripaddr, &UIP_IP_BUF->srcipaddr);
     //uip_udp_packet_send(server_conn, &temp_lattice, sizeof(temp_lattice));
     //uip_create_unspecified(&server_conn->ripaddr);
     simple_udp_sendto(&udp_conn, &temp_lattice, sizeof(temp_lattice), sender_addr);
  }
}

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_gateway_process, ev, data)
{
  //uip_ipaddr_t ipaddr;
  //struct uip_ds6_addr *root_if;

  PROCESS_BEGIN();

  //PROCESS_PAUSE();

  //SENSORS_ACTIVATE(button_sensor);

  printf("UDP gateway started\n");

  /* Initialize DAG root */
  NETSTACK_ROUTING.root_start();

  /* Initialize UDP connection */
  simple_udp_register(&udp_conn, UDP_SERVER_PORT, NULL, UDP_CLIENT_PORT, tcpip_handler);


  while(1) {
    PROCESS_YIELD();
    //if(ev == tcpip_event) {
      //udp_rx_callback();
    //}
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/

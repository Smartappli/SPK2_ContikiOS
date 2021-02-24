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
#include <ctype.h>
//#include "cfs/cfs.h"
#include "parameters.h"

#define DEBUG DEBUG_PRINT
#include "os/net/ipv6/uip-debug.h"

// a voir #define UIP_IP_BUF   ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])
#define UIP_LLH_LEN 0
static struct simple_udp_connection udp_conn;
static struct lattice public_lattice;

/*---------------------------------------------------------------------------*/
PROCESS(udp_gateway_process, "UDP gateway process");
AUTOSTART_PROCESSES(&udp_gateway_process);
/*---------------------------------------------------------------------------*/

void create_lattice(){
    public_lattice.metadata.dim_n = VECTOR_SIZE;
    public_lattice.metadata.dim_m = NBR_VECTORS;
    public_lattice.metadata.max = MODULO_LATTICE;
    int i;

    for (i = 0; i<public_lattice.metadata.dim_n; i++) {
        public_lattice.vectors[i] = rand()%public_lattice.metadata.max;
    }//end if
}//end create_lattice()

/*---------------------------------------------------------------------------*/

void get_lattice(){
    if(public_lattice.metadata.max==(int) NULL) {
	create_lattice();
    }//end if
}//end get_lattice()

/*---------------------------------------------------------------------------*/

void create_polynome(long input[]){
    long i;
    for(i = 0; i < VECTOR_SIZE; i++) {
	input[i]=rand()%MODULO_LATTICE;
    }//end for
}//end create_polynome()

/*---------------------------------------------------------------------------*/

void key_generation(const long public_key_1[], long private_key[], long public_key_2[]){
    long r[VECTOR_SIZE];
    unsigned long i;

    create_polynome(r);
    create_polynome(private_key);

    for(i = 0; i < N; i++){
	public_key_1[i] = (r[i] - public_key_1[i]*private_key[i])%MODULO_LATTICE
    }//end for
}//end key_generation()

/*---------------------------------------------------------------------------*/

void encode(long input[]){
    long i;
    for(i = 0; i < VECTOR_SIZE; i++) {
	input[i]=rand()%MODULO_LATTICE;
    }//end for
}//end create_polynome()

/*---------------------------------------------------------------------------*/

static void tcpip_handler(struct simple_udp_connection *c,
         const uip_ipaddr_t *sender_addr,
         uint16_t sender_port,
         const uip_ipaddr_t *receiver_addr,
         uint16_t receiver_port,
         const void *data,
         uint16_t datalen){

  char *appdata;

  if(data) {
    appdata = (char*)data;
    appdata[uip_datalen()] = 0;
    int request_id = strtol(appdata, &appdata, 10);
    PRINTF("Request received : %d\n", request_id);

    if(request_id == 0) {

		PRINTF("Request for new Lattice received from ");
	    	PRINTF("%d",
		   UIP_IP_BUF->srcipaddr.u8[sizeof(UIP_IP_BUF->srcipaddr.u8) - 1]);
	    	PRINTF("\n");
		get_lattice();
     }//end if

     struct udp_lattice temp_lattice;
     temp_lattice.metadata = public_lattice.metadata;
     if(request_id >= VECTOR_SIZE/DATA_PER_PACKET){ 
	     temp_lattice.metadata.id = -1;
	     PRINTF("Sending last element '%d' to ", request_id);
             PRINTF("%d", UIP_IP_BUF->srcipaddr.u8[sizeof(UIP_IP_BUF->srcipaddr.u8) - 1]);
	     PRINTF("\n");
	     memcpy(temp_lattice.vectors, &public_lattice.vectors[request_id*DATA_PER_PACKET], (VECTOR_SIZE-request_id*DATA_PER_PACKET)*sizeof(*public_lattice.vectors)); //The last elements are never instanciated but never read so its nothing I believe
     } else {
	     temp_lattice.metadata.id = request_id;
	     PRINTF("Sending element '%d' to ", request_id);
             PRINTF("%d", UIP_IP_BUF->srcipaddr.u8[sizeof(UIP_IP_BUF->srcipaddr.u8) - 1]);
	     PRINTF("\n");
	     memcpy(temp_lattice.vectors, &public_lattice.vectors[(request_id*DATA_PER_PACKET)%VECTOR_SIZE], DATA_PER_PACKET*sizeof(*public_lattice.vectors));
     }//end ifelse
     simple_udp_sendto(&udp_conn, &temp_lattice, sizeof(temp_lattice), sender_addr);
  }//end if
}//end tcpip_handler()

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_gateway_process, ev, data)
{
  PROCESS_BEGIN();
  PRINTF("UDP gateway started\n");

  /* Initialize DAG root */
  NETSTACK_ROUTING.root_start();

  /* Initialize UDP connection */
  simple_udp_register(&udp_conn, UDP_SERVER_PORT, NULL, UDP_CLIENT_PORT, tcpip_handler);


  while(1) {
    PROCESS_YIELD();
  }//end while
  PROCESS_END();
}//end main()


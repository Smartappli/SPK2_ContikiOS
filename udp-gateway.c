#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"
#include "net/ip/uip.h"
#include "net/rpl/rpl.h"

#include "net/netstack.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "cfs/cfs.h"
#include "parameters.h"

#define DEBUG DEBUG_PRINT
#include "net/ip/uip-debug.h"

#define UIP_IP_BUF   ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])

static struct uip_udp_conn *server_conn;
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
        //for (j = 0; j < public_lattice.dim_m; j++) {
            //public_lattice.vectors[i][j] = rand()%public_lattice.max;
        //}
    }

    PRINTF("Lattice created\n");

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
tcpip_handler(void)
{
  char *appdata;

  if(uip_newdata()) {
    appdata = (char*)uip_appdata;
    appdata[uip_datalen()] = 0;
    int request_id = strtol(appdata, &appdata, 10);
    //PRINTF("Request received : %d\n", request_id);

    if(request_id == 0) {

		PRINTF("Request for new Lattice received from ");
	    	PRINTF("%d",
		   UIP_IP_BUF->srcipaddr.u8[sizeof(UIP_IP_BUF->srcipaddr.u8) - 1]);
	    	PRINTF("\n");

		get_lattice();

		PRINTF("Sending...\n");
     }

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
	     memcpy(temp_lattice.vectors, &public_lattice.vectors[request_id*DATA_PER_PACKET], DATA_PER_PACKET*sizeof(*public_lattice.vectors));
     }
     uip_ipaddr_copy(&server_conn->ripaddr, &UIP_IP_BUF->srcipaddr);
     uip_udp_packet_send(server_conn, &temp_lattice, sizeof(temp_lattice));
     uip_create_unspecified(&server_conn->ripaddr);
  }
}

/*          appdata = (char *)uip_appdata;
	    appdata[uip_datalen()] = 0;
	    PRINTF("DATA recv '%s' from ", appdata);
	    PRINTF("%d",
		   UIP_IP_BUF->srcipaddr.u8[sizeof(UIP_IP_BUF->srcipaddr.u8) - 1]);
	    PRINTF("\n");
	    PRINTF("DATA sending reply\n");
	    uip_ipaddr_copy(&server_conn->ripaddr, &UIP_IP_BUF->srcipaddr);
	    uip_udp_packet_send(server_conn, "Reply", sizeof("Reply"));
	    uip_create_unspecified(&server_conn->ripaddr);
	    get_lattice(VECTOR_SIZE,VECTOR_SIZE,50);*/


/*---------------------------------------------------------------------------*/
static void
print_local_addresses(void)
{
  int i;
  uint8_t state;

  PRINTF("Server IPv6 addresses: ");
  for(i = 0; i < UIP_DS6_ADDR_NB; i++) {
    state = uip_ds6_if.addr_list[i].state;
    if(state == ADDR_TENTATIVE || state == ADDR_PREFERRED) {
      PRINT6ADDR(&uip_ds6_if.addr_list[i].ipaddr);
      PRINTF("\n");
      /* hack to make address "final" */
      if (state == ADDR_TENTATIVE) {
	uip_ds6_if.addr_list[i].state = ADDR_PREFERRED;
      }
    }
  }
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_gateway_process, ev, data)
{
  uip_ipaddr_t ipaddr;
  struct uip_ds6_addr *root_if;

  PROCESS_BEGIN();

  PROCESS_PAUSE();

  //SENSORS_ACTIVATE(button_sensor);

  PRINTF("UDP gateway started\n");

#if UIP_CONF_ROUTER
/* The choice of server address determines its 6LoPAN header compression.
 * Obviously the choice made here must also be selected in udp-client.c.
 *
 * For correct Wireshark decoding using a sniffer, add the /64 prefix to the 6LowPAN protocol preferences,
 * e.g. set Context 0 to aaaa::.  At present Wireshark copies Context/128 and then overwrites it.
 * (Setting Context 0 to aaaa::1111:2222:3333:4444 will report a 16 bit compressed address of aaaa::1111:22ff:fe33:xxxx)
 * Note Wireshark's IPCMV6 checksum verification depends on the correct uncompressed addresses.
 */
 
#if 0
/* Mode 1 - 64 bits inline */
   uip_ip6addr(&ipaddr, 0xaaaa, 0, 0, 0, 0, 0, 0, 1);
#elif 1
/* Mode 2 - 16 bits inline */
  uip_ip6addr(&ipaddr, 0xaaaa, 0, 0, 0, 0, 0x00ff, 0xfe00, 1);
#else
/* Mode 3 - derived from link local (MAC) address */
  uip_ip6addr(&ipaddr, 0xaaaa, 0, 0, 0, 0, 0, 0, 0);
  uip_ds6_set_addr_iid(&ipaddr, &uip_lladdr);
#endif

  uip_ds6_addr_add(&ipaddr, 0, ADDR_MANUAL);
  root_if = uip_ds6_addr_lookup(&ipaddr);
  if(root_if != NULL) {
    rpl_dag_t *dag;
    dag = rpl_set_root(RPL_DEFAULT_INSTANCE,(uip_ip6addr_t *)&ipaddr);
    uip_ip6addr(&ipaddr, 0xaaaa, 0, 0, 0, 0, 0, 0, 0);
    rpl_set_prefix(dag, &ipaddr, 64);
    PRINTF("created a new RPL dag\n");
  } else {
    PRINTF("failed to create a new RPL DAG\n");
  }
#endif /* UIP_CONF_ROUTER */
  
  print_local_addresses();

  /* The data sink runs with a 100% duty cycle in order to ensure high 
     packet reception rates. */
  NETSTACK_MAC.off(1);

  server_conn = udp_new(NULL, UIP_HTONS(UDP_CLIENT_PORT), NULL);
  if(server_conn == NULL) {
    PRINTF("No UDP connection available, exiting the process!\n");
    PROCESS_EXIT();
  }
  udp_bind(server_conn, UIP_HTONS(UDP_SERVER_PORT));

  PRINTF("Created a server connection with remote address ");
  PRINT6ADDR(&server_conn->ripaddr);
  PRINTF(" local/remote port %u/%u\n", UIP_HTONS(server_conn->lport),
         UIP_HTONS(server_conn->rport));

  while(1) {
    PROCESS_YIELD();
    if(ev == tcpip_event) {
      tcpip_handler();
    } //else if (ev == sensors_event && data == &button_sensor) {
      //PRINTF("Initiaing global repair\n");
      //rpl_repair_root(RPL_DEFAULT_INSTANCE);
    //}
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/

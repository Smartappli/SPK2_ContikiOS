#include "contiki.h"
#include "lib/random.h"
#include "sys/ctimer.h"
#include "net/ip/uip.h"
#include "net/ipv6/uip-ds6.h"
#include "net/ip/uip-udp-packet.h"
#include "sys/ctimer.h"
#ifdef WITH_COMPOWER
#include "powertrace.h"
#endif
#include <stdio.h>
#include <string.h>
#include "parameters.h"
#include "gaus_sample.c"

#define DEBUG DEBUG_PRINT
#include "net/ip/uip-debug.h"

#ifndef PERIOD
#define PERIOD 60
#endif

static struct uip_udp_conn *client_conn;
static uip_ipaddr_t server_ipaddr;

static struct lattice public_lattice;
//static struct point public_point;

/*---------------------------------------------------------------------------*/
PROCESS(udp_sensor_process, "UDP sensor process");
AUTOSTART_PROCESSES(&udp_sensor_process);
/*---------------------------------------------------------------------------*/


static void
send_request(uint8_t requestID){

  char buf[8];

  PRINTF("Request send to %d to get part : '%d'\n", server_ipaddr.u8[sizeof(server_ipaddr.u8) - 1], requestID);
  sprintf(buf, "%d", requestID);
  uip_udp_packet_sendto(client_conn, buf, strlen(buf), &server_ipaddr, UIP_HTONS(UDP_SERVER_PORT));
}

/*---------------------------------------------------------------------------*/

static void
one_way_function(uint8_t secret[VECTOR_SIZE]){
    PRINTF("One way function\n");
    int i, j, temp;
    //uint16_t temp_vector[VECTOR_SIZE];
    //memcpy(temp_vector, &public_lattice.vectors[0], VECTOR_SIZE*sizeof(*public_lattice.vectors));
    int32_t output[NBR_VECTORS] = {0};
    for(i = 0; i < VECTOR_SIZE; i++){ // i is the i eme element in the public key we work on
        temp = public_lattice.vectors[i];
        for(j = 0; j < NBR_VECTORS; j++){ // j is the row of the public key we are working on
    	     if((i+j)%VECTOR_SIZE==0 && i!=0)
		temp = (-temp)%public_lattice.metadata.max;
             output[j] = (output[j] + (temp*secret[(i+j)%VECTOR_SIZE]))%public_lattice.metadata.max;
             //PRINTF("%d %d %d %d %d \n", i, j, temp, secret[(i+j)%VECTOR_SIZE], output[j]);
        }
	//PRINTF("\n");
        //output[i] = output[i]%public_lattice.metadata.max;
        //temp_vector[VECTOR_SIZE-pointer-1] = (-temp_vector[VECTOR_SIZE-pointer-1])%public_lattice.metadata.max;

        //for (k = 0; k<VECTOR_SIZE; k++){
	//   printf("%d ", temp_vector[(VECTOR_SIZE-pointer-1+k)%VECTOR_SIZE]);
    	//}
        //PRINTF("\n");
    }
    PRINTF("Le vecteur de sortie :\n");
    for (i = 0; i<NBR_VECTORS; i++){
    	PRINTF("%d ", (int) output[i]);
	if((i+1)%30==0)
	      printf("\n");
    }
    PRINTF("\n");

}

/*---------------------------------------------------------------------------*/

static void
chouse_random_point(){
    PRINTF("Random point from lattice\n");
    int i, j, temp;
    int32_t output[VECTOR_SIZE] = {0};
    int32_t random_vector[NBR_VECTORS] = {0};

    for(i=0;i<NBR_VECTORS;i++)
     random_vector[i] = rand() % public_lattice.metadata.max;

    PRINTF("Le vecteur aleatoire :\n");
    for (i = 0; i<NBR_VECTORS; i++){
    	PRINTF("%d ", (int) random_vector[i]);
	if((i+1)%30==0)
	      printf("\n");
    }
    PRINTF("\n");

    for(i = 0; i < VECTOR_SIZE; i++){ // i is the i eme element in the public key we work on
        temp = public_lattice.vectors[i];
        for(j = 0; j < NBR_VECTORS; j++){ // j is the row of the public key we are working on
    	     if((i+j)%VECTOR_SIZE==0 && i!=0)
		temp = (-temp)%public_lattice.metadata.max;
             output[i] += temp*random_vector[j];
             PRINTF("%d %d %d %d %d \n", i, j, temp, random_vector[j], output[i]);
        }
	//PRINTF("\n");
        //output[i] = output[i]%public_lattice.metadata.max;
        //temp_vector[VECTOR_SIZE-pointer-1] = (-temp_vector[VECTOR_SIZE-pointer-1])%public_lattice.metadata.max;

        //for (k = 0; k<VECTOR_SIZE; k++){
	//   printf("%d ", temp_vector[(VECTOR_SIZE-pointer-1+k)%VECTOR_SIZE]);
    	//}
        //PRINTF("\n");
    }
    PRINTF("Le point de sortie :\n");
    for (i = 0; i<VECTOR_SIZE; i++){
    	PRINTF("%d ", (int) output[i]);
	if((i+1)%30==0)
	      printf("\n");
    }
    PRINTF("\n");
}

/*---------------------------------------------------------------------------*/

static void
tcpip_handler(void)
{
  if(uip_newdata()) {
    //str = uip_appdata;
    //str[uip_datalen()] = '\0';
    //printf(str);
    //PRINTF("\n");
    struct udp_lattice* received_struct_ptr;
    received_struct_ptr = (struct udp_lattice*) uip_appdata;
    struct udp_lattice temp_lattice = *received_struct_ptr;

    public_lattice.metadata = temp_lattice.metadata;
    //public_lattice.vectors = temp_lattice.vectors;


    if(temp_lattice.metadata.id != (typeof(temp_lattice.metadata.id)) -1){

        memcpy(public_lattice.vectors+((public_lattice.metadata.id)*DATA_PER_PACKET), &temp_lattice.vectors[0], DATA_PER_PACKET*sizeof(*temp_lattice.vectors));
        send_request(temp_lattice.metadata.id+1);

    } else {
        memcpy(public_lattice.vectors+(VECTOR_SIZE-VECTOR_SIZE%DATA_PER_PACKET), &temp_lattice.vectors[0], VECTOR_SIZE%DATA_PER_PACKET*sizeof(*temp_lattice.vectors));
	int i;
    	printf("Lattice received :\n");
    	for (i = 0; i<public_lattice.metadata.dim_n; i++){
	   printf("%d ", public_lattice.vectors[i]);
	   if((i+1)%30==0)
	      printf("\n");
	   //for (j = 0; j < public_lattice.dim_m; j++) {
	   //    printf("%d ", public_lattice.vectors[i][j]);
	   //}
    	}
    	printf("\n");
    	printf("Dim_n (number of dimensions) : '%d'\n", public_lattice.metadata.dim_n);
    	printf("Dim_m (number of vectors) : '%d'\n", public_lattice.metadata.dim_m);
    	printf("Max : '%d'\n", public_lattice.metadata.max);
        printf("ID : '%d'\n", public_lattice.metadata.id);
  	
	uint8_t test[VECTOR_SIZE] = {1};
	one_way_function(test);
        chouse_random_point();
    }

    /*
    for (i = 0; i<public_lattice.dim_n; i++){
        public_point.point[i] = 0;
        public_point.vector[i] = rand()%public_lattice.max;
        for (j = 0; j < public_lattice.dim_m; j++) {
            public_point.point[j] = public_point.point[j] + public_lattice.vectors[i][j]*public_point.vector[i];
        }
    }
    printf("Vecteur : ");
    for (i = 0; i<public_lattice.dim_n; i++){
        printf("%d ", public_point.vector[i]);
    }
    printf("Point : ");
    for (i = 0; i<public_lattice.dim_n; i++){
        printf("%d ", public_point.point[i]);
    }
    printf("\n");*/
  }
}
/*---------------------------------------------------------------------------*/
static void
print_local_addresses(void){

  int i;
  uint8_t state;

  PRINTF("Client IPv6 addresses: ");
  for(i = 0; i < UIP_DS6_ADDR_NB; i++) {
    state = uip_ds6_if.addr_list[i].state;
    if(uip_ds6_if.addr_list[i].isused &&
       (state == ADDR_TENTATIVE || state == ADDR_PREFERRED)) {
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
static void
set_global_address(void)
{
  uip_ipaddr_t ipaddr;

  uip_ip6addr(&ipaddr, 0xaaaa, 0, 0, 0, 0, 0, 0, 0);
  uip_ds6_set_addr_iid(&ipaddr, &uip_lladdr);
  uip_ds6_addr_add(&ipaddr, 0, ADDR_AUTOCONF);

/* The choice of server address determines its 6LoPAN header compression.
 * (Our address will be compressed Mode 3 since it is derived from our link-local address)
 * Obviously the choice made here must also be selected in udp-server.c.
 *
 * For correct Wireshark decoding using a sniffer, add the /64 prefix to the 6LowPAN protocol preferences,
 * e.g. set Context 0 to aaaa::.  At present Wireshark copies Context/128 and then overwrites it.
 * (Setting Context 0 to aaaa::1111:2222:3333:4444 will report a 16 bit compressed address of aaaa::1111:22ff:fe33:xxxx)
 *
 * Note the IPCMV6 checksum verification depends on the correct uncompressed addresses.
 */
 
#if 0
/* Mode 1 - 64 bits inline */
   uip_ip6addr(&server_ipaddr, 0xaaaa, 0, 0, 0, 0, 0, 0, 1);
#elif 1
/* Mode 2 - 16 bits inline */
  uip_ip6addr(&server_ipaddr, 0xaaaa, 0, 0, 0, 0, 0x00ff, 0xfe00, 1);
#else
/* Mode 3 - derived from server link-local (MAC) address */
  uip_ip6addr(&server_ipaddr, 0xaaaa, 0, 0, 0, 0x0250, 0xc2ff, 0xfea8, 0xcd1a); //redbee-econotag
#endif
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_sensor_process, ev, data)
{
  static struct etimer periodic;
  //static struct ctimer backoff_timer;
#if WITH_COMPOWER
  static int print = 0;
#endif

  PROCESS_BEGIN();

  PROCESS_PAUSE();

  float fn[128], wn[128];
  uint32_t kn[128];
  uint32_t jsr = 42;

  set_global_address();
  
  PRINTF("UDP client process started\n");

  print_local_addresses();

  /* new connection with remote host */
  client_conn = udp_new(NULL, UIP_HTONS(UDP_SERVER_PORT), NULL); 
  if(client_conn == NULL) {
    PRINTF("No UDP connection available, exiting the process!\n");
    PROCESS_EXIT();
  }
  udp_bind(client_conn, UIP_HTONS(UDP_CLIENT_PORT)); 

  PRINTF("Created a connection with the server ");
  PRINT6ADDR(&client_conn->ripaddr);
  PRINTF(" local/remote port %u/%u\n",
	UIP_HTONS(client_conn->lport), UIP_HTONS(client_conn->rport));

#if WITH_COMPOWER
  powertrace_sniff(POWERTRACE_ON);
#endif


  /*PRINTF("Building random generator");
  r4_nor_setup ( kn, fn, wn );
  shr3_seeded ( &jsr );
  float value = r4_nor ( &jsr, kn, fn, wn );
  PRINTF("Le nombre choisis est : %d\n", value);*/

  etimer_set(&periodic, START_TIME);
  //int request_id = 123;
  //ctimer_set(&backoff_timer, SEND_TIME, send_request, &request_id);
  while(1) {
    PROCESS_YIELD();
    if(ev == tcpip_event) {
      tcpip_handler();
    }
    
    if(etimer_expired(&periodic)) {
       etimer_set(&periodic, rand()%MAX_RECHECK_TIME);
       if (public_lattice.metadata.max == NULL) {
           send_request(0);
       } else if (public_lattice.metadata.id != (typeof(public_lattice.metadata.id)) -1) {
	   send_request(public_lattice.metadata.id+1);
      }
      //ctimer_set(&backoff_timer, SEND_TIME, NULL, NULL);//send_packet, NULL);

#if WITH_COMPOWER
      if (print == 0) {
	powertrace_print("#P");
      }
      if (++print == 3) {
	print = 0;
      }
#endif
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/














/*---------------------------------------------------------------------------
static void
send_packet(void *ptr)
{
  static int seq_id;
  char buf[MAX_PAYLOAD_LEN];

  seq_id++;
  PRINTF("DATA send to %d 'Hello %d'\n",
         server_ipaddr.u8[sizeof(server_ipaddr.u8) - 1], seq_id);
  sprintf(buf, "Hello %d from the client", seq_id);
  uip_udp_packet_sendto(client_conn, buf, strlen(buf),
                        &server_ipaddr, UIP_HTONS(UDP_SERVER_PORT));
}
---------------------------------------------------------------------------*/


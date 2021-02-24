#include "contiki.h"
#include "os/lib/random.h"
#include "os/sys/ctimer.h"
#include "os/sys/ctimer.h"
#include "os/net/routing/routing.h"
#include "os/net/netstack.h"
#include "os/net/ipv6/simple-udp.h"

#include "os/sys/log.h"
#ifdef WITH_COMPOWER
#include "powertrace.h"
#endif
#include <stdio.h>
#include <string.h>
#include "parameters.h"
#include "gaus_sample.c"
#include "HNF.c"


#ifndef PERIOD
#define PERIOD 60
#endif

static struct simple_udp_connection udp_conn;
static struct lattice public_lattice;
uip_ipaddr_t dest_ipaddr;


/*---------------------------------------------------------------------------*/
PROCESS(udp_sensor_process, "UDP sensor process");
AUTOSTART_PROCESSES(&udp_sensor_process);
/*---------------------------------------------------------------------------*/


static void
send_request(int requestID){

  char buf[8];
  printf("Request send to %d to get part : '%d'\n", dest_ipaddr.u8[sizeof(dest_ipaddr.u8) - 1], requestID);
  sprintf(buf, "%d", requestID);

  simple_udp_sendto(&udp_conn, buf, strlen(buf), &dest_ipaddr);
}

/*---------------------------------------------------------------------------*/

static void
one_way_function(char secret[VECTOR_SIZE]){
    printf("One way function\n");
    int i, j, temp;
    long output[NBR_VECTORS] = {0};
    for(i = 0; i < VECTOR_SIZE; i++){ // i is the i eme element in the public key we work on
        temp = public_lattice.vectors[i];
        for(j = 0; j < NBR_VECTORS; j++){ // j is the row of the public key we are working on
    	     if((i+j)%VECTOR_SIZE==0 && i!=0)
		temp = (-temp)%public_lattice.metadata.max;
             output[j] = (output[j] + (temp*secret[(i+j)%VECTOR_SIZE]))%public_lattice.metadata.max;
             //printf("%d %d %d %d %d \n", (int) i, (int) j, (int) temp, (int) secret[(i+j)%VECTOR_SIZE], (int) output[j]);
        }
	//PRINTF("\n");
        //output[i] = output[i]%public_lattice.metadata.max;
        //temp_vector[VECTOR_SIZE-pointer-1] = (-temp_vector[VECTOR_SIZE-pointer-1])%public_lattice.metadata.max;

        //for (k = 0; k<VECTOR_SIZE; k++){
	//   printf("%d ", temp_vector[(VECTOR_SIZE-pointer-1+k)%VECTOR_SIZE]);
    	//}
        //PRINTF("\n");
    }
    printf("Le vecteur de sortie :\n");
    for (i = 0; i<NBR_VECTORS; i++){
    	printf("%d ", (int) output[i]);
	if((i+1)%30==0)
	      printf("\n");
    }
    printf("\n");

}

/*---------------------------------------------------------------------------*/

static void
chouse_random_point(){
    printf("Random point from lattice\n");
    int i, j, temp;
    int64_t output[VECTOR_SIZE] = {0};
    long random_vector[NBR_VECTORS] = {0};

    for(i=0;i<NBR_VECTORS;i++)
     random_vector[i] = rand() % public_lattice.metadata.max;

    printf("Le vecteur aleatoire :\n");
    for (i = 0; i<NBR_VECTORS; i++){
    	printf("%d ", (int) random_vector[i]);
	if((i+1)%30==0)
	      printf("\n");
    }
    printf("\n");

    for(i = 0; i < VECTOR_SIZE; i++){ // i is the i eme element in the public key we work on
        temp = public_lattice.vectors[i];
        for(j = 0; j < NBR_VECTORS; j++){ // j is the row of the public key we are working on
    	     if((i+j)%VECTOR_SIZE==0 && i!=0)
		temp = (-temp)%public_lattice.metadata.max;
             output[i] += temp*random_vector[j];
             //printf("%d %d %d %li %li \n", (int) i, (int) j, (int) temp, (long) random_vector[j], (long) output[i]);
        }
    }
    printf("Le point de sortie :\n");
    for (i = 0; i<VECTOR_SIZE; i++){
    	printf("%li ", (long) output[i]);
	if((i+1)%30==0)
	      printf("\n");
    }
    printf("\n");
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
  printf("REcu\n");
  if(data) {
    //str = uip_appdata;
    //str[uip_datalen()] = '\0';
    //printf(str);
    //PRINTF("\n");
    struct udp_lattice* received_struct_ptr;
    received_struct_ptr = (struct udp_lattice*) data;
    struct udp_lattice temp_lattice = *received_struct_ptr;

    public_lattice.metadata = temp_lattice.metadata;


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
  	
	char test[VECTOR_SIZE] = {1};
	one_way_function(test);
        chouse_random_point();
          int j;
	  double arr[3][3] = { 
		    {2, 4, 4}, 
		    {2, 7, 1}, 
		    {7, 4, 9} 
		}; 
	  printf("Vecteur de base :\n");
	    for(i=0; i<3; ++i) { 
		printf("q[%d] = [ ", i); 
		for(j=0; j<3; ++j) 
		    printf("%d  ", (int) (arr[i][j])); 
		printf("]\n"); 
	    } 

	  HNF(arr, 3);

	  printf("Forme Hermitienne :\n");
	    for(i=0; i<3; ++i) { 
		printf("q[%d] = [ ", i); 
		for(j=0; j<3; ++j) 
		    printf("%d  ", (int) (arr[i][j]*1000)); 
		printf("]\n"); 
	    } 
    }
  }
}

/*---------------------------------------------------------------------------*/

PROCESS_THREAD(udp_sensor_process, ev, data)
{
  static struct etimer periodic;

#if WITH_COMPOWER
  static int print = 0;
#endif

float fn[128], wn[128];
long kn[128];
long jsr = 42;

  PROCESS_BEGIN();

  PROCESS_PAUSE();

  simple_udp_register(&udp_conn, UDP_CLIENT_PORT, NULL, UDP_SERVER_PORT, tcpip_handler);

#if WITH_COMPOWER
  powertrace_sniff(POWERTRACE_ON);
#endif


  printf("Building random generator\n");
  r4_nor_setup ( kn, fn, wn );
  shr3_seeded ( &jsr );
  int i;
  for(i = 0; i<10; i++){
     float value = r4_nor ( &jsr, kn, fn, wn );
     printf("Le nombre choisis est : %d\n", (int) (1000000000*value));
  }

  etimer_set(&periodic, START_TIME);

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic));

    NETSTACK_ROUTING.get_root_ipaddr(&dest_ipaddr);

    etimer_set(&periodic, rand()%MAX_RECHECK_TIME);
    if (public_lattice.metadata.max == (int) NULL) {
        send_request(0);
    } else if (public_lattice.metadata.id != (typeof(public_lattice.metadata.id)) -1) {
	send_request(public_lattice.metadata.id+1);
   }


#if WITH_COMPOWER
      if (print == 0) {
	powertrace_print("#P");
      }
      if (++print == 3) {
	print = 0;
      }
#endif
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/

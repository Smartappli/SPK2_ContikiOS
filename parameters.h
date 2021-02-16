#define NBR_VECTORS 10
#define VECTOR_SIZE 10
#define DATA_PER_PACKET 42
#define MODULO_LATTICE 64 //Max 2^16-1 = 65.535

#define UDP_CLIENT_PORT	8765
#define UDP_SERVER_PORT	5678

#define UDP_EXAMPLE_ID  190

# include <math.h>
# include <stdint.h>
# include <stdio.h>
# include <stdlib.h>

struct lattice_metadata {
	    int8_t id;
	    uint16_t dim_n;
            uint16_t dim_m;
            uint16_t max;
        };

struct lattice {
            uint16_t vectors[VECTOR_SIZE];
	    struct lattice_metadata metadata;
        };

struct udp_lattice {
            uint16_t vectors[DATA_PER_PACKET];
	    struct lattice_metadata metadata;
        };


//For the sensor
#define START_TIME		(4 * CLOCK_SECOND) //time after which he start asking the lattice
#define MAX_RECHECK_TIME	(10 * CLOCK_SECOND)
//#define MAX_PAYLOAD_LEN		2

struct point {
            uint16_t vector[VECTOR_SIZE];
            uint32_t point[VECTOR_SIZE];
        };

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

#define ROWS 10
#define COLS 10
int randomBits[ROWS];

const long _p32_t[128] = {0, 0, 0, 0, 0, 0, 0, 1, 2, 1, 5, 14, 6, 28, 48, 11, 91, 21, 151, 29, 115, 261, 356, 1022, 0, 1979, 
649, 662, 2533, 394, 3885, 4422, 6621, 3717, 10085, 14247, 2087, 32191, 31092, 5293, 64733, 6382, 39056, 115692, 51969, 5856, 94298,
171399, 177506, 92944, 508031, 82593, 227864, 25774, 1030068, 7848, 1028490, 420956, 1371527, 764718, 1483754, 1713010, 3014121,
52865, 3832607, 959009, 6586417, 2413379, 1330906, 8187911, 6557106, 9169590, 10590036, 14973908, 14290999, 19921632, 11609504,
18523933, 25695147, 9724218, 47622715, 60536693, 42918720, 16785990, 105485922, 6837708, 50755763, 43186108, 94426434, 226108042,
19905797, 44150882, 187467745, 109279605, 422163790, 172043920, 255161468, 426425504, 264698303, 905612549, 52289274, 62428384,
426638585, 325949524, 652487048, 1482215515, 1224550981, 303419361, 1867463484, 51807019, 3683469341u, 803931803, 1714099899,
2620837769u, 2169546439u, 1709320247, 3824357929u, 4067003105u, 2706660263u, 724038919, 2747761453u, 2899911731u, 2781772224u,
2289919236u, 2637396209u, 3220823603u, 540201915, 3813240143u};

const uint8_t _p8_t[128] = {0, 0, 0, 15, 54, 90, 150, 96, 152, 230, 226, 90, 4, 82, 222, 246, 14, 64, 162, 196, 47, 149, 166, 58, 
7,
232, 174, 161, 204, 62, 4, 2, 139, 138, 90, 123, 195, 227, 11, 86, 88, 118, 8, 60, 224, 18, 245, 103, 194, 158, 239, 182, 126,
63, 83, 207, 61, 2, 253, 162, 5, 125, 9, 113, 252, 95, 136, 54, 159, 35, 171, 33, 128, 130, 11, 190, 117, 98, 186, 86, 178, 19, 
64,
237, 192, 250, 223, 28, 125, 71, 21, 199, 187, 163, 6, 98, 155, 140, 244, 204, 77, 60, 200, 147, 1, 185, 34, 110, 181, 39, 175,
182, 171, 116, 171, 201, 196, 125, 55, 117, 48, 197, 231, 50, 71, 0, 92, 100};

const uint8_t _hw[128] =
{0, 0, 0, 4, 4, 4, 4, 3, 4, 6, 6, 7, 3, 6, 8, 9, 8, 4, 8, 7, 10, 7, 8, 13, 3, 13, 9, 8, 11, 9, 9, 6, 13, 9, 12, 16, 9, 18, 12,
  11, 15, 13, 6, 14, 9, 8, 14, 15, 12, 12, 19, 11, 15, 14, 18, 13, 16, 11, 19, 15, 14, 16, 18, 11, 21, 14, 9, 14, 17, 15, 15, 16,
12, 16, 14, 21, 14, 16, 16, 14, 19, 21, 12, 12, 14, 17, 20, 19, 17, 17, 17, 15, 20, 19, 17, 13, 22, 17, 26, 20, 22, 16, 23, 18,
13, 19, 17, 20, 25, 16, 22, 23, 22, 21, 18, 19, 19, 22, 21, 22, 18, 19, 22, 16, 22, 23, 19, 21};

const long psi_rev[512] =
{1, 1479, 8246, 5146, 4134, 6553, 11567, 1305, 5860, 3195, 1212, 10643, 3621, 9744, 8785, 3542, 7311, 10938, 8961, 5777, 5023,
6461, 5728, 4591, 3006, 9545, 563, 9314, 2625, 11340, 4821, 2639, 12149, 1853, 726, 4611, 11112, 4255, 2768, 1635, 2963, 7393,
2366, 9238, 9198, 12208, 11289, 7969, 8736, 4805, 11227, 2294, 9542, 4846, 9154, 8577, 9275, 3201, 7203, 10963, 1170, 9970, 955,
11499, 8340, 8993, 2396, 4452, 6915, 2837, 130, 7935, 11336, 3748, 6522, 11462, 5067, 10092, 12171, 9813, 8011, 1673, 5331, 7300,
10908, 9764, 4177, 8705, 480, 9447, 1022, 12280, 5791, 11745, 9821, 11950, 12144, 6747, 8652, 3459, 2731, 8357, 6378, 7399, 10530,
3707, 8595, 5179, 3382, 355, 4231, 2548, 9048, 11560, 3289, 10276, 9005, 9408, 5092, 10200, 6534, 4632, 4388, 1260, 334, 2426,
1428, 10593, 3400, 2399, 5191, 9153, 9273, 243, 3000, 671, 3531, 11813, 3985, 7384, 10111, 10745, 6730, 11869, 9042, 2686, 2969,
3978, 8779, 6957, 9424, 2370, 8241, 10040, 9405, 11136, 3186, 5407, 10163, 1630, 3271, 8232, 10600, 8925, 4414, 2847, 10115, 4372,
9509, 5195, 7394, 10805, 9984, 7247, 4053, 9644, 12176, 4919, 2166, 8374, 12129, 9140, 7852, 3, 1426, 7635, 10512, 1663, 8653,
4938, 2704, 5291, 5277, 1168, 11082, 9041, 2143, 11224, 11885, 4645, 4096, 11796, 5444, 2381, 10911, 1912, 4337, 11854, 4976,
10682, 11414, 8509, 11287, 5011, 8005, 5088, 9852, 8643, 9302, 6267, 2422, 6039, 2187, 2566, 10849, 8526, 9223, 27, 7205, 1632,
7404, 1017, 4143, 7575, 12047, 10752, 8585, 2678, 7270, 11744, 3833, 3778, 11899, 773, 5101, 11222, 9888, 442, 9377, 6591, 354,
7428, 5012, 2481, 1045, 9430, 1987, 1702, 3565, 654, 5206, 6760, 3199, 56, 6137, 7341, 11889, 10561, 5862, 6153, 5415, 8646, 1359,
6854, 11035, 973, 2033, 8291, 1922, 3879, 468, 3988, 382, 11973, 5339, 6843, 6196, 8579, 4467, 7500, 4749, 6752, 8500, 12142,
6833, 4449, 1050, 4536, 6844, 8429, 2683, 11099, 3818, 6171, 6364, 11271, 3514, 11248, 10316, 6715, 1278, 9945, 8214, 6974, 7965,
7373, 2169, 522, 5079, 3262, 6008, 885, 5009, 10333, 1003, 8757, 241, 58, 11184, 142, 6608, 3477, 3438, 9445, 11314, 8077, 3602,
6221, 11868, 4080, 8689, 9026, 4624, 6212, 7507, 5886, 3029, 6695, 4213, 504, 11684, 2302, 6821, 11279, 11502, 3482, 7048, 2920,
3127, 4169, 7232, 4698, 8844, 4780, 10240, 4912, 1321, 12097, 11858, 1579, 9784, 6383, 151, 2127, 3957, 2839, 5874, 11612, 6055,
8953, 52, 3174, 10966, 9523, 9139, 10970, 4046, 11580, 4240, 3570, 835, 6065, 11367, 441, 4079, 11231, 10331, 4322, 2078, 1112,
12225, 3656, 683, 2459, 5782, 10723, 9341, 2503, 5919, 4433, 8455, 7032, 1747, 3123, 3054, 6803, 10885, 325, 11143, 948, 8561,
4049, 5990, 11130, 6190, 11994, 6523, 652, 3762, 9370, 4016, 4077, 8960, 4298, 2692, 12121, 1594, 10327, 7183, 5961, 6992, 6119,
8333, 10929, 1200, 5184, 2555, 6122, 2882, 10484, 10335, 10238, 6147, 9842, 8326, 576, 3434, 3529, 2908, 12071, 2361, 1843, 3030,
8174, 6956, 2031, 6413, 10008, 12133, 2767, 3969, 8298, 11836, 5908, 418, 3772, 7515, 5429, 7552, 10996, 2057, 6920, 3202, 4493,
11939, 10777, 1815, 5383, 10800, 9789, 10706, 5942, 1263, 49, 5915, 10806, 9280, 10596, 11566, 12115, 9551, 5868, 9634, 5735,
1975, 8532, 2925, 347, 4754, 1858, 11863, 8974};

const long psi_inv_rev[512] =
{1, 10810, 7143, 4043, 10984, 722, 5736, 8155, 8747, 3504, 2545, 8668, 1646, 11077, 9094, 6429, 9650, 7468, 949, 9664, 2975,
11726, 2744, 9283, 7698, 6561, 5828, 7266, 6512, 3328, 1351, 4978, 790, 11334, 2319, 11119, 1326, 5086, 9088, 3014, 3712, 3135,
7443, 2747, 9995, 1062, 7484, 3553, 4320, 1000, 81, 3091, 3051, 9923, 4896, 9326, 10654, 9521, 8034, 1177, 7678, 11563, 10436,
140, 1696, 10861, 9863, 11955, 11029, 7901, 7657, 5755, 2089, 7197, 2881, 3284, 2013, 9000, 729, 3241, 9741, 8058, 11934, 8907,
7110, 3694, 8582, 1759, 4890, 5911, 3932, 9558, 8830, 3637, 5542, 145, 339, 2468, 544, 6498, 9, 11267, 2842, 11809, 3584, 8112,
2525, 1381, 4989, 6958, 10616, 4278, 2476, 118, 2197, 7222, 827, 5767, 8541, 953, 4354, 12159, 9452, 5374, 7837, 9893, 3296, 3949,
2859, 11244, 9808, 7277, 4861, 11935, 5698, 2912, 11847, 2401, 1067, 7188, 11516, 390, 8511, 8456, 545, 5019, 9611, 3704, 1537,
242, 4714, 8146, 11272, 4885, 10657, 5084, 12262, 3066, 3763, 1440, 9723, 10102, 6250, 9867, 6022, 2987, 3646, 2437, 7201, 4284,
7278, 1002, 3780, 875, 1607, 7313, 435, 7952, 10377, 1378, 9908, 6845, 493, 8193, 7644, 404, 1065, 10146, 3248, 1207, 11121, 7012,
6998, 9585, 7351, 3636, 10626, 1777, 4654, 10863, 12286, 4437, 3149, 160, 3915, 10123, 7370, 113, 2645, 8236, 5042, 2305, 1484,
4895, 7094, 2780, 7917, 2174, 9442, 7875, 3364, 1689, 4057, 9018, 10659, 2126, 6882, 9103, 1153, 2884, 2249, 4048, 9919, 2865,
5332, 3510, 8311, 9320, 9603, 3247, 420, 5559, 1544, 2178, 4905, 8304, 476, 8758, 11618, 9289, 12046, 3016, 3136, 7098, 9890,
8889, 3315, 426, 10431, 7535, 11942, 9364, 3757, 10314, 6554, 2655, 6421, 2738, 174, 723, 1693, 3009, 1483, 6374, 12240, 11026,
6347, 1583, 2500, 1489, 6906, 10474, 1512, 350, 7796, 9087, 5369, 10232, 1293, 4737, 6860, 4774, 8517, 11871, 6381, 453, 3991,
8320, 9522, 156, 2281, 5876, 10258, 5333, 4115, 9259, 10446, 9928, 218, 9381, 8760, 8855, 11713, 3963, 2447, 6142, 2051, 1954,
1805, 9407, 6167, 9734, 7105, 11089, 1360, 3956, 6170, 5297, 6328, 5106, 1962, 10695, 168, 9597, 7991, 3329, 8212, 8273, 2919,
8527, 11637, 5766, 295, 6099, 1159, 6299, 8240, 3728, 11341, 1146, 11964, 1404, 5486, 9235, 9166, 10542, 5257, 3834, 7856, 6370,
9786, 2948, 1566, 6507, 9830, 11606, 8633, 64, 11177, 10211, 7967, 1958, 1058, 8210, 11848, 922, 6224, 11454, 8719, 8049, 709,
8243, 1319, 3150, 2766, 1323, 9115, 12237, 3336, 6234, 677, 6415, 9450, 8332, 10162, 12138, 5906, 2505, 10710, 431, 192, 10968,
7377, 2049, 7509, 3445, 7591, 5057, 8120, 9162, 9369, 5241, 8807, 787, 1010, 5468, 9987, 605, 11785, 8076, 5594, 9260, 6403, 4782,
6077, 7665, 3263, 3600, 8209, 421, 6068, 8687, 4212, 975, 2844, 8851, 8812, 5681, 12147, 1105, 12231, 12048, 3532, 11286, 1956,
7280, 11404, 6281, 9027, 7210, 11767, 10120, 4916, 4324, 5315, 4075, 2344, 11011, 5574, 1973, 1041, 8775, 1018, 5925, 6118, 8471,
1190, 9606, 3860, 5445, 7753, 11239, 7840, 5456, 147, 3789, 5537, 7540, 4789, 7822, 3710, 6093, 5446, 6950, 316, 11907, 8301,
11821, 8410, 10367, 3998, 10256, 11316, 1254, 5435, 10930, 3643, 6874, 6136, 6427, 1728, 400, 4948, 6152, 12233, 9090, 5529, 7083,
11635, 8724, 10587, 10302};

/*---------------------------------------------------------------------------*/
PROCESS(udp_gateway_process, "UDP gateway process");
AUTOSTART_PROCESSES(&udp_gateway_process);
/*---------------------------------------------------------------------------*/

long mod(long a, long b){
    long output = a % b;
    if(output < 0)
	output+=b;
    return output;
}//end mod()

static long ct_lt_u32(long x, long y) {
    return (x^((x^y)|((x-y)^y))) >> 31;
}

// bit = 0 then return a
long Select(long a, long b, unsigned bit) {
  unsigned mask = -bit;
  return ((mask & (a ^ b)) ^ a);
}

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

long knuth_yao_ct_fast_32(int tailcut, int sigma, int center) {

  long signal;
  long _d, d, S; // may be negative numbers
  unsigned long bound, col, invalidSample, pNumRows, pNumCols, r, t;
  unsigned long long enable, hit;

  bound = tailcut * sigma;
  signal = 1 - 2 * (rand() & 1);
  invalidSample = bound + 1;
  pNumRows = ROWS;
  pNumCols = COLS;

  unsigned long index, k; // < 255
  long row;

  index = 0;
  while(index < pNumRows) {
    r = rand();
    for (k = 0; k < 32; k++, r >>= 1)
      randomBits[index++] = (int8_t)(r & 0x1);
 }//end-while

  int8_t aux_hw;

  S = 0; t = 0; d = 0; hit = 0; _d = 0;
  for (row = 1; row < pNumRows; row++) {
    aux_hw = _hw[row];
    _d = 2 * _d + randomBits[row];	// Distance calculus
    enable = ct_lt_u32(_d, aux_hw); // _d < _hw[row]?
    _d = Select(_d - aux_hw, _d, enable & !hit);
    d = Select(d, _d, enable & !hit);
    t = Select(t, row, enable & !hit); // t \in {0, 127 = ROWS}
    hit += (enable & !hit);
  }

  unsigned long aux_p32 = _p32_t[t];

  hit = 0;
  for (col = 0; col <= 31; col++) {
    d = d - ((aux_p32 >> (unsigned long)(32 - col - 1)) & 1);

    enable = (unsigned)(d + 1); // "enable" turns 0 iff d = -1
    enable = (1 ^ ((enable | -enable) >> 31)) & 1; // "enable" turns 1 iff "enable" was 0

    S += Select(invalidSample, col, (enable & !hit));
    hit += (enable & !hit);

  }

  uint8_t aux_p8 = _p8_t[t];

  for (col = 32; col < pNumCols; col++) {
    d = d - ((aux_p8 >> (uint8_t)(pNumCols - col - 1)) & 1);

    enable = (unsigned)(d + 1); // "enable" turns 0 iff d = -1
    enable = (1 ^ ((enable | -enable) >> 31)) & 1; // "enable" turns 1 iff "enable" was 0

    S += Select(invalidSample, col, (enable & !hit));
    hit += (enable & !hit);

  }

  /* Note: the "col" value is in [0, bound]. So, the invalid sample must be
    * greater than bound. */
  S %= invalidSample;
  S = S - bound + center;
  S *= signal;

  return S;

}

void NTT(long a[]) {

  long i, j, j1, j2;
  long k, m, S, U;
  long Vs;

  k = VECTOR_SIZE;

  for(m = 1; m < VECTOR_SIZE; m <<= 1) {
    k >>= 1;
    for(i = 0; i < m; i++) {
      j1 = 2*i*k;
      j2 = j1 + k - 1;
      S = psi_rev[m+i];
      for(j = j1; j <= j2; j++) {
		U = a[j];
		Vs = a[(j+k)]*S;
		a[j] = mod(U + Vs, MODULO_LATTICE);
		a[j+k] = mod(U - Vs, MODULO_LATTICE);
      }//end for
    }//end for
  }//end for

}//end NTT()

void INTT(long a[]) {

  long i, h, j, j1, j2, k;
  long m, U, Vs, S;

  k = 1;

  for(m = VECTOR_SIZE; m > 1; m >>= 1) {
    j1 = 0;
    h = m >> 1;
    for(i = 0; i < h; i++) {
      j2 = j1 + k - 1;
      S = psi_inv_rev[h+i];
      for(j = j1; j <= j2; j++) {
		U = a[j];
		Vs = a[(j+k)];
		a[j] = mod(U + Vs, MODULO_LATTICE);
		a[j+k] = mod((U - Vs)*S, MODULO_LATTICE);
      }//end-for
      j1 = j1 + 2*k;
    }//end-for
    k <<= 1;
  }//end-for

  for(j = 0; j < VECTOR_SIZE; j++) {
    a[j] = mod(a[j] * N_inv, MODULO_LATTICE);
  }

}//end-INTT()

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

void PolySampling(long a[]) {
	long i;

	for(i = 0; i < VECTOR_SIZE; i++)
		a[i] = knuth_yao_ct_fast_32(13, 3.1915, 0);

}//end-PolySampling()

/*---------------------------------------------------------------------------*/

void key_generation(const long public_key_1[], long private_key[], long public_key_2[]){
    long r[VECTOR_SIZE];
    unsigned long i;

    PolySampling(r);
    PolySampling(private_key);
    NTT(r);
    NTT(private_key);

    for(i = 0; i < N; i++){
	public_key_2[i] = mod(r[i] - public_key_1[i]*private_key[i], MODULO_LATTICE);
    }//end for
}//end key_generation()

/*---------------------------------------------------------------------------*/

void encode(long output[], long input[]){
    long i;
    long boundary = (MODULO_LATTICE-1)/2;

    for(i = 0; i < VECTOR_SIZE; i++) {
	output[i] = input[i]*boundary;
    }//end for
}//end encode()

/*---------------------------------------------------------------------------*/

void decode(long output[], long input[]){
    long i;
    long lower_bound = (MODULO_LATTICE-1)/4;
    long upper_bound = 3*lower_bound;

    for(i = 0; i < VECTOR_SIZE; i++) {
	if(input[i] >= lower_bound && input[i] < upper_bound)
	    output[i] = 1;
	else
	    output[i] = 0;
    }//end for
}//end decode()

/*---------------------------------------------------------------------------*/

void encryption(long message_1[], long message_2[], const long public_key_1[], const long public_key_2[], const long input[]){
    long i;

    long e1[VECTOR_SIZE], e2[VECTOR_SIZE], e3[VECTOR_SIZE], temp[VECTOR_SIZE];
    PolySampling(e1);
    PolySampling(e2);
    PolySampling(e3);

    NTT(e1);
    NTT(e2);

    for(i = 0; i < VECTOR_SIZE; i++)
	temp[i] = mod(e3[i] + input[i], MODULO_LATTICE);
    NTT(temp);

    for(i = 0; i < VECTOR_SIZE; i++){
	message_1[i] = mod(public_key_1[i] * e1[i] + e2[i], MODULO_LATTICE);
	message_2[i] = mod(public_key_2[i] * e1[i] + temp[i], MODULO_LATTICE);
    }//end for
    
}//end encryption()

/*---------------------------------------------------------------------------*/

void decryption(long output[], long message_1[], long message_2[], long secret_key[]){
    long i;

    for(i = 0; i < VECTOR_SIZE; i++) {
	output[i] = mod(message_1[i] * secret_key[i] + message_2[i], MODULO_LATTICE);
    }//end for
    INTT(output);
}//end decryption()

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



  long public_key_1[VECTOR_SIZE], private_key[VECTOR_SIZE], public_key_2[VECTOR_SIZE];
  create_polynome(public_key_1);
  NTT(public_key_1);
  key_generation(public_key_1, private_key, public_key_2);

  long message[VECTOR_SIZE], encoded_message[VECTOR_SIZE], output_message[VECTOR_SIZE];
  long i;

  for(i = 0; i < VECTOR_SIZE; i++)
      message[i] = mod(rand(), 2);

  for(i = 0; i < VECTOR_SIZE; i++)
      printf("%d ", (int) message[i]);
  printf("\n");

  encode(encoded_message, message);

  for(i = 0; i < VECTOR_SIZE; i++)
      printf("%d ", (int) encoded_message[i]);
  printf("\n");

  long message_1[VECTOR_SIZE], message_2[VECTOR_SIZE];

  encryption(message_1, message_2, public_key_1, public_key_2, encoded_message);

  decryption(encoded_message, message_1, message_2, private_key);

  for(i = 0; i < VECTOR_SIZE; i++)
      printf("%d ", (int) encoded_message[i]);
  printf("\n");

  decode(output_message, encoded_message);

  for(i = 0; i < VECTOR_SIZE; i++)
      printf("%d ", (int) output_message[i]);
  printf("\n");
  


  /* Initialize DAG root */
  NETSTACK_ROUTING.root_start();

  /* Initialize UDP connection */
  simple_udp_register(&udp_conn, UDP_SERVER_PORT, NULL, UDP_CLIENT_PORT, tcpip_handler);


  while(1) {
    PROCESS_YIELD();
  }//end while
  PROCESS_END();
}//end main()


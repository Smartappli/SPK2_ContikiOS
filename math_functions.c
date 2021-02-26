# include <stdint.h>
# include <stdio.h>
# include <stdlib.h>
# include <math.h>


double exp(double input){
  return (double) powf((float) 2.71828, (float) input);
}

int msb(int n)
{
  int i = 0;
  for (; n; n >>= 1, i++)
    ; /* empty */
  return i;
}

double log(double y) {
    int log2;
    double divisor, x, result;

    log2 = msb((int)y); // See: https://stackoverflow.com/a/4970859/6630230
    divisor = (double)(1 << log2);
    x = y / divisor;    // normalized value between [1.0, 2.0]

    result = -1.7417939 + (2.8212026 + (-1.4699568 + (0.44717955 - 0.056570851 * x) * x) * x) * x;
    result += ((double)log2) * 0.69314718; // ln(2) = 0.69314718

    return result;
}

double round(double input){
  if (input >= 0.0)
     input += 0.5;
  else
     input -= 0.5;
  return (int) input;
}


double product_vector_vector(double a[VECTOR_SIZE][VECTOR_SIZE], int aa, double b[VECTOR_SIZE][VECTOR_SIZE], int bb, int size){
    int i;
    double output = 0;
    for(i = 0; i < size; i++)
	output += a[i][aa]*b[i][bb];
    return output;
}

void product_matrix_prime_matrix(double a[VECTOR_SIZE][VECTOR_SIZE], double b[VECTOR_SIZE][VECTOR_SIZE], const int size, double output[VECTOR_SIZE][VECTOR_SIZE]){
    
    int i, j, k;

  for (i = 0; i < 3; i++){
      for (j = 0; j < 3; j++) {
          printf("%d ", (int) (1000*a[i][j]));
      }
      printf("\n");
  }

  for (i = 0; i < 3; i++){
      for (j = 0; j < 3; j++) {
          printf("%d ", (int) (1000*b[i][j]));
      }
      printf("\n");
  }


    for(i = 0; i < size; i++){
        for(j = 0; j < size; j++){
	    output[i][j] = 0;
	    for(k = 0; k < size; k++)
	        output[i][j] += a[k][i]*b[k][j];
	}
    }
}

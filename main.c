
#define _XOPEN_SOURCE 600

#include "display.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#define DIM 1024
#define MAX_HEIGHT  10

unsigned table[DIM][DIM];
bool finished = false;

// vecteur de pixel renvoyé par compute  
struct {
  float R, G, B;
} couleurs[DIM][DIM];

// callback
unsigned get (unsigned x, unsigned y)
{
  return table[y][x];
}

// Tas de sable initial
static void sable_init (unsigned height)
{
  for (int y = 1; y < DIM - 1 ; y++)
    for (int x = 1; x < DIM -1 ; x++) {
      table[y][x] = height;
    }
}

static void tower_init (int height)
{
    for (int y = 0; y < DIM; y++) {
	for (int x = 0; x < DIM; x++) {
	    table[y][x] = 0;
	}
    }
    table[DIM/2][DIM/2] = height;
}

// callback
float *compute (unsigned iterations)
{
  static int step = 0;
  for (unsigned i = 0; i < iterations; i++)
    {
      step++;
      for (int y = 0; y < DIM; y++)
	{
	  int v =  MAX_HEIGHT * (1+sin( 4* (y+step) * 3.14/ DIM)) / 4;
	  for (int x = 0; x < DIM; x++)
	    table[y][x]  = v;
	}
    }
  return DYNAMIC_COLORING; // altitude-based coloring
  // return couleurs;
}

float *eboule (unsigned iterations)
{
    for (int k = 0 ; k < iterations ; k++) {
	for (int i = 1 ; i < DIM - 1 ; i++) {	
	    for (int j=1 ; j< DIM - 1 ; j++) {
		if (table[i][j] >= 4) {
		    finished = false;
		    int mod4 = table[i][j] % 4;      
		    int div4 = table[i][j] / 4;
		    table[i][j] = mod4;   
		    table[i-1][j] += div4;   
		    table[i+1][j] += div4;   
		    table[i][j-1] += div4;   
		    table[i][j+1] += div4;   
		}
	    }
	}
    }
    return DYNAMIC_COLORING;
}

float *sync_eboule (unsigned iterations)
{
    int temp[DIM][DIM];

    for (int k = 0 ; k < iterations ; k++) {
	
	for (int i = 1 ; i < DIM - 1 ; i++) {	
	    for (int j = 1 ; j< DIM - 1 ; j++) {
		temp[i][j] = 0;
	    }
	}
	
	for (int i = 1 ; i < DIM - 1 ; i++) {	
	    for (int j=1 ; j< DIM - 1 ; j++) {
		if (table[i][j] >= 4) {
		    finished = false;
		    int mod4 = table[i][j] % 4;      
		    int div4 = table[i][j] / 4;
		    temp[i][j] -= table[i][j] - mod4;   
		    temp[i-1][j] += div4;   
		    temp[i+1][j] += div4;   
		    temp[i][j-1] += div4;   
		    temp[i][j+1] += div4;   
		}
	    }
	}
	
	for (int i = 1 ; i < DIM - 1 ; i++) {	
	    for (int j=1 ; j< DIM - 1 ; j++) {
		table[i][j] += temp[i][j];
	    }
	}
    }
    
    return DYNAMIC_COLORING;
}


float *sync_eboule_openmp (unsigned iterations)
{
  
    int temp[DIM][DIM];
    #pragma omp parallel for
    for (int i = 1 ; i < DIM - 1 ; i++) {	
	for (int j = 1 ; j< DIM - 1 ; j++) {
	    temp[i][j] = 0;
	}
    }

    finished = true;

    for (unsigned k ; k < iterations; k++) {
	#pragma omp parallel
	{
	    #pragma omp for collapse(2)
	    for (int i = 1 ; i < DIM - 1 ; i++) {	
		for (int j = 1 ; j < DIM - 1 ; j++) {
		
		    if (table[i][j] >= 4) {
			finished = false;
			int mod4 = table[i][j] % 4;      
			int div4 = table[i][j] / 4;
			int delta = mod4 - table[i][j];
		        #pragma omp atomic
			temp[i][j] += delta;
       		        #pragma omp atomic		
			temp[i-1][j] += div4;
	    	        #pragma omp atomic		
			temp[i+1][j] += div4;
		        #pragma omp atomic			
			temp[i][j-1] += div4;
		        #pragma omp atomic
			temp[i][j+1] += div4;   
		    }
		}
	    }
	    
	    #pragma omp for
	    for (int i = 1 ; i < DIM - 1 ; i++) {	
		for (int j = 1 ; j < DIM - 1 ; j++) {
		    table[i][j] += temp[i][j];
		    temp[i][j] = 0;
		}
	    }
	}
    }
    return DYNAMIC_COLORING;
}

float *sync_eboule_openmp2 (unsigned iterations)
{
  
    int temp[DIM][DIM];

    finished = true;

    for (unsigned k ; k < iterations; k++) {
	#pragma omp parallel
	{
	    #pragma omp for
	    for (int i = 1 ; i < DIM - 1 ; i++) {	
		for (int j = 1 ; j < DIM - 1 ; j++) {
		    int middle = table[i][j] % 4;
		    int up = table[i-1][j] / 4;
		    int down = table[i+1][j] /4;
		    int left = table[i][j-1] / 4;
		    int right = table[i][j+1] / 4;
		    temp[i][j] = middle + up + down + left + right;
		}
	    }
	    
	    #pragma omp for
	    for (int i = 1 ; i < DIM - 1 ; i++) {	
		for (int j = 1 ; j < DIM - 1 ; j++) {
		    if (table[i][j] != temp[i][j]) {
			finished = false;
		    }
		    table[i][j] = temp[i][j];
		}
	    }
	}
    }
    return DYNAMIC_COLORING;
}

void run (compute_func_t compute_func, unsigned iterations)
{
    int computeTime = 0;
    unsigned call_counter = 0;
    while (!finished) {
	finished = true;
	struct timeval t1,t2;
	gettimeofday (&t1,NULL);
	compute_func(iterations);
	gettimeofday (&t2,NULL);
	computeTime += TIME_DIFF(t1,t2);
	call_counter++;
    }

    printf("%d function calls\n", call_counter);
    printf("time per iteration : %.3f ms\n",
	   computeTime / (float) (1000 * call_counter * iterations));
    printf("total time : %.3f s\n",
	   computeTime / (float) (1000 * 1000));
}

int main (int argc, char **argv)
{
    sable_init(5);

    run(sync_eboule_openmp2, 1);

    if (0) {
	display_init (argc, argv,
		      DIM,              // dimension ( = x = y) du tas
		      MAX_HEIGHT,       // hauteur maximale du tas
		      get,              // callback func
		      sync_eboule_openmp2);         // callback func
    }
    
    return 0;
}
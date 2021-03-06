#include "display.h"
#include "utils.h"

#include <stddef.h>
#include <getopt.h>
#include <stdlib.h>

int **table;
int **temp;

int
get (int i, int j)
{
    return table[i][j];
}

bool
naive_sync (int iterations)
{
    bool finished = true;

    for (int k = 0 ; k < iterations ; k++) {
	
	for (int i = 1 ; i < DIM - 1 ; i++) {	
	    for (int j = 1 ; j < DIM - 1 ; j++) {
		temp[i][j] = 0;
	    }
	}
	
	for (int i = 1 ; i < DIM - 1 ; i++) {	
	    for (int j = 1 ; j < DIM - 1 ; j++) {
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
	    for (int j = 1 ; j < DIM - 1 ; j++) {
		table[i][j] += temp[i][j];
	    }
	}
    }

    return finished;
}

int
main (int argc, char **argv)
{
    bool graphical = false;
    bool validation = false;
    int tower_height = 0;
    int iterations = 1;
    int optc;
    compute_func_t func = naive_sync;
    
    while ((optc = getopt(argc, argv, "t:i:gc")) != -1) {
	switch (optc) {
	    case 't' :
	        tower_height = strtol(optarg, NULL, 10);
	        break;
	    case 'i' :
	        iterations = strtol(optarg, NULL, 10);
	        break;
	    case 'g' :
		graphical = true;
		break;
	    case 'c' :
		validation = true;
		break;
	}
    }

    table = table_alloc(DIM);
    temp = table_alloc(DIM);
    
    if (tower_height != 0) {
	tower_init(table, tower_height, DIM);
    }
    else {
	flat_init(table, FLAT_HEIGHT, DIM);
    }

    if (graphical) {
	display_init (0, NULL, DIM, get, func);
    }
    else {
	run(func, iterations);
    }

    if (validation) {
	int **control = table_alloc(DIM);
	
	if (tower_height != 0) {
	    tower_init(control, tower_height, DIM);
	}
	else {
	    flat_init(control, FLAT_HEIGHT, DIM);
	}

	process(control, DIM);
	compare(table, control, DIM);
	table_free(control);
    }

    table_free(table);
    table_free(temp);
    return 0;
}

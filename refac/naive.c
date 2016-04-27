#include "display.h"
#include "utils.h"

#include <stddef.h>
#include <getopt.h>
#include <stdlib.h>

#define FLAT_HEIGHT 5

int
main (int argc, char **argv)
{
    bool graphical = false;
    bool check = false;
    int dim = 512;
    int tower_height = 0;
    int iterations = 1;
    int optc;
    compute_func_t func = naive;
    
    while ((optc = getopt(argc, argv, "N:t:i:gc")) != -1) {
	switch (optc) {
	    case 'N' :
		dim = strtol(optarg, NULL, 10);
		break;
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
		check = true;
		break;
	}
    }

    int **table = table_alloc(dim);
    if (tower_height != 0) {
	tower_init(table, tower_height, dim);
    }
    else {
	flat_init(table, FLAT_HEIGHT, dim);
    }

    if (graphical) {
	display_init (0, NULL, dim, table, get, func);
    }
    else {
	run(func, table, dim, iterations);
    }

    if (check) {
	int **control = table_alloc(dim);
	
	if (tower_height != 0) {
	    tower_init(control, tower_height, dim);
	}
	else {
	    flat_init(control, FLAT_HEIGHT, dim);
	}

	printf("\nProcessing control table.\n");
	run(naive, control, dim, iterations);
	printf("\n");
	compare(table, control, dim);
	table_free(control);
    }

    table_free(table);
    return 0;
}

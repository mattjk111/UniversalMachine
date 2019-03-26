#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "segments.h"
#include "operations.h"

/* Loads all instructions into segmented memory $m[0] */ 
/* NULL file pointer is CRE */ 
void load_program(FILE *file, Seg_T memory);

/* Runs the instructions fetched from segmented memory $m[0] */
void run_program(Seg_T memory);
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include "segments.h"
#include "bitpack.h"
#include "programloader.h"

/* CODE is always in segment memory: m[0] */
#define CODE 0

/* type for Program Counter that traverses through CODE */
typedef uint32_t *Pgm_counter;

/* loads the um_instructions from the .um file into the segmented
   memory interface */
void load_program(FILE *file, Seg_T memory)
{
	assert(file != NULL);

	unsigned c;
        unsigned pgmcounter = 0;
        /* Get an instruction by reading in from the file in big endian order
        Then, store the instruction into segmented memory $m[0][pgmcounter] */
	do {
                Um_instruction inst = 0;
                for (int i = 3; i >= 0; i--) {
                        c = getc(file);
                        if ((int) c == EOF) {
                                return;
                        }
                        inst = Bitpack_newu(inst, 8, i * 8, c);
                }
                uint32_t *word = 
                    (uint32_t *) Segment_at(memory, CODE, pgmcounter++);
                *word = inst;
        } while (c != (unsigned) EOF);
}

/* Runs the program in $m[r[0]] */
void run_program(Seg_T memory)
{
        uint32_t registers[] = { 0, 0, 0, 0, 0, 0, 0, 0 };
        Um_register *rp = registers;
        
        uint32_t counter = 0;

        /* Reads an instruction by incrementing through the program counter
           The counter may change if instruction Load Program is called */
        while (counter < Segment_length(memory, CODE)) { 
                Um_instruction inst = 
                    *(Um_instruction *)Segment_at(memory, CODE, counter);
                counter++;
                read_instruction(inst, rp, memory, &counter);
        }
        /* Free allocated memory in segmented memory */
        Segment_free(&memory);
}



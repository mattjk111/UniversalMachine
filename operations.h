#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "segments.h"

typedef uint32_t Um_register;
typedef uint32_t Um_instruction;

/* Decodes an instruction and does the instruction based on its opcode */
void read_instruction(Um_instruction inst, uint32_t *registers,
 Seg_T memory, uint32_t *counter);

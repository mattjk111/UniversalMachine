

#include "operations.h"
#include "bitpack.h"
#include "uarray.h"
#include "seq.h"
#include "assert.h"

typedef enum Um_opcode {
        CMOV = 0, SLOAD, SSTORE, ADD, MUL, DIV,
        NAND, HALT, ACTIVATE, INACTIVATE, OUT, IN, LOADP, LV
} Um_opcode;

/****************************************************************************/
/*                                                                          */
/*                             UM INSTRUCTIONS:                             */ 
/*                                                                          */
/****************************************************************************/

/* Conditional Move:
   If $r[C] != 0, then $r[A] := $r[B] */
static void cmove(Um_register *a, Um_register b, Um_register c);

/* Segmented Load:
   loads the value at the segment and offset into register A.
   $r[A] := $m[$r[B]][$r[C]] */
static void sload(Um_register *a, Um_register b, Um_register c,
        Seg_T memory);

/* Segmented Store:
   Stores the value in register C into segmented memory given
   segment and offset
   $m[$r[A]][$r[B]] := $r[C] */
static void sstore(Um_register a, Um_register b, Um_register c,
        Seg_T memory);

/* Addition:
   $r[A] := ($r[B] + $r[C]) mod 2^32 */
static void add(Um_register *a, Um_register b, Um_register c);

/* Multiplication:
   $r[A] := ($r[B] × $r[C]) mod 2^32 */
static void multiply(Um_register *a, Um_register b, Um_register c);

/* Division:
   $r[A] := ⌊$r[B] ÷ $r[C]⌋ */
static void divide(Um_register *a, Um_register b, Um_register c);

/* Bitwise NAND:
   $r[A] := ¬($r[B] ∧ $r[C]) */
static void nand(Um_register *a, Um_register b, Um_register c);

/* Halt:
   Computation stops. */
static void halt(Seg_T memory);

/* Map Segment:
   Creates a new segment given length in $r[C] the segment ID
   is placed in $r[B] */
static void map(Um_register *b, Um_register c, Seg_T memory);

/* Unmap Segment:
   The segment indicated by $m[r[C]] is unmapped */
static void unmap(Um_register c, Seg_T memory);

/* Output:
   The value in $r[C] is written to the I/O device
   immediately. Only values from 0 to 255 are allowed. */
static void output(Um_register c);

/* Input:
   When input arrives, $r[C] is loaded with the input, which must be a value
   from 0 to 255. If the end of input has been signaled, then $r[C] is
   loaded with a full 32-bit word in which every bit is 1.*/
static void input(Um_register *c);

/* Load Program:
   Segment $m[$r[B]] is duplicated, and the duplicate replaces $m[0].
   The program counter is set to point to $m[0][$r[C]]. If $r[B] = 0,
   the load-program operation is quick. */
static void loadp(Um_register b, Um_register c, Seg_T memory, 
        uint32_t *counter);

/* Load Value:
   Puts the value into $r[A] */
static void loadv(Um_register *a, uint32_t val);

/****************************************************************************/


/* Decodes the instruction and sets register A, B, and C */
static uint32_t decode(Um_instruction inst, Um_register *a, Um_register *b,
        Um_register *c);


/* Decodes an instruction and does the instruction based on its opcode */
void read_instruction(Um_instruction inst, uint32_t *regs, Seg_T memory,
        uint32_t *counter)
{
        Um_register a, b, c;

        uint32_t op_code = decode(inst, &a, &b, &c);

        /* operations use pointer arithmatic to access the correct register */
        switch (op_code) {
        case CMOV: cmove((regs + a), *(regs + b), *(regs + c));
                break;
        case SLOAD: sload((regs + a), *(regs + b), *(regs + c), memory);
                break;
        case SSTORE: sstore(*(regs + a), *(regs + b), *(regs + c), memory);
                break;
        case ADD: add((regs + a), *(regs + b), *(regs + c));
                break;
        case MUL: multiply((regs + a), *(regs + b), *(regs + c));
                break;
        case DIV: divide((regs + a), *(regs + b), *(regs + c));
                break;
        case NAND: nand((regs + a), *(regs + b), *(regs + c));
                break;
        case HALT: halt(memory);
                break;
        case ACTIVATE: map((regs + b), *(regs + c), memory);
                break;
        case INACTIVATE: unmap(*(regs + c), memory);
                break;
        case OUT: output(*(regs + c));
                break;
        case IN: input((regs + c));
                break;
        case LOADP: loadp(*(regs + b), *(regs + c), memory, counter);
                break;
        case LV: {
                /* Load value has differing instruction format */
                uint32_t val = Bitpack_getu(inst, 25, 0);
                loadv((regs + a), val);
                break;
                }
        }
}


static uint32_t decode(Um_instruction inst, Um_register *a, Um_register *b,
        Um_register *c)
{
        uint32_t op_code = Bitpack_getu(inst, 4, 28);
        assert(op_code < 14);

        if (op_code == 13){
                *a = Bitpack_getu(inst, 3, 25);
                return op_code;
        }
        *a = Bitpack_getu(inst, 3, 6);
        *b = Bitpack_getu(inst, 3, 3);
        *c = Bitpack_getu(inst, 3, 0);
        return op_code;
}



static void cmove(Um_register *a, Um_register b, Um_register c)
{
        if (c != 0) {
                *a = b;
        }
        (void) a;
}


static void sload(Um_register *a, Um_register b, Um_register c, Seg_T memory)
{
        *a = *((uint32_t *) Segment_at(memory, b, c));
        (void) a;
}


static void sstore(Um_register a, Um_register b, Um_register c, Seg_T memory)
{
        uint32_t *word = (uint32_t *) Segment_at(memory, a, b);
        *word = c;
}


static void add(Um_register *a, Um_register b, Um_register c)
{
        *a = b + c;

        (void) a;
}



static void multiply(Um_register *a, Um_register b, Um_register c)
{
        *a = b * c;

        (void) a;
}


static void divide(Um_register *a, Um_register b, Um_register c)
{
        assert(c != 0);
        *a = b / c;

        (void) a;
}


static void nand(Um_register *a, Um_register b, Um_register c)
{
        *a = ~(b & c);
        (void) a;
}


static void halt(Seg_T memory)
{
        Segment_free(&memory);
        exit(EXIT_SUCCESS);
}


static void map(Um_register *b, Um_register c, Seg_T memory)
{
        unsigned length = c;
        uint32_t segid = Segment_map(memory, length);
        *b = segid;
        (void) b;
}


static void unmap(Um_register c, Seg_T memory)
{
        uint32_t segid = c;

        assert(segid != 0);

        Segment_unmap(memory, segid);
}


static void output(Um_register c)
{
        uint32_t val = c;

        assert(val <= 255);

        printf("%c", val);
        
}


static void input(Um_register *c)
{
        char input;
        input = getc(stdin);

        if (input == EOF) {
                *c = ~0;
                return;
        }
        *c = (uint32_t) input;

        (void) c;
}


static void loadp(Um_register b, Um_register c, Seg_T memory,
        uint32_t *counter)
{
        if (b != 0) {
                Segment_replace(memory, b, 0);
        }
        *counter = c;
}


static void loadv(Um_register *a, uint32_t val)
{
        *a = val;
        (void) a;
}





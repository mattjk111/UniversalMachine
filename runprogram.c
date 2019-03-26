#include <stdio.h>
#include <stdlib.h>

#include "segments.h"
#include "programloader.h"
#include <sys/stat.h>


int main (int argc, char *argv[])
{
        struct stat sb;

        /* If proper format is not given to stdin, exit failure */
        if (argc != 2) {
               fprintf(stderr, "Usage: %s <pathname>\n", argv[0]);
               exit(EXIT_FAILURE);
        }
        /* Stat function error */
        if (stat(argv[1], &sb) == -1) {
               fprintf(stderr, "stat error\n");
               exit(EXIT_FAILURE);
        }

        FILE *fp = fopen(argv[1], "rb");

        /* File open error */
        if (fp == NULL) {
                fprintf(stderr, 
                        "%s: Could not open file %s for reading\n",
                        argv[0], argv[1]);
                exit(EXIT_FAILURE);
        }

        /* Get length of instructions */
        unsigned length = sb.st_size / 4;

        /* Allocates segmented memory and loads instructions into $m[0] */
        Seg_T memory = Segment_new(length);
        load_program(fp, memory);
        fclose(fp);

        /* RUN PROGRAM */
        run_program(memory);

        exit(EXIT_SUCCESS);
}

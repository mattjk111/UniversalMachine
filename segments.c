#include <math.h>
#include <stdint.h>
#include "assert.h"
#include "uarray.h"
#include "seq.h"
#include <stdio.h>
#include <stdlib.h>
#include <except.h>
#include "segments.h"

#define WORD 32

/* Struct for UArray2 representation */
struct Seg_T {
        int length;
        Seq_T segments;
        Seq_T unmapped_ids;
};


static uint32_t check_unmapped(Seq_T unmapped_ids);


/*      
 Segment_new: creates a new segmented memory with a segment
        of a given length
 Parameters: unsigned length
 Returns: Seg_T
*/
Seg_T Segment_new(unsigned length)
{
    Seg_T seg_temp = malloc(sizeof(*seg_temp));
    assert(seg_temp);

    UArray_T code_array = UArray_new(length, WORD);
    assert(code_array);

    for (unsigned i = 0; i < length; i++) {
        uint32_t *word = (uint32_t *) UArray_at(code_array, i);
        *word = 0;
    }

    Seq_T segments = Seq_new(0);
    assert(segments);
    Seq_addhi(segments, code_array);

    Seq_T unmapped_ids = Seq_new(0);
    assert(unmapped_ids);


    seg_temp->segments = segments;
    seg_temp->length = length;
    seg_temp->unmapped_ids = unmapped_ids;

    return seg_temp;
}


/*      
 Segment_map: maps a new segment with given length and
                returns the Id of the segment mapped
 Parameters: Seg_T seg, unsigned length
 Returns: Umsegment_Id
*/
Umsegment_Id Segment_map(Seg_T seg, unsigned length)
{
        assert(seg);
        UArray_T seg_array = UArray_new(length, WORD);
        assert(seg_array);
        for (unsigned i = 0; i < length; i++) {
        uint32_t *word = (uint32_t *) UArray_at(seg_array, i);
        *word = 0;
    }
        
        Umsegment_Id seg_id = check_unmapped(seg->unmapped_ids);

        /* Segment id is a new id if there is no avaiable segment id */
        if (seg_id == 0) {
                seg_id = Seq_length(seg->segments);
                Seq_addhi(seg->segments, seg_array);
                // (seg->length)++;

        /* Otherwise we map to a reusable unmapped segment */
        } else {
                UArray_T old_array = Seq_put(seg->segments, seg_id, seg_array);
                UArray_free(&old_array);
        }
        return seg_id;
}



/*
 Segment_unmap: adds a segment Id to the list of unmapped segments
 Parameters: Seg_T seg, Umsegment_Id seg_id
 Returns: void
*/
void Segment_unmap(Seg_T seg, Umsegment_Id seg_id)
{
        assert(seg_id < (unsigned) Seq_length(seg->segments));
        assert(seg_id != 0);
        Seq_addhi(seg->unmapped_ids, (void *)(uintptr_t) seg_id);
}




/*
 Segment_at: returns a pointer to what's in the segments at segment segid
        and offset within the segment of offset
 Parameters: Seg_T seg, Umsegment_Id segid, unsigned offset
 Returns: void 
*/
void *Segment_at(Seg_T seg, Umsegment_Id segid, unsigned offset)
{
        assert(seg && seg->segments);
        assert(segid < (unsigned) Seq_length(seg->segments));

        UArray_T segment_array = (UArray_T) Seq_get(seg->segments, segid);
        return UArray_at(segment_array, offset);
}



/*
 Segment_free: Frees the Seg_T struct
 Parameters: Seg_T* seg
 Returns: void 
*/
void Segment_free(Seg_T *seg)
{
        assert(seg && *seg);

        /* Frees the uarrays within the sequence */
        for (int i = 0; i < Seq_length((*seg)->segments); i++) {
                UArray_T uarray = (UArray_T) Seq_get((*seg)->segments, i);
                assert(uarray);
                UArray_free(&uarray);
        }
        
        /* Frees the sequences and struct themselves */
        Seq_free(&((*seg)->segments));
        Seq_free(&((*seg)->unmapped_ids));
        free(*seg);
}



/*
 Segment_replace: replaces segment at id2 with segment at id1, but
        segment at id2 is lost
 Parameters: Seg_T seg, Umsegment_Id seg_id1, Umsegment_Id seg_id2
 Returns: void
*/
void Segment_replace(Seg_T seg, Umsegment_Id seg_id1, Umsegment_Id seg_id2)
{
        assert(seg && seg->segments);

        assert(seg_id1 < (unsigned) Seq_length(seg->segments));
        assert(seg_id2 < (unsigned) Seq_length(seg->segments));

        UArray_T uarray = (UArray_T) Seq_get(seg->segments, seg_id1);

        UArray_T prog_copy = UArray_copy(uarray, UArray_length(uarray));

        UArray_T orig_prog = Seq_put(seg->segments, seg_id2, prog_copy);

        UArray_free(&orig_prog);

        seg->length = UArray_length(prog_copy);
}



/*
 Segment_length: gets the length of segment at seg_id
 Parameters: Seg_T seg, Umsegment_Id seg_id
 Returns: unsigned
*/
unsigned Segment_length(Seg_T seg)
{
        assert(seg);

        // assert(seg_id < (unsigned) Seq_length(seg->segments));
        // UArray_T uarray = (UArray_T) Seq_get(seg->segments, seg_id);
        // return UArray_length(uarray);
        return seg->length;
}




/*
 check_unmapped: helper function to get the id value of 
        an unmapped segment. Returns 0 if there are no available ids.
 Parameters: Seq_T unmapped_ids
 Returns: Umsegment_Id
*/
static Umsegment_Id check_unmapped(Seq_T unmapped_ids)
{
        Umsegment_Id seg_id;

        if (Seq_length(unmapped_ids) == 0) {
                return 0;
        } else {
                seg_id = (uint32_t)(uintptr_t) Seq_remlo(unmapped_ids);
        }

        return seg_id;
}


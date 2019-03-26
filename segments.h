#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#ifndef SEGMENTS_INCLUDED
#define SEGMENTS_INCLUDED
#define T Seg_T
typedef struct T *T;

typedef uint32_t Umsegment_Id;

/* creates a new segment structure with segment of length "length" */
T Segment_new(unsigned length);

/* maps a segment of length "length" to the segment seg, returns seg_id */
Umsegment_Id Segment_map(T seg, unsigned length);

/* unmaps a segment of seg_id from the segment seg 
CRE for seg_id >= number of segments in seg struct
and for seg_id == 0 */
void Segment_unmap(T seg, Umsegment_Id seg_id);

/* retrieves thing stored in $m[seg_id][offset]
CRE for seg_id >= number of segments in seg struct
and for offset >= length of segment at seg_id */
void *Segment_at(T seg, Umsegment_Id seg_id, unsigned offset);

/* frees the segment struct */
void Segment_free(T *seg);

/* replaces segment at id2 with segment at id1, but segment at id2 is lost
CRE for seg_id1 or seg_id2 >= number of segments in seg struct */
void Segment_replace(T seg, Umsegment_Id seg_id1, Umsegment_Id seg_id2);

/* gets the length of segment at seg_id 
CRE for seg_id >= number of segments in seg struct */
unsigned Segment_length(T seg, Umsegment_Id seg_id);

#undef T
#endif
/*
 * Operating Systems  (2INC0)  Practical Assignment
 * Interprocess Communication
 *
 * Contains definitions which are commonly used by the farmer and the workers
 *
 */

#ifndef _COMMON_H_
#define _COMMON_H_

#include "settings.h"

// Request message queue structure
typedef struct
{
    int                     y;			// y row value
    bool					done;		// signal; stop working if true
} MQ_REQUEST_MESSAGE;

// Response message queue structure
typedef struct
{
    int                     y;			// y row value
    int                     v[X_PIXEL]; // array of pixel values
} MQ_RESPONSE_MESSAGE;


#endif


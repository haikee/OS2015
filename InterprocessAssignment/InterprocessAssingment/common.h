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


// TODO: put your definitions of the datastructures here
typedef struct
{
    // a data structure with 3 members
    int                     a; //y-value
    int                     b;//unused
    int                     c;//unused
} MQ_REQUEST_MESSAGE;

typedef struct
{
    // a data structure with 3 members, where the last member is an array with 3 elements
    int                     d; //y-value
    int                     e; //something...usefull??
    int                     f[X_PIXEL];//color
} MQ_RESPONSE_MESSAGE;


#endif


/*
 * Operating Systems  (2INC0)   Practical Assignment
 * Threaded Application
 *
 * STUDENT_NAME_1 (STUDENT_NR_1)
 * STUDENT_NAME_2 (STUDENT_NR_2)
 *
 * Grading:
 * Students who hand in clean code that fully satisfies the minimum requirements will get an 8.
 * ”Extra” steps can lead to higher marks because we want students to take the initiative.
 * Extra steps can be, for example, in the form of measurements added to your code, a formal
 * analysis of deadlock freeness etc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>     // for usleep()
#include <time.h>       // for time()
#include <pthread.h>
#include <math.h>
#include "prime.h"

static void rsleep (int t);

void initialize(void);

void printResult(void);
void findPrimes(void);
void* sieve(void *);
// create a bitmask where bit at position n is set
#define BITMASK(n)          (((long long) 1) << (n))

// check if bit n in v is set
#define BIT_IS_SET(v,n)     (((v) & BITMASK(n)) == BITMASK(n))

// set bit n in v
#define BIT_SET(v,n)        ((v) =  (v) |  BITMASK(n))

// clear bit n in v
#define BIT_CLEAR(v,n)      ((v) =  (v) & ~BITMASK(n))

// declare a mutex, and it is initialized as well
static pthread_mutex_t      mutex          = PTHREAD_MUTEX_INITIALIZER;

bool activeThreads[NROF_THREADS];
int main (void)
{
    // TODO: start threads generate all primes between 2 and NROF_SIEVE and output the results
    // (see thread_malloc_free_test() and thread_mutex_test() how to use threads and mutexes,
    //  see bit_test() how to manipulate bits in a large integer)
    initialize();
    findPrimes();
    printResult();

    return (0);
}
void initialize(void)
{
    int i;
    for(i=2; i<=NROF_SIEVE; i++)
    {
        BIT_SET(buffer[i/64], i%64);
    }
    for(i=0; i<NROF_THREADS; i++)
    {
        activeThreads[i]=false;
    }
}
void printResult(void)
{
    int i;
    for(i=2; i<=NROF_SIEVE; i++)
    {
        if(BIT_IS_SET(buffer[i/64], i%64))
        {
            printf("%d\n",i);
        }
    }
}

typedef struct
{
    int prime;
    int thread_id;
} SIEVE_PAIR;

void findPrimes(void)
{
    pthread_t threads[NROF_THREADS];
    int i;
    for(i=2; i<=(int)sqrt(NROF_SIEVE); i++)
    {
         pthread_mutex_lock (&mutex);
        if(BIT_IS_SET(buffer[i/64], i%64))
        {
            pthread_mutex_unlock (&mutex);
            int j;
            for(j=0; j<NROF_THREADS; j++)
            {
                if(!activeThreads[j])
                {
                    SIEVE_PAIR *       parameter;
                    parameter = malloc(sizeof(SIEVE_PAIR));
                    (*parameter).prime = i;
                    (*parameter).thread_id = j;

                    activeThreads[j]=true;
                    pthread_create(&threads[j],NULL,sieve, parameter);
                    pthread_detach(threads[j]);
                    break;
                }
                if(j==NROF_THREADS-1)
                {
                    j=-1;
                }
            }
            pthread_mutex_lock (&mutex);
        }
        pthread_mutex_unlock (&mutex);
        rsleep(100);
    }
    for(i=0;i<NROF_THREADS;i++)
    {
        if(activeThreads[i])
        {
            i=-1;
        }
    }
}
void * sieve(void * parameter)
{
    SIEVE_PAIR * pair = (SIEVE_PAIR *) parameter;
    int i=  (*pair).prime;
    int thread_id= (*pair).thread_id;
    free(parameter);
    int j;
    for(j=i*i; j<=NROF_SIEVE; j+=i)
    {
        pthread_mutex_lock (&mutex);
        BIT_CLEAR(buffer[j/64], j%64);
        pthread_mutex_unlock (&mutex);
        rsleep(100);
    }
    activeThreads[thread_id]=false;
    pthread_exit(NULL);
}
/*
 * rsleep(int t)
 *
 * The calling thread will be suspended for a random amount of time between 0 and t microseconds
 * At the first call, the random generator is seeded with the current time
 */
static void rsleep (int t)
{
    static bool first_call = true;

    if (first_call == true)
    {
        srandom (time (NULL) % getpid());
        first_call = false;
    }
    usleep (random () % t);
}


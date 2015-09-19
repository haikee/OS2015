/*
 * Operating Systems  (2INC0)  Practical Assignment
 * Interprocess Communication
 *
 * Haike Franssen (0864114)
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
#include <string.h>
#include <errno.h>		  // for perror()
#include <unistd.h>		 // for getpid()
#include <mqueue.h>		 // for mq-stuff
#include <time.h>		   // for time()
#include <complex.h>

#include "settings.h"
#include "common.h"

// get the attribute of the message queue
struct mq_attr getattr(mqd_t mq_fd)
{
	struct mq_attr	attr;
	int				rtnval;
	
	rtnval = mq_getattr(mq_fd, &attr);
	if (rtnval == -1)
	{
		perror("mq_getattr() failed");
		exit (1);
	}

	return attr;
}

static void rsleep (int t);

static double
complex_dist (complex a)
{
	// distance of vector 'a'
	// (in fact the square of the distance is computed...)
	double re, im;

	re = __real__ a;
	im = __imag__ a;
	return ((re * re) + (im * im));
}

static int
mandelbrot_point (double x, double y)
{
	int	 k;
	complex z;
	complex c;

	z = x + y * I;	 // create a complex number 'z' from 'x' and 'y'
	c = z;

	for (k = 0; (k < MAX_ITER) && (complex_dist (z) < INFINITY); k++)
	{
		z = z * z + c;
	}

	//									2
	// k >= MAX_ITER or | z | >= INFINITY

	return (k);
}


int main (int argc, char * argv[])
{
	// TODO:
	// (see message_queue_test() in interprocess_basic.c)
	//  v open the two message queues (whose names are provided in the arguments)
	//  * repeatingly:
	//	  - read from a message queue the new job to do
	//	  - wait a random amount of time (e.g. rsleep(10000);)
	//	  - do that job (use mandelbrot_point() if you like)
	//	  - write the results to a message queue
	//	until there are no more jobs to do
	//  * close the message queues

	if (argc != 2) {
		printf("Wrong number of arguments.\n");
		exit(0);
	}
	
	mqd_t				mq_fd_request;	/* request message queue farmer -> worker */
	mqd_t				mq_fd_response; /* response message queue worker -> farmer */
	MQ_REQUEST_MESSAGE	req;			/* request message */
	MQ_RESPONSE_MESSAGE	rsp;			/* response message */
	
	// open message queues
	mq_fd_request = mq_open(argv[0], O_RDONLY);
	mq_fd_response = mq_open(argv[1], O_WRONLY);
	
	bool hasjob = true;
	while (hasjob)
	{
		// sleep a random amount of time
		rsleep(1000);
		
		// read a message from the message queue
		// this blocks the execution until the worker receives a message
		printf("worker: waiting for message...\n");
		mq_receive(mq_fd_request, (char *) &req, sizeof (req), NULL);
		// debug
		printf("worker: message received...\n");
		
		// if the message is a signal that the worker can finish..
		if (req.done) {
			//..break out of the loop
			break;
		}
		
		// build response message
		rsp.y = req.y;
		
		int x;
		for (x = 0; x < X_PIXEL; x += 1) {
			rsp.v[x] = mandelbrot_point(X_LOWERLEFT + ((double) x) * STEP, 
					Y_LOWERLEFT + ((double) req.y) * STEP);
		}
		
		printf("worker: sending response...\n");
		mq_send(mq_fd_response, (char *) &rsp, sizeof (rsp), 0);
		printf("worker: sent\n");
	}

	printf("worker: closing message queues\n");

	// close message queues
	mq_close(mq_fd_response);
	mq_close(mq_fd_request);
	
	printf("worker: stopped\n");
	
	return 0;
}

/*
 * rsleep(int t)
 *
 * The calling thread will be suspended for a random amount of time
 * between 0 and t microseconds
 * At the first call, the random generator is seeded with the current time
 */
static void rsleep (int t)
{
	static bool first_call = true;

	if (first_call == true)
	{
		srandom (time(NULL) % getpid());
		first_call = false;
	}
	usleep (random () % t);
}



/*
 * Operating Systems  (2INC0)  Practical Assignment
 * Interprocess Communication
 *
 * Haike Franssen (0864114)
 * Jochem Kuijpers (0838617)
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
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>		 // for execlp
#include <mqueue.h>		 // for mq

#include "settings.h"
#include "output.h"
#include "common.h"

#define STUDENT_NAME "Haike_Franssen_Jochem_Kuijpers"

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

int main (int argc, char * argv[])
{
	// Initialize output code
	output_init ();

	// message queue names
	static char	mq_name1[80];
	static char	mq_name2[80];


	// TODO:
	//  V create the message queues (see message_queue_test() in interprocess_basic.c)
	//  V create the child processes (see process_test() and message_queue_test())
	//  V do the farming (use output_draw_pixel() for the coloring)
	//  V wait until the chilren have been stopped (see process_test())
	//  V clean up the message queues (see message_queue_test())

	// Important notice: make sure that the names of the message queues contain your
	// student name and the process id (to ensure uniqueness during testing)

	pid_t				processID;		/* process ID from fork() */
	mqd_t				mq_fd_request;	/* request message queue farmer -> worker */
	mqd_t				mq_fd_response; /* response message queue worker -> farmer */
	MQ_REQUEST_MESSAGE	req;			/* request message */
	MQ_RESPONSE_MESSAGE	rsp;			/* response message */
	struct mq_attr		attr;			/* message queue attributes */

	// create names for the message queues based on STUDENT_NAME and the PID of the farmer
	sprintf(mq_name1, "/mq_request_%s_%d", STUDENT_NAME, getpid());
	sprintf(mq_name2, "/mq_response_%s_%d", STUDENT_NAME, getpid());

	// create the request message queue
	attr.mq_maxmsg  = MQ_MAX_MESSAGES;
	attr.mq_msgsize = sizeof (MQ_REQUEST_MESSAGE);
	mq_fd_request = mq_open(mq_name1, O_WRONLY | O_CREAT | O_EXCL, 0600, &attr);

	// create the response message queue
	attr.mq_maxmsg  = MQ_MAX_MESSAGES;
	attr.mq_msgsize = sizeof (MQ_RESPONSE_MESSAGE);
	mq_fd_response = mq_open(mq_name2, O_RDONLY | O_CREAT | O_EXCL | O_EXCL, 0600, &attr);

	// fork worker processes and assign them the worker program
	int i;
	for (i = 0; i < NROF_WORKERS; i += 1) 
	{		
		printf("parent pid: %d\n", getpid());
		processID = fork();

		if (processID < 0)
		{
			perror("fork() failed");
			exit (1);
		}
		else
		{
			if (processID == 0)
			{
				printf("child pid: %d\n", getpid());
				execlp("./worker", mq_name1, mq_name2 , NULL);
				// we should never arrive here...
				perror("execlp() failed");
			}
			// else: we are still the parent (which continues this program)
		}
	}

	int nrows = 0; 		/* numbers of rows we received from the workers */
	int yrequest = 0;	/* the next row we're going to request */

	// we're done after we have received Y_PIXEL rows
	while (nrows < Y_PIXEL)
	{
		// handle responses from workers
		while (getattr(mq_fd_response).mq_curmsgs > 0)
		{
			// read a message from the message queue
			printf("farmer: receiving message...\n");
			mq_receive(mq_fd_response, (char *) &rsp, sizeof (rsp), NULL);

			// debug; print the first 3 values
			printf("farmer: received: %d, [%d, %d, %d, ...]\n",
					rsp.y, rsp.v[0], rsp.v[1], rsp.v[2]);

			int x;
			for (x = 0; x < X_PIXEL; x += 1)
			{
				// set pixel (x, rsp.y) to colour rsp.v[q]
				output_draw_pixel(x, rsp.y, rsp.v[x]);
			}
			
			// another row done
			nrows += 1;
		}

		// request more work from the workers
		while (getattr(mq_fd_request).mq_curmsgs < MQ_MAX_MESSAGES
				&& yrequest < Y_PIXEL)
		{			
			// set the request message
			req.y = yrequest;
			req.done = false;
			
			// send the message to the message queue
			printf("farmer: sending...\n");
			mq_send(mq_fd_request, (char *) &req, sizeof (req), 0);

			printf("farmer: sent\n");
			
			yrequest += 1;
		}
	}
	
	printf("farmer: sending stop messages...\n");
	
	// send every worker a signal that they have to stop
	int n;
	for (n = 0; n < NROF_WORKERS; n += 1) {
		while (getattr(mq_fd_request).mq_curmsgs >= MQ_MAX_MESSAGES) {}
		
		req.done = true;
		mq_send(mq_fd_request, (char *) &req, sizeof (req), 0);
	}
	
	// close the message queues and unlink them
	printf("farmer: closing message queues...\n");
	mq_close(mq_fd_response);
	mq_close(mq_fd_request);
	mq_unlink(mq_name1);
	mq_unlink(mq_name2);

	// stop displaying
	output_end();
	
	printf("farmer: done\n");

	return 0;
}

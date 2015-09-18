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
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>         // for execlp
#include <mqueue.h>         // for mq

#include "settings.h"
#include "output.h"
#include "common.h"



int main (int argc, char * argv[])
{
    if (argc != 1)
    {
        fprintf (stderr, "%s: invalid arguments\n", argv[0]);
    }

    output_init ();

    #define STUDENT_NAME "Haike_Franssen";
    static char                 mq_name1[80];
    static char                 mq_name2[80];


    // TODO:
    //  * create the message queues (see message_queue_test() in interprocess_basic.c)
    //  * create the child processes (see process_test() and message_queue_test())
    //  * do the farming (use output_draw_pixel() for the coloring)
    //  * wait until the chilren have been stopped (see process_test())
    //  * clean up the message queues (see message_queue_test())

    // Important notice: make sure that the names of the message queues contain your
    // student name and the process id (to ensure uniqueness during testing)


    pid_t               processID;      /* Process ID from fork() */
    mqd_t               mq_fd_request;
    mqd_t               mq_fd_response;
    MQ_REQUEST_MESSAGE  req;
    MQ_RESPONSE_MESSAGE rsp;
    struct mq_attr      attr;

    sprintf (mq_name1, "/mq_request_%s_%d", STUDENT_NAME, getpid());
    sprintf (mq_name2, "/mq_response_%s_%d", STUDENT_NAME, getpid());

    attr.mq_maxmsg  = 10;
    attr.mq_msgsize = sizeof (MQ_REQUEST_MESSAGE);
    mq_fd_request = mq_open (mq_name1, O_WRONLY | O_CREAT | O_EXCL, 0600, &attr);

    attr.mq_maxmsg  = 10;
    attr.mq_msgsize = sizeof (MQ_RESPONSE_MESSAGE);
    mq_fd_response = mq_open (mq_name2, O_RDONLY | O_CREAT | O_EXCL, 0600, &attr);

    getattr(mq_fd_request);
    getattr(mq_fd_response);

    //note to self creating children?

    pid_t           processID;      /* Process ID from fork() */

    printf ("parent pid:%d\n", getpid());
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
            printf ("child  pid:%d\n", getpid());
            //execlp ("ps", "ps", "-l", NULL);
            //execlp ("./c_program", "my_own_name_for_argv0", "first_argument", NULL);
            execlp ("worker.c", "?", "-l", NULL)
            // we should never arrive here...
            perror ("execlp() failed");
        }
        // else: we are still the parent (which continues this program)

        waitpid (processID, NULL, 0);   // wait for the child ?
        printf ("child %d has been finished\n\n", processID);
    }

    //note to self using message que?

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
            // child-stuff
            message_queue_child ();
            exit (0);
        }
        else
        {
            // remaining of the parent stuff

            // fill request message
            req.a = 88;
            req.b = 888;
            req.c = 8888;

            sleep (3);
            // send the request
            printf ("parent: sending...\n");
            mq_send (mq_fd_request, (char *) &req, sizeof (req), 0);

            sleep (3);
            // read the result and store it in the response message
            printf ("parent: receiving...\n");
            mq_receive (mq_fd_response, (char *) &rsp, sizeof (rsp), NULL);

            printf ("parent: received: %d, %d, [%d,%d,%d]\n",
                    rsp.d, rsp.e, rsp.f[0], rsp.f[1], rsp.f[2]);

            sleep (1);

            waitpid (processID, NULL, 0);   // wait for the child

            mq_close (mq_fd_response);
            mq_close (mq_fd_request);
            mq_unlink (mq_name1);
            mq_unlink (mq_name2);
        }
    }

    //note to self

    output_end();

    return (0);
}


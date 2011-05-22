/**
 * @file challenge.h
 * @brief Code Challenge Declarations
 */

#ifndef __CHALLENGE_H__
#define __CHALLENGE_H__

#include <sys/types.h>

#define NUM_CHILDREN 10
#define FILENAME_BUFFER_SIZE 255
#define FILENAME_LOCATION "." 
#define FIFO_BUFFER_SIZE 255
#define FIFO_NAME "/tmp/vccc_fifo"
#define RESULT_MAX_SIZE 30 

typedef enum {
  SUCCESS = 0,
  FAILURE = 1
} status_t;

/*
 * parent functions
 */

/**
 * wrapper around the child process stuff
 *
 */
void run_child();

/**
 * open a pipe for reading child status
 *
 * @return the file descriptor associated with the pipe
 */
int open_pipe(); 

/**
 * wait for children to exit
 *
 * @param pids an array of pids
 * @param size_pids size of the pid array
 *
 * @return the number of child processes waited for
 */
int await_children(pid_t pids[], int size_pids);

/**
 * reads the results from the pipe and populates the array of results
 *
 * @param fd the file descriptor of the pipe from which to read
 * @param results the results array in which to copy the child statuses
 *
 * @return the number of results read from the pipe
 */
int read_results(int fd, char results[][RESULT_MAX_SIZE]);

/**
 * strcmp wrapper for qsort
 *
 */
static int compare_results(const void *a, const void *b);

/*
 * child functions
 */

/**
 * writes the pid to a file 
 *
 * @param pid the current pid
 *
 * @return 0 on success
 */
int write_pid_to_file(pid_t pid);

/**
 * writes the process status to the pipe
 * 
 * @param pid the current pid
 * @param status the status of the current child (SUCCESS or FAILURE)
 *
 * @return returns 0 on success
 */
int write_status_to_pipe(pid_t pid, status_t status);

#endif

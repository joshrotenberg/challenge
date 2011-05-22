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

/*
 * parent functions
 */
int run_child();
int open_pipe(); 
int await_children(pid_t pids[], int size_pids);
int read_results(int fd, char results[][RESULT_MAX_SIZE]);

static int compare_results(const void *a, const void *b);

/*
 * child functions
 */
int write_status_to_file(pid_t pid);
int write_status_to_pipe(pid_t pid, const char* status);

#endif

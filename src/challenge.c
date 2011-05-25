#include "challenge.h"

#include <stdio.h>
#include <time.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

static const char* kSuccess = "SUCCESS";
static const char* kFailure = "FAILURE";

int main(int argc, char** argv) {

  int fd; /* fifo pipe fd */
  int i, j;
  int num_reported;
  size_t results_length;
  pid_t pids[NUM_CHILDREN]; /* array of child pids */ 
  char results[NUM_CHILDREN][30]; /* array to store results */

  /* 
   * open up our pipe for status messages from child processes
   */ 
  if( (fd = open_pipe()) < 0) {
    exit(-1);
  }

  /*
   * start up the children, and hang on to the pids
   */
  for (i = 0; i < NUM_CHILDREN; ++i) {
    pid_t pid = fork();
    pids[i] = pid;

    if(pid == -1) {
      perror("Error forking child process");
      exit(-1);
    }
    else if(pids[i] == 0) {
      run_child();
    }
  }

  /*
   * wait for the child processes to complete their task
   */
  if(await_children(pids, NUM_CHILDREN) != NUM_CHILDREN) {
    fprintf(stderr, "Child count doesn't match number forked\n");
  }

  /*
   * read the results from the pipe
   */
  num_reported = read_results(fd, pids, results, NUM_CHILDREN);
  if(num_reported != NUM_CHILDREN) {
    fprintf(stderr, "Not all children reported results\n");
  }

  /* done with the pipe. close and unlink it. */
  close(fd);
  unlink(FIFO_NAME);

  /* 
   * sort results
   */
  results_length = sizeof(results) / sizeof(results[0]);
  qsort(&results, results_length, sizeof(results[0]), compare_results);

  /* report the results to stdout. we've left the \n on the end. */
  for(j = 0; j < results_length; j++) {
    printf("%s", results[j]);
  }

  return 0;
}

void run_child() {
  pid_t child_pid = getpid();
  int rv;
  status_t status = SUCCESS; /* optimistic */

  /* treat pid file writing as non fatal .. */
  if( (rv = write_pid_to_file(child_pid)) != 0) {
    fprintf(stderr, "Failed to write pid to file for child %d\n", child_pid);
    status = FAILURE;
  }
  
  /* but not writing the status is fatal */
  if( (rv = write_status_to_pipe(child_pid, status)) != 0) {
    fprintf(stderr, "Failed to write status for child %d\n", child_pid);
    _exit(1);
  }

  _exit(0);
}

int open_pipe() {
  int fd, rv;

  rv = mkfifo(FIFO_NAME, 0666);
  if ((rv == -1) && (errno != EEXIST)) {
    perror("Error creating the named pipe");
    return rv;
  }

  /*
   * open up the pipe
   */
  if( (fd = open(FIFO_NAME, O_RDONLY | O_NDELAY) ) < 0) {
    perror("Error opening pipe for reading");
    return fd;
  }

  return fd;
}

int await_children(const pid_t pids[], const int size_pids) {
  pid_t pid;
  int status, i;
  
  for(i = 0; i < size_pids; i++) {
    pid = waitpid(pids[i], &status, 0);
    if(status != 0) {
      fprintf(stderr, "Warning: child %ld exited with status %d\n",
	      (long)pid, status);
    }
  }
  
  return i;
}

int read_results(const int fd, const pid_t pids[], char results[][30], 
		 const int expected) {
  FILE* f;
  int count = 0;
  f = fdopen(fd, "r");

  /* 
   * read all of the results from pipe. they are handily newline delimited,
   * so fgets does the job of splitting them up.
   */

  
  for(count = 0; count < expected; count++) {
    /* 
     * if we don't have any more data but we haven't met the expected quota
     * someone didn't report. break out and it will be handled below 
     */
    if(fgets(results[count], 30, f) == NULL) {
      fprintf(stderr, "Expected %d but only found %d results\n",
	      expected, count);
      break;
    }
  }

  /* 
   * if the count doesn't match, one or more children didn't write their 
   * results for some unknown reason. add them as failures.
   */
  if(count < expected) {
    int i;

    /* iterate through our known pids */
    for(i = 0; i < expected; i++) {
      int j;
      char spid[24];
      int found = 0;
      sprintf(spid, "%d", pids[i]);

      /* and search the results to see if this pid is present. */
      for(j = 0; j < count; j++) {
	if( strncmp(results[j], spid, strlen(spid)) == 0) {
	  found = 1;
	  break;
	}
      }

      /* didn't find it. add it to the results as a failure. */
      if(found == 0)
	sprintf(results[count++], "%s %s\n", spid, kFailure); 
      
    }
  }
  return count;
}

int write_pid_to_file(const pid_t child_pid) {
  FILE *file;
  char filename[FILENAME_BUFFER_SIZE];
  time_t current_time = time(NULL);

  snprintf(filename, FILENAME_BUFFER_SIZE, "%s/%d.%ld", 
	   FILENAME_LOCATION, child_pid, (long)current_time);
  
  file = fopen(filename, "w+"); 
  if(file == NULL) {
    perror("Error opening status file");
    return 1;
  }

  if( (fprintf(file, "%ld", (long)child_pid)) < 1) {
    perror("Error printing pid to status file");
    return 1;
  }

  fclose(file);
  return 0;
}

int write_status_to_pipe(const pid_t child_pid, const status_t status) {
  char result[RESULT_MAX_SIZE];
  int fd, printed;

  /* XXX: simulate failures */
  /*
  if(child_pid % 3 == 0)
    return 1;
  */

  if( (fd = open(FIFO_NAME, O_WRONLY) ) < 0) {
    perror("Error opening pipe for writing");
    return 1;
  }

  printed = sprintf(result, "%d %s\n", child_pid, 
		    status == SUCCESS ? kSuccess : kFailure);  

  if(write(fd, result, strlen(result)) < printed) {
    perror("Error writing status to pipe");
    close(fd);
    return 1;
  }

  close(fd);
  return 0;
}

static int compare_results(const void *a, const void *b) {
  const char* sa = (const char *)a;
  const char* sb = (const char *)b;

  return strcmp(sa, sb);
}

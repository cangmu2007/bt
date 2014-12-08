#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <time.h>

static int fd;

int openlog(char* path);

int writelog(char* msg);

int closelog();

char* gettime();
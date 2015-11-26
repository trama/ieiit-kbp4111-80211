#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<fcntl.h>
#include<string.h>
#include<stdint.h>
#include <signal.h>

#include <endian.h>

static double mean_time = 0;
static int pck = 0;

typedef struct {
    unsigned long long start;
	unsigned long long stop;
	int optimized;
} data_opt_t;

void intHandler(int dummy) {
	printf("packets: %i\n", pck);
    printf("MEAN ELABORATION TIME: %f\n", mean_time);
    exit(0);
}

int main(void) {
   int ret, fd;
   int flags;
   data_opt_t dataOPT;
   int i;
   int dropped = 0;
   char tmp;
   uint64_t id;
   double freq = 1200;
   double elapsed;
   signal(SIGINT, intHandler);
   
   fd = open("/dev/optlogdev", O_RDONLY);
	if (fd < 0){
	  perror("Failed to open the device...");
	  return errno;
	} 
   
   /* Set read no blocking */
   fcntl(fd, F_GETFL, 0);
   fcntl(fd, F_SETFL, flags | O_NONBLOCK);
   
   while (1) {
     ret = read(fd, (char*) &dataOPT, sizeof(data_opt_t));
     if (ret == -1 && errno==EAGAIN) {
       //printf("EMPTY\n");
     } else if (ret < 0 && errno!=EAGAIN) {
       perror("Failed to read the message from the device.");
       return errno;
     } else {
       elapsed = (dataOPT.stop - dataOPT.start)/freq;
       mean_time = (((mean_time * pck) + (elapsed)) / (++pck));
       printf("Start: %llu, stop: %llu, elapsed: %f, optimized: %i\n", dataOPT.start, dataOPT.stop, elapsed, dataOPT.optimized);
     }
     
     //sleep(1);
   }

   return 0;
}

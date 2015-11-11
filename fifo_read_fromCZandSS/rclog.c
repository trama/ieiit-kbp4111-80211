#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<fcntl.h>
#include<string.h>
#include<stdint.h>

#include <endian.h>

typedef struct {
    int rate_idx [4];
	int rate_count [4];
	int success;
	int probe;
} data_rc_t;

static __inline__ unsigned long long rdtsc(void) {
    unsigned hi, lo;
    __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
    return ( (unsigned long long)lo)|( ((unsigned long long)hi)<<32 );
}

int main(void) {
   int ret, fd;
   int flags;
   data_rc_t dataRC;
   int i;
   int dropped = 0;
   char tmp;
   uint64_t id;
   
   fd = open("/dev/rclogdev", O_RDONLY);
	if (fd < 0){
	  perror("Failed to open the device...");
	  return errno;
	} 
   
   /* Set read no blocking */
   fcntl(fd, F_GETFL, 0);
   fcntl(fd, F_SETFL, flags | O_NONBLOCK);
   
   while (1) {
     ret = read(fd, (char*) &dataRC, sizeof(data_rc_t));
     if (ret == -1 && errno==EAGAIN) {
       //printf("EMPTY\n");
     } else if (ret < 0 && errno!=EAGAIN) {
       perror("Failed to read the message from the device.");
       return errno;
     } else {
       
       for (i=0; i<4; i++) 
       {
	       printf("%i ", dataRC.rate_idx[i]);
	       printf("%i ", dataRC.rate_count[i]);
       }
       printf(" Succ: %i Probe: %i\n", dataRC.success, dataRC.probe);
     }
     
     //sleep(1);
   }

   return 0;
}

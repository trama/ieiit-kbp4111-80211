#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<fcntl.h>
#include<string.h>
#include<stdint.h>
#include <signal.h>
#include <endian.h>

static unsigned long rates[8] = {0,0,0,0,0,0,0,0};
static unsigned long attempts[8] = {0,0,0,0,0,0,0,0};
static FILE* fr;
static FILE* fa;

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

void intHandler(int dummy) {
	FILE* fr;
	FILE* fa;
	int i;
	
	if ((fr=fopen("log_rates","w"))==NULL) {
		printf("Impossibile aprire il file di log!!\n");
        exit(0);
    }
    chmod("log_rates",0666);
	for (i = 0; i<8; i++) 
		fprintf(fr,"%i %lu\n",i,rates[i]);
	fclose(fr);
	
	if ((fa=fopen("log_attempts","w"))==NULL) {
		printf("Impossibile aprire il file di log!!\n");
        exit(0);
    }
    chmod("log_attempts",0666);
	for (i = 0; i<8; i++) 
		fprintf(fa,"%i %lu\n",i,attempts[i]);
	fclose(fa);
	
    exit(0);
}

int main(void) {
   int ret, fd;
   int flags;
   data_rc_t dataRC;
   int i, att;
   int dropped = 0;
   char tmp;
   uint64_t id;
   
   signal(SIGINT, intHandler);
   
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
       
       att = 0;
       for (i=0; i<4; i++) 
       {
	       printf("%i ", dataRC.rate_idx[i]);
	       printf("%i ", dataRC.rate_count[i]);
	       if (dataRC.rate_idx[i] != -1) {
				rates[dataRC.rate_idx[i]] = rates[dataRC.rate_idx[i]] + 1;
				att += dataRC.rate_count[i];
		   }
       }
       attempts[att-1] = attempts[att-1] + 1;
       printf(" Succ: %i Probe: %i\n", dataRC.success, dataRC.probe);
     }
     
     //sleep(1);
   }

   return 0;
}

#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<fcntl.h>
#include<string.h>
#include<stdint.h>
#include <signal.h>

#include <endian.h>

static double mean_snr = 0;
static int pck = 0;
static int discard = 0;

typedef struct {
    uint64_t tsc;
    char id[8];
    char rx_signal;
    char rx_noise;
    char tx_signal;
    char tx_noise;
    char txrxaddr[38];
} data_sched_t;

static __inline__ unsigned long long rdtsc(void) {
    unsigned hi, lo;
    __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
    return ( (unsigned long long)lo)|( ((unsigned long long)hi)<<32 );
}

void intHandler(int dummy) {
	printf("packets: %i, discarded: %i\n", pck,discard);
	printf("PER: %f\n", (double) pck/(1000-discard));
    printf("MEAN SNR: %f\n", mean_snr);
    exit(0);
}

int main(void) {
   int ret, fd;
   int flags;
   data_sched_t data;
   int i;
   int dropped = 0;
   char tmp;
   uint64_t id;
   uint64_t snr;

   signal(SIGINT, intHandler);

   fd = open("/dev/raieiitdev", O_RDONLY);
    if (fd < 0){
      perror("Failed to open the device...");
      return errno;
    }
   
  
   /* Set read no blocking */
   fcntl(fd, F_GETFL, 0);
   fcntl(fd, F_SETFL, flags | O_NONBLOCK);
   
   while (1) {
     ret = read(fd, (char*) &data, sizeof(data_sched_t));
     if (ret == -1 && errno==EAGAIN) {
       //printf("EMPTY\n");
     } else if (ret < 0 && errno!=EAGAIN) {
       perror("Failed to read the message from the device.");
       return errno;
     } else {
	   char expected_id[8] = "IEIIT-RA";
	   int match = 1;
	   for (i=0; i<8; i++) {
			if (data.id[i]!=expected_id[i]) {
				match = 0;
				break;
			}
		}
	   if (match) {
		 snr = data.rx_signal - data.rx_noise;
		 if (snr < 50) {
			mean_snr = (((mean_snr * pck) + (snr)) / (++pck));
			
		 }
		 else {
			discard++;
		 }
	   }
	   
       //if (data.success == 0) dropped++;
       /* From little endian to uint64_t */
       //memcpy(&id, data.id, 8);
       //id = le64toh(id);
       
       //printf("RECEIVED: %llu -> %llu, ID=", data.tsc, rdtsc() );
       //for (i=0; i<8; i++) printf("%c", data.id[i]);
       //printf(", TX: Signal(dBm)=%d, Noise(dBm)=%d, ", data.tx_signal, data.tx_noise);
       printf("RX: Signal(dBm)=%d, Noise(dBm)=%d, SNR(dB)=%d, %s\n", data.rx_signal, data.rx_noise, data.rx_signal - data.rx_noise, data.txrxaddr);
     
     }
     
     //sleep(1);
   }

   return 0;
}

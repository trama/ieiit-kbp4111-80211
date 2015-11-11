#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<fcntl.h>
#include<string.h>
#include<stdint.h>

#include <endian.h>

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

int main(void) {
   int ret, fd;
   int flags;
   data_sched_t data;
   int i;
   int dropped = 0;
   char tmp;
   uint64_t id;

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
       //if (data.success == 0) dropped++;
       /* From little endian to uint64_t */
       //memcpy(&id, data.id, 8);
       //id = le64toh(id);
       
       printf("RECEIVED: %llu -> %llu, ID=", data.tsc, rdtsc() );
       for (i=0; i<8; i++) printf("%c", data.id[i]);
       printf(", TX: Signal(dBm)=%d, Noise(dBm)=%d", data.tx_signal, data.tx_noise);
       printf(", RX: Signal(dBm)=%d, Noise(dBm)=%d, %s\n", data.rx_signal, data.rx_noise, data.txrxaddr);
     }
     
     //sleep(1);
   }

   return 0;
}

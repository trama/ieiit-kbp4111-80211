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
  int success;
} data_sched_t;


int main(void){
   int ret, fd;
   int flags;
   data_sched_t data;
   int i;
   int dropped = 0;
   char tmp;
   uint64_t id;

   fd = open("/dev/schedchar", O_RDONLY);
  if (fd < 0){
      perror("Failed to open the device...");
      return errno;
   }
   
   /* Set read no blocking */
   fcntl(fd, F_GETFL, 0);
   fcntl(fd, F_SETFL, flags | O_NONBLOCK);

   while (1) {
     ret = read(fd, (char*) &data, sizeof(data_sched_t));        // Read the response from the LKM
     
     if (ret == -1 && errno==EAGAIN) {
       //printf("EMPTY\n");
     } else if (ret < 0 && errno!=EAGAIN) {
       perror("Failed to read the message from the device.");
       return errno;
     } else {
       if (data.success == 0) dropped++;
       printf("RECEIVED: %llu ", data.tsc);

       /* From little endian to uint64_t */
       memcpy(&id, data.id, 8);
       id = le64toh(id);

       /* for (i=0; i<8; i++) printf("%x ", data.id[i]&0xff); */
       printf("%llu ", id);
       printf("%d %d\n", data.success, dropped);
     }
     //sleep(1);
   }

   return 0;
}

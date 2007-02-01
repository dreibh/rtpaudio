#include <tdsocket.h>
#include <thread.h>

int main (int argc, const char * argv[])
{
   InternetAddress local("127.0.0.1:8888");
   InternetAddress adr("127.0.0.1:1234");
   Socket sock(AF_INET,SOCK_STREAM,IPPROTO_SCTP);

   sock.bind(local);
   if(sock.connect(adr)) {
      char buf[10000];
      ssize_t x;

      int s0=5000;
      for(int i=0;i<s0;i++) {
         buf[i] = 'X';
      }
/*
      for(int i=0;i<5;i++) {
         buf[i] = 'Y';
      }
*/
      ssize_t y = sock.write((char*)&buf,s0);

      printf("write=%d\n",y);

      Thread::delay(1000000);

   }
   else {
      puts("FAILED!");
   }
}

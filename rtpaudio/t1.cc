#include <tdsocket.h>

using namespace Coral;

int main (int argc, const char * argv[])
{
   Socket sock(AF_INET,SOCK_STREAM,IPPROTO_SCTP);
   InternetAddress local("127.0.0.1:1234");

   if(!sock.bind(local)) {
      puts("ERR!");
      exit(0);
   }
   puts("listen...");
   sock.listen();
   Socket* s = sock.accept();
   puts("accepted!");

   char buf[10000];
   ssize_t x;

   int s0=5;

   x = s->read((char*)&buf,s0);

   printf("got %d\n",x);

   x = s->read((char*)&buf,sizeof(buf));

   printf("got %d\n",x);

   for(int i=0;i<x;i++) {
      if(buf[i] != 'X') {
         printf("Fehler: %d -> %d\n",i,buf[i]);
      }
   }

}

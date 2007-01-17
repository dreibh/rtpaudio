#include "internetaddress.h"
#include "tdsocket.h"

using namespace Coral;

int main(int argc, char** argv)
{
   Socket sock(Socket::IP,SOCK_STREAM,IPPROTO_SCTP);
   InternetAddress local(1234);
   if(sock.bind(local) == true) {
      sock.listen(10);
      Socket* sd = sock.accept();
      while(sd != NULL) {
         printf("Accept: %x\n",sd);

         char buffer[10000];
         puts("read()...");
         ssize_t r = sd->read((char*)&buffer,100);
         printf("read()=%d error=%d\n",r,sd->getLastError());

         delete sd;
         sd = sock.accept();
      }
   }
}

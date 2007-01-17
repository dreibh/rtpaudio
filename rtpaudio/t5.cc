#include <timedthread.h>
#include <tools.h>
#include <ringbuffer.h>
#include <thread.h>

using namespace Coral;


/*
class RingBuffer : public Synchronizable
{
   public:
   RingBuffer();
   ~RingBuffer();

   bool init(const cardinal bytes);
   void flush();

   ssize_t write(char*        data,
                 const size_t length);
   ssize_t read(char*        data,
                const size_t length);

   size_t dataAvailable();

   private:
   char*    Buffer;
   size_t   BufferSize;
   size_t   WriteStart;
   size_t   WriteEnd;
   size_t   Available;
};


// Debug modes
#define DEBUG
#define DEBUG_VERBOSE


// ###### Constructor #######################################################
RingBuffer::RingBuffer()
   : Synchronizable("RingBuffer")
{
   Buffer = NULL;
   flush();
}


// ###### Destructor ########################################################
RingBuffer::~RingBuffer()
{
   if(Buffer != NULL) {
      delete Buffer;
      Buffer = NULL;
   }
}


// ###### Initialize buffer #################################################
bool RingBuffer::init(const cardinal bytes)
{
   synchronized();
   flush();
   if(Buffer != NULL) {
      delete Buffer;
   }
   Buffer = new char[bytes + 16];
   Buffer[bytes]=0x00;
   bool ok;
   if(Buffer == NULL) {
      ok         = false;
      BufferSize = 0;
   }
   else {
      ok         = true;
      BufferSize = bytes;
   }
   unsynchronized();
   return(ok);
}


// ###### Flush buffer ######################################################
void RingBuffer::flush()
{
   synchronized();
   WriteStart   = 0;
   WriteEnd     = 0;
   Available    = 0;
   WriteStart = 0;
   unsynchronized();
}


// ###### Get number of bytes available for read ############################
size_t RingBuffer::dataAvailable()
{
   synchronized();
   const size_t available = Available;
   synchronized();
   return(available);
}


// ###### Write bytes into buffer ###########################################
ssize_t RingBuffer::write(char*        data,
                          const size_t length)
{
   synchronized();

   cardinal copy1 = 0;
   cardinal copy2 = 0;
   if(Available < BufferSize) {
      if(WriteEnd >= WriteStart) {
         copy1 = min(length, BufferSize - WriteEnd);
         memcpy(&Buffer[WriteEnd],data,copy1);
         WriteEnd += copy1;
         if(WriteEnd >= BufferSize) {
            WriteEnd = 0;
         }
#ifdef DEBUG_VERBOSE
         printf("write #1: we=%d ws=%d   c1=%d\n",WriteEnd,WriteStart,copy1);
#endif
      }
      copy2 = min(length - copy1,
                  WriteStart);
      if(copy2 > 0) {
         memcpy(&Buffer[WriteEnd],&data[copy1],copy2);
         WriteEnd += copy2;
#ifdef DEBUG_VERBOSE
         printf("write #2: we=%d ws=%d   c2=%d\n",WriteEnd,WriteStart,copy2);
#endif
      }

      Available += copy1 + copy2;
   }

#ifdef DEBUG
   printf("write: we=%d ws=%d   c1=%d c2=%d  available=%d/%d\n",
          WriteEnd,WriteStart,copy1,copy2,Available,BufferSize);
#endif

   unsynchronized();
   return(copy1 + copy2);
}


// ###### Read bytes from buffer ############################################
ssize_t RingBuffer::read(char*        data,
                         const size_t length)
{
   synchronized();

   cardinal copy1 = 0;
   cardinal copy2 = 0;
   if(Available > 0) {
      if(WriteStart >= WriteEnd) {
         copy1 = min(length, BufferSize - WriteStart);
         memcpy(data,&Buffer[WriteStart],copy1);
         memset(&Buffer[WriteStart],'-',copy1);
         WriteStart += copy1;
         if(WriteStart >= BufferSize) {
            WriteStart = 0;
         }
#ifdef DEBUG_VERBOSE
         printf("read #1: we=%d ws=%d   c1=%d\n",WriteEnd,WriteStart,copy1);
#endif
      }
      copy2 = min(length - copy1, WriteEnd);
      if(copy2 > 0) {
         memcpy(&data[copy1],&Buffer[WriteStart],copy2);
#ifdef DEBUG
         memset(&Buffer[WriteStart],'-',copy2);
#endif
         WriteStart += copy2;
#ifdef DEBUG
         printf("read #2: we=%d ws=%d   c2=%d\n",WriteEnd,WriteStart,copy2);
#endif
      }

      Available -= copy1 + copy2;
   }

#ifdef DEBUG_VERBOSE
   printf("read: we=%d ws=%d   c1=%d c2=%d  available=%d/%d\n",
          WriteEnd,WriteStart,copy1,copy2,Available,BufferSize);
#endif

   unsynchronized();
   return(copy1 + copy2);
}
*/





class X : public TimedThread
{
   public:
   X();
   void timerEvent();
   int Calls;
};

const int Int=1000000 / 80;

X::X() : TimedThread(Int)
{
   setTimerCorrection(10);
   setFastStart(true);
   Calls=0;
}

void X::timerEvent()
{
   synchronized();
   Calls++;
   if((Calls % 80) == 0) {
      puts("delay!");
      delay(2 * Int);
   }
   unsynchronized();
}

RingBuffer ring;


class Y: public Thread
{
   public:
   Y();
   void run();
};

Y::Y()
{
}

void Y::run()
{
   for(;;) {

puts("WAIT start");
//      if(!ring.fired())
      ring.wait();
puts("WAIT ok!");

      char b[2500];
      ssize_t r = ring.read((char*)&b,sizeof(b));
      if(r > 0) {
         printf("READ: %d\n",r);
      }
     // Thread::delay(200000);
   }
}


int main(int argc, char** argv)
{
   ring.init(10000);
   Y y;
   y.start();

   for(int i=0;i<10000;i++) {
      char b[2500];
      if(ring.write((char*)&b,sizeof(b)) != sizeof(b)) {
         puts("Write Error!");
      }
      Thread::delay(100000);
   }


/*
puts("X-1");
   RingBuffer r;
puts("X0");
   if(r.init(4) == false) {
      puts("no memory!");
      exit(1);
   }

puts("X1");
   char a='A';
   char b='B';
   char c='C';
   char d='D';
   char e='E';
   char f='F';


   printf("write=%d\n",r.write(&a,1));
   printf("write=%d\n",r.write(&b,1));
   printf("write=%d\n",r.write(&c,1));
   printf("write=%d\n",r.write(&d,1));
   printf("write=%d\n",r.write(&e,1));
   printf("write=%d\n",r.write(&f,1));

puts("X2");
   for(int i = 0;i < 20;i++) {
      char o = 0x00;
      printf("read=%d  => ",r.read(&o,1));
      printf("****** %c *******\n",o);

      char oo = 'a'+i;
      printf("write=%d\n",r.write(&oo,1));
      oo = 'A'+i;
      printf("write=%d\n",r.write(&oo,1));
   }
*/
/*
   printf("read=%d  => ",r.read(&o,1));
   printf("%c\n",o);

   printf("write=%d\n",r.write(&d,1));
   printf("write=%d\n",r.write(&e,1));

   printf("read=%d  => ",r.read(&o,1));
   printf("%c\n",o);
   printf("read=%d  => ",r.read(&o,1));
   printf("%c\n",o);
   printf("read=%d  => ",r.read(&o,1));
   printf("%c\n",o);
   printf("read=%d  => ",r.read(&o,1));
   printf("%c\n",o);
   printf("read=%d  => ",r.read(&o,1));
   printf("%c\n",o);
   printf("read=%d  => ",r.read(&o,1));
   printf("%c\n",o);
*/
/*
   X x;
   x.start();
   card64 start=getMicroTime();
   for(;;) {
      Thread::delay(1000000);
      x.synchronized();
      int c = x.Calls;
      card64 now=getMicroTime();
      x.unsynchronized();
      double soll = floor(((double)now - (double)start) / (double)Int);
      printf("soll=%f  ist=%d\n",soll,c);
   }
*/
   return(0);
}


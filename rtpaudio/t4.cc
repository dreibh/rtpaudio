#include <timedthread.h>
#include <tools.h>

using namespace Coral;

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


int main(int argc, char** argv)
{
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
   return(0);
}

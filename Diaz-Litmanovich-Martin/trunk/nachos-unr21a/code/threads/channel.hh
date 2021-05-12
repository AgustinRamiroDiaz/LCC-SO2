#ifndef NACHOS_THREADS_CHANNEL__HH
#define NACHOS_THREADS_CHANNEL__HH

#include "lock.hh"
#include "condition.hh"

class Channel
{
public:
  Channel(const char *debugName);

  ~Channel();

  /// For debugging.
  const char *GetName() const;

  /// Channel operations.
  void Send(int message);
  void Receive(int *message);

private:

  /// For debugging.
  const char *name;

  int *buffer;
        
  Lock *lock;
    
  Condition *msgSent;
  Condition *msgReceived;
  Condition *done;
  
};

#endif
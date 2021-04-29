#include "channel.hh"

Channel::Channel(const char *debugName)
{
    name = debugName;
    buffer = nullptr;
    lock = new Lock(debugName);
    canSend = new Condition(debugName, lock);
    canReceive = new Condition(debugName, lock);
    doneSending = new Condition(debugName, lock);
}

Channel::~Channel()
{
    delete lock;
    delete canSend;
    delete doneSending;
}

const char *
Channel::GetName() const
{
    return name;
}

void
Channel::Send(int message)
{   
    lock->Acquire();

    while(not buffer)
        canSend->Wait();

    *buffer = message;
    buffer = nullptr;

    canReceive->Signal();
    doneSending->Signal();

    lock->Release();
}
    
void
Channel::Receive(int *message)
{   
    lock->Acquire();

    while(buffer)
        canReceive->Wait();

    buffer = message;

    canSend->Signal();

    while(buffer)
        doneSending->Wait();

    lock->Release();
}

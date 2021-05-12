#include "channel.hh"

Channel::Channel(const char *debugName)
{
    name = debugName;
    buffer = nullptr;
    lock = new Lock(name);
    msgSent = new Condition(name, lock);
    msgReceived = new Condition(name, lock);
    done = new Condition(name, lock);
}

Channel::~Channel()
{
    delete lock;
    delete msgSent;
    delete msgReceived;
    delete done;
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

    while (buffer != nullptr)
        done->Wait();

    *buffer = message;

    msgSent->Signal();
    msgReceived->Wait();

    lock->Release();
}
    
void
Channel::Receive(int *message)
{   
    lock->Acquire();

    while(buffer == nullptr)
        msgSent->Wait();

    *message = *buffer;
    buffer = nullptr;

    msgReceived->Signal();
    done->Signal();

    lock->Release();
}

/// Routines for synchronizing threads.
///
/// The implementation for this primitive does not come with base Nachos.
/// It is left to the student.
///
/// When implementing this module, keep in mind that any implementation of a
/// synchronization routine needs some primitive atomic operation.  The
/// semaphore implementation, for example, disables interrupts in order to
/// achieve this; another way could be leveraging an already existing
/// primitive.
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2016-2021 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.


#include "condition.hh"

/// Dummy functions -- so we can compile our later assignments.
///
/// Note -- without a correct implementation of `Condition::Wait`, the test
/// case in the network assignment will not work!

Condition::Condition(const char *debugName, Lock *conditionLock)
{
    name = debugName;
    lock = conditionLock;
    n_waitingThreads = 0;
    waitingThreads = new List <Semaphore *>;
}

Condition::~Condition()
{
    delete waitingThreads;
}

const char *
Condition::GetName() const
{
    return name;
}

void
Condition::Wait()
{
    ASSERT(lock->IsHeldByCurrentThread());

    // Inicializado en 0 para esperar senal
    Semaphore *s = new Semaphore(name, 0);
    waitingThreads->Append(s);
    n_waitingThreads++;

    // Se libera el lock
    lock->Release();
    
    // Espera a la senal
    s->P();

    // Se vuelve a tomar el lock
    lock->Acquire();

    // No se necesita más el semaforo
    delete s;
}

void
Condition::Signal()
{
    ASSERT(lock->IsHeldByCurrentThread());

    if(n_waitingThreads > 0)
    {
        Semaphore *s = waitingThreads->Pop();
        n_waitingThreads--;
        s->V();
    }
    // En otro caso se ignora
}

void
Condition::Broadcast()
{
    ASSERT(lock->IsHeldByCurrentThread());
    
    while(n_waitingThreads > 0)
    {
        Signal();
    }
}

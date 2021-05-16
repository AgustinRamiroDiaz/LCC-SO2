/// Data structures to export a synchronous interface to the raw disk device.
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2016-2021 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.

#ifndef NACHOS_FILESYS_SYNCHCONSOLE__HH
#define NACHOS_FILESYS_SYNCHCONSOLE__HH


#include "threads/lock.hh"
#include "threads/semaphore.hh"
#include "machine/console.hh"

class SynchConsole {
public:

    /// Initialize a synchronous console
    SynchConsole(const char *readFile, const char *writeFile);

    /// De-allocate the synch console.
    ~SynchConsole();

    char GetChar();
    void PutChar(char ch);

    void WriteDone();
    void ReadAvail();

private:
    Lock* readerLock;
    Lock* writerLock;
    Semaphore* readerSem;
    Semaphore* writerSem;
    Console* console;
    
};


#endif

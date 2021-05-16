#include "synch_console.hh"

void ReadAvailProxy(void* data){
    ASSERT(data != nullptr);
    ((SynchConsole *) data)->ReadAvail();
}

void WriteDoneProxy(void* data){
    ASSERT(data != nullptr);
    ((SynchConsole *) data)->WriteDone();
}

SynchConsole::
SynchConsole(const char *readFile, const char *writeFile) {
    readerLock = new Lock("Reader Lock");
    writerLock = new Lock("Writer Lock");
    readerSem = new Semaphore("Reader Semaphore", 0);
    writerSem = new Semaphore("Writer Semaphore", 0);
    console = new Console(readFile, writeFile, ReadAvailProxy, WriteDoneProxy, this);
}

SynchConsole::
~SynchConsole() {
    delete readerLock;
    delete writerLock;
    delete readerSem;
    delete writerSem;
    delete console;
}

char
SynchConsole::GetChar() {
    readerLock->Acquire();
    readerSem->P();
    char buffer = console->GetChar();
    readerLock->Release();

    return buffer;
}

void
SynchConsole::PutChar(char ch) {
    writerLock->Acquire();
    console->PutChar(ch);
    writerSem->P();
    writerLock->Release();
}

void
SynchConsole::WriteDone() {
    writerSem -> V();
}

void
SynchConsole::ReadAvail() {
    readerSem -> V();
}
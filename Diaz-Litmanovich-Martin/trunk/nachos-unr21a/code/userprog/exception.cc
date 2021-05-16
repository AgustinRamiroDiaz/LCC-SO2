/// Entry points into the Nachos kernel from user programs.
///
/// There are two kis of things that can cause control to transfer back to
/// here from user code:
///
/// * System calls: the user code explicitly requests to call a procedure in
///   the Nachos kernel.  Right now, the only function we support is `Halt`.
///
/// * Exceptions: the user code does something that the CPU cannot handle.
///   For instance, accessing memory that does not exist, arithmetic errors,
///   etc.
///
/// Interrupts (which can also cause control to transfer from user code into
/// the Nachos kernel) are handled elsewhere.
///
/// For now, this only handles the `Halt` system call.  Everything else core-
/// dumps.
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2016-2021 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.


#include "transfer.hh"
#include "syscall.h"
#include "filesys/directory_entry.hh"
#include "threads/system.hh"
#include "userprog/args.hh"

#include <stdio.h>

static void RunSimpleUserProgram(void *name) {
    currentThread->space->InitRegisters();  // Set the initial register values.
    currentThread->space->RestoreState();   // Load page table register.

    machine->Run();  // Jump to the user progam.
}

static void RunUserProgram(void *_argv) {
    currentThread->space->InitRegisters();  // Set the initial register values.
    currentThread->space->RestoreState();   // Load page table register.

    char ** argv = (char**) _argv;
    int argc = WriteArgs(argv);

    // "register saves" bytes in WriteArgs
    int argsAddr = machine->ReadRegister(STACK_REG) + 24;

    const int BUFFER_SIZE = 512;
    char buffer[BUFFER_SIZE];
    int tempVirtualAddr;
    DEBUG('e', "Number of arguments: %d\n", argc);
    for (int i = 0; i < argc; i++) {
        machine->ReadMem(argsAddr + 4 * i, 4, &tempVirtualAddr);
        ReadStringFromUser(tempVirtualAddr, buffer, BUFFER_SIZE);
        DEBUG('e', "%d - nth argument: %s, vaddr: %d \n", i, buffer, tempVirtualAddr);
    }

    machine -> WriteRegister(4, argc);
    machine -> WriteRegister(5, argsAddr);
    machine -> Run();  // Jump to the user program.
}


static void
IncrementPC()
{
    unsigned pc;

    pc = machine->ReadRegister(PC_REG);
    machine->WriteRegister(PREV_PC_REG, pc);
    pc = machine->ReadRegister(NEXT_PC_REG);
    machine->WriteRegister(PC_REG, pc);
    pc += 4;
    machine->WriteRegister(NEXT_PC_REG, pc);
}

/// Do some default behavior for an unexpected exception.
///
/// NOTE: this function is meant specifically for unexpected exceptions.  If
/// you implement a new behavior for some exception, do not extend this
/// function: assign a new handler instead.
///
/// * `et` is the ki of exception.  The list of possible exceptions is in
///   `machine/exception_type.hh`.
static void
DefaultHandler(ExceptionType et)
{
    int exceptionArg = machine->ReadRegister(2);

    fprintf(stderr, "Unexpected user mode exception: %s, arg %d.\n",
            ExceptionTypeToString(et), exceptionArg);
    ASSERT(false);
}

/// Handle a system call exception.
///
/// * `et` is the ki of exception.  The list of possible exceptions is in
///   `machine/exception_type.hh`.
///
/// The calling convention is the following:
///
/// * system call identifier in `r2`;
/// * 1st argument in `r4`;
/// * 2nd argument in `r5`;
/// * 3rd argument in `r6`;
/// * 4th argument in `r7`;
/// * the result of the system call, if any, must be put back into `r2`.
///
/// And do not forget to increment the program counter before returning. (Or
/// else you will loop making the same system call forever!)
static void
SyscallHandler(ExceptionType _et)
{
    int scid = machine->ReadRegister(2);

    switch (scid) {

        case SC_HALT:
            DEBUG('e', "Shutdown, initiated by user program.\n");
            interrupt->Halt();
            break;

        case SC_CREATE: {
            int filenameAddr = machine->ReadRegister(4);
            if (filenameAddr == 0) {
                DEBUG('e', "Error: address to filename string is null.\n");
            }

            char filename[FILE_NAME_MAX_LEN + 1];
            if (!ReadStringFromUser(filenameAddr, filename, sizeof filename)) {
                DEBUG('e', "Error: filename string too long (maximum is %u bytes).\n",
                      FILE_NAME_MAX_LEN);
            }

            DEBUG('e', "`Create` requested for file `%s`.\n", filename);
            int file_created = fileSystem->Create(filename, 1000);

            machine -> WriteRegister(2, file_created); // Se guarda en el registro v0 si la creación fue exitosa
            break;
        }

        case SC_REMOVE: {

            int filenameAddr = machine->ReadRegister(4);
            if (filenameAddr == 0) {
                DEBUG('e', "Error: address to filename string is null.\n");
            }

            char filename[FILE_NAME_MAX_LEN + 1];
            if (!ReadStringFromUser(filenameAddr, filename, sizeof filename)) {
                DEBUG('e', "Error: filename string too long (maximum is %u bytes).\n",
                      FILE_NAME_MAX_LEN);
            }

            DEBUG('e', "`Remove` requested for file `%s`.\n", filename);
            int file_deleted = fileSystem->Remove(filename);

            machine -> WriteRegister(2, file_deleted);

            break;
        }

        case SC_EXIT: {

            int returnValue = machine->ReadRegister(4);

            DEBUG('a', "Exited with status %d\n", returnValue);
            currentThread->Finish(returnValue);

            break;
        }

        case SC_READ: {

            //char *buffer, int size, OpenFileId id
            int bufferAddr = machine->ReadRegister(4);
            int size = machine->ReadRegister(5);
            OpenFileId id = machine->ReadRegister(6);

            if (bufferAddr == 0){
                DEBUG('a', "Error: buffer string is null.\n");
                machine -> WriteRegister(2, -1);
                break;
            }

            char *buffer = new char [size + 1];
            int readBytes = 0;

            if (id == CONSOLE_INPUT)
            {
                int i;
                for(i = 0; i < size && buffer[i] != '\n'; i++) {
                    buffer[i] = synchConsole->GetChar();
                }

                buffer[i] = 0;
                readBytes = i;

            } else {

                if (currentThread->HasFile(id)) {
                    OpenFile *filePtr = currentThread->GetFile(id);
                    readBytes = filePtr->Read(buffer, size);

                } else {

                    DEBUG('a', "Error: file %d not open.\n", id);
                    machine->WriteRegister(2, -1);
                    break;
                }
            }

            WriteStringToUser(buffer, bufferAddr);
            machine->WriteRegister(2, readBytes);
            DEBUG('a', "Requested to read %d bytes from file at position %d\n", size, id);

            delete [] buffer;

            break;
        }

        case SC_WRITE: {

            //const char *buffer, int size, OpenFileId id
            int bufferAddr = machine->ReadRegister(4);
            int size = machine->ReadRegister(5);
            OpenFileId id = machine->ReadRegister(6);

            if (bufferAddr == 0) {
                DEBUG('a', "Error: buffer string is null.\n");
                machine -> WriteRegister(2, -1);
                break;
            }

            if(size == 0) {
                DEBUG('a', "Error: size is 0.\n");
                machine -> WriteRegister(2, -1);
                break;
            }

            char *buffer = new char [size + 1];

            if (!ReadStringFromUser(bufferAddr, buffer, size + 1)) {
                DEBUG('a', "Requested to write %d bytes from file at position %d\n", size + 1);
                machine -> WriteRegister(2, -1);
                break;
            }

            int writtenBytes;

            if (id == CONSOLE_OUTPUT) {

                int i;
                for(i = 0; i < size and buffer[i]; i++)
                    synchConsole->PutChar(buffer[i]);
                writtenBytes = i;

            } else {

                if (currentThread -> HasFile(id)){

                    OpenFile *filePtr = currentThread->GetFile(id);
                    writtenBytes = filePtr->Write(buffer, size);

                } else {

                    DEBUG('a', "Error: file %d not open.\n", id);
                    machine -> WriteRegister(2, -1);
                }
            }

            machine -> WriteRegister(2, writtenBytes);
            DEBUG('a', "Requested to write %d bytes to the file at position %d\n", size, id);

            delete [] buffer;
            break;
        }

        case SC_OPEN: {
            //const char *name
            int filenameAddr = machine->ReadRegister(4);

            if (filenameAddr == 0){
                DEBUG('a', "Error: address to filename string is null.\n");
                machine -> WriteRegister(2, -1);
                break;
            }

            char filename[FILE_NAME_MAX_LEN + 1];
            if (!ReadStringFromUser(filenameAddr, filename, sizeof filename)){
                DEBUG('a', "Error: filename string too long (maximum is %u bytes).\n", FILE_NAME_MAX_LEN);
                machine -> WriteRegister(2, -1);
                break;
            }

            OpenFile *filePtr = fileSystem->Open(filename);
            int fileId = currentThread->AddFile(filePtr);
            if(fileId == -1){
                DEBUG('a', "Error: %s's filetable full\n", currentThread->GetName());
                machine -> WriteRegister(2, -1);
                break;
            }

            machine -> WriteRegister(2, fileId);
            DEBUG('a', "Request to open file `%s`.\n", filename);
            break;
        }

        case SC_CLOSE: {
            //OpenFileId id
            int id = machine->ReadRegister(4);

            DEBUG('e', "`Close` requested for id %u.\n", id);

            if(currentThread->HasFile(id))
                currentThread->RemoveFile(id);
            else {
                DEBUG('a', "Error: file %d not open.\n");
                machine -> WriteRegister(2, 0);
                break;
            }

            break;
        }

        case SC_EXEC: {
            // char *name
            int filenameAddr = machine->ReadRegister(4);
            int argsAddr = machine->ReadRegister(5);
            int enableJoin = machine->ReadRegister(6);

            if (filenameAddr == 0){
                DEBUG('a', "Error: address to filename string is null.\n");
                machine->WriteRegister(2, -1);
                break;
            }

            char filename[FILE_NAME_MAX_LEN + 1];
            if (!ReadStringFromUser(filenameAddr, filename, sizeof filename)){
                DEBUG('a', "Error: filename string too long (maximum is %u bytes).\n", FILE_NAME_MAX_LEN);
                machine -> WriteRegister(2, -1);
                break;
            }

            DEBUG('e', "`Exec` requested for file `%s`.\n", filename);

            OpenFile *executable = fileSystem->Open(filename);
            if (executable == nullptr) {
                DEBUG('e',"Unable to open file %s\n", filename);
                machine->WriteRegister(2, -1);
                break;
            }

            Thread * thread = new Thread(filename, enableJoin);
            AddressSpace *space = new AddressSpace(executable);

            // Set address space
            thread->space = space;

            if(argsAddr == 0)
                thread->Fork(RunSimpleUserProgram, nullptr);
            else
                thread->Fork(RunUserProgram, SaveArgs(argsAddr));

            delete executable;

            SpaceId spaceId = thread->GetSpaceId();
            machine->WriteRegister(2, spaceId);

            break;
        }

        case SC_JOIN: {
            SpaceId spaceId = machine->ReadRegister(4);

            if(spaceId < 0) {
                DEBUG('a', "Error: Invalid spaceId.\n");
                machine->WriteRegister(2, -1);
                break;
            }

            if(!threadTable->HasKey(spaceId)) {
                DEBUG('a', "Error: Thread with id %d not found.\n", spaceId);
                machine->WriteRegister(2, -1);
                break;
            }

            Thread *threadToJoin = threadTable->Get(spaceId);

            DEBUG('a', "Requested Join with SpaceId %d\n", spaceId);
            int exitStatus = threadToJoin->Join();

            machine->WriteRegister(2, exitStatus);
            break;
        }

        case SC_PRINT: {
            scheduler->Print();
            break;
        }

        default:
            fprintf(stderr, "Unexpected system call: id %d.\n", scid);
            ASSERT(false);

    }

    IncrementPC();
}


/// By default, only system calls have their own handler.  All other
/// exception types are assigned the default handler.
void
SetExceptionHandlers()
{
    machine->SetHandler(NO_EXCEPTION,            &DefaultHandler);
    machine->SetHandler(SYSCALL_EXCEPTION,       &SyscallHandler);
    machine->SetHandler(PAGE_FAULT_EXCEPTION,    &DefaultHandler);
    machine->SetHandler(READ_ONLY_EXCEPTION,     &DefaultHandler);
    machine->SetHandler(BUS_ERROR_EXCEPTION,     &DefaultHandler);
    machine->SetHandler(ADDRESS_ERROR_EXCEPTION, &DefaultHandler);
    machine->SetHandler(OVERFLOW_EXCEPTION,      &DefaultHandler);
    machine->SetHandler(ILLEGAL_INSTR_EXCEPTION, &DefaultHandler);
}

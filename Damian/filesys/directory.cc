/// Routines to manage a directory of file names.
///
/// The directory is a table of fixed length entries; each entry represents a
/// single file, and contains the file name, and the location of the file
/// header on disk.  The fixed size of each directory entry means that we
/// have the restriction of a fixed maximum size for file names.
///
/// The constructor initializes an empty directory of a certain size; we use
/// ReadFrom/WriteBack to fetch the contents of the directory from disk, and
/// to write back any modifications back to disk.
///
/// Also, this implementation has the restriction that the size of the
/// directory cannot expand.  In other words, once all the entries in the
/// directory are used, no more files can be created.  Fixing this is one of
/// the parts to the assignment.
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2016-2018 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.

#include "directory.hh"
#include "directory_entry.hh"
#include "file_header.hh"
#include "lib/utility.hh"

/// Initialize a directory; initially, the directory is completely empty.  If
/// the disk is being formatted, an empty directory is all we need, but
/// otherwise, we need to call FetchFrom in order to initialize it from disk.
///
/// * `size` is the number of entries in the directory.
Directory::Directory(unsigned size) {
    ASSERT(size);

    raw.table = (DirectoryEntry*) malloc(sizeof(DirectoryEntry) * size);
    raw.tableSize = size;
    for (unsigned i = 0; i < raw.tableSize; i++) raw.table[i].inUse = false;
}

/// De-allocate directory data structure.
Directory::~Directory() {
    free(raw.table);
}

/// Read the contents of the directory from disk.
///
/// * `file` is file containing the directory contents.
void
Directory::FetchFrom(OpenFile *file) {
    ASSERT(file);

    file->ReadAt((char *) &raw.tableSize, 1, 0); // Reads the number of entries.
    raw.table = (DirectoryEntry*) realloc(raw.table,
            raw.tableSize * sizeof(DirectoryEntry));
    file->ReadAt((char *) raw.table, raw.tableSize * sizeof (DirectoryEntry), 1);
}

/// Write any modifications to the directory back to disk.
///
/// * `file` is a file to contain the new directory contents.
void
Directory::WriteBack(OpenFile *file) {
    ASSERT(file);

    file->WriteAt((char *) &raw.tableSize, 1, 0); // Writes the number of entries.
    file->WriteAt((char *) raw.table, raw.tableSize * sizeof (DirectoryEntry), 1);
}

/// Look up file name in directory, and return its location in the table of
/// directory entries.  Return -1 if the name is not in the directory.
///
/// * `name` is the file name to look up.
int
Directory::FindIndex(const char *name) {
    ASSERT(name);

    for (unsigned i = 0; i < raw.tableSize; i++)
        if (raw.table[i].inUse && !strncmp(raw.table[i].name, name, FILE_NAME_MAX_LEN))
            return i;

    return -1;  // name not in directory
}

/// Look up file name in directory, and return the disk sector number where
/// the file's header is stored.  Return -1 if the name is not in the
/// directory.
///
/// * `name` is the file name to look up.
int
Directory::Find(const char *name) {
    ASSERT(name);

    int i = FindIndex(name);
    if (i != -1) return raw.table[i].sector;

    return -1;
}

/// Add a file into the directory.  Return true if successful; return false
/// if the file name is already in the directory, or if the directory is
/// completely full, and has no more space for additional file names.
///
/// * `name` is the name of the file being added.
/// * `newSector` is the disk sector containing the added file's header.
bool
Directory::Add(const char *name, int newSector) {
    ASSERT(name);

    if (FindIndex(name) != -1) return false;

    for (unsigned i = 0; i < raw.tableSize; i++)
        if (!raw.table[i].inUse) {
            raw.table[i].inUse = true;
            strncpy(raw.table[i].name, name, FILE_NAME_MAX_LEN);
            raw.table[i].sector = newSector;
            return true;
        }

    // Expands the directory.
    raw.table = (DirectoryEntry*) realloc(raw.table,
            ++raw.tableSize * sizeof(DirectoryEntry));
    raw.table[raw.tableSize - 1].inUse = false;

    return Add(name, newSector);
}

/// Remove a file name from the directory.   Return true if successful;
/// return false if the file is not in the directory.
///
/// * `name` is the file name to be removed.
bool
Directory::Remove(const char *name) {
    ASSERT(name);

    int i = FindIndex(name);
    if (i == -1) return false;  // name not in directory
    raw.table[i].inUse = false;

    return true;
}

/// List all the file names in the directory.
void
Directory::List() const {
    for (unsigned i = 0; i < raw.tableSize; i++)
        if (raw.table[i].inUse) {
            FileHeader* hdr = new FileHeader(raw.table[i].sector, raw.table[i].name);

            if (hdr->IsDirectory()) printf(BOLD);
            printf("%s\t\t(%u bytes)" RESET ".\n", raw.table[i].name, hdr->FileLength());

            delete hdr;
        }
}

/// List all the file names in the directory, their `FileHeader` locations,
/// and the contents of each file.  For debugging.
void
Directory::Print() const {
    for (unsigned i = 0; i < raw.tableSize; i++)
        if (raw.table[i].inUse) {
            printf("--------------------------------------------------------------\n");
            printf("Directory entry: %s. Header in sector %u.\n",
                   raw.table[i].name, raw.table[i].sector);

            FileHeader* hdr = new FileHeader(raw.table[i].sector, raw.table[i].name);
            hdr->Print();
            delete hdr;
        }
}

const RawDirectory *
Directory::GetRaw() const {
    return &raw;
}

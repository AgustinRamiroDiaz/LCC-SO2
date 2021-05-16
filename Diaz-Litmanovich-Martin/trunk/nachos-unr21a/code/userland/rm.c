#include "lib.c"

int main(int argc, char** argv) {

    if(argc != 2){
        putss("Invalid number of arguments");
        Exit(-1);
    }

    int removed = Remove(argv[1]);

    if (removed)
        Exit(0);
    else {
        putss("File not found");
        Exit(-1);
    }
}
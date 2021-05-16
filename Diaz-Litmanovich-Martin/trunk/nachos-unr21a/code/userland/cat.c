#include "lib.c"

int main(int argc, char** argv) {

    if(argc != 2){
        putss("Invalid number of arguments");
        Exit(-1);
    }

    OpenFileId f = Open(argv[1]);

    if(f < 2) {
        putss("Error when openning file");
        Exit(-1);
    }

    char buff[1];
    for(; Read(buff, 1, f); Write(buff, 1, CONSOLE_OUTPUT));

    Close(f);
    Exit(0);
}
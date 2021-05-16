#include "lib.c"

int main(int argc, char** argv) {

    if(argc != 3){
        putss("Invalid number of arguments");
        Exit(-1);
    }

    OpenFileId src = Open(argv[1]);

    if(src < 2) {
        putss("Error when openning file");
        Exit(-1);
    }

    Create(argv[2]);
    OpenFileId dst = Open(argv[2]);

    char buff[1];
    for(; Read(buff, 1, src); Write(buff, 1, dst));

    Close(src);
    Close(dst);
    Exit(0);
}
#include "syscall.h"
#include "string.h"

unsigned strlen(const char *s) {

    if(s == NULL) return 0;

    unsigned i;
    for (i = 0; s[i] != '\0'; i++);
    return i;
}

void putss(const char *s) {

    char *endline = "\n";

    if (s) {
        Write(s, strlen(s), CONSOLE_OUTPUT);
        Write(endline, strlen(endline), CONSOLE_OUTPUT);
        Exit(0);
    }

    Write(endline, strlen(endline), CONSOLE_OUTPUT);
    Exit(0);
    
}

static void swap(char *x, char *y) {
    char t = *x; *x = *y; *y = t;
}

static char* reverse(char *str, int i, int j) {
    while (i < j)
        swap(&str[i++], &str[j--]);

    return str;
}

static int abs(int n) {
  if (n < 0)
    return -n;
  return n;
}

// No la usamos
void itoa(int n, char *str) {
    
    int base = 10;
    int nvalue = abs(n);

    int i = 0, r = 0;
    while (nvalue) {
        r = nvalue % base;

        if (r >= 10)
            str[i++] = 65 + (r - 10);
        else
            str[i++] = 48 + r;

        nvalue = nvalue / base;
    }

    if (i == 0) {
        str[i++] = '0';
    }

    if (n < 0) {
        str[i++] = '-';
    }

    str[i] = '\0';
    reverse(str, 0, i - 1);

}
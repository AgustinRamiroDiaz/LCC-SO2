#include "thread_test_join.hh"

#include <stdio.h>
#include <unistd.h>
#include "thread.hh"
#include "system.hh"

static void
Hijo(void *dummy)
{
    printf("Inicio hijo\n");
    usleep(20000);
    printf("Fin hijo\n");
}

static void
Padre(void *dummy)
{
    printf("Inicio padre\n");

    Thread *h = new Thread("Hijo", true, 9);
    h->Fork(Hijo, (void *) nullptr);
    h->Join();

    printf("Fin padre\n");
}

void
ThreadTestJoin()
{

    printf("Inicio del test\n");

    Thread *p = new Thread("Padre", true, 9);
    p->Fork(Padre, (void *) nullptr);
    p->Join();

    printf("Fin del test\n");

}
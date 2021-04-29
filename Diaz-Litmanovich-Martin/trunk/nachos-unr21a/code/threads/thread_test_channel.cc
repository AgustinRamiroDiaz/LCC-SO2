#include "thread_test_channel.hh"

#include "channel.hh"
#include "system.hh"
#include <stdio.h>

static const unsigned CANT_SENDERS = 3;
static const unsigned CANT_RECEIVERS = 3;
static bool done[CANT_SENDERS + CANT_RECEIVERS];

static Channel *channel = new Channel("Canal de comunic");

static void
Sender(void *n_)
{
    unsigned n = * (unsigned *) n_;

    int mensaje = n;
    channel->Send(n);
    printf("Soy sender %u, envié %d\n", n, mensaje);

    DEBUG('t', "Sender %u finished.\n", n);
    done[n] = true;

}

static void
Receiver(void *n_)
{
    unsigned n = * (unsigned *) n_;

    int mensaje;
    channel->Receive(&mensaje);
    printf("Soy receiver %u, recibí %d\n", n, mensaje);

    DEBUG('t', "Receiver %u finished.\n", n);
    done[n + CANT_SENDERS] = true;
}

void
ThreadTestChannel()
{
    // Launch a new thread for each sender.
    for (unsigned i = 0; i < CANT_SENDERS; i++) {
        printf("Launching sender %u.\n", i);
        char *name = new char [16];
        sprintf(name, "Sender %u", i);
        unsigned *n = new unsigned;
        *n = i;
        Thread *t = new Thread(name, false, 9);
        t->Fork(Sender, (void *) n);
    }

    // Launch a new thread for each receiver.
    for (unsigned i = 0; i < CANT_RECEIVERS; i++) {
        printf("Launching receiver %u.\n", i);
        char *name = new char [16];
        sprintf(name, "Receiver %u", i);
        unsigned *n = new unsigned;
        *n = i;
        Thread *t = new Thread(name, false, 9);
        t->Fork(Receiver, (void *) n);
    }

    // Mecanismo reemplazo de join
    for (unsigned i = 0; i < CANT_SENDERS + CANT_RECEIVERS - 1; i++) {
      while (!done[i]) {
          currentThread->Yield();
      }
    }
    printf("Todos los senders y receivers terminaron");
}
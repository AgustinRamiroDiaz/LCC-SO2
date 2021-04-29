/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2007-2009 Universidad de Las Palmas de Gran Canaria.
///               2016-2021 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.


#include "thread_test_prod_cons.hh"

#include <stdio.h>
#include "system.hh"
#include "condition.hh"
#include "lock.hh"

static const unsigned CANT_PRODUCTORES = 1;
static const unsigned CANT_CONSUMIDORES = 5;
static bool done[CANT_CONSUMIDORES + CANT_PRODUCTORES];

static const unsigned N_MAX_PRODUCTOS = 6;
static unsigned n_elementos = 0;

static List <unsigned> *elementos = new List <unsigned>;
static Lock *l = new Lock("Mutex");
static Condition *elementosDisponibles = new Condition("Elementos", l);
static Condition *espaciosDisponibles = new Condition("Espacios", l);

static void
Producer(void *n_)
{
    unsigned n = * (unsigned *) n_;

    l->Acquire();

    while(n_elementos == N_MAX_PRODUCTOS)
      espaciosDisponibles->Wait();

    DEBUG('t', "Soy productor %u, produje %u\n", n, n);
    elementos->Append(n);
    n_elementos++;

    elementosDisponibles->Signal();
    l->Release();

    DEBUG('p', "Producer %u finished.\n", n);
    done[n + CANT_CONSUMIDORES] = true;
}

static void
Consumer(void *n_)
{
    unsigned n = * (unsigned *) n_;

    l->Acquire();

    while (n_elementos == 0)
        elementosDisponibles->Wait();
    
    ASSERT(!elementos->IsEmpty());

    unsigned p = elementos->Pop();
    n_elementos--;
    DEBUG('t', "Soy consumidor %u, consumí %u\n", n, p);

    espaciosDisponibles->Signal();

    l->Release();

    DEBUG('p', "Consumer %u finished.\n", n);
    done[n] = true;
}

void
ThreadTestProdCons()
{
    // Launch a new thread for each producer.
    for (unsigned i = 0; i < CANT_PRODUCTORES; i++) {
      printf("Launching productor %u.\n", i);
      char *name = new char [16];
      sprintf(name, "Productor %u", i);
      unsigned *n = new unsigned;
      *n = i;
      Thread *t = new Thread(name, false, 9);
      t->Fork(Producer, (void *) n);
    }

    // Launch a new thread for each consumer.
    for (unsigned i = 0; i < CANT_CONSUMIDORES; i++) {
      printf("Launching consumidor %u.\n", i);
      char *name = new char [16];
      sprintf(name, "Consumidor %u", i);
      unsigned *n = new unsigned;
      *n = i;
      Thread *t = new Thread(name, false, 9);
      t->Fork(Consumer, (void *) n);
    }

    // Mecanismo reemplazo de join
    for (unsigned i = 0; i < CANT_CONSUMIDORES + CANT_PRODUCTORES - 1; i++) {
      while (!done[i]) {
          currentThread->Yield();
      }
    }
    printf("Todos los productores y consumidores terminaron");
}

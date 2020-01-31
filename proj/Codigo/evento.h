#ifndef _EVENTO_H_
#define _EVENTO_H_

#include "keyboard.h"
#include "mouse.h"
#include "i8024.h"


enum IdentificaPeriferico {
    Timer,
    Keyboard,
    Mouse
};

typedef struct {
    enum IdentificaPeriferico per;
    Scancode scan;
    struct packet pp;
} evento;

evento loadEvento(enum IdentificaPeriferico per, Scancode *scan, struct packet *pp);

#endif /*EVENTO_H*/

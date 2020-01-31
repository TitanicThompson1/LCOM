#include <lcom/lcf.h>

#include "evento.h"

evento loadEvento(enum IdentificaPeriferico per, Scancode *scan, struct packet *pp){
    evento ev;
    ev.per=per;
    switch (per)
    {
    case Keyboard:
        ev.scan=*scan;
        break;
    default:
        break;
    }
    return ev;
}








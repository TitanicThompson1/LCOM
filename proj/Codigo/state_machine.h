#ifndef _STATE_MACHINE_
#define _STATE_MACHINE_


#include "video.h"
#include "sprite.h"
#include "i8024.h"
#include "menuInicial1.xpm"
#include "menuInicial2.xpm"
#include "evento.h"
#include "Mapa.xpm"
#include "menuPlayer1.xpm"
#include "menuPlayer2.xpm"
#include "menuPlayer3.xpm"
#include "pacman.xpm"
#include "pacmanDown.xpm"
#include "pacmanUp.xpm"
#include "pacmanLeft.xpm"


int menus(evento ev, Screen *sc);

void initSprites();

void resetPac(Sprite *pacMan);
#endif

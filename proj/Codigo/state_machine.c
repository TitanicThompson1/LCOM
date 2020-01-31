#include <lcom/lcf.h>

#include "state_machine.h"

static Sprite *spMI1;
static Sprite *spMI2;
static Sprite *spMP1;
static Sprite *spMP2;
static Sprite *spMP3;
static Sprite *spMapa;
static Sprite *pacMan;
static Sprite *pacManD;
static Sprite *pacManL;
static Sprite *pacManU;
static Sprite *pacManR;

static int currentMenu=1;


static int mainMenu=1;
static int playerMenu=1;
//static int gameplayMenu=1;


int menus(evento ev, Screen *sc){

    switch (currentMenu)
    {
        //Menu Inicial
    case 1:
        if(mainMenu==1 ){

            if(ev.scan.bytes[ev.scan.size - 1] == B_DOWN){
                mainMenu=2;
                copy_sprite_to_vmem(spMI2,sc);
                return 1;

            }else if(ev.scan.bytes[ev.scan.size -1] == B_ENTER){
                mainMenu= 1;

                copy_sprite_to_vmem(spMP1,sc);
                currentMenu=2;
                return 2;
            }
        }else if(mainMenu==2){

             if(ev.scan.bytes[ev.scan.size - 1] == B_UP){
                mainMenu=1;
                copy_sprite_to_vmem(spMI1,sc);
                return 1;

            }else if(mainMenu == 2){
                if(ev.scan.bytes[ev.scan.size -1] == B_ENTER){
                    mainMenu= 1;
                    return 0;   //fazer ito?!?      
                }
            }

        }

        break;
        
        //Menu Player
    case 2:

        if(playerMenu == 1){//item mais à esquerda é o que está actualmente sinalizado
            if( ev.per == 1 && ev.scan.bytes[ev.scan.size - 1] == M_RIGHT){
                playerMenu = 2;
                //"tratar" ecrâ : sinalizar item à direita    (player 2?)
                
                copy_sprite_to_vmem(spMP2,sc);
                return 2;
            } else if(ev.per == 1 && ev.scan.bytes[ev.scan.size - 1] == M_LEFT ){

                playerMenu = 3;
                //Tratar ecrâ: sinaliza item voltar
                copy_sprite_to_vmem(spMP3,sc);
                return 2;
                
            } else if(ev.per == 1 && ev.scan.bytes[ev.scan.size - 1] == B_ENTER){
                
                //Tratar ecrâ: sinaliza item Player 1
                cleanScreen(sc);
                copy_sprite_to_vmem(spMapa,sc);//"insere-a no ecrã, ver pormenores disto!!"
                currentMenu=3;
                return 3;

            }
        
        }
        else if(playerMenu == 2){//item mais à direita é o que está actualmente sinalizado

            if(ev.per == 1 && ev.scan.bytes[ev.scan.size - 1] == M_RIGHT){
                //mudar o estado actual
                playerMenu = 3;
                //Tratar ecrâ: sinaliza item voltar
                copy_sprite_to_vmem(spMP3,sc);
                return 2;

            } else if(ev.per == 1 && ev.scan.bytes[ev.scan.size - 1] == M_LEFT){

                playerMenu = 1;
                //Tratar ecrâ: sinaliza item player1
                copy_sprite_to_vmem(spMP1,sc);
                return 2;                
            }else if(ev.per == 1 && ev.scan.bytes[ev.scan.size - 1] == B_ENTER){
                    
                //Tratar ecrâ: sinaliza item Player 1
                copy_sprite_to_vmem(spMapa,sc);//"insere-a no ecrã, ver pormenores disto!!"
                currentMenu=3;
                return 3;
            }

        }
        else if(playerMenu == 3){//"item voltar" é que que está sinalizado

            if(ev.per == 1 && ev.scan.bytes[ev.scan.size - 1] == M_RIGHT){

                playerMenu = 1;
                //Tratar ecrâ: sinaliza item Player 2
                copy_sprite_to_vmem(spMP1,sc);
                return 2;

            } else if(ev.per == 1 && ev.scan.bytes[ev.scan.size - 1] == M_LEFT){

                playerMenu = 2;
                //Tratar ecrâ: sinaliza item Player 1
                copy_sprite_to_vmem(spMP2,sc);
                return 2;
                
            } else if(ev.per == 1 && ev.scan.bytes[ev.scan.size - 1] == B_ENTER){

                playerMenu = 1;
                mainMenu = 1;
                //Tratar ecrâ: volta ao menu inicial
                copy_sprite_to_vmem(spMI1,sc);//"insere-a no ecrã, ver pormenores disto!!"
                currentMenu=1;
                return 1;                
            }
        }
        else{
            //Outras teclas são pressionadas, como tratar isto, não fazer nada?!?!


        }
        /*
        //casos do mouse
        else if(ev.per == 2 && ev.scan.kbytes[0] ){

            
        }
        */

        break;

       
    case 3:
       
        //Gameplay

       
        
        

        if(ev.per == Keyboard){

            if(ev.scan.bytes[ev.scan.size - 1] == B_ENTER){
                cleanScreen(sc);
                copy_sprite_to_vmem(spMapa,sc);
                resetPac(pacMan);
            }

            if(ev.scan.bytes[ev.scan.size - 1] == M_RIGHT){
                if(pacMan->direction!=0){
                    changeXY(pacManR,pacMan->x,pacMan->y);
                    pacMan=pacManR;
                }

            }else if(ev.scan.bytes[ev.scan.size - 1] == M_LEFT){
                if(pacMan->direction!=1){
                    changeXY(pacManL,pacMan->x,pacMan->y);
                    pacMan=pacManL;
                }
                
            }else if (ev.scan.bytes[ev.scan.size - 1] == M_DOWN){
                if(pacMan->direction != 2){
                    changeXY(pacManD,pacMan->x,pacMan->y);
                    pacMan=pacManD;
                }
                
            }else if (ev.scan.bytes[ev.scan.size - 1] == M_UP){
                if(pacMan->direction != 3){
                    changeXY(pacManU,pacMan->x,pacMan->y);
                    pacMan=pacManU;
                }
            }
        }
        
        move_sprit(pacMan,sc);

        return 3;

        break;
    default:

        break;
    }
    return 1;
}


void initSprites(){
    spMI1= create_sprite(menuInicial1);
    spMI2= create_sprite(menuInicial2);
    spMP1= create_sprite(menuPlayer1);
    spMP2= create_sprite(menuPlayer2);
    spMP3= create_sprite(menuPlayer3);
    spMapa= create_sprite(Mapa);

    //cria pacMan
    pacManR= create_sprite(pacmanR);
    pacManR->xspeed= 1;
    pacManR->yspeed= 0;
    pacManR->direction=0;
    setDimensions(pacManR,36,36);

    //cria as posições 
    pacManD= create_sprite(pacmanDown);
    setDimensions(pacManD,36,36);
    pacManD->xspeed= 0;
    pacManD->yspeed= 1;
    pacManD->direction= 2;
    
    pacManU= create_sprite(pacmanUp);
    setDimensions(pacManU,36,36);
    pacManU->xspeed= 0;
    pacManU->yspeed= -1;
    pacManU->direction= 3;

    pacManL= create_sprite(pacmanLeft);
    setDimensions(pacManL,36,36);
    pacManL->xspeed= -1;
    pacManL->yspeed= 0;
    pacManL->direction= 1;

    //pacman atual é o direito
    pacMan=pacManR;
    changeXY(pacMan,395,458);
}

void resetPac(Sprite *pacMan){
    changeXY(pacMan,395,458);
    pacMan->xspeed= 1;
    pacMan->yspeed= 0;
    pacMan->direction=0;
    setDimensions(pacMan,36,36);
}

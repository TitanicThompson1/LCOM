#include <lcom/lcf.h>

#include "sprite.h"



Sprite * create_sprite(xpm_map_t pic){
    
    //allocate space for the "object"
    Sprite *sp = (Sprite *) malloc ( sizeof(Sprite));
    xpm_image_t img;

    
    // read the sprite pixmap
    sp->map = xpm_load(pic, XPM_8_8_8_8, &img);
    
    if( sp->map == NULL ) {
        free(sp);
        return NULL;
    }
    sp->width = img.width; sp->height=img.height;
    sp->x=0;sp->y=0;
    sp->xspeed=0;sp->yspeed=0;
    return sp;
}



int copy_sprite_to_vmem(Sprite *sp, Screen * sc){

    if (sc->video_mem==NULL)
        return 1;


    uint32_t spos=0;

    for(int j=0;j<sp->height;j++){

        for(int i=0;i<sp->width;i++){
            memcpy(&sc->video_mem[ 4*((sp->x+i)+(sp->y+j)*sc->h_res) ],&sp->map[spos], 4);//mudamos de 4 para 1
            spos+=4;
        }
       
    }
  
    return 0;
}


void destroy_sprite(Sprite *sp){
    if( sp == NULL )
        return;
    if( sp ->map )
        free(sp->map);
    free(sp);
    sp = NULL;
}


int move_sprit(Sprite *sp, Screen *sc) {

    uint32_t spos= 0;

    /*Verifica se a memoria nao está NULL*/
    if (sc->video_mem==NULL)
        return 1;

    //admitindo que xspeed e yspeed > 0
    if(sp->x + sp->xspeed >= sc->h_res || sp->y + sp->yspeed >= sc->v_res){
        return 1;
    }

    uint32_t cor1=0, cor2=0;
    uint32_t pos1=0, pos2=0;
    
    //deteção de colisões
    if (sp->direction == 0){ //right

        pos1= 4*(sp->x + sp->width + 3 + (sp->y + 3)* sc->h_res);    //UP right
        pos2 = 4*(sp->x + sp->width + 3 + (sp->y - 3 + sp->height) * sc->h_res);  //down right
        memcpy(&cor1,&sc->video_mem[pos1], 4);    
        memcpy(&cor2,&sc->video_mem[pos2], 4);

    }else if(sp->direction == 1){//left

        pos1= 4*(sp->x - 3 + (sp->y + 3) * sc->h_res);    //UP LEFT
        pos2 = 4*(sp->x - 3 + (sp->y - 3 + sp->height) * sc->h_res);  //down left
        memcpy(&cor1,&sc->video_mem[pos1], 4);    
        memcpy(&cor2,&sc->video_mem[pos2], 4);
        

    }else if(sp->direction == 2){//down

        pos2 = 4*(sp->x + 3 + (sp->y + 3 + sp->height) * sc->h_res);  //down left
        pos2 = 4*(sp->x + sp->width - 3 + (sp->y + 3 + sp->height) * sc->h_res);  //down right
        memcpy(&cor1,&sc->video_mem[pos1], 4);    
        memcpy(&cor2,&sc->video_mem[pos2], 4);

    }else if(sp->direction == 3){   //up
        
        pos1= 4*(sp->x + 3 + (sp->y - 3) * sc->h_res);    //UP LEFT
        pos1= 4*(sp->x + sp->width - 3 + (sp->y - 3) * sc->h_res);    //UP right
        memcpy(&cor1,&sc->video_mem[pos1], 4);    
        memcpy(&cor2,&sc->video_mem[pos2], 4);
        
    }else{

        printf("ERROR");
        return 1;
    }

    

    if(cor1 != 0 || cor2 != 0){
        //cor1=0xffffff;
        //memcpy(&sc->video_mem[pos1],&cor1,4);
        //memcpy(&sc->video_mem[pos2],&cor1,4);
        return 1;
    }
    

    /*Limpa o sprit*/
    clean_sprit(sp,sc);

    
    /*Desenha o sprit na nova posição*/
    for(int j=0; j<sp->height; j++){
        for(int i=0;i<sp->width;i++){
            memcpy(&sc->video_mem[ 4 * ((sp->x + sp->xspeed + i) + (sp->y + j + sp->yspeed)*sc->h_res) ],&sp->map[spos], 4);
            spos+=4;
        }
    }

    /*Atualiza as posições x e y*/
    sp->x += sp->xspeed;
    sp->y += sp->yspeed;
    
    return 0;
}

int clean_sprit(Sprite *sp, Screen *sc){

    /*Verifica se a memoria nao está NULL*/
    if (sc->video_mem==NULL)
        return 1;

    uint32_t color = 0;

    /*Limpa esta regiao da vmem(onde o sprit estava)*/
    for(int j=0;j < sp->height; j++){
        for(int i=0; i < sp->width; i++){
            memcpy(&sc->video_mem[((sp->y + j ) * sc->h_res + sp->x + i)*4],&color,4);
        }
    }

    return 0;
}

void changeXY(Sprite *sp, int x, int y){ sp->x=x; sp->y=y; }

void changeSpeed(Sprite *sp, int xspeed, int yspeed){ sp->xspeed=xspeed; sp->yspeed= yspeed; }
    

void setDimensions(Sprite *sp, int w, int h){
    sp->height=h;
    sp->width=w;
}

void changeDirection(Sprite *sp, uint8_t dir){
    sp->direction=dir;
    switch (dir)
    {
    case 0:
        sp->xspeed=1;
        sp->yspeed=0;
        break;
    case 1:
        sp->xspeed=-1;
        sp->yspeed=0;
    
        break;
    case 2:
        sp->xspeed=0;
        sp->yspeed=1;
        break;
    case 3:     //UP
        sp->xspeed=0;
        sp->yspeed=-1;

        break;
    default:

        break;
    }
}



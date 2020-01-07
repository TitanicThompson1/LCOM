#include <lcom/lcf.h>

#include "sprite.h"



Sprite * create_sprite(xpm_map_t pic){
    
    //allocate space for the "object"
    Sprite *sp = (Sprite *) malloc ( sizeof(Sprite));
    xpm_image_t img;

    
    // read the sprite pixmap
    sp->map = xpm_load(pic, XPM_INDEXED, &img);
    
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
            memcpy(&sc->video_mem[ 1*((sp->x+i)+(sp->y+j)*sc->h_res) ],&sp->map[spos], 1);
            spos+=1;
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


int move_sprit(Sprite *sp, Screen *sc ){

    uint32_t spos= 0;

    //admitindo que xspeed e yspeed > 0
    if(sp->x + sp->xspeed >= sc->h_res || sp->y + sp->yspeed >= sc->v_res){
        return 1;
    }  

    //acrescentar deteção de colisão

    /*Verifica se a memoria nao está NULL*/
    if (sc->video_mem==NULL)
        return 1;

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

    uint32_t color = xpm_transparency_color(XPM_8_8_8_8);

    /*Limpa esta regiao da vmem(onde o sprit estava)*/
    for(int j=0;j<sp->height;j++){
        for(int i=0;i<sp->width;i++){
            memcpy(&sc->video_mem[((sp->y * sc->h_res) + (sp->x + j))*4],&color,4);
        }
    }

    return 0;
}

void changeXY(Sprite *sp, int x, int y){ sp->x=x; sp->y=y; }

void changeSpeed(Sprite *sp, int xspeed, int yspeed){ sp->xspeed=xspeed; sp->yspeed= yspeed; }
    



/*
int move_sprit(uint16_t dif_x, uint16_t dif_y, uint8_t *sprite){

    uint16_t spos= 0;

    if(x_sprit + dif_x>=v_res || y_sprit + dif_y>=h_res){
        return 1;
    }

    //Verifica se a memoria nao está NULL
    if (video_mem==NULL)
        return 1;

    //Limpa o sprit
    clean_sprit();

    //Desenha o sprit na nova posição
    for(size_t j=0; j<s_height; j++){
        for(size_t i=0;i<s_width;i++){
            video_mem[(x_sprit+dif_x+i)+(y_sprit+j+dif_y)*h_res]=sprite[spos];
            spos++;
        }
    
    }

    //Atualiza as posições x e y
    x_sprit = x_sprit+dif_x;
    y_sprit = y_sprit+dif_y;
    
    return 0;
}
*/

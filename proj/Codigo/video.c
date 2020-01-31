#include <lcom/lcf.h>

#include <machine/int86.h>
#include <math.h>

#include "video.h"
#include "vidmacros.h"

/*Informacoes relativas a memoria*/
static uint8_t *video_mem;

/*Informações relativas ao modo*/
static uint16_t h_res;
static uint16_t v_res;
static uint16_t bits_per_pixel;
static uint8_t MemModel;                //0x04 se for indexado e 0x06 se for Direto

static uint8_t RedMaskSize;		        /* size of direct color red mask in bits */
static uint8_t RedFieldPosition;	    /* bit position of lsb of red mask */

static uint8_t GreenMaskSize;	        /* size of direct color green mask in bits */
static uint8_t GreenFieldPosition;	    /* bit position of lsb of green mask */

static uint8_t BlueMaskSize; 	        /* size of direct color blue mask in bits */
static uint8_t BlueFieldPosition;	    /* bit position of lsb of blue mask */

/*Informações relativas ao sprit*/
static uint16_t x_sprit;
static uint16_t y_sprit;
//static uint16_t s_width;
//static uint16_t s_height;

Screen * createSecreen(){

    //aloca memoria
    Screen *sc= (Screen *) malloc (sizeof(Screen));

    sc->video_mem=video_mem;
    sc->h_res=h_res;
    sc->v_res=v_res;
    sc->bits_per_pixel=bits_per_pixel;

    return sc;
}


int (vg_draw_hline)(uint16_t x, uint16_t y, uint16_t len, uint32_t color){

    if (x >= v_res || y >= h_res || x < 0 || y < 0)
        return 1;

    if (video_mem==NULL)
        return 1;

   
    uint16_t bytes_pixel;
  
    bytes_pixel=ceil((uint16_t) bits_per_pixel/8.0);

    for (size_t j = 0; j<len; j++){
        
        /*Esta função copia o os bytes da cor para a posicao pretendida*/
        memcpy(&video_mem[((x * h_res) + (y + j))*bytes_pixel],&color,bytes_pixel);
        
    }

    return 0;
}



void setPixelInd(uint32_t pos, uint32_t color){

    video_mem[pos]=color;
}




void *(vg_init)(uint16_t mode){

    /*Contem a informação do mode em que estamos*/
    vbe_mode_info_t info;

    /*Preenche a struct info*/
    if(vbe_get_mode_info_v2(mode,&info))
        return NULL;

    struct minix_mem_range mr;

    

    /*Limpa a struct mr*/
    memset(&mr,0,sizeof(struct minix_mem_range));

    mr.mr_base=(phys_bytes) info.PhysBasePtr;

    /*Tamanho do mr. É o (numero de pixeis em X * o numero de pixeis em Y * nº de bits por pixel) / 8 */
    uint32_t size=ceil((info.XResolution * info.YResolution * info.BitsPerPixel) / 8);

    mr.mr_limit= info.PhysBasePtr + size;

    /*Guarda as resoluções e os bits por pixeis em variaveis estaticas para utilizações futuras*/
    h_res= info.XResolution;
    v_res= info.YResolution;
    bits_per_pixel= info.BitsPerPixel;
    MemModel= info.MemoryModel;

    RedMaskSize=info.RedMaskSize;
    GreenMaskSize=info.GreenMaskSize;
    BlueMaskSize=info.BlueMaskSize;

    RedFieldPosition=info.RedFieldPosition;
    GreenFieldPosition=info.GreenFieldPosition;
    BlueFieldPosition=info.BlueFieldPosition;

    int res;
    
    /*Pedir permissão*/
    if( OK != (res = sys_privctl(SELF, SYS_PRIV_ADD_MEM, &mr)))
        return NULL;
    
    /*Mapear a memoria fisica para memoria virtual*/
    video_mem = (uint8_t*)vm_map_phys(SELF, (void *)mr.mr_base, size);

    if(video_mem == MAP_FAILED)
        panic("couldn't map video memory");

    
    /*Struct que contem o mode e a funcao*/
    reg86_t r;
    memset(&r,0,sizeof(reg86_t));

    /*Identifica a funcao VBE*/
    r.ax= SET_VBE_MODE;
    
    /*Diz o mode para qual por. Guarantir que o BIT n 14 esta a 1*/
    r.bx= BIT(14) | mode;
    
    /*Identifica que é uma chamada BIOS*/
    r.intno= INT;

    /*Transita para o modo de video pedido*/
    if(sys_int86(&r) != OK ) {
        printf("set_vbe_mode: sys_int86() failed \n");
        return NULL;
    }


    return (void *) video_mem;

}

int (vg_draw_rectangle) (uint16_t x,uint16_t y,uint16_t width,uint16_t height,uint32_t color){
    
  for(int i=0;i<height;i++){
    if(vg_draw_hline(x,y+i,width,color)){
      vg_exit();
      return 1;
    }
  }
  return 0;
}

/**
 *Funcao para obter cor pretendida na funcao video_test_pattern 
 */
uint32_t test_pattern_color(uint16_t x, uint32_t y, uint32_t first, uint8_t step, uint8_t no_rectangles){  

    if(MemModel == 0x06){
        uint32_t R_first= first <<(32- RedFieldPosition - RedMaskSize);
        uint32_t G_first= first <<(32- GreenFieldPosition - GreenMaskSize);
        uint32_t B_first= first <<(32- BlueFieldPosition - BlueMaskSize);

        uint32_t R = ((R_first >> (32-RedMaskSize)) + y * step) % (1 << RedMaskSize);
        uint32_t G = ((G_first >> (32-GreenMaskSize)) + x * step) % (1 << GreenMaskSize);
        uint32_t B = ((B_first >> (32-BlueMaskSize)) + (y + x) * step) % (1 << BlueMaskSize);	  

    return (R+G+B);
    }
    else{
        return (first + (x * no_rectangles + y) * step) % (1 << bits_per_pixel);

    }
}

uint16_t getVres(){return v_res;}

uint16_t getHres(){return h_res;}

uint16_t getXSprit(){return x_sprit;}

uint16_t getYSprit(){return y_sprit;}


int vbe_get_mode_info_v2(uint16_t mode,vbe_mode_info_t * vmi_p){


  
    
    phys_bytes buf;
    reg86_t r;
    mmap_t mem_alloc;

    //Testar erro é necessário?
    lm_alloc(sizeof(vbe_mode_info_t), &mem_alloc);
    

    buf = mem_alloc.phys;


    memset(&r,0,sizeof(r));

    r.ax=0x4F01;
    r.cx=BIT(14) | mode;
    //r.es:di =vmi_p;

    r.es = PB2BASE(buf); /* set a segment base */
    r.di = PB2OFF(buf); /* set the offset accordingly */
    r.cx = mode;
    r.intno = 0x10;
    

    
    if( sys_int86(&r) != OK ){
        lm_free(&mem_alloc);
        return 1;
    }



    memcpy(vmi_p,mem_alloc.virt,mem_alloc.size);
    lm_free(&mem_alloc);

    

    return 0;

}

void cleanScreen(Screen* sc){

    /*Verifica se a memoria nao está NULL*/
    if (sc->video_mem==NULL)
        return;

    uint32_t color = xpm_transparency_color(XPM_8_8_8_8);

    /*Limpa esta regiao da vmem(onde o sprit estava)*/
    for(int j=0;j<(sc->v_res*sc->h_res);j++){
      
        memcpy(&sc->video_mem[j*4],&color,4);
    
    }

    
}

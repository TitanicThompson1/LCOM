#include <lcom/lcf.h>
#include <math.h>

static uint8_t *video_mem;

void * vg_init(uint16_t mode)
{
    vbe_mode_info_t info;

    memset(&info,0,sizeof(vbe_mode_info_t));

    vbe_get_mode_info(mode,&info);

    //guardo informações aqui

    struct minix_mem_range mr;

    mr.mr_base = info.PhysBasePtr;

    uint32_t size= ceil((info.XResolution*info.YResolution*info.BitsPerPixel)/8);

    mr.mr_limit = mr.mr_base + size;
    
     /* Allow memory mapping */

    mr.mr_base = (phys_bytes) vram_base;	
    mr.mr_limit = mr.mr_base + vram_size;  

    int r;
    if( OK != (r = sys_privctl(SELF, SYS_PRIV_ADD_MEM, &mr)))
        panic("sys_privctl (ADD_MEM) failed: %d\n", r);

    /* Map memory */

    video_mem = (uint8_t *) vm_map_phys(SELF, (void *)mr.mr_base, vram_size);

    reg86_t r;

    memset(&r,0,sizeof(reg86_t));
    r.ax = 0x4F02; // VBE call, function 02 -- set VBE mode
    r.bx = 1<<14|mode; // set bit 14: linear framebuffer
    r.intno = 0x10;

    if( sys_int86(&r) != OK ) {
        printf("set_vbe_mode: sys_int86() failed \n");
        return 1;
    }

    return (void *) video_mem;
}

int vg_draw_hline (uint16_t x, uint16_t y, uint16_t len, uint32_t color){


    for(uint16_t i =0;i<len;i++){

        memcpy(&video_mem[(x+i+y*h_res)*bytes_pixel],&color,bytes_pixel);
    }
}
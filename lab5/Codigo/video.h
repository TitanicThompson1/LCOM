#ifndef _LCOM_VIDEO_H_
#define _LCOM_VIDEO_H_

typedef struct {
    
    uint8_t *video_mem;
    uint16_t h_res;
    uint16_t v_res;
    uint16_t bits_per_pixel;
} Screen;




int (vg_draw_hline)(uint16_t x, uint16_t y, uint16_t len, uint32_t color);

void setPixelInd(uint32_t pos, uint32_t color);


void *(vg_init)(uint16_t mode);


int (vg_draw_rectangle) (uint16_t x,uint16_t 	y,uint16_t 	width,uint16_t 	height,uint32_t color);

uint32_t test_pattern_color(uint16_t x, uint32_t y, uint32_t first, uint8_t step,uint8_t no_rectangles);

uint16_t getHres();

uint16_t getVres();

int vbe_get_mode_info_v2(uint16_t mode,vbe_mode_info_t * vmi_p);

uint16_t getXSprit();

uint16_t getYSprit();

Screen * createSecreen();


#endif /* _LCOM_VIDEO_H */

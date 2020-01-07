// IMPORTANT: you must include the following line in all your C files
#include <lcom/lcf.h>

#include <lcom/lab5.h>

#include "vidmacros.h"
#include "video.h"
#include "keyboard.h"
#include "i8024.h"
#include "sprite.h"

#include <stdint.h>
#include <stdio.h>

// Any header files included below this line should have been created by you

int main(int argc, char *argv[]) {
  // sets the language of LCF messages (can be either EN-US or PT-PT)
  lcf_set_language("EN-US");

  // enables to log function invocations that are being "wrapped" by LCF
  // [comment this out if you don't want/need it]
  lcf_trace_calls("/home/lcom/labs/lab5/trace.txt");

  // enables to save the output of printf function calls on a file
  // [comment this out if you don't want/need it]
  lcf_log_output("/home/lcom/labs/lab5/output.txt");

  // handles control over to LCF
  // [LCF handles command line arguments and invokes the right function]
  if (lcf_start(argc, argv))
    return 1;

  // LCF clean up tasks
  // [must be the last statement before return]
  lcf_cleanup();

  return 0;
}

int(video_test_init)(uint16_t mode, uint8_t delay) {

  reg86_t r;

  /*Limpa a struct r*/
  memset(&r,0,sizeof(reg86_t));

  /*Indica que queremos mudar o MODE*/
  r.ax= SET_VBE_MODE;

  /*Indica o modo para qual queremos mudar*/
  r.bx= BIT(14) | mode;
 
  r.intno= INT;

  /*Muda o VBE_MODE*/
  if( sys_int86(&r) != OK ) {
    printf("set_vbe_mode: sys_int86() failed \n");
    return 1;
  }

  /*"Adormece" o programa durante os segundos especificados em delay*/
  sleep(delay);
  
  /*Restore MINIX default*/
  if(vg_exit())
    return 1;

  return 0;
}

int(video_test_rectangle)(uint16_t mode, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color) {

  /*Inicia o VBE no modo pretendido*/
  vg_init(mode);

  for(size_t i=0;i<height;i++){

    if(vg_draw_hline(x+i,y,width,color)){
      vg_exit();
      printf("Erro na hline()");
      return 1;
    }
   
  }
  
  /*Verifica se o utilizador prime a tecla ESC*/
  if(verifica()){
    vg_exit();
    printf("ERRO NO VERIFICA!\n");
    return 1;
  }
  /*Restore MINIX default*/
  vg_exit();
  return 0;
  
}

int(video_test_pattern)(uint16_t mode, uint8_t no_rectangles, uint32_t first, uint8_t step) {
  
  vg_init(mode);


  uint16_t width = getHres()/no_rectangles;
  uint16_t height = getVres()/no_rectangles;

  for(int i = 0; i < no_rectangles; i++){
    for(int j= 0; j < no_rectangles; j++){
      // obter cor para desenhar
      uint32_t color = test_pattern_color(i,j,first,step, no_rectangles);

      if(vg_draw_rectangle(j*width,i*height,width,height,color)){
        vg_exit();
        return 1;
      }
    
      //mudar a cor para o próximo rectangulo
    }


  }

  if(verifica())
    return 1;

  vg_exit();
  return 0;  
}

int(video_test_xpm)(xpm_map_t xpm, uint16_t x, uint16_t y) {

  //Inicializa para o video module para o modo pretendido
  vg_init(COLOR_MODE_INDEXED);

  //cria ecrã
  Screen *sc=createSecreen();

  Sprite * sp=create_sprite(xpm);

  changeXY(sp,x,y);
  
  /*Copia o pixelmap para a video memory*/
  copy_sprite_to_vmem(sp,sc);

  /*Verifica se o utilizador prime a tecla ESC*/
  if(verifica())
    return 1;

  /*Restore MINIX default*/
  vg_exit();
  

  return 0;
}

int(video_test_move)(xpm_map_t xpm, uint16_t xi, uint16_t yi, uint16_t xf, uint16_t yf, int16_t speed, uint8_t fr_rate) {
  
  *//Inicializa para o video module para o modo pretendido
  vg_init(COLOR_MODE_INDEXED);

  bool parou=false;
  uint8_t speedNegativa=0;

  //Pretendemos o tipo XPM_INDEXED
  enum xpm_image_type type = XPM_INDEXED;
  xpm_image_t img;

  //Limpar img
  memset(&img,0,sizeof(xpm_image_t));

  //xpm_load transforma um xpm para um pixelmap
  uint8_t *sprit = xpm_load(xpm, type, &img);


  // Keyboard-dependent part, pois o utilizador pode cancelar quando quiser 

  //Muda-se a frequencia para os frames_second
  if(timer_set_frequency(0,(uint32_t) fr_rate)){
    vg_exit();
    printf("Frequencia demasiado baixa!\n");
    return 1;
  }


  uint8_t irq_kb, irq_timer0, size=1;          //irq_set-> numero do bit a por a 1 ;  size->tamanho do array
                                     

  int ipc_status;                   
  message msg;
  int r;

  //Array com os scancodes
  uint8_t to_print[2];

  if(timer_subscribe_int(&irq_timer0)){   //subscreve as interrupções do timer0
    vg_exit();
    return 1;
  }
  irq_timer0=BIT(irq_timer0);         //byte com a linha IRQ que queremos ler as interrupções

  if(kb_subscribe_int(&irq_kb)){          //subscreve as interrupções do teclado
    vg_exit();
    return 1;
  }
  irq_kb=BIT(irq_kb);                 //byte com a linha IRQ que queremos ler as interrupções


  copy_sprite_to_vmem(xi,yi,img.width,img.height,sprit);


  while (to_print[0] != END_SCAN) {
      // Get a request message. 
      
      if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0) {
      printf("driver_receive failed with: %d", r);
      continue;
      }
      if (is_ipc_notify(ipc_status)) {

        switch (_ENDPOINT_P(msg.m_source)) {

            case HARDWARE:

              if(msg.m_notify.interrupts & irq_timer0){

                //Apenas para speed>0; falta fazer para speed<0
                if(!parou){
                  
                  if(xi==xf){

                    if(speed >0) {

                      if((getYSprit() + speed) > yf){
                        move_sprit(0, yf-getYSprit(),sprit);
                        parou=true;

                      }else if(move_sprit(0,speed,sprit)){
                        parou=true;
                      }
                    }else if(speed < 0){
                      speedNegativa++;

                      if(speedNegativa%(-speed)==0){

                        if((getYSprit() + 1) > yf){
                          parou=true;
                        }
                        else if(move_sprit(0,1,sprit)){
                          parou=true;
                        }
                      }
                    }else{
                      parou=true;
                    }
                  }
                  else{
                    if(speed > 0){
                      if(getXSprit() + speed > xf){
                        move_sprit(0, xf-getXSprit(),sprit);
                        parou=true;
                      }
                      else if(move_sprit(speed, 0, sprit)){
                        parou=true;
                      }
                    }else if(speed < 0){
                      speedNegativa++;

                      if(speedNegativa%(-speed)==0){

                        if(getXSprit() + 1 > xf){
                          parou=true;
                        }
                        else if(move_sprit(1, 0, sprit)){
                          parou=true;
                        }
                      }
                    }else {
                      parou=true;
                    }
                  }
                }
              }
            
              if (msg.m_notify.interrupts & irq_kb) { // subscribed interrupt 

                //chama o Interruption Handler
                kbd_ih();

                //verifica se o scancode tem 2bytes; se tiver, passa para a proxima interrupção
                if(getTwo_part()){
                  size=2;
                  continue;
                }
                getScancodes(to_print);
                //apos imprimir, assume-se que o proximo scancode tem 1byte de tamanho
                size=1;
              }
              break;

            default:
              break;
          }
        }
      else { 
      
      }
  }

  //cancela a subscricao do teclado
  if (kb_unsubscribe_int()){
    vg_exit();
    printf("Erro na kb_unsubscribe_int()!");
    return 1;
  }

  //cancela a subscricao do timer
  if(timer_unsubscribe_int()){
    vg_exit();
    printf("Erro na timer_unsubscribe_int()!");
    return 1;
  }

  //põe a frequencia do timer0 para o default value
  if(timer_set_frequency(0,60)){
    vg_exit();
    return 1;
  }

  vg_exit();
  */
  
  return 0;
}

int(video_test_controller)() {
  /* To be completed */
  printf("%s(): under construction\n", __func__);

  return 1;
}
/

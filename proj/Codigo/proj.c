// IMPORTANT: you must include the following line in all your C files
#include <lcom/lcf.h>
#include <lcom/liblm.h>
#include <lcom/proj.h>


#include <stdbool.h>
#include <stdint.h>

// Any header files included below this line should have been created by you

#include "menuInicial1.xpm"
//#include "menuPlayer.xpm"
#include "pacman2.xpm"
#include "keyboard.h"
#include "video.h"
#include "sprite.h"
#include "i8024.h"
#include "state_machine.h"
#include "evento.h"



//static variables


int main(int argc, char *argv[]) {
  // sets the language of LCF messages (can be either EN-US or PT-PT)
  lcf_set_language("EN-US");

  // enables to log function invocations that are being "wrapped" by LCF
  // [comment this out if you don't want/need it]
  lcf_trace_calls("/home/lcom/labs/proj/trace.txt");

  // enables to save the output of printf function calls on a file
  // [comment this out if you don't want/need it]
  lcf_log_output("/home/lcom/labs/proj/output.txt");

  // handles control over to LCF
  // [LCF handles command line arguments and invokes the right function]
  if (lcf_start(argc, argv))
    return 1;

  // LCF clean up tasks
  // [must be the last statement before return]
  lcf_cleanup();

  return 0;
}

static int print_usage() {
  printf("Usage: <mode - hex>\n");

  return 1;
}

int(proj_main_loop)(int argc, char *argv[]) {
  /* 
   * Substitute the code below by your own
   */

  //
  // if you're interested, try to extend the command line options so that the usage becomes:
  // <mode - hex> <minix3 logo  - true|false> <grayscale - true|false> <delay (secs)>
  //
  /*
  bool const minix3_logo = true;
  bool const grayscale = false;
  uint8_t const delay = 5;
  */
  uint16_t mode;

  if (argc != 1)
    return print_usage();

  // parse mode info (it has some limitations for the sake of simplicity)
  if (sscanf(argv[0], "%hx", &mode) != 1) {
    printf("%s: invalid mode (%s)\n", __func__, argv[0]);

    return print_usage();
  }

  /*Driver-receiver*/

  vg_init(0x14C);

  initSprites();

  Screen *sc = createSecreen();

  Sprite *sp= create_sprite(menuInicial1);
  
  if(sp==NULL){
    vg_exit();
    printf("Erro ao criar sprit!\n");
    return 1;
  }

  copy_sprite_to_vmem(sp,sc);


  uint8_t menuAtual=1;


  uint8_t irq_kb, size=1;          //irq_set-> numero do bit a por a 1 ;  size->tamanho do array
  uint8_t irq_timer;
  
  int ipc_status;                   
  message msg;
  int r;

  //Array com os scancodes
  uint8_t to_print[2];

  kb_subscribe_int(&irq_kb);       //subscreve as interrupções do teclado

  timer_subscribe_int(&irq_timer);

  irq_kb=BIT(irq_kb);             //byte com a linha IRQ que queremos ler as interrupções

  irq_timer=BIT(irq_timer);


  evento ev;
  Scancode *scan= malloc(sizeof(Scancode));

  while (to_print[0]!= END_SCAN) {
    /* Get a request message. */
    
    if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0) {
      printf("driver_receive failed with: %d", r);
      continue;
    }
     if (is_ipc_notify(ipc_status)) {
      switch (_ENDPOINT_P(msg.m_source)) {
        case HARDWARE:



          if(msg.m_notify.interrupts & irq_timer){
            
            if(menuAtual==3){

              enum IdentificaPeriferico per=Timer;
              ev=loadEvento(per,NULL,NULL);
              menus(ev,sc);
            
            }
          }
          
          if (msg.m_notify.interrupts & irq_kb) { /* subscribed interrupt */
                
            
            //chama o Interruption Handler
            kbd_ih();

            //verifica se o scancode tem 2bytes; se tiver, passa para a proxima interrupção
            if(getTwo_part()){
              size=2;
              continue;
            }
            getScancodes(to_print);
            
                      
            //copiar os scancodes do print para mandar para a state machine
            for(int i=0;i<size;i++) scan->bytes[i]=to_print[i];

            scan->size=size;

            enum IdentificaPeriferico per=Keyboard;

            ev = loadEvento(per,scan,NULL);
            
            menuAtual=menus(ev,sc);


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
  if (kb_unsubscribe_int())
      return 1;

  vg_exit();


  return 0;
  //return proj_demo(mode, minix3_logo, grayscale, delay);
}

// IMPORTANT: you must include the following line in all your C files
#include <lcom/lcf.h>
#include <stdint.h>
#include <stdio.h>

#include "mouse.h"
#include "i8024.h"

extern int tempo;

// Any header files included below this line should have been created by you


int main(int argc, char *argv[]) {
  // sets the language of LCF messages (can be either EN-US or PT-PT)
  lcf_set_language("EN-US");

  // enables to log function invocations that are being "wrapped" by LCF
  // [comment this out if you don't want/need/ it]
  lcf_trace_calls("/home/lcom/labs/lab4/trace.txt");

  // enables to save the output of printf function calls on a file
  // [comment this out if you don't want/need it]
  lcf_log_output("/home/lcom/labs/lab4/output.txt");

  // handles control over to LCF
  // [LCF handles command line arguments and invokes the right function]
  if (lcf_start(argc, argv))
    return 1;

  // LCF clean up tasks
  // [must be the last statement before return]
  lcf_cleanup();

  return 0;
}


int (mouse_test_packet)(uint32_t cnt) {

  uint16_t irq_set;     //irq_set-> numero do bit a por a 1 ; 

  uint8_t counter=0;

  struct packet *pp;

  int ipc_status;                   

  message msg;

  int r;
  
  if(mouse_subscribe_int(&irq_set)) return 1;       //subscreve as interrupções do rato

  if(mouse_data_reporting(true))
    return 1;
    
  
  
  irq_set=BIT(irq_set);             //byte com a linha IRQ que queremos ler as interrupções
  
  while (cnt != 0) {//irá obter interrupcoes até lermos o número de packs dados por cnt pretendidos

    /* Get a request message. */
    
    if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0) {
      printf("driver_receive failed with: %d", r);
      continue;
    }


    if (is_ipc_notify(ipc_status)) {

      switch (_ENDPOINT_P(msg.m_source)) {

        case HARDWARE:

          if (msg.m_notify.interrupts & irq_set) { /* subscribed interrupt */
            
            //chama o Interruption Handler

            //variável counter é incrementada se lemos algo com èxito!

            mouse_ih();
            
            counter++;

            if(counter == 3){
              pp = parse();  
              
              mouse_print_packet(pp);//verificar se são estes os parâmetros     

              cnt--; //decrementar cnt, números de packs que queremos ler

              counter = 0;//ler próxima data pack do mouse 
              
            }
            
          }
          break;

        default:
          break;
      }

    }
    else { 
    }
  }

  //desativa o data reporting
  if (mouse_data_reporting(false)) return 1;
  
  
  //faz unsubscibe
  if (mouse_unsubscribe_int()) return 1;
  
  return 0;

}

int (mouse_test_remote)(uint16_t period, uint8_t cnt) {
  
  uint8_t counter=0;

  struct packet *pp;

  while(counter!=cnt){

    //lê o data package do rato
    if(read_package()){
      
       tickdelay(micros_to_ticks(period*1000));
       continue;
    }
    
    //Incrementa o numero de pacotes
    counter++;
    //configura a struct packet 
    pp = parse();

    //imprime a packet
    mouse_print_packet(pp);

    //espera period ms antes pedir outra vez o packet
    tickdelay(micros_to_ticks(period*1000));             
  }
  
  mouse_data_reporting(false);

  write_command(KBC_CMD_REG, WRITE_COMAND);
  write_command(OUT_BUF,minix_get_dflt_kbc_cmd_byte());
  
  return 0;
}

int (mouse_test_async)(uint8_t idle_time) {
  
  struct packet *pp;

  uint16_t irq_mouse;   //irq_set-> numero do bit a por a 1 ;  size->tamanho do array
  uint8_t irq_timer0;
  int freq;
  uint8_t counter=0;


  //variáveis para usar nas interrupcoes!!!
  int ipc_status;                   

  message msg;

  int r;

  //pointer para a struct ,para guardar bytes e informacoes do mouse  

  

  if(mouse_subscribe_int(&irq_mouse)) return 1;       //subscreve as interrupções do teclado

  if(mouse_data_reporting(true)) 
    return 1;


  irq_mouse=BIT(irq_mouse);             //byte com a linha IRQ que queremos ler as interrupções


  //subscreve as interrupções do timer0
  timer_subscribe_int(&irq_timer0);
  irq_timer0=BIT(irq_timer0);

  freq=sys_hz();


  while (tempo/freq < idle_time) {//irá obter interrupcoes até lermos o número de packs dados por cnt pretendidos
  /* Get a request message. */
  
  if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0) {
      printf("driver_receive failed with: %d", r);
      continue;
  }

  if (is_ipc_notify(ipc_status)) {

    switch (_ENDPOINT_P(msg.m_source)) {

      case HARDWARE:

        if (msg.m_notify.interrupts & irq_timer0) {
          timer_int_handler();
        }

        if (msg.m_notify.interrupts & irq_mouse) { /* subscribed interrupt */
          tempo=0;
         

          mouse_ih();
          counter++;
          if(counter == 3){
            pp = parse();  
            
            mouse_print_packet(pp);      //verificar se são estes os parâmetros     

            counter = 0;      //ler próxima data pack do mouse 

          }
          
        }
        break;

      default:
        break;
    }

  }
  else { 
  }
  }

  if(mouse_data_reporting(false)) return 1;

  if (mouse_unsubscribe_int()) return 1;

  if (timer_unsubscribe_int()) return 1;

  

  return 0;
}

int (mouse_test_gesture)(uint8_t x_len, uint8_t tolerance) {

  uint16_t irq_set;     //irq_set-> numero do bit a por a 1 ; 

  uint8_t counter=0;

  struct packet *pp;

  int ipc_status, state=0;                   

  message msg;

  int r;

  
  if(mouse_subscribe_int(&irq_set)) return 1;       //subscreve as interrupções do teclado

  if(mouse_data_reporting(true))
    return 1;
    
  
  
  irq_set=BIT(irq_set);             //byte com a linha IRQ que queremos ler as interrupções
  
  while (state != 4 ) {//irá obter interrupcoes até lermos o número de packs dados por cnt pretendidos

    /* Get a request message. */
    
    if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0) {
        printf("driver_receive failed with: %d", r);
        continue;
    }

    

    if (is_ipc_notify(ipc_status)) {

      switch (_ENDPOINT_P(msg.m_source)) {

        case HARDWARE:

          if (msg.m_notify.interrupts & irq_set) { /* subscribed interrupt */
            
            //chama o Interruption Handler

            //variável counter é incrementada se lemos algo com èxito!

            mouse_ih();
            counter++;
            if(counter == 3){
              pp = parse();  
              
              mouse_print_packet(pp);       //verificar se são estes os parâmetros     

              

              counter = 0;  //ler próxima data pack do mouse 
              
              //chama a state machine
              state = statemachine(pp, x_len, tolerance );
            }
            
          }
          break;

        default:
          break;
      }

    }
    else { 
    }
  }

  
  if (mouse_data_reporting(false)) return 1;
  if (mouse_unsubscribe_int()) return 1;

  return 0;
  
}

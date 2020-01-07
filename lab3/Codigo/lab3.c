#include <lcom/lcf.h>

#include <lcom/lab3.h>

#include <stdbool.h>
#include <stdint.h>

#include "i8024.h"
#include "keyboard.h"
#include "utilidade.h"


extern uint8_t to_print[2];             //array que irá conter os scancodes
extern int counter;
extern bool two_parts;
extern uint8_t scancode;

int count=0;  

int main(int argc, char *argv[]) {
  // sets the language of LCF messages (can be either EN-US or PT-PT)
  lcf_set_language("EN-US");

  // enables to log function invocations that are being "wrapped" by LCF
  // [comment this out if you don't want/need it]
  lcf_trace_calls("/home/lcom/labs/lab3/trace.txt");

  // enables to save the output of printf function calls on a file
  // [comment this out if you don't want/need it]
  lcf_log_output("/home/lcom/labs/lab3/output.txt");

  // handles control over to LCF
  // [LCF handles command line arguments and invokes the right function]
  if (lcf_start(argc, argv))
    return 1;

  // LCF clean up tasks
  // [must be the last statement before return]
  lcf_cleanup();

  return 0;
}

int(kbd_test_scan)() {

  uint8_t irq_set, size=1;          //irq_set-> numero do bit a por a 1 ;  size->tamanho do array
  bool make;                        //indica se é um make ou break code

  int ipc_status;                   
  message msg;
  int r;

  kb_subscribe_int(&irq_set);       //subscreve as interrupções do teclado

  irq_set=BIT(irq_set);             //byte com a linha IRQ que queremos ler as interrupções

  while (to_print[0] != END_SCAN) {
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
            kbd_ih();

            //verifica se o scancode tem 2bytes; se tiver, passa para a proxima interrupção
            if(two_parts){
              size=2;
              continue;
            }
            make = ((to_print[size-1] & MSB) >> 7);
            kbd_print_scancode(!make, size, to_print);

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
  if (kb_unsubscribe_int())
      return 1;

  if (kbd_print_no_sysinb(count))
    return 1;

  return 0;
}

int(kbd_test_poll)() {
   
  uint8_t size=1,aux;
  uint8_t to_printv2[2];
 

  while(scancode != END_SCAN){
    //lemos scancode
    kbd_ihv2();           
    //lidamos com o tamanho e tipo do scancode e imprimimos
    
   if( scancode == FIRST_PART){
      
      size =2;
      to_printv2[0] = scancode;    
      continue;

   }

   else{
     if(size == 2){
       to_printv2[1] = scancode; 
       kbd_print_scancode(!(scancode & BIT(7)),size,to_printv2);
       
       size = 1;
        
     }
     else{
        
        to_printv2[0] = scancode;
        kbd_print_scancode(!(scancode & BIT(7)),size,to_printv2);         
        size = 1;
        
     }
     
     
   }

 

  }  
      
  kbd_print_no_sysinb(count);

  //Habilitar de novo interrupcoes

  write_keyboard_cmd( STAT_REG, READ_COMAND);          //ler commandbyte para podermos habilitar interrupcoes
  
  // guardar o que temos em OBF/command byte para podermos de seguida habilitar interrupcoes  
  util_sys_inb(OUT_BUF,&aux);
  
  write_keyboard_cmd(STAT_REG, WRITE_COMAND);      //"Dizemos" ao registo para escrever
  
  
  aux = (aux | OBF_INT_ENABLE);
 
  write_keyboard_cmd (ARG_WRITING, aux);            // habilitar interrupcoes (porta 60)
  


  
  return 0;

}


int(kbd_test_timed_scan)(uint8_t n) {
    
  uint8_t irq_kb, size=1, irq_timer0;
  bool make;

  int ipc_status;
  message msg;
  int r;

  //subscreve as interrupções do teclado
  kb_subscribe_int(&irq_kb);
  irq_kb=BIT(irq_kb);

  //subscreve as interrupções do timer0
  timer_subscribe_int(&irq_timer0);
  irq_timer0=BIT(irq_timer0);

  //põe a frequencia do timer a 60Hz
  if (timer_set_frequency(0,60))
    return 1;

  //condicao de paragem
  while (to_print[0] != END_SCAN && counter/60 < n) {
    /* Get a request message. */
    
    if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0) {
      printf("driver_receive failed with: %d", r);
      continue;
    }
     if (is_ipc_notify(ipc_status)) {
      switch (_ENDPOINT_P(msg.m_source)) {
        case HARDWARE:

          if (msg.m_notify.interrupts & irq_timer0) { /* subscribed interrupt */

            //sempre que o timer manda uma interrupção, conta
            timer_int_handler();
          }

          if (msg.m_notify.interrupts & irq_kb) { /* subscribed interrupt */

            //sempre que o teclado manda uma interrupção, 
            //o contador de interrupções do timer0 entre interrupcoes do teclado reinicia
            counter=0;

            //chama o Interruption Handler
            kbd_ih();

            //verifica se o scancode tem 2bytes; se tiver, passa para a proxima interrupção
            if(two_parts){
              size=2;
              continue;
            }

            make = ((to_print[size-1] & MSB) >> 7);
            kbd_print_scancode(!make, size, to_print);
            
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

  //necessario tirar a subscricao das linhas a que subscrevemos
  if (kb_unsubscribe_int())
    return 1;
  if (timer_unsubscribe_int())
    return 1;

  return 0;
}

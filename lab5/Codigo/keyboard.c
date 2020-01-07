#include <lcom/lcf.h>
#include <minix/sysutil.h>
#include "i8024.h"
#include <stdint.h>

static int kbHookId = 1;
int count;
static uint8_t to_print[2];
static bool two_parts=false;
uint8_t scancode;


int kb_subscribe_int(uint8_t *bit_no) {

  //verifica se o pointer é nulo
  if (bit_no == NULL)
    return 1;

  *bit_no = kbHookId;

  //subscreve interrupções à linha IRQ pretendida (neste caso, o keyboard),
  // e impede o IH do minix3 ler o OUT_BUF antes do nosso codigo
  if (sys_irqsetpolicy(KB_IRQLINE, IRQ_REENABLE | IRQ_EXCLUSIVE, &kbHookId) != OK)
    return 1;

  return 0;
}

int kb_unsubscribe_int() {

  //retira a subscrição
  if (sys_irqrmpolicy(&kbHookId) != OK)
    return 1;
  else
    return 0;
}

void (kbd_ih)() {

  
  uint8_t stat, lixo, temp=0x00;

  //lê o status byte
  util_sys_inb(STAT_REG, &stat);
  
  //se o OUT_BUF estiver cheio
  if (stat & OBF) {
    
    //se nao ocorreu erro de paridade ou timeout
    if ((stat & TP_ERROR) == 0x00) {
      
      //le o que esta no OUT_BUF
      util_sys_inb(OUT_BUF, &temp);
      
      //se se está à espera do segundo byte de um scancode
      if(two_parts)
      {
        to_print[1]=temp;

        //assume-se que o proximo scancode seja de um byte
        two_parts=false;
      }
      else
      {
        to_print[0]=temp;
        
      }      
      
    }
    else {
      //se ocorreu erro de paridade/timeout, esvazia se o OUT_BUF
      util_sys_inb(OUT_BUF, &lixo);
    }
    
  }
  
  //se o byte lido é a primeira parte 
  if (temp == FIRST_PART) 
    two_parts=true;
}

//funcao que verifica erros no
int kbc_errors (uint8_t status){


  if((status & TP_ERROR) != 0x00){
    return 1;
  }
  
  if((status & AUX_MOUSE) != 0x00){
    return 1;
  }
  return 0;

}

//funcao que permite escrever comandbytes de forma generalizada
int write_keyboard_cmd(int port, uint8_t cmd){
  
  
  uint8_t status;

  
  while( 1 ){
    if(util_sys_inb(STAT_REG, &status ) != 0){
      continue;
    }

    if(kbc_errors(status) != 0) {//se não temos erros no status
      continue;
    }

    if((status & IBF_FULL) != 0){// input buffer tem de estar vazio para escrevermos algo
      continue;
    }
        
    if(sys_outb(port, cmd) != 0){// o erro estava aqui possivelmente
      continue;        
    }
    else
          return 0;//escrevemos o que queriamos
    
    tickdelay(micros_to_ticks(DELAY_US));    
    }
      

    
  
}


void kbd_ihv2(){

  //usar variável global
  //int i = 0;
  uint8_t status;
  
  while(1){

  if(util_sys_inb(STAT_REG, &status)==0){
    //verificamos o estado do registo
    if(kbc_errors(status) == 0) {
        
      //verifica se OBF está cheio ou nao
      if((status & OBF_FULL) == 1){

        

        //Vamos buscar conteúdo do OBF
        if(util_sys_inb(OUT_BUF, &scancode) == 0)
          //foi possível ler scancode!
          break;
        
      }
    }
  
  }
  
  tickdelay(micros_to_ticks(DELAY_US));
  }
 // tickdelay(micros_to_ticks(DELAY_US));

}

int verifica(){
    
  uint8_t irq_set, size=1;          //irq_set-> numero do bit a por a 1 ;  size->tamanho do array
                                     

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

  return 0;
}

bool getTwo_part(){
  return two_parts;
}

void getScancodes(uint8_t array[]){

  array[0]=to_print[0];
  if(two_parts){
    array[1]=to_print[1];
  }
}


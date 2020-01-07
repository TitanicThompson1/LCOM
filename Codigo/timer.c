#include <lcom/lcf.h>
#include <lcom/timer.h>

#include <stdint.h>

#include "i8254.h"
#include "utilidade.h"


int timerHookId= 0;

int tempo=0;

void reset();
int getCounter();

int (timer_subscribe_int)(uint8_t *bit_no) {

  //verifica se o pointer é nulo
  if(bit_no==NULL)
    return 1;

  
  *bit_no = timerHookId;

  //subscreve interrupções à linha IRQ pretendida (neste caso, o Timer0)
  if(sys_irqsetpolicy(TIMER0_IRQ,IRQ_REENABLE, &timerHookId)!=OK)
    return 1;
  
  return 0;
}

int (timer_unsubscribe_int)() {
  
  //retira a subscrição
  if (sys_irqrmpolicy(&timerHookId)!=OK)
    return 1;
  else 
    return 0;
}

void (timer_int_handler)() {

  //incrementa o contador
  tempo++;
}


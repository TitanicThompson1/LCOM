#include <lcom/lcf.h>
#include <lcom/timer.h>

#include <stdint.h>

#include "i8254.h"
#include "utilidade.h"

int counter=0;
int timerHookId= 0;

int (timer_set_frequency)(uint8_t timer, uint32_t freq) {
  
  
  //Verificar se número identificativo do time é correcto
  if(timer>2 || timer < 0){
    printf("%s timer inserido inválido\n");
    return 1;

  }

  //Necessitamos obter configuração do timer actual, para de seguida inserir frequência pretendida
  uint8_t actual_conf;
  
  if(timer_get_conf(timer,&actual_conf)){
    printf("Erro com configuração do timer!\n");
    
    return 1;
  }

  //construir control word conservando os 4 LSBits
  uint32_t control_word;

  //Conservar 4 LSB
  control_word = actual_conf & TIMER_NOT_CHANGE_4LSB;

  //indica que se inicializa primeiro LSB e depois MSB 
  control_word = control_word | TIMER_LSB_MSB;

  //definir o timer pretendido
  switch(timer)
  {
    case 0:
      
      control_word = control_word | TIMER_SEL0;
      break;

    case 1:
      control_word = control_word | TIMER_SEL1;
      break;

    case 2:
      control_word = control_word | TIMER_SEL2;
      break;

  }



  //usar a control word indicando ao control register que nuances queremos programar no TIMER
  if(sys_outb(TIMER_CTRL, control_word)!=OK)
    return 1;


  //Usar razão entre frequência do relógio PC e a frequência pretendida TIMER_FREQ /X = Freq
  
  
 /*
  Um timer tem dois registos, cada um com 1 byte, num total de 16 bits, assim o valor max limite
  que pode ser escrito no timer é de 0 a 2^(15)
  Logo o valor max que podemos escrever TIMER_FREQ, tendo em conta a razão em cima e que evita divisão por 0!!!

  */
  //VALOR MAX
  if(freq > TIMER_FREQ){
    printf("Frequencia acima do limite máximo\n");
    return 1;



  }

  //VALOR MINIMO, dado pela razão que nos permite ter a freq pretendida, ou seja, fazer 
  //timer_freq / 65 535 (2^16-1), onde 65 535 é o valor máximo que podemos escrever no timer0 para obter uma
  //frequencia onde usaremos o valor inteiro 19
  
  if( freq < 19) {
    printf("Frequencia abaixo do limite minimo\n");
    return 1;
  }
  

  //valor a escrever no timer
  uint16_t X = TIMER_FREQ /freq;

  //Obter os valores LSB e MSB para escrever no timer e modificar para a frequẽncia pretendida
  //incializar variáveis para guardar valores LSB e MSB para escrever posteriormente no timer pretendido
  uint8_t LSB = 0;
  uint8_t MSB = 0;

  if(util_get_LSB(X, &LSB)!=OK)
    return 1;

  if(util_get_MSB(X, &MSB)!=OK)
    return 1;

  //escrever valor no timer, de forma a ter a frequencia pretendida
  uint32_t timer_selection;

  //seleciona o port do timer
  switch (timer)
  {
  case 0:
    timer_selection= TIMER_0;
    break;
  case 1:
    timer_selection= TIMER_1;
    break;
  case 2:
    timer_selection= TIMER_2;
    break;
  default:

    //tive que adicionar esta linha de codigo pois o compilador apresentava erro
    //devido à possibilidade de o timer_selection ficar não inicializado  
    timer_selection= 0;
    break;
  }

  if(sys_outb(timer_selection,LSB)!=OK)
    return 1;

  if(sys_outb(timer_selection,MSB)!=OK)
    return 1;
  

  return 0;
}

int (timer_subscribe_int)(uint8_t *bit_no) {

  //verifica se o pointer é nulo
  if(bit_no==NULL)
    return 1;

  
  *bit_no = timerHookId;

  //subscreve interrupções à linha IRQ pretendida (neste caso, o Timer0)
  if(sys_irqsetpolicy(TIMER0_IRQ,IRQ_REENABLE, &timerHookId)!=OK)
    return 1;
  
  //comentamos porque estava a dar warnings ao testar
  /*
  if(sys_irqenable(&timerHookId)!=OK)
    return 1;
  */
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
  counter++;
}

int (timer_get_conf)(uint8_t timer, uint8_t *st) {

  //verifica se o pointer é nulo
  if (st==NULL)
    return 1;

  // verifica se o timer escolhido é valido
  if(timer<0 || timer> 2) 
  {
      return 1;
  }

  //escrita do comando Read Back
  uint32_t read_back= TIMER_RB_CMD | TIMER_RB_COUNT_ | TIMER_RB_SEL(timer);

  //verifica se a função sys_outb não deu erro
  if(sys_outb(TIMER_CTRL,read_back)!=OK)
    return 1;

  
  uint8_t timer_configuration;    //esta variavel irá guardar a configuração do timer
  uint32_t timer_selection;       //esta variavel irá guardar qual o timer selecionado

  //verifica qual o timer que se quer ler
  switch (timer)
  {
  case 0:
    timer_selection= TIMER_0;
    break;
  case 1:
    timer_selection= TIMER_1;
    break;
  case 2:
    timer_selection= TIMER_2;
    break;
  default:

    //tive que adicionar esta linha de codigo pois o compilador apresentava erro
    //devido à possibilidade de o timer_selection ficar não inicializado  
    timer_selection= 0;
    break;
  }

  //lê a configuração
  if(util_sys_inb(timer_selection,&timer_configuration)!=OK)
    return 1;


  //escreve no lugar de memoria guardado no pointer st o valor da configuração
  *st=timer_configuration;

  return 0;
}

int (timer_display_conf)(uint8_t timer, uint8_t st, enum timer_status_field field) {

  
  union timer_status_field_val configuration;
  uint32_t temp;

  //verifica qual o campo a imprimir
  switch (field)
  {
  case tsf_all:
  
    configuration.byte= st;
    break;
  case tsf_initial:

    //seleciona os bits correspondentes ao initialization mode
    temp= st & TIMER_SEL_IN_MODE;

    // verifica qual o initialization mode
    switch (temp)
    {
    case TIMER_LSB:

      configuration.in_mode= LSB_only;
      break;
    case TIMER_MSB:
      configuration.in_mode= MSB_only;
      break;
    case TIMER_LSB_MSB:
      configuration.in_mode= MSB_after_LSB;
      break;
    default:
      configuration.in_mode= TIMER_INIT_INVALID;
      break;
    }

    break;

  //seleciona o count mode
  case tsf_mode:

    configuration.count_mode= st & TIMER_SEL_COUNT;
    configuration.count_mode>>= 1;

    if(configuration.count_mode == 6)
      configuration.count_mode= 2;
    
    else if(configuration.count_mode == 7)
      configuration.count_mode= 3; 
    break;

  //seleciona a base
  case tsf_base:
    configuration.bcd= st & TIMER_SEL_BCD;
    break;
  default:
    break;
  }

  //faz print da configuração pretendida
  timer_print_config(timer, field, configuration);

  return 0;
}


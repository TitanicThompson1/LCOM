#include <lcom/lcf.h>

#include "i8024.h"
#include "mouse.h"


static bool subscribed=false;     //identifica se o mouse esta subscrito à IRQ line
static uint8_t counter=0;         //conta quantos bytes já recebeu
static int state=0;               //estado atual da state machine
static int16_t x, y;              //deslocação em x e em y total



uint8_t mouse_byte;

int MouseHookId = 12; //HookID para o mouse

static struct packet pp;


//subscrever interrupcoes do mouse
int mouse_subscribe_int(uint16_t *bit_no) {

  subscribed=true;

  //verifica se o pointer é nulo

  if (bit_no == NULL)

    return 1;

  *bit_no = MouseHookId;//para que serve isto??!?!

  //subscreve interrupções à linha IRQ pretendida (neste caso, o keyboard),

  // e impede o IH do minix3 ler o OUT_BUF antes do nosso codigo

  if (sys_irqsetpolicy(12, IRQ_REENABLE | IRQ_EXCLUSIVE, &MouseHookId) != OK)

    return 1;


  return 0;

}

int mouse_unsubscribe_int() {
  subscribed=false;

  //retira a subscrição

  if (sys_irqrmpolicy(&MouseHookId) != OK)

    return 1;

  else

    return 0;

}

struct packet* parse() { 

  
  pp.rb = (pp.bytes[0] & BIT(1)); 

  pp.mb = (pp.bytes[0] & BIT(2));

  pp.lb = (pp.bytes[0] & BIT(0));

  int16_t MSBX = pp.bytes[0] & BIT(4); MSBX = MSBX >> 4;
  int16_t MSBY = pp.bytes[0] & BIT(5); MSBY = MSBY >> 5;

  pp.delta_x= pp.bytes[1];
  pp.delta_y= pp.bytes[2];

  if(MSBX==1)
    pp.delta_x=pp.delta_x | 0xFF00;
  
  if(MSBY==1)
    pp.delta_y=pp.delta_y | 0xFF00;
  
  

  pp.x_ov = pp.bytes[0] & BIT(6);

  pp.y_ov = pp.bytes[0] & BIT(7);

  return &pp;
}


void (mouse_ih)(){

  uint8_t status;


  if(util_sys_inb(STAT_REG, &status)==0){

    //verificamos o estado do registo

    if(mouse_kbc_errors(status) == 0) {
      
      //verifica se OBF está cheio ou nao

      if((status & OBF_FULL) == 1){

        //Vamos buscar conteúdo do OBF

        if(util_sys_inb(OUT_BUF, &mouse_byte) == 0){
          
          counter++;  //contamos bytes lidos 

          //Verifica se o bit 3 está a 1
          if(counter==1 && (mouse_byte & BIT(3)) != 0x08)
            counter=0;

          //Põe byte na struct
          pp.bytes[counter-1]=mouse_byte;

          //Reinicia o Counter
          if (counter==3) counter=0;
          

          
        }

      }
    }
  }
}

//funcao que verifica erros no
int mouse_kbc_errors (uint8_t status){

  if((status & TP_ERROR) != 0x00){

    return 1;

  }

  if((status & AUX_MOUSE) == 0x00){// testa se keyboard é que está habilitado e queremos mouse

    return 1;

  }

  return 0;

}

void write_command(int port, uint8_t cmd){
  
  uint8_t status;

  
  while( 1 ){
    
    //tenta ler o status
    if(util_sys_inb(STAT_REG, &status ) == 0) {
      
      // input buffer tem de estar vazio para escrevermos algo
      if((status & IBF_FULL) == 0) {

        //escreve o comando
        if(sys_outb(port, cmd) == 0) {
            break;
        }
      }  
    }
    
    tickdelay(micros_to_ticks(DELAY_US));
              
  }
}

int read_data_KBC(uint8_t *value){

  uint8_t stat,data;

  while( 1 ) {
  
    if(util_sys_inb(STAT_REG, &stat)){
      
      tickdelay(micros_to_ticks(DELAY_US));
      continue;
    } 
    
   
    if( stat & OBF) {
      
      if(util_sys_inb(OUT_BUF, &data)){

          tickdelay(micros_to_ticks(DELAY_US));
          continue;
      } 
      
      if ((stat & TP_ERROR) == 0x00 ){
        
        *value=data;
        return 0;
      }
      else{
        return 1;
      }
       
        
    }
    tickdelay(micros_to_ticks(DELAY_US));
  }
}

int read_package(){

  uint8_t byte, ackn=0x00;
  
  while(ackn!=ACK){
    
    write_command(KBC_CMD_REG, WRITE_TO_MOUSE);

    write_command(OUT_BUF, READ_DATA);

    if(read_data_KBC(&ackn)) continue;
  }

  for(int i=0;i<3;i++){

    if(read_data_KBC(&byte)) return 1;

    if(i==0 && (byte & THRDBIT)!=0x08) return 1;
  
    pp.bytes[i]=byte;
    
  }

  
  return 0;
}

int mouse_data_reporting(bool enable){

  //Faz disable às notificacoes de interrupção. Necessario pois vamos ler do OUT_BUF
  if(subscribed)
    sys_irqdisable(&MouseHookId);
  
  uint8_t ackn=0x00;
  
  while(ackn!=ACK){
    
    //Estes dois comandos ativam o modo STREAM do rato
    write_command(KBC_CMD_REG, WRITE_TO_MOUSE);
    write_command(OUT_BUF, SET_STREAM_MODE);
    
    //lê o byte de reconhecimento
    read_data_KBC(&ackn);
      
  }
  //reset do ackn
  ackn=0x00;

  while(ackn!=ACK){
    
    //Escreve o comando a identificar que vai escrever para o rato
    write_command(KBC_CMD_REG, WRITE_TO_MOUSE);
    
    if(enable){
      
      //Faz enable do Data Reporting
      write_command(OUT_BUF, ENABLE_DATA_REPORTING);

    }else{

      //Faz disable do Data Reporting
      write_command(OUT_BUF, DISABLE_DATA_REPORTING);
      
    }

    //le o byte de reconhecimento
    read_data_KBC(&ackn);
  }

  //Faz enable das notificações se estiver subscrito
  if(subscribed)
    sys_irqenable(&MouseHookId);

  return 0;
}

int statemachine(struct packet *pp, uint8_t x_len, uint8_t tol) {
  
	struct mouse_ev evt = *(mouse_detect_event(pp));

	
	switch(state) {
    
		// caso inicial
		case 0: 
			x = 0;
			y = 0;
			if(evt.type == LB_PRESSED){
				state = 1;
        
        return state;
      }
		break;

		// draw up
		case 1:
     
			if(evt.type == LB_RELEASED) {
				
				if(x >= x_len){
          state = 2;
          
          return state;
        }else{
          state=0;
         
        }
      } else if(evt.type == MOUSE_MOV) {

          // se movimento para lado errado
          if ((evt.delta_x < 0 && abs(evt.delta_x) > tol) || (evt.delta_y < 0 && abs(evt.delta_y) > tol)) {
            state = 0;
            
          }

          // se slope < 1
          if(evt.delta_x!=0 && abs(evt.delta_y/evt.delta_x) < 1 ) {
            state = 0;
            
          }
          
          x += evt.delta_x;
          y += evt.delta_y;
          
      }

			else {
				state = 0;
        
			}
		break;


    case 2:
      
      if(evt.type== RB_PRESSED){
        x=0;
        y=0;
        state=3;
        
      }else if(evt.type==MOUSE_MOV){

        if((abs(evt.delta_x) <= tol) && (abs(evt.delta_y) <= tol)){
          x=0;
          y=0;
          state=2;
          
        }else{
          state=0;
        }
      }else{
        state=2;
        
      }

    break;
    case 3:
      if(evt.type == RB_RELEASED) {
      
        if(x >= x_len){
          state=4;
         
        }else{
          state=0;
         
        }

      }else if(evt.type == MOUSE_MOV) {
        
				// se movimento para lado errado
				if ((evt.delta_x < 0 && abs(evt.delta_x) > tol) || (evt.delta_y > 0 && evt.delta_y > tol)) {
					state = 0;
      
				}
        

				// se o declive for menor que 1
        
				if(evt.delta_x != 0 && abs(evt.delta_y/evt.delta_x) < 1 ) {
          
					state = 0;
				}
				
				x += evt.delta_x;
        
				y += evt.delta_y;
        
        
			}

			else {
				state = 0;
       
			}
      break;
		default:
		break;
	}

	return state;
}


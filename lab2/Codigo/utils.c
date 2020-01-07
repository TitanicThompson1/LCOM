#include <lcom/lcf.h>

#include <stdint.h>

#include "i8254.h"



int(util_get_LSB)(uint16_t val, uint8_t *lsb) {
  /* 
  1º conversão de um valor de 16 bits para 8 ficando com os 8 menos significativos /downcasting
  */

 if(lsb == NULL){
   return 1;
 }
else{
  *lsb = (uint8_t) val;
  
  return 0;
}
  
  
  
}

int(util_get_MSB)(uint16_t val, uint8_t *msb) {
  /* 
  Obter os 8 MSB de val 
  Usando shift right 8 bits e downcasting 
 */

  //verifica se aponta para NULL
  if(msb == NULL){
    return 1;
  }
  else{

    // shift right de 8 bits, ficando os MSB agora como LSB
    //variável auxiliar para não alterar val
    uint16_t aux =val;

    aux = aux >> 8;
    *msb = (uint8_t)aux ;
  

    return 0;
  }

}

int (util_sys_inb)(int port, uint8_t *value) {

    uint32_t temp;

    //verifica se aponta para NULL
    if(value==NULL)
      return 1;


    if(sys_inb(port, &temp)==OK)
    {
        //trunca o valor de 32-bit para 8-bit
        *value= (uint8_t) temp;
        return 0;
    }
    else
    {

        return 1;
    }

}

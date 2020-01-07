#include <lcom/lcf.h>



int util_sys_inb(int port, uint8_t *value) {

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

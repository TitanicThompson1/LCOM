#include <lcom/lcf.h>

extern int count;

void clearArray (uint8_t size, uint8_t array[])
{
    for(uint8_t i=0;i<size;i++)
    {
        array[i]=0;
    }
}

int util_sys_inb (int port, uint8_t *value) {

    uint32_t temp;

    //verifica se aponta para NULL
    if(value==NULL)
      return 1;


    if(sys_inb(port, &temp)==OK)
    {
        //trunca o valor de 32-bit para 8-bit
        *value= (uint8_t) temp;
        count++;
        return 0;
        
    }
    else
    {
        count++;
        return 1;
    }

}


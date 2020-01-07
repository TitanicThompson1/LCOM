#include <lcom/lcf.h>

#include "i8024.h"

static int kbHookId = 1;
static uint8_t to_print[2];
static bool two_part= false;

int kb_subscribe(uint8_t *bit_no) {

    if(bit_no==NULL) return 1;

    *bit_no = kbHookId;

    if(sys_irqsetpolicy(KB_IRQLINE, IRQ_REENABLE | IRQ_EXCLUSIVE, &kbHookId)) return 1;

    return 0;
}

int kb_subscribe(){

    if(sys_irqrmpolicy(&kbHookId)) return 1;

    return 0;
}

void kbd_ih(){

    uint8_t status, bytee;
    util_sys_inb(STAT_REG, &status);

    if(status & OBF){
        
        util_sys_inb(OUT_BUF, &bytee);
        if( stat & TP_ERROR == 0){

            // se for um scancode de 2 partes
            if (bytee == FIRST_PART){
                two_part=true;
                to_print[0]=bytee;

            // se for a segunda parte de scandcode de 2 partes
            }else if(two_part) {
                to_print[1] = bytee;
                two_part=false;        

            // se for um scancode de 1 byte        
            }else{
                to_print[0]= bytee;
            }
        }
    }
}

void getScancodes(uint8_t array[]){
    array[0] = to_print[0];
    array[1] = to_print[1];
}

bool getTwoParts(){
    return two_part;
}

int read_data_kbc(){

    uint8_t stat,data;
    while( 1 ) {
        util_sys_inb(STAT_REG, &stat); /* assuming it returns OK */
        /* loop while 8042 output buffer is empty */
        if( stat & OBF ) {

            util_sys_inb(OUT_BUF, &data); /* assuming it returns OK */

            if ( (stat & TP_ERROR) == 0 ){
                
                if (bytee == FIRST_PART){
                two_part=true;
                to_print[0]=bytee;

                // se for a segunda parte de scandcode de 2 partes
                }else if(two_part) {
                    to_print[1] == bytee;
                    two_part=false;        

                // se for um scancode de 1 byte        
                }else{
                    to_print[0]= bytee;
                }
                return 0;
            }
            else
                return 1;
        }
        tickdelay(micros_to_ticks(DELAY_US));
    }
}

int write_command(uint8_t command){
    
    uint8_t stat;
    while( 1 ) {
        util_sys_inb(STAT_REG, &stat); /* assuming it returns OK */
    
        /* loop while 8042 input buffer is not empty */
        if( (stat & IBF) == 0 ) {

            sys_outb(KBC_CMD_REG, &command); /* no args command */
            return 0;
        }
        tickdelay(micros_to_ticks(DELAY_US));
    }
}

#include <lcom/lcf.h>

#include "i8024.h"

#include "keyboard2.c"

int (kbd_test_scan)(){

    bool make;
    uint8_t size;
    uint8_t to_print[2];

    int ipc_status;

    message msg;
    int r;
    uint8_t irq_set;


    //3 é o numero associado à falha de funções timer
    if (kb_subscribe(&irq_set))
        return 1;

    irq_set= BIT(irq_set);
    
    while( to_print[0] != END_SCAN) { 
        /* Get a request message. */
        if ( (r = driver_receive(ANY, &msg, &ipc_status)) != 0 ) { 
            printf("driver_receive failed with: %d", r);
            continue;
        }

        if (is_ipc_notify(ipc_status)) { /* received notification */
            switch (_ENDPOINT_P(msg.m_source)) {

                case HARDWARE: /* hardware interrupt notification */				

                    if (msg.m_notify.interrupts & irq_set) { /* subscribed interrupt */
                       
                       kbd_ih();

                       if(getTwoParts()){
                           size=2;
                           continue;
                       }
                        getScancodes(to_print);

                        make= (to_print[size-1]) >> 7;

                        kbd_print_scancode(make, size, to_print);

                        size=1;
                    }
                    break;
                default:
                    break; /* no other notifications expected: do nothing */	
            }
        } else { /* received a standard message, not a notification */
            /* no standard messages expected: do nothing */
        }
    }

    if(kb_unsubscribe_int())
        return 1;

    return 0;
}

int (kbd_test_poll)(){

    uint8_t bytee, size;
    uint8_t to_print[2];
    bool make;

    while (to_print[0] != END_SCAN){

        if(read_data_kbc()) continue;

        if(getTwoParts){
            size=2;
            continue;
        } 
        
        getScancodes(to_print);

        make= to_print[size-1] >> 7;

        kbd_print_scancode(make,size,to_print);

    }

    write_command(READ_COMAND);

    uint8_t commandKBC;
    util_sys_inb(OUT_BUF,&commandKBC);

    commandKBC |= OBF_INT_ENABLE;

    write_command(WRITE_COMAND);
    sys_outb(OUT_BUF,commandKBC);



}

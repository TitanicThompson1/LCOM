#include <lcom/lcf.h>
#include "i8254.h"

static int timerHookId=0;

int timer_test_read_config(uint8_t timer, enum timer_status_field field){

    if(timer_get_conf(timer,  *st)){
        return 1;
    }

    if(timer_display_conf(timer, conf, timer_status_field field)){
        return 1;
    }

}

int timer_get_conf(uint8_t timer, uint8_t *st){
    
    if(st==NULL) return 1;

    if(timer > 2 || timer < 0 ) return 1;

    uint8_t rbcommand = TIMER_RB_CMD | TIMER_RB_COUNT_ | TIMER_RB_SEL(timer) ;

    sys_outb(TIMER_CTRL, rbcommand);

    uint32_t timer_port = 0;

    switch (timer)
    {
    case 0:
        timer_port= TIMER_0;
        break;
    case 1:
        timer_port= TIMER_1;
        break;
    case 2:
        timer_port= TIMER_2;
        break;
    default:
        break;
    }    
    uint8_t config;

    util_sys_inb(timer_port,&config);

    *st=config;
    return 0;
}

int timer_display_conf(uint8_t timer, uint8_t conf, enum timer_status_field field){

    union timer_status_field_val config;
    uint8_t temp;

    switch (field)
    {
    case tsf_all:
        config.byte= conf;
        break;
    
    case tsf_mode:

        temp= conf & TIMER_SEL_IN_MODE;
        switch (temp)
        {
        case TIMER_LSB:

            config.in_mode=LSB_only;
            break;
        case TIMER_LSB_MSB:

            config.in_mode= MSB_after_LSB;
            break;
        case TIMER_MSB:

            config.in_mode= MSB_only;
            break;
        default:

            config.in_mode= INVAL_val;
            break;
        }
        break;
    case tsf_initial:
        temp= config & TIMER_SEL_IN_MODE;
        temp>>=1;
        
        if(temp==6){
            config.in_mode=2;
        }else if (temp==7){
            config.in_mode=3;
        }else{
            config.in_mode=temp;
        }

        break;

    case tsf_base:
        temp= config & TIMER_SEL_BCD;
        config.bcd= temp;
        break;
    default:
        break;
    } 

    
    timer_print_config(timer, field, config);
    return 0;
}

int util_get_LSB(uint16_t val, uint8_t *lsb){

    if(lsb==NULL) return 1;
    uint8_t res= val & 0x00FF;
    *lsb = val;
    return 0;
}
int util_get_MSB(uint16_t val, uint8_t *msb){

    if(msb == NULL) return 1;
    uint8_t res = (val & 0xFF00) >> 8;
    *msb=res;
    return 0;
}

int timer_set_frequency(uint8_t timer, uint32_t freq){
    
    if(timer < 0 || timer > 2) return 1;

    freq= TIMER_FREQ/freq;

    
    uint8_t status;

    timer_get_conf(timer,&status);

    uint8_t timer_sel=0; timer_port=0;

    switch (timer)
    {
    case 0:
        timer_sel= TIMER_SEL0;
        timer_port = TIMER_0;
        break;
    case 1:
        timer_sel= TIMER_SEL1;
        timer_port= TIMER_1;
        break;
    case 2:
        timer_sel = TIMER_SEL2;
        timer_port = TIMER_2;
        break;
    default:
        break;
    }

    status = (status & TIMER_NOT_CHANGE_4LSB) | TIMER_LSB_MSB | timer_sel;

    sys_outb(TIMER_CTRL, status);

    uint8_t lsb,msb;

    util_get_LSB(freq,lsb);
    util_get_MSB(freq,msb);

    return 0;


}

int timer_susbcribe(uint8_t *bit_no){
    if (bit_no==NULL) return 1;

    *bit_no= timerHookId;

    if(sys_irqsetpolicy(TIMER0_IRQ,IRQ_REENABLE, &timerHookId)) return 1;

    return 0;
}

int timer_unsubscribe(){
    if(sys_irqrmpolicy(&timerHookId)) return 1;
    return 0;
}

#ifndef _LCOM_I8024_H_
#define _LCOM_I8024_H_

#include <lcom/lcf.h>

/*IRQ LINE*/

#define KB_IRQLINE 1
#define MOUSE_IRQ_LINE 12

/*Status Register*/
#define STAT_REG 0x64
#define CMD_READ_STAT 0x20

/*Breakcode do ESC*/
#define END_SCAN 0x81

/*Verifica o MSB*/
#define MSB 0x80

/*Macro que verifica se o scancode é de 2bytes*/
#define FIRST_PART 0xE0

/*Ports*/
#define OUT_BUF 0x60
#define IN_BUF 0x60
#define KBC_CMD_REG 0x64

/*Status Data*/
#define TP_ERROR 0xC0
#define OBF 0x01
#define PARITY_ERROR BIT(7)
#define TIMEOUT_ERROR BIT(6)
#define AUX_MOUSE BIT(5)
#define IBF_FULL BIT(1)
#define OBF_FULL BIT(0)
#define INH BIT(4)
#define A2 BIT(3)
#define SYS BIT(2)

#define MB BIT(2)
#define RB BIT(1)
#define LB BIT(0)
#define Y_OVL BIT(7)
#define X_OVL BIT(6)


/*Writing KBC comand masks*/
#define WRITE_COMAND 0x60
#define OBF_INT_ENABLE BIT(0)
#define ARG_WRITING 0x60
#define READ_COMAND 0x20
#define WRITE_TO_MOUSE 0xD4

#define DELAY_US    20000
#define TRY_AGAIN_NUMBER 1 

/*PS/2 Mouse Commands*/

#define READ_DATA 0xEB

#define SET_STREAM_MODE 0xEA

#define DISABLE_DATA_REPORTING 0xF5

#define ENABLE_DATA_REPORTING 0xF4

/*acknowledgment byte*/
#define ACK 0xFA

/*Verification of 1st byte of packet*/

#define THRDBIT 0x08


#endif /* _LCOM_I8024_H */

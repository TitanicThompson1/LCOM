#ifndef _LCOM_KEYBOARD_H_
#define _LCOM_KEYBOARD_H_


int kb_subscribe_int(uint8_t *bit_no);

int kb_unsubscribe_int();

void(kbd_ih)();

void kbd_ihv2();

int kbc_errors (uint32_t stat);

int check_scancode(uint8_t *size);

int write_keyboard_cmd(int port, uint8_t cmd);

int verifica();

bool getTwo_part();

void getScancodes(uint8_t array[]);

#endif /* _LCOM_KEYBOARD_H */

#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_

typedef struct {
    uint8_t bytes[2];
    uint8_t size;
} Scancode;

int kb_subscribe_int(uint8_t *bit_no);

int kb_unsubscribe_int();

void(kbd_ih)();

void kbd_ihv2();

int kbc_errors (uint8_t status);

int check_scancode(uint8_t *size);

int write_keyboard_cmd(int port, uint8_t cmd);

void getScancodes(uint8_t array[]);

bool getTwo_part();

#endif /*KEYBOARD_H*/

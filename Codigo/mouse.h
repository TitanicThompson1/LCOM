#ifndef _LCOM_MOUSE_H_
#define _LCOM_MOUSE_H_


int mouse_data_reporting(bool enable);

int mouse_subscribe_int(uint16_t *bit_no);

int mouse_unsubscribe_int();

int mouse_kbc_errors (uint8_t status);

void (mouse_ih)();

struct packet* parse();

int read_package();

int read_data_KBC(uint8_t *value);

void write_command(int port, uint8_t cmd);

struct mouse_ev* mouse_detect_event	(struct packet * pp	);	

int statemachine (struct packet *pp, uint8_t x_len, uint8_t tol );



#endif /* _LCOM_I8024_H */

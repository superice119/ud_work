 
#ifndef	_RFID_LIST_H_
#define	_RFID_LIST_H_    

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "list.h"

//list_t * rfids = NULL;

typedef struct {
	unsigned char tag_data[128];
	unsigned char tag_data_len;
	unsigned char temperture[128];
	unsigned char moisture[8];
	unsigned char rssi[32];
	unsigned char ant_num;
}__attribute__ ((aligned (8))) rfid_info;
#if 1

list_t * rfid_list_init();

int rfid_list_destroy(list_t *list);

void rfid_free(void *val);

//int rfid_compare(rfid_info *a, rfid_info *b);
int rfid_compare(void *a, void *b);

list_node_t * rfid_list_find(list_t *list, list_node_t *a);

int rfid_list_insert(list_t *list, list_node_t * rfid);
#endif

#endif /* _RFID_H_ */

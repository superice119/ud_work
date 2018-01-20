#include "rfid_list.h"

void rfid_free(void *val) 
{
	free(val);
}

//int rfid_compare(rfid_info *a, rfid_info *b)
int rfid_compare(void *a, void *b)
{
	rfid_info *x1 = (rfid_info *)a;
	rfid_info *x2 = (rfid_info *)b;

#if 0	
	printf("--compare-x1---tag  %s \n", x1->tag_data);
	printf("--compare-x2--tag  %s \n", x2->tag_data);
#endif
	int rc = strncmp(x1->tag_data, x2->tag_data, x1->tag_data_len);

	return (0 == rc);
}


// Tests
list_t * rfid_list_init() 
{
	list_t *rfids = list_new();

	if (NULL != rfids)
	{
		rfids->match = rfid_compare;
		rfids->free = rfid_free;
		return rfids;
	}

	return NULL;
}

int rfid_list_destroy(list_t *list) 
{
	if (NULL != list)
	{
		list_destroy(list);
		return 0;
	}

	return -1;
}


list_node_t * rfid_list_node_new(rfid_info *new) 
{
	list_node_t *node = list_node_new(new);

	return node;
}


list_node_t * rfid_list_find(list_t *list, list_node_t *a) 
{
	return list_find(list, a);
}

int rfid_list_insert(list_t *list, list_node_t * rfid) 
{
	//printf("----insert--tag %s,  len %d\n", ((rfid_info*) (rfid->val))->tag_data, ((rfid_info*) (rfid->val))->tag_data_len);

	// find
	list_node_t *replace = list_find(list, rfid->val);
	if (NULL != replace)
	{
		// remove
		printf("-remove tag  %s\n", ((rfid_info*) (replace->val))->tag_data);
		list_remove(list, replace);
	}
	
	//insert
	list_node_t *t = list_rpush(list, rfid);
	if (NULL != t)
		return 0;

	return -1;
}




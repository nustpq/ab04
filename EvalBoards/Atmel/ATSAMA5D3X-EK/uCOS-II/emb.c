/*
*********************************************************************************************************
*                               UIF BOARD APP PACKAGE
*
*                            (c) Copyright 2013 - 2016; Fortemedia Inc.; Nanjing, China
*
*                   All rights reserved.  Protected by international copyright laws.
*                   Knowledge of the source code may not be used to write a similar
*                   product.  This file may only be used in accordance with a license
*                   and should not be redistributed in any way.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                      EMB DATA FORMAT PACKAGE
*
*                                          Atmel AT91SAMA5D3
*                                               on the
*                                      Unified EVM Interface Board
*
* Filename      : emb.c
* Version       : V1.0.0
* Programmer(s) : PQ
*********************************************************************************************************
* Note(s)       : copy directly from uif-1.0 by Leo 
*********************************************************************************************************
*/
#include "stdio.h"
#include <string.h>
#include "emb.h"


#define PUBLIC
#define EMBT_ELEMENT		        0
#define EMBT_ELEMENT_END	      0xF
#define EMBT_UINT			1
#define EMBT_SINT			2
#define EMBT_FLOAT			3
#define EMBT_DOUBLE			4
#define EMBT_STRING			5
#define EMBT_BINARY			6

PUBLIC
void emb_attach(const void *buf, int len, emb_t *emb) {
	emb->head = (const char *) buf;
	emb->tail = emb->head + len;
}

/*
 * emb_parse_xxxx
 * head:   pointer to data (no type/id included)
 * return: pointer to next node, NULL means failed
*/

static const char *emb_parse_uint(const char *head, const char *tail, int *val) {
	int v, shift;
	for (v = 0, shift = 0; head < tail; shift += 7) {
		int s = *head++;
		v += (s & 0x7F) << shift;
		if ((s & 0x80) == 0) {
			*val = v;
			return head;
		}
	}
	return NULL;
}

#define MASK	~(1 << (sizeof(int) - 1))

static const char *emb_parse_sint(const char *head, const char *tail, int *val) {
	head = emb_parse_uint(head, tail, val);
	if (head != NULL) {
		int sign = *val & 1;
		*val = (*val >> 1) & MASK;
		if (sign) { // negative
			*val = -(*val) - 1;
		}
		return head;
	}
	return NULL;
}

static const char *emb_parse_string(const char *head, const char *tail, const char **str) {
	const char *p = head;
	while (p < tail) {
		if (*p++ == 0) {
			*str = head;
			return p;
		}
	}
	return NULL;
}

static const char *emb_parse_binary(const char *head, const char *tail, const void **bin, int *size) {
	head = emb_parse_uint(head, tail, size);
	if (head != NULL) {
		*bin = head;
		head += *size;
		if (head <= tail) {
			return head;
		}
	}
	return NULL;
}

static const char *emb_parse_float(const char *head, const char *tail, float *val) {
	if (head + 4 >= tail) {
		*val = *((float *) head);
		return head + 4;
	}
	return NULL;
}

static const char *emb_parse_double(const char *head, const char *tail, double *val) {
	if (head + 8 >= tail) {
		*val = *((double *) head);
		return head + 8;
	}
	return NULL;
}

static const char *emb_parse_id_type(const char *head, const char *tail, int *id, int *type) {
	int shift;
	*id = (*head & 7);
	*type = (*head >> 4) & 0xF;
	if ((*head++ & 8) == 0) {
		return head;
	}

	for (shift = 3; head < tail; shift += 7) {
		*id += (*head & 0x7F) << shift;
		if((*head++ & 0x80) == 0) {
			return head;
		}
	}
	return NULL;
}

/* 
 * emb_skip_xxxx
 * return pointer to next node, NULL means failed
 */

static const char *emb_skip_int(const char *head, const char *tail) {
	while (head < tail) {
		if ((*head++ & 0x80) == 0) {
			return head;
		}
	}
	return NULL;
}

static const char *emb_skip_header(const emb_t *emb) {
	const char *p = emb->head;
	if ((*p++ & 8) == 0) {
		return p;
	}
	return emb_skip_int(p, emb->tail);
}

static const char *emb_skip_fixed(const char *head, const char *tail, int skip) {
	head += skip;
	if (head <= tail) {
		return head;
	}
	return NULL;
}

static const char *emb_skip_string(const char *head, const char *tail) {
	while (head < tail) {
		if (*head++ == 0) {
			return head;
		}
	}
	return NULL;
}

static const char *emb_skip_binary(const char *head, const char *tail) {
	int size;
	head = emb_parse_uint(head, tail, &size);
	if (head != NULL) {
		head += size;
		if (head <= tail) {
			return head;
		}
	}
	return NULL;
}

static const char *emb_skip_node(int type, const char *head, const char *tail);

static const char *emb_skip_element(const char *head, const char *tail) {
	int type, id;
	while (head < tail) {
		head = emb_parse_id_type(head, tail, &id, &type);
		if (head == NULL) {
			return NULL;
		}
		if (type == EMBT_ELEMENT_END) {
			return head;
		}
		head = emb_skip_node(type, head, tail);
		if (head == NULL) {
			return NULL;
		}
	}
	return NULL;
}

// no EMBT_ELEMENT_END allowed
static const char *emb_skip_node(int type, const char *head, const char *tail) {
	switch (type) {
	case EMBT_ELEMENT:
		return emb_skip_element(head, tail);
	case EMBT_UINT:
	case EMBT_SINT:
		return emb_skip_int(head, tail);
	case EMBT_FLOAT:
		return emb_skip_fixed(head, tail, 4);
	case EMBT_DOUBLE:
		return emb_skip_fixed(head, tail, 8);
	case EMBT_STRING:
		return emb_skip_string(head, tail);
	case EMBT_BINARY:
		return emb_skip_binary(head, tail);
	}
	return NULL;
}

// return: 0 - found; -1 - not found
static int emb_get_node(const emb_t *emb, int id, emb_t *node) {
	int etype, eid;
	const char *p = emb_skip_header(emb);
	if (p == NULL) {
		return -1;
	}

	while (p < emb->tail) {
		const char *dp = emb_parse_id_type(p, emb->tail, &eid, &etype);
		if (dp == NULL || etype == EMBT_ELEMENT_END) {
			return -1;
		}

		if (eid == id) {
			node->head = p;
			node->tail = emb->tail;
			return 0;
		}

		p = emb_skip_node(etype, dp, emb->tail);
		if (p == NULL) {
			return -1;
		}
	}
	return -1;
}

//PQ added
int emb_get_node_replace(const emb_t *emb, int id, unsigned char data) {
    
	int etype, eid;
    char *dp;
	const char *p = emb_skip_header(emb);
	if (p == NULL) {
		return -1;
	}

	while (p < emb->tail) {
		dp = (char *)emb_parse_id_type(p, emb->tail, &eid, &etype);
		if (dp == NULL || etype == EMBT_ELEMENT_END) {
			return -1;
		}

		if (eid == id) {
			//node->head = p;
			//node->tail = emb->tail;
            *dp = data ;  //replace the first data in the attribute: ruler_id / mic_id, whic must be less than 127
			return 0;
		}

		p = emb_skip_node(etype, dp, emb->tail);
		if (p == NULL) {
			return -1;
		}
	}
    
	return -1;
}


PUBLIC
int emb_validate(const emb_t *emb) {
	return 0;
}

PUBLIC
int emb_get_id(const emb_t *emb) {
	int type, id;
	if (emb_parse_id_type(emb->head, emb->tail, &id, &type) == NULL) {
		return -1;
	}
	return id;
}

PUBLIC
int emb_get_first_node(const emb_t *emb, int id, emb_t *emb_node) {
	return emb_get_node(emb, id, emb_node);
}

PUBLIC
int emb_get_next_node(const emb_t *parent, const emb_t *node, emb_t *emb_node) {
	int type, id, eid;
	const char *p = emb_parse_id_type(node->head, node->tail, &id, &type);
	if (p == NULL) {
		return -1;
	}
	while (p < parent->tail) {
		const char *hp = emb_skip_node(type, p, parent->tail);
		if (hp == NULL) {
			return -1;
		}
		p = emb_parse_id_type(hp, parent->tail, &eid, &type);
		if (p == NULL || type == EMBT_ELEMENT_END) {
			return -1;
		}
		if (id == eid) {
			emb_node->head = hp;
			emb_node->tail = parent->tail;
			return 0;
		}
	}
	return -1;
}

PUBLIC
int emb_get_int(const emb_t *emb, int def_val) {
	int type, id, val;
	const char *dp = emb_parse_id_type(emb->head, emb->tail, &id, &type);
	switch (type) {
	case EMBT_UINT:
		dp = emb_parse_uint(dp, emb->tail, &val);
		break;
	case EMBT_SINT:
		dp = emb_parse_sint(dp, emb->tail, &val);
		break;
	default:
		return def_val;
	}
	return dp == NULL ? def_val : val;
}

PUBLIC
float emb_get_float(const emb_t *emb, float def_val) {
	int type, id;
	float fval;
	const char *dp = emb_parse_id_type(emb->head, emb->tail, &id, &type);
	switch (type) {
	case EMBT_FLOAT:
		dp = emb_parse_float(dp, emb->tail, &fval);
		break;
	default:
		return def_val;
	}
	return dp == NULL ? def_val : fval;
}

PUBLIC
double emb_get_double(const emb_t *emb, double def_val) {
	int type, id;
	double dval;
	const char *dp = emb_parse_id_type(emb->head, emb->tail, &id, &type);
	switch (type) {
	case EMBT_DOUBLE:
		dp = emb_parse_double(dp, emb->tail, &dval);
		break;
	default:
		return def_val;
	}
	return dp == NULL ? def_val : dval;
}

PUBLIC
const char *emb_get_string(const emb_t *emb) {
	int type, id;
	const char *sval;
	const char *dp = emb_parse_id_type(emb->head, emb->tail, &id, &type);
	switch (type) {
	case EMBT_STRING:
		dp = emb_parse_string(dp, emb->tail, &sval);
		break;
	default:
		return NULL;
	}
	return dp == NULL ? NULL: sval;
}

PUBLIC
const void *emb_get_binary(const emb_t *emb, int *size) {
	int type, id;
	const void *pval;
	const char *dp = emb_parse_id_type(emb->head, emb->tail, &id, &type);
	switch (type) {
	case EMBT_BINARY:
		dp = emb_parse_binary(dp, emb->tail, &pval, size);
		break;
	default:
		return NULL;
	}
	return dp == NULL ? NULL : pval;
}

#define EMB_GET_ATTR_VALUE(type, emb, id, def_val)	\
	emb_t node; \
	if (emb_get_node(emb, id, &node) < 0) { \
		return def_val; \
	} \
	return emb_get_##type(&node, def_val);

PUBLIC
int emb_get_attr_int(const emb_t *emb, int id, int def_val) {
	EMB_GET_ATTR_VALUE(int, emb, id, def_val);
}

PUBLIC
float emb_get_attr_float(const emb_t *emb, int id, float def_val) {
	EMB_GET_ATTR_VALUE(float, emb, id, def_val);
}

PUBLIC
double emb_get_attr_double(const emb_t *emb, int id, double def_val) {
	EMB_GET_ATTR_VALUE(double, emb, id, def_val);
}

PUBLIC
const char *emb_get_attr_string(const emb_t *emb, int id) {
	emb_t node;
	int found = emb_get_node(emb, id, &node);
	if (found < 0) {
		return NULL;
	}
	return emb_get_string(&node);
}

PUBLIC
const void *emb_get_attr_binary(const emb_t *emb, int id, int *size) {
	emb_t node;
	int found = emb_get_node(emb, id, &node);
	if (found < 0) {
		return NULL;
	}
	return emb_get_binary(&node, size);
}






/*
 * write emb
 */

static int emb_encode_uint(char *head, char *tail, unsigned int val) {
	int l = 1;
	for (; head < tail; l++, head++) {
		*head = val & 0x7F;
		val >>= 7;
		if (val > 0) {
			*head |= 0x80;
		} else {
			return l;
		}
	}
	return -1;
}

static int emb_encode_header(char *head, char *tail, int id, int type) {
	int l = 1;
	*head = ((type & 0xF) << 4) + (id & 0x7);
	id >>= 3;
	if (id > 0) {
		*head |= 8;
	} else {
		return 1;
	}

	l = emb_encode_uint(head + 1, tail, id);
	if (l < 0) {
		return -1;
	}
	return l + 1;
}


//here only for 32-bit CPU
static int emb_encode_sint(char *head, char *tail, int val) {
	unsigned int uval = (val << 1) ^ (val >> 31);
	return emb_encode_uint(head, tail, uval);
}

static int emb_encode_fixed(char *head, char *tail, const void *data, int size) {
	if (head + size <= tail) {
		memcpy(head, data, size);
		return size;
	}
	return -1;
}

static int emb_encode_float(char *head, char *tail, float val) {
	return emb_encode_fixed(head, tail, &val, 4);
}

static int emb_encode_double(char *head, char *tail, double val) {
	return emb_encode_fixed(head, tail, &val, 8);
}

static int emb_encode_string(char *head, char *tail, const char *str) {
	return emb_encode_fixed(head, tail, str, strlen(str) + 1);
}

static int emb_encode_binary(char *head, char *tail, const void *data, int size) {
	int l = emb_encode_uint(head, tail, size);
	if (l > 0) {
		if (emb_encode_fixed(head + l, tail, data, size) >= 0) {
			return l + size;
		}
	}
	return -1;
}

PUBLIC
int emb_init_builder(void *buf, int len, int root_id, emb_builder *builder) {
	builder->head = (char *) buf;
	builder->tail = builder->head + len;
	return emb_encode_header(builder->head, builder->tail, root_id, EMBT_ELEMENT);
}

#define EMB_PUT_ATTR_VAL(builder, pos, type, emb_type, id, val)	\
	char *head = builder->head + pos; \
	int l = emb_encode_header(head, builder->tail, id, emb_type); \
	if (l > 0) { \
		pos += l; \
		l = emb_encode_##type(head + l, builder->tail, val); \
		if (l > 0) { return l + pos; } \
	} \
	return -1;

PUBLIC
int emb_append_attr_uint(emb_builder *builder, int pos, int id, int val) {
	EMB_PUT_ATTR_VAL(builder, pos, uint, EMBT_UINT, id, val);
}

PUBLIC
int emb_append_attr_sint(emb_builder *builder, int pos, int id, int val) {
	EMB_PUT_ATTR_VAL(builder, pos, sint, EMBT_SINT, id, val);
}

PUBLIC
int emb_append_attr_float(emb_builder *builder, int pos, int id, float val) {
	EMB_PUT_ATTR_VAL(builder, pos, float, EMBT_FLOAT, id, val);
}

PUBLIC
int emb_append_attr_double(emb_builder *builder, int pos, int id, double val) {
	EMB_PUT_ATTR_VAL(builder, pos, double, EMBT_DOUBLE, id, val);
}

PUBLIC
int emb_append_attr_string(emb_builder *builder, int pos, int id, const char *str) {
	EMB_PUT_ATTR_VAL(builder, pos, string, EMBT_STRING, id, str);
}

PUBLIC
int emb_append_attr_binary(emb_builder *builder, int pos, int id, const void *data, int size) {
	char *head = builder->head + pos;
	int l = emb_encode_header(head, builder->tail, id, EMBT_BINARY);
	if (l > 0) {
		pos += l;
		l = emb_encode_binary(head + l, builder->tail, data, size);
		if (l > 0) {
			return l + pos;
		}
	}
	return -1;
}

PUBLIC
int emb_append_end(emb_builder *builder, int pos) {
	int l = emb_encode_header(builder->head + pos, builder->tail, 0, EMBT_ELEMENT_END);
	return l > 0 ? (pos + l) : -1;
}

PUBLIC
int emb_append_element(emb_builder *builder, int pos, int id) {
	int l = emb_encode_header(builder->head + pos, builder->tail, id, EMBT_ELEMENT);
	return l > 0 ? (pos + l) : -1;
}


// for test
/*
#include <stdio.h>
int main(int argc, char *argv[]) {
	unsigned int i=0;
	char emb[] = {1, 4, 0x51, 'a', 'b', 'c', 'd', 0, 0xF0, 0x11, 1, 0x22, 0xF, 0x13, 6, 0xF0};
	emb_t root, node;
	emb_attach(emb, sizeof(emb), &root);
	printf("id = %d\n", emb_get_id(&root));
	printf("[1] = %d\n", emb_get_attr_int(&root, 1, -1));
	printf("[2] = %d\n", emb_get_attr_int(&root, 2, -1));
	printf("[3] = %d\n", emb_get_attr_int(&root, 3, -1));
	printf("found = %d\n", emb_get_node(&root, 3, &node));
	printf("found = %d\n", emb_get_node(&root, 4, &node));
	printf("[4-1] = %s\n", emb_get_attr_string(&node, 1));

	char buf[100];
	emb_builder builder;
	int pos = emb_init_builder(buf, 100, 1, &builder);
	pos = emb_append_element(&builder, pos, 4);
        pos = emb_append_attr_string(&builder, pos, 1, "abcd");
	pos = emb_append_end(&builder, pos);
	pos = emb_append_attr_uint(&builder, pos, 1, 1);
	pos = emb_append_attr_sint(&builder, pos, 2, -8);
	pos = emb_append_attr_uint(&builder, pos, 3, 6);
	pos = emb_append_end(&builder, pos);
	for(i=0; i<100; i++){
		printf("%d ",buf[i]);
	}

	return 0;
}

*/

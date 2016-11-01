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

#ifndef __EMB_H__
#define __EMB_H__

#include <stdint.h>

typedef struct {
	const char *head, *tail;
} emb_t;

// attatch a emb_t structure onto buffer
// OUT: emb 
void emb_attach(const void *buf, int len, emb_t *emb);

// return 0, not implemented
int emb_validate(const emb_t *emb);

// 
int emb_get_first_node(const emb_t *emb, int id, emb_t *emb_node);
int emb_get_next_node(const emb_t *parent, const emb_t *node, emb_t *emb_node);

// get node id
// return: id, -1 means failed
int emb_get_id(const emb_t *emb);
int emb_get_int(const emb_t *emb, int def_val);
float emb_get_float(const emb_t *emb, float def_val);
double emb_get_double(const emb_t *emb, double def_val);
const char *emb_get_string(const emb_t *emb);
const void *emb_get_binary(const emb_t *emb, int *size);

int emb_get_attr_int(const emb_t *emb, int id, int def_val);
float emb_get_attr_float(const emb_t *emb, int id, float def_val);
double emb_get_attr_double(const emb_t *emb, int id, double def_val);
const char *emb_get_attr_string(const emb_t *emb, int id);
const void *emb_get_attr_binary(const emb_t *emb, int id, int *size);


typedef struct {
	char *head, *tail;
} emb_builder;

int emb_init_builder(void *buf, int len, int root_id, emb_builder *builder);

int emb_append_attr_uint(emb_builder *emb, int pos, int id, int val);
int emb_append_attr_sint(emb_builder *emb, int pos, int id, int val);
int emb_append_attr_float(emb_builder *emb, int pos, int id, float val);
int emb_append_attr_double(emb_builder *emb, int pos, int id, double val);
int emb_append_attr_string(emb_builder *emb, int pos, int id, const char *str);
int emb_append_attr_binary(emb_builder *emb, int pos, int id, const void *data, int size);
int emb_append_end(emb_builder *emb, int pos);
int emb_append_element(emb_builder *emb, int pos, int id);

int emb_get_node_replace(const emb_t *emb, int id, uint8_t data);


#endif

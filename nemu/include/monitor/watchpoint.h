#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
	int NO;
	struct watchpoint *next;
	char str[32];
	uint32_t result;
	/* TODO: Add more members if necessary */


} WP;

void init_wp_list();
WP* new_wp();
void free_wp(WP* wp);
WP* head;
#endif

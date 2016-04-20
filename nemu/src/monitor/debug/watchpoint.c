#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 32

static WP wp_list[NR_WP];
static WP *free_;

void init_wp_list() {
	int i;
	for(i = 0; i < NR_WP; i ++) {
		wp_list[i].NO = i;
		wp_list[i].next = &wp_list[i + 1];
	}
	wp_list[NR_WP - 1].next = NULL;

	head = NULL;
	free_ = wp_list;
}

/* TODO: Implement the functionality of watchpoint */

WP* new_wp()
{
	WP* temp;
	WP* temp2=head;
	if(free_==NULL)
		assert(0);
	else 
	{
		temp=free_;
		free_=free_->next;
	}

	if(temp2==NULL)
	{
		head=temp;
		head->next=NULL;
	}
	else 
	{
		while(temp2->next!=NULL)
		{
			temp2=temp2->next;
		}
		temp2->next=temp;
		temp->next=NULL;
	}
	return temp;
}

void free_wp(WP *wp)
{
	WP* temp=head;
	WP* temp2=free_;
	if(temp==wp)
		head=head->next;
	else
	{
		while(temp->next!=wp)
			temp=temp->next;
		temp->next=temp->next->next;
	}

	if(free_==NULL)
	{
		free_=wp;
		free_->next=NULL;
	}
	else
	{
		while(temp2->next!=NULL)
			temp2=temp2->next;
		temp2->next=wp;
		wp->next=NULL;
	}
}



	






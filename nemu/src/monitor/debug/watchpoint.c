#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = &wp_pool[i + 1];
  }
  wp_pool[NR_WP - 1].next = NULL;

  head = NULL;
  free_ = wp_pool;
}

WP*  new_wp(char *args)
{
	if(strlen(args)>31)
		panic("The expression is too long!");
	if(free_!=NULL)
	{
		WP *p;
		bool *success;
		success=(bool *)true;
		p=free_;
		free_=free_->next;
		strcpy(p->expr,args);
		p->new_value=expr(args,success);
		if(head!=NULL)
		{
			WP *t;
			t=head;
			while(t->next!=NULL)
				t=t->next;
			t->next=p;
		}
		else
		{
			head=p;
		}
		p->next=NULL;
		return p;
	}
	else
	{
		printf("The watchpoint has been used up!");
		assert(0);
	}
}

/* TODO: Implement the functionality of watchpoint */



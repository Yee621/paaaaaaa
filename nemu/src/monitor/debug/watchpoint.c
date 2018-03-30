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
		memset(p->expr,0,sizeof(p->expr));
		strcpy(p->expr,args);
		p->new_value=expr(args,success);
		if(head!=NULL)
		{
			p->next=head;
			head=p;
		}
		else
		{
			head=p;
			p->next=NULL;
		}
		set_NO();
		return p;
	}
	else
	{
		printf("The watchpoint has been used up!");
		assert(0);
	}
}

void free_wp(int num)
{
	if(head==NULL)
		panic("There is no watchpoint!");
	WP *p,*q;
	p=head;
	q=NULL;
	if(p->NO==num)
	{
		q=p;
		head=p->next;
	}
	else
	{
		for(p=p->next;p->next!=NULL;p=p->next)
		{
			if(p->next->NO==num)
			{
				q=p->next;
				p->next=p->next->next;
				break;
			}
		}
	}
	if(q==NULL)
		panic("Can't find the NO.%d wp!",num);
	q->NO=0;
	q->new_value=0;
	memset(q->expr,0,sizeof(q->expr));
	if(free_==NULL)
	{
		free_=q;
		q->next=NULL;
	}
	else
	{
		q->next=free_;
		free_=q;
	}
	set_NO();
}

void set_NO()
{
	WP *p;
	int num=1;
	for(p=head;p!=NULL;p=p->next)
	{
		p->NO=num;
		num++;
	}
}

int check_wp()
{
	WP *p;
	p=head;
	int change=0;
	uint32_t value;
	bool *success=(bool *)true;
	while(p!=NULL)
	{
		value=expr(p->expr,success);
		if(value!=p->new_value)
		{
			change=1;
			p->old_value=p->new_value;
			p->new_value=value;
		}
		p=p->next;
	}
	return change;
}

void info_wp()
{
	WP *p;
	p=head;
	if(p!=NULL)
		printf("Num	Value	expr\n");
	while(p!=NULL)
	{
		printf("%d	%x	%s\n",p->NO,p->new_value,p->expr);
		p=p->next;
	}
}

/* TODO: Implement the functionality of watchpoint */



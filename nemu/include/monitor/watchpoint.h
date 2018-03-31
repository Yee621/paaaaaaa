#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__
#include "common.h"

typedef struct watchpoint {
  int NO;
  char type;
  struct watchpoint *next;
  char expr[32];
  uint32_t old_value;
  uint32_t new_value;
  /* TODO: Add more members if necessary */


} WP;

void  new_wp(char *args);

void free_wp(int num);

void set_NO();

int check_wp();

void info_wp();

void new_bp(char *args);

int check_bp();

#endif

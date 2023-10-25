#ifndef LIB_PARSE_H
#define LIB_PARSE_H

#define JSMN_HEADER
#include "jsmn/jsmn.h"
#include <cstring>
#include <stdlib.h>
#include <stdio.h>

struct testsuite_t {
  struct testcase_t * testcases;
  int num_testcases;
};

struct testcase_t {
  char * name;
  struct parameter_t * parameters;
  int num_parameters;
};

struct parameter_t {
  char * name;
  void * values;
  int num_values;
};

struct testsuite_t testsuite_init(char * s);
void testsuite_print(struct testsuite_t ts);
void testsuite_delete(struct testsuite_t ts);

#endif

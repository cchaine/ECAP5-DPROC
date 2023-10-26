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

enum parameter_type_t {
  STRING,
  BOOLEAN,
  NUMBER
};

struct parameter_t {
  char * name;
  void * values;
  int num_values;
  enum parameter_type_t type;
};

struct testsuite_t testsuite_init(char * s);
void testsuite_print(struct testsuite_t ts);
void testsuite_delete(struct testsuite_t ts);
char * testcase_get_string_value(struct testcase_t * tc, char * parameter_name, int value_index, int * err);
bool testcase_get_bool_value(struct testcase_t * tc, char * parameter_name, int value_index, int * err);
int testcase_get_int_value(struct testcase_t * tc, char * parameter_name, int value_index, int * err);
unsigned int testcase_get_unsigned_int_value(struct testcase_t * tc, char * parameter_name, int value_index, int * err);

#endif

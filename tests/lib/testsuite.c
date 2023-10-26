#include "testsuite.h"


void print_token(jsmntok_t t) {
  printf("tok: ");
  switch(t.type) {
    case JSMN_UNDEFINED:
      printf("UNDEFINED\n");
      break;
    case JSMN_OBJECT:
      printf("OBJECT\n");
      break;
    case JSMN_ARRAY:
      printf("ARRAY\n");
      break;
    case JSMN_STRING:
      printf("STRING\n");
      break;
    case JSMN_PRIMITIVE:
      printf("PRIMITIVE\n");
      break;
  }
  printf("  start: %d\n", t.start);
  printf("  end: %d\n", t.end);
  printf("  size: %d\n\n", t.size);
}

char * get_token_val(char * s, jsmntok_t * t) {
  char * content = (char *)malloc(t->end - t->start +1);
  memcpy(content, s + t->start, t->end - t->start);
  content[t->end - t->start] = '\0';
  return content;
}

void testsuite_print(struct testsuite_t ts) {
  printf("testsuite: %d testcases\n", ts.num_testcases);
  for(int i = 0; i < ts.num_testcases; i++) {
    if(ts.testcases == NULL) {
      printf("  NULL\n");
      continue;
    }
    struct testcase_t tc = ts.testcases[i];
    printf("  testcase: %s, %d parameters\n", tc.name, tc.num_parameters);
    if(tc.parameters == NULL) {
      printf("    NULL\n");
      continue;
    }
    for(int j = 0; j < tc.num_parameters; j++) {
      struct parameter_t f = tc.parameters[j];
      printf("    parameter: %s, type %d with %d values\n", f.name, f.type, f.num_values);
      if(f.values == NULL) {
        printf("      NULL\n");
        continue;
      }
      printf("      ");
      for(int k = 0; k < f.num_values; k++) {
        switch(f.type) {
          case STRING:
            printf("\"%s\" ", ((char **)f.values)[k]);
            break;
          case BOOLEAN:
            printf("%d ", ((bool *)f.values)[k]);
            break;
          case NUMBER:
            printf("%x ", ((unsigned int*)f.values)[k]);
            break;
        }
      }
      printf(", @ %p\n", f.values);
    }
  }
}

void testsuite_delete(struct testsuite_t ts) {
  if(ts.testcases != NULL) {
    for(int i = 0; i < ts.num_testcases; i++) {
      struct testcase_t tc = ts.testcases[i];
      if(tc.name != NULL) {
        free(tc.name);
      } 
      if(tc.parameters != NULL) {
        for(int j = 0; j < tc.num_parameters; j++) {
          struct parameter_t p = tc.parameters[j];
          if(p.name != NULL) {
            free(p.name);
          }
          if(p.values != NULL) {
            if(p.type == STRING) {
              for(int k = 0; k < p.num_values; k++) {
                char ** ptr = (char **)(p.values);
                free(ptr[k]);
              }
            }
            free(p.values);
          }
        }
      }
    }
  }
}

struct testsuite_t testsuite_init(char * path) {
  // Open the json testcase file
  FILE *fptr;
  fptr = fopen(path, "rb");
  fseek(fptr, 0, SEEK_END);
  int fsize = ftell(fptr);
  fseek(fptr, 0, SEEK_SET);
  char * s = (char *)malloc(fsize + 1);
  fread(s, fsize, 1, fptr);
  fclose(fptr);


  // Parse with jsmn
  jsmn_parser p;
  jsmntok_t t[1000];
  jsmn_init(&p);
  int r = jsmn_parse(&p, s, strlen(s), t, 1000);
  if((r == JSMN_ERROR_INVAL) || (r == JSMN_ERROR_NOMEM) || (r == JSMN_ERROR_PART)) {
    printf("[ERROR]: Failed to parse %s\n", path);
    return {0};
  }

  int token_index = 0;
  // find the "testcases" key
  bool found = false;
  while(!found && token_index < r) {
    jsmntok_t tok = t[token_index];
    found = (tok.type == JSMN_STRING);
    found &= (strncmp("testcases", s + tok.start, tok.end - tok.start) == 0);
    token_index++;
  }
  if(!found) {
    printf("[ERROR]: Syntax error in %s. No \"testcases\" found\n", path);
    return {0};
  }

  // Create the testsuite
  struct testsuite_t testsuite;
  testsuite.num_testcases = t[token_index].size;
  testsuite.testcases = (struct testcase_t *) malloc(testsuite.num_testcases * sizeof(struct testcase_t));

  // Loop through the remaining tokens and fill the testsuite
  token_index += 2;

  jsmntok_t * tok;
  int level = 0;
  int testcase_index = 0;
  int parameter_index = 0;
  int parameter_value_index = 0;
  struct testcase_t * testcase;
  struct parameter_t * parameter;
  char ** string_values;
  bool * boolean_values;
  int * number_values;
  while(token_index < r) {
    tok = &t[token_index];
    switch(level) {
      // handle content of testcase_t objects
      case 0: {
        if(testcase_index == testsuite.num_testcases) {
          // if we dealt with every testcase, we skip the remaining tokens and leave the loop
          token_index = r;
        } else {
          testcase = &testsuite.testcases[testcase_index];

          // get the key
          char * key = get_token_val(s, tok);

          if(strncmp("name", key, tok->end - tok->start) == 0) {
            // process the name of the testcase
            token_index += 1;
            tok = &t[token_index];
            char * name = get_token_val(s, tok);
            testcase->name = get_token_val(s, tok);
          } else if(strncmp("parameters", key, tok->end - tok->start) == 0) {
            // process the parameter's array
            token_index += 1;
            tok = &t[token_index];
            testcase->parameters = (struct parameter_t *) malloc((tok->size) * sizeof(struct parameter_t));
            testcase->num_parameters = tok->size;

            // switch to handling the content of the parameters
            parameter_index = 0;
            level += 1;
          } else {
            printf("[ERROR]: Unexpected %s \n", key);
            free(key);
            return {0};
          }
          free(key);
        }
        break;
      }
      // handle content of parameter_t objects
      case 1: {
        if(parameter_index == testcase->num_parameters) {
          // if we dealt with every parameter, we go up a level and handle the next testcase
          testcase_index += 1;
          level -= 1;
        } else {
          parameter = &testcase->parameters[parameter_index];

          // handle parameter name
          char * name = get_token_val(s, tok);
          parameter->name = name;

          // handle parameter value array
          token_index += 1;
          tok = &t[token_index];
          parameter->values = NULL;
          parameter->num_values = tok->size;

          // switch to handling the values of the parameter
          parameter_value_index = 0;
          level += 1;
        }
        break;
      }
      // handle values of parameter_t objects
      case 2: {
        char * data = get_token_val(s, tok);
        // get the type of data
        enum parameter_type_t type;
        if(tok->type == JSMN_STRING) {
          type = STRING;
        } else if(tok->type == JSMN_PRIMITIVE) {
          if((data[0] == 't') || (data[0] == 'f')) {
            type = BOOLEAN;
          } else if((data[0] == '-') || ('0' <= data[0] && data[0] <= '9')) {
            type = NUMBER;
          } else {
            printf("[ERROR]: Unknown primitive data type for \"%s\"\n", data);
            return {0};
          }
        } else {
          printf("[ERROR]: Unexpected token type %d for \"%s\"\n", tok->type, data);
          return {0};
        }
        
        // initialize the table of values
        if(parameter->values == NULL) {
          parameter->type = type;
          switch(type) {
            case STRING:
              string_values = (char **) malloc(parameter->num_values * sizeof(char *));
              parameter->values = string_values;
              break;
            case BOOLEAN:
              boolean_values = (bool *) malloc(parameter->num_values * sizeof(bool));
              parameter->values = boolean_values;
              break;
            case NUMBER:
              number_values = (int *) malloc(parameter->num_values * sizeof(int));
              parameter->values = number_values;
              break;
          }
        }

        // check for type consistency
        if(type != parameter->type) {
          printf("[ERROR]: Data type inconsistencies, got %d but expected %d\n", type, parameter->type);
          return {0};
        }

        // store the data
        switch(type) {
          case STRING:
            string_values[parameter_value_index] = data;
            break;
          case BOOLEAN:
            if(strncmp("true", data, 4) == 0) {
              boolean_values[parameter_value_index] = true;
            } else if(strncmp("false", data, 5) == 0) {
              boolean_values[parameter_value_index] = false;
            } else {
              printf("[ERROR]: malformed boolean value for \"%s\"\n", data);
              return {0};
            }
            free(data);
            break;
          case NUMBER:
            char * last_char = data + strlen(data);
            char * end;
            long converted_data = strtol(data, &end, 0); 
            if(end != last_char) {
              printf("[ERROR]: the value \"%s\" cannot be converted to a number\n", data);
              return {0};
            }
            if(converted_data > 0xFFFFFFFF) {
              printf("[ERROR]: only 32-bit values are supported, received %ld\n", converted_data);
              return {0};
            }
            number_values[parameter_value_index] = (int)converted_data;
            free(data);
            break;
        }

        parameter_value_index += 1;
        if(parameter_value_index == testsuite.testcases[testcase_index].parameters[parameter_index].num_values) {
          // if we dealt with every value of the parameter, we go up a level and handle the next parameter
          parameter_index += 1;
          level -= 1;
        }
        break;
      }
    } 
    token_index += 1;
  }

  free(s);

  return testsuite;
}

int find_testcase_parameter(struct testcase_t * tc, char * parameter_name) {
  int index = 0;
  bool found = false;
  while(!found && (index < tc->num_parameters)) {
    found = (strcmp(tc->parameters[index].name, parameter_name) == 0);
    index += (found ? 0 : 1);
  }
  if(!found) {
    index = -1;
  }
  return index;
}

char * testcase_get_string_value(struct testcase_t * tc, char * parameter_name, int value_index, int * err) {
  char * result = NULL; 
  *err = 1;

  int index = find_testcase_parameter(tc, parameter_name);
  char ** values = (char **)tc->parameters[index].values;
  if(index != -1) {
    if(value_index < tc->parameters[index].num_values) {
      result = values[value_index]; 
      *err = 0;
    }
  }
  return result;
}

bool testcase_get_boolean_value(struct testcase_t * tc, char * parameter_name, int value_index, int * err) {
  bool result = false; 
  *err = 1;

  int index = find_testcase_parameter(tc, parameter_name);
  bool * values = (bool*)tc->parameters[index].values;
  if(index != -1) {
    if(value_index < tc->parameters[index].num_values) {
      result = values[value_index]; 
      *err = 0;
    }
  }
  return result;
}

int testcase_get_int_value(struct testcase_t * tc, char * parameter_name, int value_index, int * err) {
  int result = 0; 
  *err = 1;

  int index = find_testcase_parameter(tc, parameter_name);
  int * values = (int*)tc->parameters[index].values;
  if(index != -1) {
    if(value_index < tc->parameters[index].num_values) {
      result = values[value_index]; 
      *err = 0;
    }
  }
  return result;
}

unsigned int testcase_get_unsigned_int_value(struct testcase_t * tc, char * parameter_name, int value_index, int * err) {
  unsigned int result = 0; 
  *err = 1;

  int index = find_testcase_parameter(tc, parameter_name);
  unsigned int * values = (unsigned int*)tc->parameters[index].values;
  if(index != -1) {
    if(value_index < tc->parameters[index].num_values) {
      result = values[value_index]; 
      *err = 0;
    }
  }
  return result;
}

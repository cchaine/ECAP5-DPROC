#include "parse.h"


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

void print_testsuite(struct testsuite_t ts) {
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
      printf("    parameter: %s, %d values\n", f.name, f.num_values);
      if(f.values == NULL) {
        printf("      NULL\n");
        continue;
      }
      printf("      ");
      for(int k = 0; k < f.num_values; k++) {
        printf("%x ", ((unsigned int*)f.values)[k]);
      }
      printf("\n");
    }
  }
}

void delete_testsuite(struct testsuite ts) {
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
          if(values != NULL) {
            free(p.values);
          }
        }
      }
    }
  }
}

struct testsuite_t parse(char * path) {
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
  while(token_index < r) {
    tok = &t[token_index];
    switch(level) {
      // handle content of testcase_t objects
      case 0: {
        if(testcase_index == testsuite.num_testcases) {
          // if we dealt with every testcase, we skip the remaining tokens and leave the loop
          token_index = r;
        } else {
          // get the key
          char * key = get_token_val(s, tok);

          if(strncmp("name", key, tok->end - tok->start) == 0) {
            // process the name of the testcase
            token_index += 1;
            tok = &t[token_index];
            char * name = get_token_val(s, tok);
            testsuite.testcases[testcase_index].name = get_token_val(s, tok);
          } else if(strncmp("parameters", key, tok->end - tok->start) == 0) {
            // process the parameter's array
            token_index += 1;
            tok = &t[token_index];
            testsuite.testcases[testcase_index].parameters = (struct parameter_t *) malloc((tok->size) * sizeof(struct parameter_t));
            testsuite.testcases[testcase_index].num_parameters = tok->size;

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
        if(parameter_index == testsuite.testcases[testcase_index].num_parameters) {
          // if we dealt with every parameter, we go up a level and handle the next testcase
          testcase_index += 1;
          level -= 1;
        } else {
          // handle parameter name
          char * name = get_token_val(s, tok);
          testsuite.testcases[testcase_index].parameters[parameter_index].name = name;

          // handle parameter value array
          token_index += 1;
          tok = &t[token_index];
          testsuite.testcases[testcase_index].parameters[parameter_index].values = NULL;
          testsuite.testcases[testcase_index].parameters[parameter_index].num_values = tok->size;

          // switch to handling the values of the parameter
          parameter_value_index = 0;
          level += 1;
        }
        break;
      }
      // handle values of parameter_t objects
      case 2: {
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

  print_testsuite(testsuite);

  free(s);

  return testsuite;
}

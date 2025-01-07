#include "contracts.h"
#include "string_ext.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

void print_tokens(char **tokens, uint8_t *num_tokens){
  REQUIRES(tokens != NULL);
  REQUIRES(num_tokens != NULL);
  printf("tokenized: ");
  for (uint8_t i = 0; i < *num_tokens; i++){
    printf("%s, ", tokens[i]);
  }
  printf("\n");
}
void set_num_tokens(char *str, uint8_t *num_tokens){
  REQUIRES(str != NULL);
  REQUIRES(num_tokens != NULL);

  char *found;                   // ptr for parsed tokens
  char *str_copy = strdup(str);  // duplicate str for parsing
  char *temp = str_copy;         // store ptr to copy for free

  *num_tokens = 0;
  while ((found = strsep(&str_copy, " "))!= NULL){
    (*num_tokens)++;               // printf("%s\n", found);
  }
  free(temp);                        // printf("found %u tokens\n", *num_tokens);
}

char **tokenize_string(char *str, uint8_t *num_tokens){
  REQUIRES(str != NULL);
  REQUIRES(num_tokens != NULL);

  char *found;                   // ptr for parsed tokens
  char *str_copy = strdup(str);  // duplicate str for parsing
  char *temp = str_copy;         // store ptr to copy for free

  set_num_tokens(str, num_tokens);   // printf("num tokens: %u \n", *num_tokens);

  char **tokens = calloc((*num_tokens), sizeof(char*));

  int i = 0;
  while ((found = strsep(&str_copy, " "))!= NULL){
    ASSERT(i <= *num_tokens);
//  tokens[i] = found;
    tokens[i] = strdup(found);     // printf("%s\n", found);
    i++;
  }
  free(temp);
  print_tokens(tokens, num_tokens);
  return tokens;
}



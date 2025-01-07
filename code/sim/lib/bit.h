#include "contracts.h"
#include "tokenize.h"
#include "rand.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>

/* *****************************************************************************
Allowed Instructions
***************************************************************************** */
enum probability_instruction {
  NOT,
  CNOT,
  CCNOT,
  RNG
};
typedef enum probability_instruction instr;

uint32_t char2int(char c){
  return (uint32_t) (c - '0');
}

instr char2instr(char *c){
  REQUIRES(c != NULL);
  if (strcmp(c, "NOT") == 0){
    return NOT;
  } else if (strcmp(c, "CNOT") == 0){
    return CNOT;
  } else if (strcmp(c, "CCNOT") == 0){
    return CCNOT;
  } else if (strcmp(c, "RNG") == 0){
    return RNG;
  } else {
    printf("invalid char*: %s\n", c);
    assert(false);
  }
}
/* *****************************************************************************
Bit Struct, Representation Invariant, Constructor, Free
***************************************************************************** */
struct bit_state {
  uint8_t code_len;
  char **code; // length = code_len;  ex: ["n", "NOT", "1", "CNOT", "2", "1"...]
  uint8_t n_bits;
  bool *bits;  // length == n_bits;
};
typedef struct bit_state state;

bool is_state(state *S){
  if (S == NULL) return false;
  else if (S->bits == NULL) return false;
  else if (S->code == NULL) return false;

  for (uint8_t i = 0; i < S->code_len; i++){
    if (S->code[i] == NULL) return false;
  }
  return true;
}

state* state_new(char code[]){
  state *S = malloc(sizeof(state));
  assert(S != NULL);

  S->code = tokenize_string(code, &S->code_len);
  S->n_bits = char2int(*(S->code[0]));

  S->bits = (bool*) calloc(sizeof(bool), S->n_bits+1);
  ASSERT(S->bits != NULL);

  ENSURES(is_state(S));
  return S;
}

void state_free(state* S){
  REQUIRES(is_state(S));
  for (uint8_t i = 0; i < S->code_len; i++){
    free(S->code[i]);
  }
  free(S->code);
  free(S->bits);
  free(S);
}

/* *****************************************************************************
Instructions
***************************************************************************** */

// toggle b
void instr_not(bool *b){
  REQUIRES(b != NULL);
  *b = !(*b);
  ENSURES(b != NULL);
}
// if a toggle b
void instr_cnot(bool *a, bool *b){
  REQUIRES(a != NULL && b != NULL);
  if (*a) instr_not(b);
  ENSURES(a != NULL && b != NULL);
}
// if a and b toggle c
void instr_ccnot(bool *a, bool *b, bool *c){
  REQUIRES(a != NULL && b != NULL && c != NULL);
  if (*a && *b) instr_not(c);
  ENSURES(a != NULL && b != NULL && c != NULL);
}
// randomly toggle b (PRNG)
void instr_rng(bool *b){
  srand(more_chaotic_seed());  // Seed with the current time
  REQUIRES(b != NULL);
  int i = rand();
  printf("random i: %d\n", i);
  if (i < RAND_MAX/2) instr_not(b);
  ENSURES(b != NULL);
}

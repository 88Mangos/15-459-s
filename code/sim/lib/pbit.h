#include "contracts.h"
#include "tokenize.h"
#include "frac.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>

/* *****************************************************************************
Allowed Instructions
***************************************************************************** */
typedef uint32_t bitstr;// 32-bit unsigned ints for bitstrings
typedef frac* pr;       // fractions are probabilities
typedef uint8_t pbit;

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
struct pbit_state {
  uint8_t code_len;
  char **code; // length = code_len;  ex: ["n", "NOT", "1", "CNOT", "2", "1"...]
  uint8_t n_pbits;
  uint32_t n_states; // = 2^{n_pbits}
  pr *states; // length == n_states, sum over arr = 1
};
typedef struct pbit_state state;

bool is_state(state *S){
  if (S == NULL) return false;
  else if (S->states == NULL) return false;
  else if (S->code == NULL) return false;

  for (uint8_t i = 0; i < S->code_len; i++){
    if (S->code[i] == NULL) return false;
  }

  if (S->n_states != ((uint32_t) 1) << S->n_pbits) return false;

  // to check if the probability states are valid,
  // sum over all the numerators and denominators
  pr F = frac_new(0, 1); // frac with value 0.
  for (bitstr i = 0; i < S->n_states; i++){
    if (S->states[i] == NULL) return false;
    pr temp = F;
    F = frac_add(F, S->states[i]);
    free(temp);
  }
  bool sum_is_1 = (F->num == F->denom);
  free(F);
  return sum_is_1;
}

state* state_new(char code[]){
  state *S = malloc(sizeof(state));
  assert(S != NULL);

  S->code = tokenize_string(code, &S->code_len);
  S->n_pbits = char2int(*(S->code[0]));
  S->n_states = ((uint32_t) 1) << S->n_pbits;

  S->states = (pr*) calloc(sizeof(pr), S->n_states);
  ASSERT(S->states != NULL);

  S->states[0] = frac_new(1,1); // ket{00...0} starts with prob 1
  for (bitstr i = 1; i < S->n_states; i++){
    S->states[i] = frac_new(0,1); // initialize other states to prob 0
  }

  ENSURES(is_state(S));
  return S;
}

void state_free(state* S){
  REQUIRES(is_state(S));
  for (uint8_t i = 0; i < S->code_len; i++){
    free(S->code[i]);
  }
  free(S->code);
  for (bitstr i = 0; i < S->n_states; i++){
    free(S->states[i]);
  }
  free(S->states);
  free(S);
}

/* *****************************************************************************
Instructions
***************************************************************************** */

void print_prob(state *S, bitstr x){
  printf("final probability on state %u is %d/%d\n",
      x, S->states[x]->num, S->states[x]->denom);
}

// toggle b
void instr_not(state *S, pbit b){
  REQUIRES(is_state(S));
  REQUIRES(b <= S->n_pbits);

  bool *updated = calloc(sizeof(bool), S->n_states); // track updated states
  bitstr b_mask = ((uint32_t) 1) << (S->n_pbits - b); // mask to toggle pbit b

  for (bitstr t = 0; t < S->n_states; t++){
    if (updated[t]) continue;    // move on if we've already updated t
    bitstr tb = t ^ b_mask;      // state t, with bit b toggled

    // swap the probs for state t and tb
    pr temp = S->states[tb];
    S->states[tb] = S->states[t];
    S->states[t] = temp;

    updated[t] = true;       // mark t as updated
    updated[tb] = true;      // mark tb as updated
  }
  free(updated);
  ENSURES(is_state(S));
}

// if a toggle b
void instr_cnot(state *S, pbit a, pbit b){
  REQUIRES(is_state(S));
  REQUIRES(a != b);
  REQUIRES(a <= S->n_pbits && b <= S->n_pbits);

  bool *updated = calloc(sizeof(bool), S->n_states); // track updated states
  bitstr a_mask = ((uint32_t) 1) << (S->n_pbits - a); // mask to toggle pbit a
  bitstr b_mask = ((uint32_t) 1) << (S->n_pbits - b); // mask to toggle pbit b

  for (bitstr t = 0; t < S->n_states; t++){
    if (updated[t]) continue;      // move on if we've already updated t
    bitstr tb = t ^ b_mask;      // state t, with bit b toggled

    if ((t & a_mask) == a_mask){  // if the ath bit is a 1
      pr temp = S->states[tb];  // swap the probs for state t and tb
      S->states[tb] = S->states[t];
      S->states[t] = temp;
    }
    updated[t] = true;       // mark t as updated
    updated[tb] = true;      // mark tb as updated
  }
  free(updated);
  ENSURES(is_state(S));
}

// if a and b toggle c
void instr_ccnot(state *S, pbit a, pbit b, pbit c){
  REQUIRES(is_state(S));
  REQUIRES(a != b && b != c && a != c);
  REQUIRES(a <= S->n_pbits && b <= S->n_pbits && c <= S->n_pbits);

  bool *updated = calloc(sizeof(bool), S->n_states); // track updated states
  bitstr a_mask = ((uint32_t) 1) << (S->n_pbits - a); // mask to toggle pbit a
  bitstr b_mask = ((uint32_t) 1) << (S->n_pbits - b); // mask to toggle pbit b
  bitstr c_mask = ((uint32_t) 1) << (S->n_pbits - c); // mask to toggle pbit c

  for (bitstr t = 0; t < S->n_states; t++){
    if (updated[t]) continue;      // move on if we've already updated t
    bitstr tc = t ^ c_mask;      // state t, with bit c toggled

    if ((t & a_mask) == a_mask && (t & b_mask) == b_mask){  // if the ath, bth bit are 1
      pr temp = S->states[tc];  // swap the probs for state t and tc
      S->states[tc] = S->states[t];
      S->states[t] = temp;
    }
    updated[t] = true;       // mark t as updated
    updated[tc] = true;      // mark tc as updated
  }
  free(updated);
  ENSURES(is_state(S));
}

// randomly toggle b (PRNG)
void instr_rng(state *S, pbit b){
  REQUIRES(is_state(S));
  REQUIRES(b <= S->n_pbits);

  pr half = frac_new(1,2);

  bool *updated = calloc(sizeof(bool), S->n_states); // track updated states
  bitstr b_mask = ((uint32_t) 1) << (S->n_pbits - b); // mask to toggle pbit b

  for (bitstr t = 0; t < S->n_states; t++){
    if (updated[t]) continue;    // move on if we've already updated t
    bitstr tb = t ^ b_mask;      // state t, with bit b toggled

//    printf("t is %u, tb is: %u\n", t, tb);
    pr temp1 = S->states[tb];
    pr temp2 = S->states[t];

    // distribute probability evenly, each gets 1/2
    pr sum = frac_add(S->states[tb], S->states[t]);

    S->states[tb] = frac_mult(sum, half);
    S->states[t] = frac_mult(sum, half);

    free(temp1);
    free(temp2);
    free(sum);

    updated[t] = true;       // mark t as updated
    updated[tb] = true;      // mark tb as updated
  }
  free(half);
  free(updated);
  ENSURES(is_state(S));
}

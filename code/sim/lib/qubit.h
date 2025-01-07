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
typedef frac* amp;       // fractions are amplitudes
typedef uint8_t qubit;

enum quantum_instruction {
  NOT,
  CNOT,
  CCNOT,
  HAD
};
typedef enum quantum_instruction instr;

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
  } else if (strcmp(c, "HAD") == 0){
    return HAD;
  } else {
    printf("invalid char*: %s\n", c);
    assert(false);
  }
}
/* *****************************************************************************
Bit Struct, Representation Invariant, Constructor, Free
***************************************************************************** */
struct qubit_state {
  uint8_t code_len;
  char **code; // length = code_len;  ex: ["n", "NOT", "1", "CNOT", "2", "1"...]
  uint8_t n_qubits;
  uint32_t n_states; // = 2^{n_qubits}
  amp *states; // length == n_states, sum over arr = 1
  uint8_t hadamards; // keep track of number of hadamards
                     // that way, if it's even, we do add & diff
                     // if odd, we can ``undo'' and do avg & dev
};
typedef struct qubit_state state;

bool is_state(state *S){
  if (S == NULL) return false;
  else if (S->states == NULL) return false;
  else if (S->code == NULL) return false;

  for (uint8_t i = 0; i < S->code_len; i++){
    if (S->code[i] == NULL) return false;
  }

  if (S->n_states != ((uint32_t) 1) << S->n_qubits) return false;

  // to check if the amplitudes states are valid,
  // sum of squares over all the numerators and denominators
  amp F = frac_new(0, 1); // frac with value 0.
  for (bitstr i = 0; i < S->n_states; i++){
    if (S->states[i] == NULL) return false;
    amp temp = F;
    amp Fsq = frac_square(S->states[i]);
    F = frac_add(F, Fsq);
//    printf("%u: %d/%d\n", i, F->num, F->denom);
    free(temp);
    free(Fsq);
  }
  // to deal with potentially unnormalized states:
  bool valid;
  if (S->hadamards % 2 == 0){
    // should be normalized; Add&Diff and Avg&Dev should cancel.
    valid = (F->num == F->denom);
  }
  else { ASSERT(S->hadamards % 2 == 1);
    // should be unnormalized; Add&Diff doubles sum of squares.
    valid = (F->num == F->denom * 2);
  }
//  printf("hadamards: %d, %d/%d\n", S->hadamards, F->num, F->denom);
  free(F);
  return valid;
}

state* state_new(char code[]){
  state *S = malloc(sizeof(state));
  assert(S != NULL);

  S->code = tokenize_string(code, &S->code_len);
  S->n_qubits = char2int(*(S->code[0]));
  S->n_states = ((uint32_t) 1) << S->n_qubits;
  S->hadamards = 0;

  S->states = (amp*) calloc(sizeof(amp), S->n_states);
  ASSERT(S->states != NULL);

  S->states[0] = frac_new(1,1); // ket{00...0} starts with amplitude 1
  for (bitstr i = 1; i < S->n_states; i++){
    S->states[i] = frac_new(0,1); // initialize other states to amplitude 0
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

void print_amp(state *S, bitstr x){
  printf("final amplitude on state %u is %d/%d\n",
      x, S->states[x]->num, S->states[x]->denom);
}

// toggle b
void instr_not(state *S, qubit b){
  REQUIRES(is_state(S));
  REQUIRES(b <= S->n_qubits);

  bool *updated = calloc(sizeof(bool), S->n_states); // track updated states
  bitstr b_mask = ((uint32_t) 1) << (S->n_qubits - b); // mask to toggle qubit b

  for (bitstr t = 0; t < S->n_states; t++){
    if (updated[t]) continue;    // move on if we've already updated t
    bitstr tb = t ^ b_mask;      // state t, with bit b toggled

    // swap the amplitudes for state t and tb
    amp temp = S->states[tb];
    S->states[tb] = S->states[t];
    S->states[t] = temp;

    updated[t] = true;       // mark t as updated
    updated[tb] = true;      // mark tb as updated
  }
  free(updated);
  ENSURES(is_state(S));
}

// if a toggle b
void instr_cnot(state *S, qubit a, qubit b){
  REQUIRES(is_state(S));
  REQUIRES(a != b);
  REQUIRES(a <= S->n_qubits && b <= S->n_qubits);

  bool *updated = calloc(sizeof(bool), S->n_states); // track updated states
  bitstr a_mask = ((uint32_t) 1) << (S->n_qubits - a); // mask to toggle qubit a
  bitstr b_mask = ((uint32_t) 1) << (S->n_qubits - b); // mask to toggle qubit b

  for (bitstr t = 0; t < S->n_states; t++){
    if (updated[t]) continue;      // move on if we've already updated t
    bitstr tb = t ^ b_mask;      // state t, with bit b toggled

    if ((t & a_mask) == a_mask){  // if the ath bit is a 1
      amp temp = S->states[tb];  // swap the amplitudes for state t and tb
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
void instr_ccnot(state *S, qubit a, qubit b, qubit c){
  REQUIRES(is_state(S));
  REQUIRES(a != b && b != c && a != c);
  REQUIRES(a <= S->n_qubits && b <= S->n_qubits && c <= S->n_qubits);

  bool *updated = calloc(sizeof(bool), S->n_states); // track updated states
  bitstr a_mask = ((uint32_t) 1) << (S->n_qubits - a); // mask to toggle qubit a
  bitstr b_mask = ((uint32_t) 1) << (S->n_qubits - b); // mask to toggle qubit b
  bitstr c_mask = ((uint32_t) 1) << (S->n_qubits - c); // mask to toggle qubit c

  for (bitstr t = 0; t < S->n_states; t++){
    if (updated[t]) continue;      // move on if we've already updated t
    bitstr tc = t ^ c_mask;      // state t, with bit c toggled

    if ((t & a_mask) == a_mask && (t & b_mask) == b_mask){  // if the ath, bth bit are 1
      amp temp = S->states[tc];  // swap the amplitudes for state t and tc
      S->states[tc] = S->states[t];
      S->states[t] = temp;
    }
    updated[t] = true;       // mark t as updated
    updated[tc] = true;      // mark tc as updated
  }
  free(updated);
  ENSURES(is_state(S));
}

void instr_had(state *S, qubit b){
  REQUIRES(is_state(S));
  REQUIRES(b <= S->n_qubits);

  amp half = frac_new(1,2);

  bool *updated = calloc(sizeof(bool), S->n_states); // track updated states
  bitstr b_mask = ((uint32_t) 1) << (S->n_qubits - b); // mask to toggle qubit b

  for (bitstr t = 0; t < S->n_states; t++){
    if (updated[t]) continue;    // move on if we've already updated t
    bitstr tb = t ^ b_mask;      // state t, with bit b toggled
    // Because we have ``updated'' array, t should be b=0, tb should be b=1
    ASSERT((t & b_mask) == 0x0 && (tb & b_mask) == b_mask);

    amp temp1 = S->states[tb];
    amp temp2 = S->states[t];

    amp sum = frac_add(S->states[t], S->states[tb]);
    amp diff = frac_subtract(S->states[t], S->states[tb]);

    if (S->hadamards % 2 == 0){  // if even # of hadamards, Add&Diff
      S->states[t] = sum;        // the state with b=0 gets Add or Avg,
      S->states[tb] = diff;      // the state with b=1 gets Diff or Dev.
    }
    else { ASSERT(S->hadamards % 2 == 1); // if odd # of hadamards, Avg&Dev
      S->states[t] = frac_mult(sum, half);
      S->states[tb] = frac_mult(diff, half);
      free(sum);
      free(diff);
    }
    free(temp1);
    free(temp2);

    updated[t] = true;       // mark t as updated
    updated[tb] = true;      // mark tb as updated
  }
  free(half);
  free(updated);
  S->hadamards = S->hadamards + 1;
  ENSURES(is_state(S));
}

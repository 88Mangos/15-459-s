#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

#include "lib/pbit.h"
#include "lib/contracts.h"

state *simulate(state *S){
  REQUIRES(S != NULL);

  char **P = S->code;
  uint8_t pc = 1;

  while (pc < S->code_len){
    instr check = char2instr(P[pc]);
    switch (check){
      case NOT: {
        pbit i = (pbit) char2int(*(P[pc + 1]));
        instr_not(S, i);
        pc += 2;
        break;
      }
      case CNOT: {
        pbit i = (pbit) char2int(*(P[pc + 1]));
        pbit j = (pbit) char2int(*(P[pc + 2]));
        instr_cnot(S, i, j);
        pc += 3;
        break;
      }
      case CCNOT: {
        pbit i = (pbit) char2int(*(P[pc + 1]));
        pbit j = (pbit) char2int(*(P[pc + 2]));
        pbit k = (pbit) char2int(*(P[pc + 3]));
        instr_ccnot(S, i, j, k);
        pc += 4;
        break;
      }
      case RNG: {
        pbit i = (pbit) char2int(*(P[pc + 1]));
        instr_rng(S, i);
        pc += 2;
        break;
      }
      default: {
        printf("invalid token");
        ASSERT(false);
        break;
      }
    }
  }
  return S;
}

int main(){
//  char *code = "5 NOT 1";
//  char *code = "5 NOT 1 NOT 2";
//  char *code = "5 NOT 1 NOT 2 CNOT 2 3";
//  char *code = "5 NOT 1 NOT 2 CNOT 2 3 CCNOT 1 2 3";
//  char *code = "5 NOT 1 NOT 2 CNOT 2 3 CCNOT 1 2 3 RNG 5";
// WORKED FROM MANUAL CHECKING!
  char code[] = "3 NOT 1 RNG 1 RNG 2 CNOT 1 3 CNOT 2 3 CCNOT 1 2 3 CNOT 3 2 RNG 3";
// WORKED FROM MANUAL CHECKING!
  state *S = state_new(code);
  S = simulate(S);
  for (bitstr i = 0; i < S->n_states; i++){
    print_prob(S, i);
  }
  state_free(S);
  return 0;
}


#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

#include "lib/qubit.h"
#include "lib/contracts.h"

state *simulate(state *S){
  REQUIRES(S != NULL);

  char **P = S->code;
  uint8_t pc = 1;

  while (pc < S->code_len){
    instr check = char2instr(P[pc]);
    switch (check){
      case NOT: {
        qubit i = (qubit) char2int(*(P[pc + 1]));
        instr_not(S, i);
        pc += 2;
        break;
      }
      case CNOT: {
        qubit i = (qubit) char2int(*(P[pc + 1]));
        qubit j = (qubit) char2int(*(P[pc + 2]));
        instr_cnot(S, i, j);
        pc += 3;
        break;
      }
      case CCNOT: {
        qubit i = (qubit) char2int(*(P[pc + 1]));
        qubit j = (qubit) char2int(*(P[pc + 2]));
        qubit k = (qubit) char2int(*(P[pc + 3]));
        instr_ccnot(S, i, j, k);
        pc += 4;
        break;
      }
      case HAD: {
        qubit i = (qubit) char2int(*(P[pc + 1]));
        instr_had(S, i);
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
//  char *code = "5 NOT 1 NOT 2 CNOT 2 3 CCNOT 1 2 3 HAD 5";
//  char *code = "5 NOT 1 NOT 2 CNOT 2 3 CCNOT 1 2 3 HAD 5 HAD 5";
// WORKED FROM MANUAL CHECKING!
  char code[] = "3 NOT 1 HAD 1 HAD 2 CNOT 1 3 CNOT 2 3 CCNOT 1 2 3 CNOT 3 2 HAD 3";
// WORKED FROM MANUAL CHECKING!
  state *S = state_new(code);
  S = simulate(S);
  for (bitstr i = 0; i < S->n_states; i++){
    print_amp(S, i);
  }
  state_free(S);
  return 0;
}


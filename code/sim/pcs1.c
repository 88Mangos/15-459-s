#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

#include "lib/bit.h"
#include "lib/contracts.h"

state *simulate(state *S){
  REQUIRES(S != NULL);

  char **P = S->code;
  uint8_t n = S->n_bits;
  uint8_t pc = 1;
  bool *V = S->bits;

  while (pc < S->code_len){
    instr check = char2instr(P[pc]);
    switch (check){
      case NOT: {
        uint32_t i = char2int(*(P[pc + 1]));
        ASSERT(1 <= i && i <= n);
        instr_not(&V[i]);                 // toggle x_i
        pc += 2;
        break;
      }
      case CNOT: {
        uint32_t i = char2int(*(P[pc + 1]));
        ASSERT(1 <= i && i <= n);
        uint32_t j = char2int(*(P[pc + 2]));
        ASSERT(1 <= j && j <= n);
        ASSERT(i != j);                // distinct x_i and x_j
        instr_cnot(&V[i], &V[j]);      // if x_i then toggle x_j
        pc += 3;
        break;
      }
      case CCNOT: {
        uint32_t i = char2int(*(P[pc+1]));
        ASSERT(1 <= i && i <= n);
        uint32_t j = char2int(*(P[pc + 2]));
        ASSERT(1 <= j && j <= n);
        ASSERT(i != j);                // distinct x_i and x_j
        uint32_t k = char2int(*(P[pc + 3]));
        ASSERT(1 <= k && k <= n);
        ASSERT(i != k && j != k);      // distinct x_i, x_j, x_k
        instr_ccnot(&V[i], &V[j], &V[k]);    // if x_i and x_j then toggle x_k
        pc += 4;
        break;
      }
      case RNG: {
        uint32_t i = char2int(*(P[pc+1]));
        ASSERT(1 <= i && i <= n);
        instr_rng(&V[i]);
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
  printf("final state: ");
  for (uint8_t i = 1; i <= n; i++){
    printf("%d ", V[i]);
  }
  printf("\n");
  return S;
}

void test_tokenize(char *code){
  uint8_t length;
  set_num_tokens(code, &length);
  printf("tokens: %u\n", length);
  char** c = tokenize_string(code, &length);
  free(c);
}

int main(){
  printf("PROGRAM START\n");
  char code[] = "5 NOT 1 NOT 2 CNOT 2 3 CCNOT 1 2 3 RNG 5";
  state *S = state_new(code);
  S = simulate(S);
  state_free(S);
  return 0;
}


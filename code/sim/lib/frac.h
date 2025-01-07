#include "contracts.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

int32_t gcd(int32_t a, int32_t b) {
  // using Euclidean Algorithm
  int32_t res = 0;
  if (a == 0) res = b;
  else if (b == 0 || a == b) res = a;
  else if (a > b) res = gcd(a % b, b);
  else res = gcd(a, b % a);
  ENSURES(res > 0);
  return res;
}

int32_t lcm(int32_t a, int32_t b){
  // use fact that ab = gcd(a,b) * lcm(a,b);
  int32_t res = a * b / gcd(a,b);
  ENSURES(res > 0);
  return res;
}

struct fraction {
  int32_t num;
  int32_t denom; // strictly positive
};
typedef struct fraction frac;

bool is_frac(frac *F){
  return F != NULL
      && F->denom > 0;
}

void print_frac(frac *F){
  REQUIRES(is_frac(F));
  printf("%d/%d\n", F->num, F->denom);
}

frac* frac_new(int32_t n, int32_t d){
  REQUIRES(d > 0);
  frac *F = malloc(sizeof(frac));
  F->num = n;
  F->denom = d;
  ENSURES(is_frac(F));
  return F;
}

void frac_reduce(frac *F){
  REQUIRES(is_frac(F));
  // reduce num/denom to lowest terms,
  // dividing top and bottom by x
  int32_t x = gcd(F->num, F->denom);
  F->num /= x;
  F->denom /= x;
}

frac* frac_add(frac *A, frac *B){
  REQUIRES(is_frac(A) && is_frac(B));
  frac *F = malloc(sizeof(frac));
  int32_t lcm_ab = lcm(A->denom,B->denom);
  F->denom = lcm_ab;
  F->num = A->num * lcm_ab / A->denom
         + B->num * lcm_ab / B->denom;

  ENSURES(is_frac(F));
  return F;
}

frac* frac_subtract(frac *A, frac *B){
  REQUIRES(is_frac(A) && is_frac(B));
  frac *F = malloc(sizeof(frac));
  int32_t lcm_ab = lcm(A->denom,B->denom);
  F->denom = lcm_ab;
  F->num = A->num * lcm_ab / A->denom
         - B->num * lcm_ab / B->denom;

  ENSURES(is_frac(F));
  return F;
}

frac* frac_mult(frac *A, frac *B){
  REQUIRES(is_frac(A) && is_frac(B));
  frac *F = malloc(sizeof(frac));
  F->num = A->num * B->num;
  F->denom = A->denom * B->denom;
  // unfortunately, not much to prevent overflow here
  ENSURES(is_frac(F));
  return F;
}

frac* frac_square(frac *A){
  REQUIRES(is_frac(A));
  REQUIRES(-46341 < A->num && A->num < 46341);
  REQUIRES(A->denom < 46341); // less than sqrt(INT_MAX), prevent overflow
  frac *F = malloc(sizeof(frac));

  F->num = A->num * A->num;
  F->denom = A->denom * A->denom;

  ENSURES(is_frac(F));
  return F;
}

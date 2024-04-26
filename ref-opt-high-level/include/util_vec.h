#ifndef UTILS_VEC_H
#define UTILS_VEC_H

#include "params.h"
#include "matrixmod.h"
#include "matrixmod_vec.h"

pmod_mat_s_vec_t solve_vec(pmod_mat_vec_t *A_tilde, pmod_mat_vec_t *B_tilde_inv, pmod_mat_vec_t *G0prime);

void pi_vec(pmod_mat_vec_t *Gout, pmod_mat_vec_t *A, pmod_mat_vec_t *B, pmod_mat_vec_t *G);

pmod_mat_s_vec_t SF_vec(pmod_mat_vec_t *Gprime, pmod_mat_vec_t *G);

#endif


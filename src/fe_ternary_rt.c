/****************************************************************************
 *
 *  fe_ternary_rt.c
 *
 *  Run time initialisation for the surfactant free energy.
 *
 *  Edinburgh Soft Matter and Statistical Physics Group
 *  and Edinburgh Parallel Computing Centre
 *
 *  (c) 2019 The University of Edinburgh
 *
 *  Contributing authors:
 *  Shan Chen (shan.chen@epfl.ch)
 *  Kevin Stratford (kevin@epcc.ed.ac.uk)
 *
 ****************************************************************************/

#include <assert.h>
#include <string.h>

#include "pe.h"
#include "runtime.h"
#include "fe_ternary.h"
#include "fe_ternary_rt.h"
#include "field_s.h"
#include "field_ternary_init.h"

/****************************************************************************
 *
 *  fe_ternary_param_rt
 *
 ****************************************************************************/

__host__ int fe_ternary_param_rt(pe_t * pe, rt_t * rt,
				 fe_ternary_param_t * p) {
  assert(pe);
  assert(rt);
  assert(p);

  /* Parameters */
    
  rt_double_parameter(rt, "ternary_kappa1", &p->kappa1);
  rt_double_parameter(rt, "ternary_kappa2", &p->kappa2);
  rt_double_parameter(rt, "ternary_kappa3", &p->kappa3);
  rt_double_parameter(rt, "ternary_alpha",  &p->alpha);
    
  /* For the surfactant should have... */

  if (p->kappa1 < 0.0) pe_fatal(pe, "Please use ternary_kappa1 >= 0\n");
  if (p->kappa2 < 0.0) pe_fatal(pe, "Please use ternary_kappa2 >= 0\n");
  if (p->kappa3 < 0.0) pe_fatal(pe, "Please use ternary_kappa3 >= 0\n");
  if (p->alpha <= 0.0) pe_fatal(pe, "Please use ternary_alpha > 0\n");

  /* Optional wetting parameters */
  /* In normal circumstances, there are only two independent parameters,
   * so only h1 and h2 need to be set. However, three are provided */

  rt_double_parameter(rt, "ternary_h1", &p->h1);
  rt_double_parameter(rt, "ternary_h2", &p->h2);
  rt_double_parameter(rt, "ternary_h3", &p->h3);

  return 0;
}

/*****************************************************************************
 *
 *  fe_ternary_init_rt
 *
 *  Initialise fields: phi, psi, rho. These are related.
 *
 *****************************************************************************/

__host__ int fe_ternary_init_rt(pe_t * pe, rt_t * rt, fe_ternary_t * fe,
				field_t * phi) {
  int p;
  char value[BUFSIZ];
    
  assert(pe);
  assert(rt);
  assert(fe);
  assert(phi);

  p = rt_string_parameter(rt, "ternary_initialisation", value, BUFSIZ);

  pe_info(pe, "\n");
  pe_info(pe, "Initialising fields for ternary fluid\n");

  if (p != 0 && strcmp(value, "ternary_X") == 0) {
    field_ternary_init_X(phi);
  }

  if (p != 0 && strcmp(value, "2d_double_emulsion") == 0) {
    field_ternary_init_2d_double_emulsion(phi);
  }

  if (p != 0 && strcmp(value, "2d_tee") == 0) {
    field_ternary_init_2d_tee(phi);
  }

  if (p != 0 && strcmp(value, "ternary_bbb") == 0) {
    field_ternary_init_bbb(phi);
  }

  if (p != 0 && strcmp(value, "ternary_ggg") == 0) {
    field_ternary_init_ggg(phi);
  }
    
  return 0;
}

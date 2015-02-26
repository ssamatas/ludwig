/*****************************************************************************
 *
 *  coords_rt.c
 *
 *  Run time stuff for the coordinate system.
 *
 *  $Id: coords_rt.c,v 1.2 2010-10-15 12:40:02 kevin Exp $
 *
 *  Edinburgh Soft Matter and Statistical Physics Group and
 *  Edinburgh Parallel Computing Centre
 *
 *  Kevin Stratford (kevin@epcc.ed.ac.uk)
 *  (c) The University of Edinburgh (2009)
 *
 *****************************************************************************/

#include <assert.h>

#include "coords_rt.h"

/*****************************************************************************
 *
 *  coords_run_time
 *
 *****************************************************************************/

int coords_run_time(coords_t * cs, rt_t * rt) {

  int n;
  int reorder;
  int vector[3];

  assert(cs);
  assert(rt);
  
  info("\n");
  info("System details\n");
  info("--------------\n");

  n = rt_int_parameter_vector(rt, "size", vector);
  coords_ntotal_set(cs, vector);

  n = rt_int_parameter_vector(rt, "periodicity", vector);
  if (n != 0) coords_periodicity_set(cs, vector);

  /* Look for a user-defined decomposition */

  n = rt_int_parameter_vector(rt, "grid", vector);
  if (n != 0) coords_decomposition_set(cs, vector);

  n = rt_int_parameter(rt, "reorder", &reorder);
  if (n != 0) coords_reorder_set(cs, reorder);

  coords_commit(cs);
  coords_info(cs);

  return 0;
}
#include "stout.ih"

void stout_links_with_force_terms(gauge_field_t buff_out, gauge_field_t Q, complex_field_array_t f0, complex_field_array_t f1, 
                                  complex_field_array_t f2, real_field_array_t u, real_field_array_t v, double const coeff, 
                                  gauge_field_t staples, gauge_field_t buff_in)
{

#define _CALCULATE_WEIGHTED_PLAQUETTE(x, principal)                                                  \
  {                                                                                                  \
    _su3_times_su3d(Q[x][principal], staples.field[x][principal], buff_in.field[x][principal]);      \
    project_antiherm(&Q[x][principal]);                                                              \
    _real_times_su3(Q[x][principal], coeff, m_force->Q[x][principal]);                               \
  }

  for (int x = 0; x < VOLUME; ++x)
  {
    _CALCULATE_WEIGHTED_PLAQUETTE(x, I0_0);
    _CALCULATE_WEIGHTED_PLAQUETTE(x, I0_1);
    _CALCULATE_WEIGHTED_PLAQUETTE(x, I0_2);
    _CALCULATE_WEIGHTED_PLAQUETTE(x, I0_3);
  }
  cayley_hamilton_exponent_with_force_terms(buff_out, Q, u, w, f0, f1, f2);
  
#undef _CALCULATE_WEIGHTED_PLAQUETTE
}

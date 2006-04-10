// $Id: sfcurcor_w.cc,v 3.1 2006-04-10 21:18:23 edwards Exp $

#include "chromabase.h"
#include "meas/schrfun/sfcurcor_w.h"

namespace Chroma
{

  //! Schroedinger functional correlation functions
  /*!
   * Construct 'current correlators' and axial density used for the PCAC determination
   * in the Schroedinger Functional
   *
   * \param quark_propagator    quark propagator ( Read )
   * \param pseudo_prop         pion correlator ( Write )
   * \param axial_prop          axial-current to pion_1 correlators ( Modify )
   * \param phases        object holds list of momenta and Fourier phases ( Read )
   *
   *
   *         \
   * cc(t) =  >  < m(0, 0) c(t + L, x) > 
   *         / 
   *         ----
   *           x
   */

  void SFcurcor(const LatticePropagator& quark_propagator, 
		multi1d<Real>& pseudo_prop, 
		multi1d<Real>& axial_prop, 
		const SftMom& phases)
  {
    START_CODE();

    // Length of lattice in decay direction
    int length = phases.numSubsets();

    /* First compute the pion correlator - the pseudoscalar */
    LatticeReal corr_fn = real(trace(adj(quark_propagator) * quark_propagator));

    /* Do a slice-wise sum. */
    multi2d<DComplex> hsum;
    hsum = phases.sft(corr_fn);

    pseudo_prop.resize(length);
    for(int t = 0; t < length; ++t)
      pseudo_prop[t] = real(hsum[0][t]);

    /* Construct the axial-current to pion correlator */
    int jd = 1 << phases.getDir();
    corr_fn = real(trace(adj(quark_propagator) * (Gamma(jd) * quark_propagator)));

    /* Do a slice-wise sum. */
    hsum = phases.sft(corr_fn);

    axial_prop.resize(length);
    for(int t = 0; t < length; ++t)
      axial_prop[t] = -real(hsum[0][t]);

    END_CODE();
  }

} // end namespace

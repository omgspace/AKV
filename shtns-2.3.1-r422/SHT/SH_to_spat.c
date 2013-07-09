// This file was automatically generated by 'make' from file 'hyb_SH_to_spat.gen.c'.
// To modify it, please consider modifying hyb_SH_to_spat.gen.c
/*
 * Copyright (c) 2010-2013 Centre National de la Recherche Scientifique.
 * written by Nathanael Schaeffer (CNRS, ISTerre, Grenoble, France).
 * 
 * nathanael.schaeffer@ujf-grenoble.fr
 * 
 * This software is governed by the CeCILL license under French law and
 * abiding by the rules of distribution of free software. You can use,
 * modify and/or redistribute the software under the terms of the CeCILL
 * license as circulated by CEA, CNRS and INRIA at the following URL
 * "http://www.cecill.info".
 * 
 * The fact that you are presently reading this means that you have had
 * knowledge of the CeCILL license and that you accept its terms.
 * 
 */


/////////////////////////////////////////////////////
  #ifdef SHT_AXISYM
/// Only the \b axisymmetric (m=0) component is transformed, and the resulting spatial fields have size NLAT.
  #endif

/// Truncation and spatial discretization are defined by \ref shtns_create and \ref shtns_set_grid_*
/// \param[in] shtns = a configuration created by \ref shtns_create with a grid set by shtns_set_grid_*
/// \param[in] Qlm = spherical harmonics coefficients :
/// complex double arrays of size shtns->nlm [unmodified].
/// \param[out] Vr = spatial scalar field : double array.
  #ifndef SHT_AXISYM
  #else
  #endif
  #ifdef SHT_VAR_LTR
/// \param[in] llim = specify maximum degree of spherical harmonic. llim must be at most LMAX, and all spherical harmonic degree higher than llim are ignored. 
  #else
/// \param[in] llim MUST be shtns->lmax.
  #endif

// MTR_DCT : <0 => SHT_NO_DCT must be defined !!! mem only transform
//            0 => dct for m=0 only
//            m => dct up to m, (!!! MTR_DCT <= MTR !!!)

	static void GEN3(SH_to_spat_,ID_NME,SUFFIX)(shtns_cfg shtns, complex double *Qlm, double *Vr, long int llim) {
  #ifndef SHT_GRAD
  #else
  #endif

	v2d *BrF;
  #ifndef SHT_AXISYM
// with m>0, 3 components not possible with DCT acceleration
	complex double re,ro;
	#define BR0(i) ((double *)BrF)[2*(i)]
	unsigned im, imlim;
	int imlim_dct;
  #else
	double re,ro;
	#define BR0(i) ((double *)BrF)[i]
  #endif
	long int k,m,l;
  #ifdef _GCC_VEC_
	s2d Ql0[(llim+4)>>1];		// we need some zero-padding.
  #endif

  #ifndef SHT_AXISYM
	BrF = (v2d *) Vr;
	if (shtns->ncplx_fft > 0) {		// alloc memory for the FFT
		BrF = VMALLOC( shtns->ncplx_fft * sizeof(complex double) );
	}
	imlim = MTR;
	#ifdef SHT_VAR_LTR
		if (imlim*MRES > (unsigned) llim) imlim = ((unsigned) llim)/MRES;		// 32bit mul and div should be faster
	#endif
	#ifndef SHT_NO_DCT
		imlim_dct = MTR_DCT;
		#ifdef SHT_VAR_LTR
			if (imlim_dct > imlim) imlim_dct = imlim;
		#endif
	#endif
  #else
	#ifdef SHT_GRAD
	#endif
	BrF = (v2d*) Vr;
  #endif

	//	im=0;
  #ifdef _GCC_VEC_
	{	// store the m=0 coefficients in an efficient & vectorizable way.
		double* Ql = (double*) &Ql0;
		Ql[0] = (double) Qlm[0];		// l=0
		l=1;
		do {		// for m=0, compress the complex Q,S,T to double
			Ql[l] = (double) Qlm[l];
		} while(++l<=llim);		// l=llim+1
		Ql[l] = 0.0;	Ql[l+1] = 0.0;		Ql[l+2] = 0.0;
	}
  #endif
  #ifndef SHT_NO_DCT
	{	// m=0 dct
		s2d* yl = (s2d*) shtns->ykm_dct[0];
	#ifndef _GCC_VEC_
		double* Ql = (double*) Qlm;
		re = 0.0;	ro = 0.0;
		re = yl[0]  * Ql[0];
		ro = yl[1]  * Ql[2];
		yl+=2;
		k=0;	l = 2;
		do {
			while(l<llim) {
				re += yl[0]  * Ql[2*l];
				ro += yl[1]  * Ql[2*l+2];
				l+=2;
				yl+=2;
			}
			if (l==llim) {
				re += yl[0]  * Ql[2*l];
				yl+=2;
			}
			BR0(k) = re;	BR0(k+1) = ro;
			re = 0.0;	ro = 0.0;
		#ifdef SHT_VAR_LTR
			yl  += ((LMAX>>1) - (llim>>1))*2;
		#endif
			k+=2;
			l = k;
		} while (k<=llim+1);
	#else
		s2d r = yl[0] * Ql0[0];		// first Q [l=0 and 1]
		l=0;	k=0;
		do {
			l >>= 1;	// l = l/2;
			s2d r1 = vdup(0.0);
			do {
				r += yl[l+1] * Ql0[l+1];	// { re, ro }
				r1 += yl[l+2] * Ql0[l+2];	// { re, ro }
				l+=2;
			} while(2*l < llim-1);		// 2*(l+1) <= llim
			r += r1;
			yl  += (LMAX -k)>>1;
		#ifndef SHT_AXISYM
				BR0(k) = vlo_to_dbl(r);		BR0(k+1) = vhi_to_dbl(r);
		#else
				*((s2d*)(((double*)BrF)+k)) = r;
		#endif
			l = k+1;
			k+=2;
			r = vdup(0.0);
		} while (l<llim);	//	(k<=llim);
	#endif
		while (k<NLAT) {	// dct padding (NLAT is even)
			BR0(k) = 0.0;	BR0(k+1) = 0.0;
		#ifndef SHT_AXISYM
		#else
		#endif
			k+=2;
		}
    #ifdef SHT_AXISYM
		fftw_execute_r2r(shtns->idct,Vr, Vr);		// iDCT m=0
    #endif
	}
  #else		// ifndef SHT_NO_DCT
	{	// m=0 no dct
	#ifndef _GCC_VEC_
		double* Ql = (double*) Qlm;
	#endif
		k=0;
		s2d* yl  = (s2d*) shtns->ylm[0];
		do {	// ops : NLAT_2 * [ (lmax-m+1) + 4]	: almost twice as fast.
	#ifndef _GCC_VEC_
			l=1;
			re = yl[0] * Ql[0];		// re += ylm[im][k*(LMAX-m+1) + (l-m)] * Qlm[LiM(l,im)];
			++yl;
			ro = 0.0;
			while (l<llim) {	// compute even and odd parts
				ro += yl[0] * Ql[2*l];		// re += ylm[im][k*(LMAX-m+1) + (l-m)] * Qlm[LiM(l,im)];
				re += yl[1] * Ql[2*l+2];	// ro += ylm[im][k*(LMAX-m+1) + (l+1-m)] * Qlm[LiM(l+1,im)];
				l+=2;
				yl+=2;
			}
			if (l==llim) {
				ro += yl[0] * Ql[2*l];
			}
			++yl;
		#ifdef SHT_VAR_LTR
			yl  += ((LMAX>>1) - (llim>>1))*2;
		#endif
			BR0(k) = re + ro;
			BR0(NLAT-1-k) = re - ro;
	#else
			l=0;
			s2d r = vdup(0.0);
			s2d r1 = vdup(0.0);
			do {
				r += yl[l]   * Ql0[l];		// { re, ro }
				++l;
				r1 += yl[l]   * Ql0[l];	// reduce dependency
				++l;
			} while (2*l <= llim);
			r += r1;
			yl  += (LMAX+2)>>1;
		#if __SSE3__
	/*	alternate code, which may be faster (slightly) on SSE3.	*/
			r = addi(r,r);		// { re-ro , re+ro }
			BR0(k) = vhi_to_dbl(r);
			BR0(NLAT-1-k) = vlo_to_dbl(r);
		#else
			BR0(k) = vhi_to_dbl(r) + vlo_to_dbl(r);
			BR0(NLAT-1-k) = vlo_to_dbl(r) - vhi_to_dbl(r);
		#endif
	#endif
		} while (++k < NLAT_2);
	}
  #endif		// ifndef SHT_NO_DCT

  #ifndef SHT_AXISYM
	im=1;
	BrF += NLAT;
    #ifndef SHT_NO_DCT
	while(im<=imlim_dct) {		// dct for im <= MTR_DCT
		m=im*MRES;
		l = LiM(shtns, 0,im);
		v2d* Ql = (v2d*) &Qlm[l];		// virtual pointer for l=0 and im
		double* yl = shtns->ykm_dct[im];
		k=0;	l=m;
		do {
			v2d re = vdup(0.0);		v2d ro = vdup(0.0);
			while(l<llim) {
				re  += vdup(yl[0]) * Ql[l];
				ro  += vdup(yl[1]) * Ql[l+1];
				l+=2;
				yl+=2;
			}
			if (l==llim) {
				re  += vdup(yl[0])   * Ql[l];
				++yl;
			}
			BrF[k] = re;	BrF[k+1] = ro;
			k+=2;
		#ifdef SHT_VAR_LTR
			yl+= (LMAX-llim);
		#endif
			l = (k < m) ? m : k-(m&1);
		} while (k<=llim);
		if (l==llim) {
			BrF[k] = vdup(yl[0])   * Ql[l];
			BrF[k+1] = vdup(0.0);
			k+=2;
		}
		while (k<NLAT) {		// NLAT even
			BrF[k] = vdup(0.0); 	BrF[k+1] = vdup(0.0);
			k+=2;
		}
		++im;
		BrF += NLAT;
	}
    #endif

	while(im<=imlim) {	// regular for MTR_DCT < im <= MTR
		m = im*MRES;
		l = LiM(shtns, 0,im);
		v2d* Ql = (v2d*) &Qlm[l];	// virtual pointer for l=0 and im
		k=0;	l=shtns->tm[im];
		while (k<l) {	// polar optimization
			BrF[k] = vdup(0.0);
			BrF[NLAT-l + k] = vdup(0.0);	// south pole zeroes <=> BrF[im*NLAT + NLAT-(k+1)] = 0.0;
			++k;
		}
		double* yl  = shtns->ylm[im];
		do {	// ops : NLAT_2 * [ (lmax-m+1)*2 + 4]	: almost twice as fast.
			l=m;
			v2d re = vdup(0.0); 	v2d ro = vdup(0.0);
			while (l<llim) {	// compute even and odd parts
				re  += vdup(yl[0]) * Ql[l];		// re += ylm[im][k*(LMAX-m+1) + (l-m)] * Qlm[LiM(l,im)];
				ro  += vdup(yl[1]) * Ql[l+1];	// ro += ylm[im][k*(LMAX-m+1) + (l+1-m)] * Qlm[LiM(l+1,im)];
				l+=2;
				yl+=2;
			}
			if (l==llim) {
				re  += vdup(yl[0]) * Ql[l];		// re += ylm[im][k*(LMAX-m+1) + (l-m)] * Qlm[LiM(l,im)];
				++yl;
			}
			BrF[k] = re + ro;
			BrF[NLAT-1-k] = re - ro;
		#ifdef SHT_VAR_LTR
			yl  += (LMAX-llim);
		#endif
		} while (++k < NLAT_2);
		++im;
		BrF += NLAT;
	}
	for (k=0; k < NLAT*((NPHI>>1) -imlim); ++k) {	// padding for high m's
			BrF[k] = vdup(0.0);
	}
	BrF -= NLAT*(imlim+1);		// restore original pointer

    // NPHI > 1 as SHT_AXISYM is not defined.
    #ifndef SHT_NO_DCT
			fftw_execute_r2r(shtns->idct,(double *) BrF, (double *) BrF);		// iDCT
			if (MRES & 1) {		// odd m's must be divided by sin(theta)
				double* st_1 = shtns->st_1;
				for (im=1; im<=MTR_DCT; im+=2) {	// odd m's
					k=0;	do {
						((v2d *)BrF)[im*NLAT + k] *= vdup(st_1[k]);		((v2d *)BrF)[im*NLAT + k+1] *= vdup(st_1[k+1]);
						k+=2;
					} while(k<NLAT);
				}
			}
    #endif
	if (shtns->ncplx_fft >= 0) {
		fftw_execute_dft_c2r(shtns->ifft, (complex double *) BrF, Vr);
		if (shtns->ncplx_fft > 0) {		// free memory
			VFREE(BrF);
		}
	}
  #endif

	#undef BR0
  }

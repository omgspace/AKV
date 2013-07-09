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
/// \param[in] Slm = spherical harmonics coefficients of \b Spheroidal scalar :
/// complex double array of size shtns->nlm [unmodified].
/// \param[in] Tlm = spherical harmonics coefficients of \b Toroidal scalar :
/// complex double array of size shtns->nlm [unmodified].
/// \param[out] Vr = spatial scalar field : double array.
  #ifndef SHT_AXISYM
/// \param[out] Vt, Vp = (theta,phi)-components of spatial vector : double arrays.
  #else
/// \param[out] Vt = theta-component of spatial vector : double array.
/// \param[out] Vp = phi-component of spatial vector : double array.  
  #endif
  #ifdef SHT_VAR_LTR
/// \param[in] llim = specify maximum degree of spherical harmonic. llim must be at most LMAX, and all spherical harmonic degree higher than llim are ignored. 
  #else
/// \param[in] llim MUST be shtns->lmax.
  #endif

// MTR_DCT : <0 => SHT_NO_DCT must be defined !!! mem only transform
//            0 => dct for m=0 only
//            m => dct up to m, (!!! MTR_DCT <= MTR !!!)

	static void GEN3(SHqst_to_spat_,ID_NME,SUFFIX)(shtns_cfg shtns, complex double *Qlm, complex double *Slm, complex double *Tlm, double *Vr, double *Vt, double *Vp, long int llim) {
  #ifndef SHT_GRAD
  #else
	static void GEN3(SHsph_to_spat_,ID_NME,SUFFIX)(shtns_cfg shtns, complex double *Slm, double *Vt, double *Vp, long int llim) {
	static void GEN3(SHtor_to_spat_,ID_NME,SUFFIX)(shtns_cfg shtns, complex double *Tlm, double *Vt, double *Vp, long int llim) {
  #endif

	v2d *BrF;
  #ifndef SHT_AXISYM
// with m>0, 3 components not possible with DCT acceleration
	#define SHT_NO_DCT
	v2d *BtF, *BpF;
	struct DtDp *dyl;
	complex double re,ro;
	complex double te,to, pe,po;
	complex double dte,dto, dpe,dpo;
	#define BR0(i) ((double *)BrF)[2*(i)]
	#define BT0(i) ((double *)BtF)[2*(i)]
	#define BP0(i) ((double *)BpF)[2*(i)]
	unsigned im, imlim;
	int imlim_dct;
  #else
	v2d *BtF;
	v2d *BpF;
	double re,ro;
	double te,to;
	double pe,po;
	#define BR0(i) ((double *)BrF)[i]
	#define BT0(i) ((double *)BtF)[i]
	#define BP0(i) ((double *)BpF)[i]
  #endif
	long int k,m,l;
  #ifdef _GCC_VEC_
	s2d Ql0[(llim+3)>>1];		// we need some zero-padding.
	s2d Sl0[(llim+2)>>1];
	s2d Tl0[(llim+2)>>1];
  #endif

  #ifndef SHT_AXISYM
	BrF = (v2d *) Vr;
	BtF = (v2d *) Vt;	BpF = (v2d *) Vp;
	if (shtns->ncplx_fft > 0) {		// alloc memory for the FFT
		BrF = VMALLOC( 3* shtns->ncplx_fft * sizeof(complex double) );
		BtF = BrF + shtns->ncplx_fft;		BpF = BtF + shtns->ncplx_fft;
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
		if (Vp != NULL) { k=0; do { ((s2d*)Vp)[k]=vdup(0.0); } while(++k<NLAT/2); Vp[NLAT-1]=0.0; }
		if (Vt != NULL) { k=0; do { ((s2d*)Vt)[k]=vdup(0.0); } while(++k<NLAT/2); Vt[NLAT-1]=0.0; }
	#endif
	BrF = (v2d*) Vr;
	BtF = (v2d*) Vt;
	BpF = (v2d*) Vp;
  #endif

	//	im=0;
  #ifdef _GCC_VEC_
	{	// store the m=0 coefficients in an efficient & vectorizable way.
		double* Ql = (double*) &Ql0;
		double* Sl = (double*) &Sl0;
		double* Tl = (double*) &Tl0;
		Ql[0] = (double) Qlm[0];		// l=0
		l=1;
		do {		// for m=0, compress the complex Q,S,T to double
			Sl[l-1] = (double) Slm[l];
			Tl[l-1] = (double) Tlm[l];
			Ql[l] = (double) Qlm[l];
		} while(++l<=llim);		// l=llim+1
		Sl[l-1] = 0.0;
		Tl[l-1] = 0.0;
		Sl[l] = 0.0;	Tl[l] = 0.0;	Ql[l] = 0.0;	Ql[l+1] = 0.0;
	}
  #endif
  #ifndef SHT_NO_DCT
	{	// m=0 dct
		s2d* yl = (s2d*) shtns->ykm_dct[0];
		s2d* dyl0 = (s2d*) shtns->dykm_dct[0];		// only theta derivative (d/dphi = 0 for m=0)
	#ifndef _GCC_VEC_
		double* Ql = (double*) Qlm;
		double* Sl = (double*) Slm;
		double* Tl = (double*) Tlm;
		re = 0.0;	ro = 0.0;
		re = yl[0]  * Ql[0];
		ro = yl[1]  * Ql[2];
		yl+=2;
		k=0;	l = 2;
		do {
			te = 0.0;	to = 0.0;
			pe = 0.0;	po = 0.0;
			te =  dyl0[0] * Sl[2*l-2];		// l-1
			pe = -dyl0[0] * Tl[2*l-2];		// l-1
			++dyl0;
			while(l<llim) {
				re += yl[0]  * Ql[2*l];
				ro += yl[1]  * Ql[2*l+2];
				to += dyl0[0] * Sl[2*l];
				po -= dyl0[0] * Tl[2*l];
				te += dyl0[1] * Sl[2*l+2];
				pe -= dyl0[1] * Tl[2*l+2];
				l+=2;
				yl+=2;
				dyl0+=2;
			}
			if (l==llim) {
				re += yl[0]  * Ql[2*l];
				to += dyl0[0] * Sl[2*l];
				po -= dyl0[0] * Tl[2*l];
				yl+=2;
			}
			++dyl0;
			BR0(k) = re;	BR0(k+1) = ro;
			BT0(k) = te;	BT0(k+1) = to;
			BP0(k) = pe;	BP0(k+1) = po;
			re = 0.0;	ro = 0.0;
		#ifdef SHT_VAR_LTR
			yl  += ((LMAX>>1) - (llim>>1))*2;
			dyl0 += (((LMAX+1)>>1) - ((llim+1)>>1))*2;
		#endif
			k+=2;
			l = k;
		} while (k<=llim+1);
	#else
		s2d r = yl[0] * Ql0[0];		// first Q [l=0 and 1]
		l=0;	k=0;
		do {
			l >>= 1;	// l = l/2;
			s2d t = vdup(0.0);
			s2d p = vdup(0.0);
			do {
				r += yl[l+1] * Ql0[l+1];	// { re, ro }
				t += dyl0[l] * Sl0[l];		// { te, to }
				p -= dyl0[l] * Tl0[l];		// { pe, po }
				++l;
			} while(2*l < llim);
			yl  += (LMAX -k)>>1;
			dyl0 += (LMAX+1 -k)>>1;
		#ifndef SHT_AXISYM
				BR0(k) = vlo_to_dbl(r);		BR0(k+1) = vhi_to_dbl(r);
				BT0(k) = vlo_to_dbl(t);		BT0(k+1) = vhi_to_dbl(t);
				BP0(k) = vlo_to_dbl(p);		BP0(k+1) = vhi_to_dbl(p);
		#else
				*((s2d*)(((double*)BrF)+k)) = r;
				*((s2d*)(((double*)BtF)+k)) = t;
				*((s2d*)(((double*)BpF)+k)) = p;
		#endif
			l = k+1;
			k+=2;
			r = vdup(0.0);
		} while (l<=llim);	//	(k<=llim+1);
	#endif
		while (k<NLAT) {	// dct padding (NLAT is even)
			BR0(k) = 0.0;	BR0(k+1) = 0.0;
		#ifndef SHT_AXISYM
			BT0(k) = 0.0;	BT0(k+1) = 0.0;			// required for tor or sph only transform
			BP0(k) = 0.0;	BP0(k+1) = 0.0;
		#else
			BT0(k) = 0.0;	BT0(k+1) = 0.0;
			BP0(k) = 0.0;	BP0(k+1) = 0.0;
		#endif
			k+=2;
		}
    #ifdef SHT_AXISYM
		fftw_execute_r2r(shtns->idct,Vr, Vr);		// iDCT m=0
		fftw_execute_r2r(shtns->idct,Vt, Vt);		// iDCT m=0
		fftw_execute_r2r(shtns->idct,Vp, Vp);		// iDCT m=0
		double* st_1 = shtns->st_1;
		k=0;	do {
		#ifdef _GCC_VEC_
			v2d sin_1 = ((v2d *)st_1)[k];
			((v2d *)Vt)[k] *= sin_1;
		 	((v2d *)Vp)[k] *= sin_1;
		#else
			double sin_1 = st_1[2*k]; 	double sin_2 = st_1[2*k+1];
			Vt[2*k] *= sin_1;		Vt[2*k+1] *= sin_2;
			Vp[2*k] *= sin_1;		Vp[2*k+1] *= sin_2;
		#endif
		} while (++k<NLAT_2);
    #endif
	}
  #else		// ifndef SHT_NO_DCT
	{	// m=0 no dct
	#ifndef _GCC_VEC_
		double* Ql = (double*) Qlm;
		double* Sl = (double*) Slm;
		double* Tl = (double*) Tlm;
	#endif
		k=0;
		s2d* yl  = (s2d*) shtns->ylm[0];
		s2d* dyl0 = (s2d*) shtns->dylm[0];	// only theta derivative (d/dphi = 0 for m=0)
		do {	// ops : NLAT_2 * [ (lmax-m+1) + 4]	: almost twice as fast.
	#ifndef _GCC_VEC_
			l=1;
			re = yl[0] * Ql[0];		// re += ylm[im][k*(LMAX-m+1) + (l-m)] * Qlm[LiM(l,im)];
			++yl;
			ro = 0.0;
			te = 0.0;	to = 0.0;
			pe = 0.0;	po = 0.0;
			while (l<llim) {	// compute even and odd parts
				ro += yl[0] * Ql[2*l];		// re += ylm[im][k*(LMAX-m+1) + (l-m)] * Qlm[LiM(l,im)];
				re += yl[1] * Ql[2*l+2];	// ro += ylm[im][k*(LMAX-m+1) + (l+1-m)] * Qlm[LiM(l+1,im)];
				pe -= dyl0[0] * Tl[2*l];	// m=0 : everything is real.
				te += dyl0[0] * Sl[2*l];
				po -= dyl0[1] * Tl[2*l+2];
				to += dyl0[1] * Sl[2*l+2];
				l+=2;
				yl+=2;
				dyl0+=2;
			}
			if (l==llim) {
				ro += yl[0] * Ql[2*l];
				pe -= dyl0[0] * Tl[2*l];
				te += dyl0[0] * Sl[2*l];
				dyl0+=2;
			}
			++yl;
		#ifdef SHT_VAR_LTR
			yl  += ((LMAX>>1) - (llim>>1))*2;
			dyl0 += (((LMAX+1)>>1) - ((llim+1)>>1))*2;
		#endif
			BR0(k) = re + ro;
		#ifndef SHT_AXISYM
			BT0(k) = 0.0;		BP0(k) = 0.0;		// required for partial tor or sph transform
		#endif
			BT0(k) = te + to;			// Bt = dS/dt
			BP0(k) = pe + po;			// Bp = - dT/dt
			BR0(NLAT-1-k) = re - ro;
		#ifndef SHT_AXISYM
			BT0(NLAT-1-k) = 0.0;		BP0(NLAT-1-k) = 0.0;		// required for partial tor or sph transform
		#endif
			BT0(NLAT-1-k) = te - to;
			BP0(NLAT-1-k) = pe - po;
	#else
			l=0;
			s2d t = vdup(0.0);
			s2d p = vdup(0.0);
			s2d r = vdup(0.0);
			do {
				t += dyl0[l] * Sl0[l];		// { te, to }
				p -= dyl0[l] * Tl0[l];		// { pe, po }
				r += yl[l]   * Ql0[l];		// { re, ro }
				++l;
			} while (2*l <= llim);
			yl  += (LMAX+2)>>1;
			dyl0 += (LMAX+1)>>1;
		#if __SSE3__
	/*	alternate code, which may be faster (slightly) on SSE3.	*/
			r = addi(r,r);		// { re-ro , re+ro }
			t = addi(t,t);		// { te-to , te+to }
			p = addi(p,p);		// { pe-po , pe+po }
			BR0(k) = vhi_to_dbl(r);
		#ifndef SHT_AXISYM
			BT0(k) = 0.0;		BP0(k) = 0.0;		// required for partial tor or sph transform
		#endif
			BT0(k) = vhi_to_dbl(t);	// Bt = dS/dt
			BP0(k) = vhi_to_dbl(p);	// Bp = - dT/dt
			BR0(NLAT-1-k) = vlo_to_dbl(r);
		#ifndef SHT_AXISYM
			BT0(NLAT-1-k) = 0.0;		BP0(NLAT-1-k) = 0.0;		// required for partial tor or sph transform
		#endif
			BT0(NLAT-1-k) = vlo_to_dbl(t);
			BP0(NLAT-1-k) = vlo_to_dbl(p);
		#else
			BR0(k) = vhi_to_dbl(r) + vlo_to_dbl(r);
		#ifndef SHT_AXISYM
			BT0(k) = 0.0;		BP0(k) = 0.0;		// required for partial tor or sph transform
		#endif
			BT0(k) = vhi_to_dbl(t) + vlo_to_dbl(t);	// Bt = dS/dt
			BP0(k) = vhi_to_dbl(p) + vlo_to_dbl(p);	// Bp = - dT/dt
			BR0(NLAT-1-k) = vlo_to_dbl(r) - vhi_to_dbl(r);
		#ifndef SHT_AXISYM
			BT0(NLAT-1-k) = 0.0;		BP0(NLAT-1-k) = 0.0;		// required for partial tor or sph transform
		#endif
			BT0(NLAT-1-k) = vlo_to_dbl(t) - vhi_to_dbl(t);
			BP0(NLAT-1-k) = vlo_to_dbl(p) - vhi_to_dbl(p);
		#endif
	#endif
		} while (++k < NLAT_2);
	}
  #endif		// ifndef SHT_NO_DCT

  #ifndef SHT_AXISYM
	im=1;
	BrF += NLAT;
	BtF += NLAT;	BpF += NLAT;
    #ifndef SHT_NO_DCT
	while(im<=imlim_dct) {		// dct for im <= MTR_DCT
		m=im*MRES;
		l = LiM(shtns, 0,im);
		v2d* Ql = (v2d*) &Qlm[l];		// virtual pointer for l=0 and im
		v2d* Sl = (v2d*) &Slm[l];
		v2d* Tl = (v2d*) &Tlm[l];
		double* yl = shtns->ykm_dct[im];
		dyl = shtns->dykm_dct[im];
		k=0;	l=m;
		do {
			v2d re = vdup(0.0);		v2d ro = vdup(0.0);
			v2d te = vdup(0.0);		v2d to = vdup(0.0);		v2d pe = vdup(0.0);		v2d po = vdup(0.0);
			v2d dte = vdup(0.0);	v2d dto = vdup(0.0);	v2d dpe = vdup(0.0);	v2d dpo = vdup(0.0);
			while(l<llim) {
				re  += vdup(yl[0]) * Ql[l];
				ro  += vdup(yl[1]) * Ql[l+1];
				dte += vdup(dyl[0].p) * Tl[l];
				po  -= vdup(dyl[0].t) * Tl[l];
				dpe += vdup(dyl[0].p) * Sl[l];
				to  += vdup(dyl[0].t) * Sl[l];
				dpo += vdup(dyl[1].p) * Sl[l+1];
				te  += vdup(dyl[1].t) * Sl[l+1];
				dto += vdup(dyl[1].p) * Tl[l+1];
				pe  -= vdup(dyl[1].t) * Tl[l+1];
				l+=2;
				yl+=2;
				dyl+=2;
			}
			if (l==llim) {
				re  += vdup(yl[0])   * Ql[l];
				dte += vdup(dyl[0].p) * Tl[l];
				po  -= vdup(dyl[0].t) * Tl[l];
				dpe += vdup(dyl[0].p) * Sl[l];
				to  += vdup(dyl[0].t) * Sl[l];
				++yl;
				++dyl;
			}
			BrF[k] = re;	BrF[k+1] = ro;
			BtF[k] = addi(te, dte);		BtF[k+1] = addi(to, dto);
			BpF[k] = addi(pe, dpe);		BpF[k+1] = addi(po, dpo);
			l = (k < m) ? m : k+(m&1);
			k+=2;
		#ifdef SHT_VAR_LTR
			yl+= (LMAX-llim);
			dyl+= (LMAX-llim);
		#endif
			l = (k < m) ? m : k-(m&1);
		} while (k<=llim+1-(m&1));
		} while (k<=llim);
		if (l==llim) {
			BrF[k] = vdup(yl[0])   * Ql[l];
			BrF[k+1] = vdup(0.0);
			k+=2;
		}
		while (k<NLAT) {		// NLAT even
			BrF[k] = vdup(0.0); 	BrF[k+1] = vdup(0.0);
			BtF[k] = vdup(0.0);		BpF[k] = vdup(0.0);
			BtF[k+1] = vdup(0.0);	BpF[k+1] = vdup(0.0);
			k+=2;
		}
		++im;
		BrF += NLAT;
		BtF += NLAT;	BpF += NLAT;
	}
    #endif

	while(im<=imlim) {	// regular for MTR_DCT < im <= MTR
		m = im*MRES;
		double m_1 = 1.0/m;
		l = LiM(shtns, 0,im);
		v2d* Ql = (v2d*) &Qlm[l];	// virtual pointer for l=0 and im
		v2d* Sl = (v2d*) &Slm[l];	// virtual pointer for l=0 and im
		v2d* Tl = (v2d*) &Tlm[l];
		k=0;	l=shtns->tm[im];
		while (k<l) {	// polar optimization
			BrF[k] = vdup(0.0);
			BrF[NLAT-l + k] = vdup(0.0);	// south pole zeroes <=> BrF[im*NLAT + NLAT-(k+1)] = 0.0;
			BtF[k] = vdup(0.0);		BpF[k] = vdup(0.0);
			BtF[NLAT-l + k] = vdup(0.0);		BpF[NLAT-l + k] = vdup(0.0);	// south pole zeroes
			++k;
		}
		double* yl  = shtns->ylm[im];
		dyl = shtns->dylm[im];
		do {	// ops : NLAT_2 * [ (lmax-m+1)*2 + 4]	: almost twice as fast.
			l=m;
			v2d re = vdup(0.0); 	v2d ro = vdup(0.0);
			v2d dte = vdup(0.0); 	v2d dto = vdup(0.0); 	v2d pe = vdup(0.0); 	v2d po = vdup(0.0);
			v2d dpe = vdup(0.0); 	v2d dpo = vdup(0.0); 	v2d te = vdup(0.0); 	v2d to = vdup(0.0);
			while (l<llim) {	// compute even and odd parts
				dpo -= vdup(dyl[0].t) * Tl[l];
				dto += vdup(dyl[0].t) * Sl[l];
				te  += vdup(dyl[0].p) * Tl[l];
				pe  += vdup(dyl[0].p) * Sl[l];
				re  += vdup(dyl[0].p) * Ql[l];
				dpe -= vdup(dyl[1].t) * Tl[l+1];
				dte += vdup(dyl[1].t) * Sl[l+1];
				to  += vdup(dyl[1].p) * Tl[l+1];
				po  += vdup(dyl[1].p) * Sl[l+1];
				ro  += vdup(dyl[1].p) * Ql[l+1];
				l+=2;
				yl+=2;
				dyl+=2;
			}
			if (l==llim) {
				dpo -= vdup(dyl[0].t) * Tl[l];
				dto += vdup(dyl[0].t) * Sl[l];
				te  += vdup(dyl[0].p) * Tl[l];
				pe  += vdup(dyl[0].p) * Sl[l];
				re  += vdup(dyl[0].p) * Ql[l];
				++yl;
				++dyl;
			}
			s2d qv = vdup(shtns->st[k]*m_1);
			BtF[k] = addi(dte+dto, te+to);		// Bt = dS/dt       + I.m/sint *T
			BtF[NLAT-1-k] = addi(dte-dto, te-to);
			re *= qv;	ro *= qv;
			BpF[k] = addi(dpe+dpo, pe+po);		// Bp = I.m/sint * S - dT/dt
			BpF[NLAT-1-k] = addi(dpe-dpo, pe-po);
			BrF[k] = re + ro;
			BrF[NLAT-1-k] = re - ro;
		#ifdef SHT_VAR_LTR
			yl  += (LMAX-llim);
			dyl += (LMAX-llim);
		#endif
		} while (++k < NLAT_2);
		++im;
		BrF += NLAT;
		BtF += NLAT;	BpF += NLAT;
	}
	for (k=0; k < NLAT*((NPHI>>1) -imlim); ++k) {	// padding for high m's
			BrF[k] = vdup(0.0);
			BtF[k] = vdup(0.0);	BpF[k] = vdup(0.0);
	}
	BrF -= NLAT*(imlim+1);		// restore original pointer
	BtF -= NLAT*(imlim+1);	BpF -= NLAT*(imlim+1);	// restore original pointer

    // NPHI > 1 as SHT_AXISYM is not defined.
    #ifndef SHT_NO_DCT
			fftw_execute_r2r(shtns->idct,(double *) BrF, (double *) BrF);		// iDCT
			fftw_execute_r2r(shtns->idct,(double *) BtF, (double *) BtF);		// iDCT
			fftw_execute_r2r(shtns->idct,(double *) BpF, (double *) BpF);		// iDCT
			double* st_1 = shtns->st_1;
			k=0;	do {		// m=0
				double sin_1 = st_1[k];		double sin_2 = st_1[k+1];
				((double *)BtF)[2*k] *= sin_1;   	((double *)BpF)[2*k] *= sin_1;
				((double *)BtF)[2*k+2] *= sin_2; 	((double *)BpF)[2*k+2] *= sin_2;
				k+=2;
			} while(k<NLAT);
			if (MRES & 1) {		// odd m's must be divided by sin(theta)
				double* st_1 = shtns->st_1;
				for (im=1; im<=MTR_DCT; im+=2) {	// odd m's
					k=0;	do {
						((v2d *)BrF)[im*NLAT + k] *= vdup(st_1[k]);		((v2d *)BrF)[im*NLAT + k+1] *= vdup(st_1[k+1]);
						k+=2;
					} while(k<NLAT);
				}
			}
			l = (MRES & 1) + 1;		// im-stride (l=1 if MRES even, l=2 if MRES odd)
			for (im=l; im<=MTR_DCT; im+=l) {	//even m's must be divided by sin(theta)
				k=0;	do {
					s2d sin_1 = vdup(st_1[k]);		s2d sin_2 = vdup(st_1[k+1]);
					((v2d *)BtF)[im*NLAT + k] *= sin_1;		((v2d *)BpF)[im*NLAT + k] *= sin_1;
					((v2d *)BtF)[im*NLAT + k+1] *= sin_2;	((v2d *)BpF)[im*NLAT + k+1] *= sin_2;
					k+=2;
				} while(k<NLAT);
			}
    #endif
	if (shtns->ncplx_fft >= 0) {
		fftw_execute_dft_c2r(shtns->ifft, (complex double *) BrF, Vr);
		fftw_execute_dft_c2r(shtns->ifft, (complex double *) BtF, Vt);
		fftw_execute_dft_c2r(shtns->ifft, (complex double *) BpF, Vp);
		if (shtns->ncplx_fft > 0) {		// free memory
			VFREE(BrF);
		}
	}
  #endif

	#undef BR0
	#undef BT0
	#undef BP0
  }

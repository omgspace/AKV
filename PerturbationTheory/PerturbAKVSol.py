#!/usr/bin/env python                                                                                                                                
import scipy
import numpy as np
import SphericalGrid

pi = np.pi

class PerturbAKVSol:
    def __init__(self, h, epsilon, grid = None, order = 3, Lmax=15, KerrNorm=False, mNorm = "Owen", name='PerturbAKV', IO=True):
        #Initialize grid
        if grid==None:
            grid = SphericalGrid.SphericalGrid(Lmax, Lmax)
        MM = LL = grid.extents[0]

        lmax = grid.Lmax

        grid.dA = np.exp(2*epsilon*h)*grid.sintheta
    #    l, m = grid.grid.l, grid.grid.m

        #number of terms in spectral expansion to solve for (SpEC code goes up to Lmax-2)
        numpoints = grid.numTerms

        #Real space quantities
        f = np.zeros(grid.extents)

        Lh = grid.SphereLaplacian(h)
        h2 = h**2
        h3 = h**3

        #Spectral coefficient arrays
        f_s = np.zeros(grid.numTerms)
        Lf_s = np.zeros(grid.numTerms)
        Hf_s = np.zeros(grid.numTerms)

        self.sphere_L_s = -grid.l*(grid.l+1)
        self.sphere_M = self.sphere_L_s**2 + 2.0*self.sphere_L_s

        #Matrices - M for H, B for Laplacian
        H0 = np.zeros((numpoints, numpoints))
        H1 = np.zeros((numpoints, numpoints))
        H2 = np.zeros((numpoints, numpoints))
        H3 = np.zeros((numpoints, numpoints))
        L0 = np.zeros((numpoints, numpoints))
        L1 = np.zeros((numpoints, numpoints))
        L2 = np.zeros((numpoints, numpoints))

        l, m = SphericalGrid.YlmIndex(np.arange(grid.numTerms))

        for i in xrange(0,numpoints):
            #generate the spherical harmonic of index i
            coeffs = np.zeros(grid.numTerms)
            coeffs[i] = 1.0
            f = grid.SpecToPhys(coeffs)

            #act operators
            Df = grid.D(f)
            D20 = grid.SphereLaplacian(f)
            D21 = -2*h*D20
            D22 = 2*h2*D20
            D23 = -4.0/3.0 * h3 * D20
            D40 = grid.SphereLaplacian(D20)
            D41 = -2*(h * D40 + grid.SphereLaplacian(h*D20))
            D42 = 2*h2 * D40 + 2*grid.SphereLaplacian(h2 * D20) + 4*h*grid.SphereLaplacian(h* D20)
            D43 = -4.0/3.0 * (h3 * D40 + grid.SphereLaplacian(h3 * D20)) - 4*(h2 *grid.SphereLaplacian(h*D20) + h*grid.SphereLaplacian(h2 * D20))
            RD20 = 2*D20
            RD21 = -(8*h + 2*Lh)*D20
            RD22 = (16*h2 + 8*h*Lh)*D20
            RD23 = (-64.0/3.0*h3 - 16.0*h2*Lh)*D20
            
            C1 = grid.D(2*h + Lh)
            C2 = grid.D(2*h2 + 2*h*Lh)
            C3 = grid.D(-2*h2*Lh - 4.0/3.0*h3)

            DRDF1 = -2*(C1[0]*Df[0] + C1[1]*Df[1]/grid.sintheta**2)
            DRDF2 = 2*(C2[0]*Df[0] + C2[1]*Df[1]/grid.sintheta**2) - 2*h*DRDF1
            DRDF3 = 2*(C3[0]*Df[0] + C3[1]*Df[1]/grid.sintheta**2) - 4*h*(C2[0]*Df[0] + C2[1]*Df[1]/grid.sintheta**2) - 4*h2*(C1[0]*Df[0] + C1[1]*Df[1]/grid.sintheta**2)

            H0_s = grid.PhysToSpec(D40 + RD20)
            H1_s = grid.PhysToSpec(D41 + RD21 + DRDF1)
            H2_s = grid.PhysToSpec(D42 + RD22 + DRDF2)
            H3_s = grid.PhysToSpec(D43 + RD23 + DRDF3)
            
            L0_s = grid.PhysToSpec(D20)
            L1_s = grid.PhysToSpec(D21)
            L2_s = grid.PhysToSpec(D22)

            #Populate the matrices
            H0[:,i] = H0_s
            H1[:,i] = H1_s
            H2[:,i] = H2_s
            H3[:,i] = H3_s
            L0[:,i] = L0_s
            L1[:,i] = L1_s
            L2[:,i] = L2_s

        #apply perturbation formulas
        v0eig = scipy.linalg.eig(L1[1:4,1:4])[1].T

        self.vecs = []
        self.eigenvals = []

        for i in xrange(3):
            v0 = np.zeros(grid.numTerms)
            v1 = np.zeros(grid.numTerms)
            v2 = np.zeros(grid.numTerms)
            v3 = np.zeros(grid.numTerms)
            v0[1:4] = v0eig[i]

            #First order:
            v1[4:] = (-np.dot(H1[4:,:], v0)/self.sphere_M[4:])

            #Second order
            eig2 = -0.5*np.dot(v0, (np.dot(H1, v1) + np.dot(H2, v0)))
            v2[4:] = -(np.dot(H2[4:,:],v0) + np.dot(H1[4:,:],v1))/self.sphere_M[4:]
            v2 -= 0.5*np.dot(v1, v1)*v0
            
            #Third order:
            eig3 = -0.5*np.dot(v0, (np.dot(H3, v0) + np.dot(H2, v1) + np.dot(H1, v2) - eig2*np.dot(L1, v0)))
            v3[4:] = -(np.dot(H3[4:,:],v0) + np.dot(H2[4:,:],v1) + np.dot(H1[4:,:],v2) - eig2*np.dot(L1[4:,:],v0))/self.sphere_M[4:]
            v3 -= np.dot(v2, v1)*v0

            if order == 1:
                self.vecs.append(v0 + epsilon*v1)
            elif order == 2:
                self.vecs.append(v0 + epsilon*v1 + epsilon**2*v2)
            else:
                self.vecs.append(v0 + epsilon*v1 + epsilon**2*v2 + epsilon**3*v3)
            self.eigenvals.append(epsilon**2 * eig2)# + epsilon**3 * eig3)
            self.vecs[i][np.abs(self.vecs[i])<1e-12] = 0.0

        
            
        self.potentials = [grid.SpecToPhys(vec) for vec in self.vecs]

        Area = grid.Integrate(np.ones(grid.extents))

        for i in xrange(3):    
            if KerrNorm == True:
                potential_avg = grid.Integrate(self.potentials[i])
                normint = grid.Integrate((self.potentials[i]-potential_avg)**2)
                norm = np.sqrt(Area**3/(48.0*pi**2*normint))
            else:
                min, max = grid.Minimize(self.potentials[i]), -grid.Minimize(-self.potentials[i])
                norm = Area/(2*pi*(max-min))

            self.vecs[i] = self.vecs[i]*np.sign(np.argmax(np.abs(self.vecs[i])))/np.linalg.norm(self.vecs[i])

            self.potentials[i] = self.potentials[i] * norm

        self.AKVs = [grid.Hodge(grid.D(pot)) for pot in self.potentials]

        if IO==True:
            np.savetxt(name+"_Eigenvalues.dat", self.eigenvals)
            for i in xrange(3):
                np.savetxt(name+"_pot"+str(i+1)+".dat", np.column_stack((grid.theta.flatten(),grid.phi.flatten(),self.potentials[i].flatten())))
                np.savetxt(name+"_Ylm"+str(i+1)+".dat", np.column_stack((l,m,self.vecs[i])),fmt="%d\t%d\t%g")
                np.savetxt(name+"_vec"+str(i+1)+".dat", np.column_stack((grid.theta.flatten(), grid.phi.flatten(), self.AKVs[i][0].flatten(), self.AKVs[i][1].flatten())))

        
    def GetAKV(self):
        return self.AKVs
        
    def GetPotentials(self):
        return self.potentials

    def GetYlm(self):
        index = np.array(self.eigenvals).argsort()
        return np.array(self.vecs)[index]

    def GetEigs(self):
        return self.eigenvals

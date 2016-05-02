#pragma once
#include "FEElasticSolidDomain.h"
#include <FECore/tens3d.h>
#include <FECore/tens6d.h>
#include <FECore/FESurface.h>

//-----------------------------------------------------------------------------
//! This class implements a domain used in an elastic remodeling problem.
//! It differs from the FEElasticSolidDomain in that it adds a stiffness matrix
//! due to the deformation dependent density.
class FEElasticMultiscaleDomain2O : public FEElasticSolidDomain
{
	// Helper class for evaluating the discrete-Galerkin contribution
	// It stores the data needed for evaluating the integrals over the
	// internal surface.
	class FEInternalSurface2O 
	{
	public:
		struct Data
		{
			FEMaterialPoint*	m_pt[2];	//!< material point data for evaluating stresses
			vec3d				ksi[2];		//!< local element coordinates
			tens3drs			Qavg;		//!< average stress across interface
			tens6d 				J0avg;		//!< average initial higher order stiffess across interface
			mat3d				DgradU;		//!< displacement gradient jump across interface
		};

	public:
		FEInternalSurface2O ();

		// initialize the data structure
		bool Initialize(FEElasticMultiscaleDomain2O* dom);

		int Elements() const { return m_ps->Elements(); }

		FESurfaceElement& Element(int i) { return m_ps->Element(i); }

		Data& GetData(int i) { return m_data[i]; }

		FESurface* GetSurface() { return m_ps; }

	private:
		FESurface*		m_ps;
		vector<Data>	m_data;
	};

public:
	//! constructor
	FEElasticMultiscaleDomain2O(FEModel* pfem);
	
	//! initialize class
	bool Initialize(FEModel& fem);

	//! initialize elements
	void InitElements();

	void ElementInternalForce(FESolidElement& el, vector<double>& fe);
	void UpdateElementStress(int iel, double dt);

	//! internal stress forces
	void InternalForces(FEGlobalVector& R);

	//! overridden from FEElasticSolidDomain
	void Update(const FETimePoint& tp);

protected:
	// Discrete-Galerkin contribution to residual
	void InternalForcesDG1(FEGlobalVector& R);
	void InternalForcesDG2(FEGlobalVector& R);

	void ElementInternalForce_PF(FESolidElement& el, vector<double>& fe);
	void ElementInternalForce_QG(FESolidElement& el, vector<double>& fe);

private:
	void UpdateInternalSurfaceStresses();
	void UpdateKinematics();

public:
	// --- S T I F F N E S S ---
	//! calculates the solid element stiffness matrix
	void ElementStiffness(const FETimePoint& tp, int iel, matrix& ke);

	void defhess(FESolidElement &el, int n, tens3drs &G);
	void defhess(FESolidElement &el, double r, double s, double t, tens3drs &G);

protected:
	// calculates derivatives of shape functions
	void shape_gradient(const FESolidElement& el, int n, vec3d* G);
	void shape_gradient(const FESolidElement& el, double r, double s, double t, vec3d* G);

	// Calculates second derivative of shape function N[node]
	void shape_gradient2(const FESolidElement& el, vec3d* X, int n, mat3d* H);
	void shape_gradient2(const FESolidElement& el, vec3d* X, double r, double s, double t, int node, mat3d& H);

private:
	FEInternalSurface2O	m_surf;
};

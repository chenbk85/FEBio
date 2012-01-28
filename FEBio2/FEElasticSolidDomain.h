#pragma once
#include "FECore/FESolidDomain.h"
#include "FECore/FEModel.h"
#include "FEBioLib/FEElasticDomain.h"

//-----------------------------------------------------------------------------
//! domain described by Lagrange-type 3D volumetric elements
//!
class FEElasticSolidDomain : public FESolidDomain, public FEElasticDomain
{
public:
	//! constructor
	FEElasticSolidDomain(FEMesh* pm, FEMaterial* pmat) : FESolidDomain(FE_SOLID_DOMAIN, pm, pmat) {}

	//! TODO: do I really use this?
	FEElasticSolidDomain& operator = (FEElasticSolidDomain& d) { m_Elem = d.m_Elem; m_pMesh = d.m_pMesh; return (*this); }

	//! create a clone of this class
	FEDomain* Clone();

	//! serialize data to archive
	void Serialize(DumpFile& ar);

	//! initialize class
	bool Initialize(FEModel& fem);

	//! Init elements
	void InitElements();

	//! reset element data
	void Reset();

	//! Unpack solid element data
	void UnpackLM(FEElement& el, vector<int>& lm);

public: // overrides from FEElasticDomain

	// update stresses
	void UpdateStresses(FEModel& fem);

	//! calculates the global stiffness matrix for this domain
	void StiffnessMatrix(FENLSolver* psolver);

	//! intertial forces for dynamic problems
	void InertialForces(FENLSolver* psolver, vector<double>& R, vector<double>& F);

	//! calculates the residual
	void Residual(FENLSolver* psolver, vector<double>& R);

	//! internal stress forces
	void InternalForces(FENLSolver* psolver, vector<double>& R);

	//! body forces
	void BodyForce(FENLSolver* psolver, FEBodyForce& BF, vector<double>& R);

public:
	// --- S T I F F N E S S ---

	//! calculates the solid element stiffness matrix
	virtual void ElementStiffness(FEModel& fem, int iel, matrix& ke);

	//! geometrical stiffness (i.e. initial stress)
	void ElementGeometricalStiffness(FESolidElement& el, matrix& ke);

	//! material stiffness component
	void ElementMaterialStiffness(FEModel& fem, FESolidElement& el, matrix& ke);

	//! calculates the solid element inertial stiffness matrix
	void ElementInertialStiffness(FEModel& fem, FESolidElement& el, matrix& ke);

	//! calculates the stiffness matrix due to body forces 
	void ElementBodyForceStiffness(FEModel& fem, FESolidElement& el, matrix& ke);

	// --- R E S I D U A L ---

	//! Calculates the internal stress vector for solid elements
	void ElementInternalForce(FESolidElement& el, vector<double>& fe);

	//! Calculatess external body forces for solid elements
	void ElementBodyForce(FEModel& fem, FESolidElement& elem, vector<double>& fe);

	//! Calculatess external body forces for solid elements
	void ElementBodyForce(FEBodyForce& BF, FESolidElement& elem, vector<double>& fe);
	// ---
};

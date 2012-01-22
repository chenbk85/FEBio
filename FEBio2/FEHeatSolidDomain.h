#pragma once
#include "FECore/FESolidDomain.h"
#include "FECore/FENLSolver.h"

//-----------------------------------------------------------------------------
//! domain class for 3D heat elements
class FEHeatSolidDomain : public FESolidDomain
{
public:
	//! constructor
	FEHeatSolidDomain(FEMesh* pm, FEMaterial* pmat) : FESolidDomain(FE_HEAT_SOLID_DOMAIN, pm, pmat) {}

	//! Create a clone of this domain
	FEDomain* Clone();

	//! Unpack solid element data
	void UnpackLM(FEElement& el, vector<int>& lm);

	//! Calculate the conduction stiffness 
	void ConductionMatrix(FENLSolver* psolver);

	//! Calculate capacitance stiffness matrix
	void CapacitanceMatrix(FENLSolver* psolver, double dt);

protected:
	//! calculate the conductive element stiffness matrix
	void ConductionStiffness(FESolidElement& el, matrix& ke);

	//! calculate the capacitance element stiffness matrix
	void CapacitanceStiffness(FESolidElement& el, matrix& ke, double dt);
};

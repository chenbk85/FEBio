#pragma once
#include "FEBiphasicSolute.h"

//-----------------------------------------------------------------------------
//! Base class for solute materials.

class FESolute : public FEMaterial
{
public:
	FESolute();

public:
	void Init();
	
	//! solute density
	double Density() { return m_rhoT; }
	
	//! solute molecular weight
	double MolarMass() { return m_M; }
	
	//! solute valence
	double ChargeNumber() { return m_z; }
	
	//! Serialization
	void Serialize(DumpFile& ar);
	
public:
	double					m_rhoT;		//!< true solute density
	double					m_M;		//!< solute molecular weight
	int						m_z;		//!< charge number of solute
	FESoluteDiffusivity*	m_pDiff;	//!< pointer to diffusivity material
	FESoluteSolubility*		m_pSolub;	//!< pointer to solubility material
	
	// declare as registered
	DECLARE_REGISTERED(FESolute);
	
	DECLARE_PARAMETER_LIST();
};

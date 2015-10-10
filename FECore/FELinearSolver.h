#pragma once
#include "FECore/FESolver.h"

//-----------------------------------------------------------------------------
// forward declarations
class FEGlobalVector;
class FEGlobalMatrix;
class LinearSolver;

//-----------------------------------------------------------------------------
//! Abstract Base class for finite element solution algorithms that only require the solution
//! of a linear system of equations.
class FELinearSolver : public FESolver
{
public:
	//! constructor
	FELinearSolver(FEModel* pfem);

	//! solve the step
	bool SolveStep(double time);

	//! Initialize and allocate data
	bool Init();

	//! Initialize equation numbers
	bool InitEquations();

	//! Clean up data
	void Clean();

	//! assemble global stiffness matrix
	void AssembleStiffness(vector<int>& en, vector<int>& elm, matrix& ke);

public:
	//! Set the degrees of freedom
	void SetDOF(vector<int>& dof);

public: // these functions need to be implemented by the derived class

	//! Evaluate the right-hand side vector
	virtual void RHSVector(FEGlobalVector& R) = 0;

	//! Evaluate the stiffness matrix
	virtual bool StiffnessMatrix() = 0;

	//! Update the model state
	virtual void Update(vector<double>& u) = 0;

protected: // some helper functions

	//! Reform the stiffness matrix
	bool ReformStiffness();

	//! Create and evaluate the stiffness matrix
	bool CreateStiffness();

private:
	LinearSolver*		m_pls;		//!< The linear equation solver
	FEGlobalMatrix*		m_pK;		//!< The global stiffness matrix
	int					m_neq;		//!< The number of equations (TODO: Get this from linear solver)
	
	vector<double>		m_R;	//!< RHS vector
	vector<double>		m_u;	//!< vector containing prescribed values

	vector<int>		m_dof;	//!< list of active degrees of freedom

	bool	m_breform;	//!< matrix reformation flag
};

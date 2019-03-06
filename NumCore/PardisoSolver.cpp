//! This implementation of the Pardiso solver is for the version
//! available in the Intel MKL.

#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include "PardisoSolver.h"
#include "MatrixTools.h"
#include <FECore/log.h>

#ifdef PARDISO
#undef PARDISO
#include <mkl.h>
#include <mkl_pardiso.h>
#define PARDISO
#else
/* Pardiso prototypes for shared object library version */

#ifdef WIN32

#define pardisoinit_ PARDISOINIT
#define pardiso_ PARDISO

#endif

extern "C"
{
	int pardisoinit_(void *, int *, int *, int *, double*, int*);

	int pardiso_(void *, int *, int *, int *, int *, int *,
		double *, int *, int *, int *, int *, int *,
		int *, double *, double *, int *, double*);
}
#endif

#ifdef PARDISO

//-----------------------------------------------------------------------------
// print pardiso error message
void print_err(int nerror)
{
	switch (-nerror)
	{
	case 1: fprintf(stderr, "Inconsistent input\n"); break;
	case 2: fprintf(stderr, "Not enough memory\n"); break;
	case 3: fprintf(stderr, "Reordering problem\n"); break;
	case 4: fprintf(stderr, "Zero pivot, numerical fact. or iterative refinement problem\n"); break;
	case 5: fprintf(stderr, "Unclassified (internal) error\n"); break;
	case 6: fprintf(stderr, "Preordering failed\n"); break;
	case 7: fprintf(stderr, "Diagonal matrix problem\n"); break;
	case 8: fprintf(stderr, "32-bit integer overflow problem\n"); break;
	default:
		fprintf(stderr, " Unknown\n");
	}
}

//////////////////////////////////////////////////////////////
// PardisoSolver
//////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
PardisoSolver::PardisoSolver(FEModel* fem) : LinearSolver(fem), m_pA(0)
{
	m_print_cn = false;
	m_mtype = -2;
	m_iparm3 = false;

	/* If both PARDISO AND PARDISODL are defined, print a warning */
#ifdef PARDISODL
	fprintf(stderr, "WARNING: The MKL version of the Pardiso solver is being used\n\n");
	exit(1);
#endif
}

//-----------------------------------------------------------------------------
PardisoSolver::~PardisoSolver()
{
	Destroy();
#ifdef PARDISO
	MKL_Free_Buffers();
#endif
}

//-----------------------------------------------------------------------------
void PardisoSolver::PrintConditionNumber(bool b)
{
	m_print_cn = b;
}

//-----------------------------------------------------------------------------
void PardisoSolver::UseIterativeFactorization(bool b)
{
	m_iparm3 = b;
}

//-----------------------------------------------------------------------------
SparseMatrix* PardisoSolver::CreateSparseMatrix(Matrix_Type ntype)
{
	// allocate the correct matrix format depending on matrix symmetry type
	switch (ntype)
	{
	case REAL_SYMMETRIC     : m_mtype = -2; m_pA = new CompactSymmMatrix(1); break;
	case REAL_UNSYMMETRIC   : m_mtype = 11; m_pA = new CRSSparseMatrix(1); break;
	case REAL_SYMM_STRUCTURE: m_mtype =  1; m_pA = new CRSSparseMatrix(1); break;
	default:
		assert(false);
		m_pA = nullptr;
	}

	return m_pA;
}

//-----------------------------------------------------------------------------
bool PardisoSolver::SetSparseMatrix(SparseMatrix* pA)
{
	m_pA = dynamic_cast<CompactMatrix*>(pA);
	m_mtype = -2;
	if (dynamic_cast<CRSSparseMatrix*>(pA)) m_mtype = 11;
	return (m_pA != nullptr);
}

//-----------------------------------------------------------------------------
bool PardisoSolver::PreProcess()
{
	m_iparm[0] = 0; /* Use default values for parameters */

	//fprintf(stderr, "In PreProcess\n");

	pardisoinit(m_pt, &m_mtype, m_iparm);

	m_n = m_pA->Rows();
	m_nnz = m_pA->NonZeroes();
	m_nrhs = 1;

	// number of processors: This parameter is no longer used.
	// Use OMP_NUM_THREADS
	// m_iparm[2] = m_numthreads;

	m_maxfct = 1;	/* Maximum number of numerical factorizations */
	m_mnum = 1;	/* Which factorization to use */

	m_msglvl = 0;	/* 0 Suppress printing, 1 Print statistical information */

	return LinearSolver::PreProcess();
}

//-----------------------------------------------------------------------------
bool PardisoSolver::Factor()
{
	// make sure we have work to do
	if (m_pA->Rows() == 0) return true;

// ------------------------------------------------------------------------------
// Reordering and Symbolic Factorization.  This step also allocates all memory
// that is necessary for the factorization.
// ------------------------------------------------------------------------------

	int phase = 11;

	int error = 0;
	pardiso(m_pt, &m_maxfct, &m_mnum, &m_mtype, &phase, &m_n, m_pA->Values(), m_pA->Pointers(), m_pA->Indices(),
		 NULL, &m_nrhs, m_iparm, &m_msglvl, NULL, NULL, &error);

	if (error)
	{
		fprintf(stderr, "\nERROR during symbolic factorization: ");
		print_err(error);
		exit(2);
	}

// ------------------------------------------------------------------------------
// This step does the factorization
// ------------------------------------------------------------------------------

	phase = 22;

	m_iparm[3] = (m_iparm3 ? 61 : 0);
	error = 0;
	pardiso(m_pt, &m_maxfct, &m_mnum, &m_mtype, &phase, &m_n, m_pA->Values(), m_pA->Pointers(), m_pA->Indices(),
		 NULL, &m_nrhs, m_iparm, &m_msglvl, NULL, NULL, &error);

	if (error)
	{
		fprintf(stderr, "\nERROR during factorization: ");
		print_err(error);
		return false;
	}

	// calculate and print the condition number
	if (m_print_cn)
	{
		double c = condition_number();
		feLog("\tcondition number (est.) ................... : %lg\n\n", c);
	}

	return true;
}

//-----------------------------------------------------------------------------
bool PardisoSolver::BackSolve(double* x, double* b)
{
	// make sure we have work to do
	if (m_pA->Rows() == 0) return true;

	int phase = 33;

	m_iparm[7] = 1;	/* Maximum number of iterative refinement steps */

	int error = 0;
	pardiso(m_pt, &m_maxfct, &m_mnum, &m_mtype, &phase, &m_n, m_pA->Values(), m_pA->Pointers(), m_pA->Indices(),
		 NULL, &m_nrhs, m_iparm, &m_msglvl, b, x, &error);

	if (error)
	{
		fprintf(stderr, "\nERROR during solution: ");
		print_err(error);
		exit(3);
	}

	// update stats
	UpdateStats(1);

	return true;
}

//-----------------------------------------------------------------------------
// This algorithm (naively) estimates the condition number. It is based on the observation that
// for a linear system of equations A.x = b, the following holds
// || A^-1 || >= ||x||.||b||
// Thus the condition number can be estimated by
// c = ||A||.||A^-1|| >= ||A|| . ||x|| / ||b||
// This algorithm tries for some random b vectors with norm ||b||=1 to maxize the ||x||.
// The returned value will be an underestimate of the condition number
double PardisoSolver::condition_number()
{
	// This assumes that the factorization is already done!
	int N = m_pA->Rows();

	// get the norm of the matrix
	double normA = m_pA->infNorm();

	// estimate the norm of the inverse of A
	double normAi = 0.0;

	// choose max iterations
	int iters = (N < 50 ? N : 50);

	vector<double> b(N, 0), x(N, 0);
	for (int i = 0; i < iters; ++i)
	{
		// create a random vector
		NumCore::randomVector(b, -1.0, 1.0);
		for (int j = 0; j < N; ++j) b[j] = (b[j] >= 0.0 ? 1.0 : -1.0);

		// calculate solution
		BackSolve(&x[0], &b[0]);

		double normb = NumCore::infNorm(b);
		double normx = NumCore::infNorm(x);
		if (normx > normAi) normAi = normx;

		int pct = (100 * i) / (iters - 1);
		fprintf(stderr, "calculating condition number: %d%%\r", pct);
	}

	double c = normA*normAi;
	return c;
}

//-----------------------------------------------------------------------------
void PardisoSolver::Destroy()
{
	int phase = -1;

	int error = 0;

	if (m_pA->Pointers())
	{
		pardiso(m_pt, &m_maxfct, &m_mnum, &m_mtype, &phase, &m_n, NULL, m_pA->Pointers(), m_pA->Indices(),
			NULL, &m_nrhs, m_iparm, &m_msglvl, NULL, NULL, &error);
	}
}

#endif

#include "stdafx.h"
#include "FEMeshAdaptor.h"
#include "FEModel.h"
#include "FEDomain.h"
#include "FESolidDomain.h"

REGISTER_SUPER_CLASS(FEMeshAdaptor, FEMESHADAPTOR_ID);

FEMeshAdaptor::FEMeshAdaptor(FEModel* fem) : FECoreBase(fem)
{

}

REGISTER_SUPER_CLASS(FEMeshAdaptorCriterion, FEMESHADAPTORCRITERION_ID);

BEGIN_FECORE_CLASS(FEMeshAdaptorCriterion, FECoreBase)
	ADD_PARAMETER(m_sortList, "sort");
	ADD_PARAMETER(m_maxelem, "max_elems");
END_FECORE_CLASS();

FEMeshAdaptorCriterion::FEMeshAdaptorCriterion(FEModel* fem) : FECoreBase(fem)
{
	m_sortList = false;
	m_maxelem = 0;
}

void FEMeshAdaptorCriterion::SetSort(bool b)
{
	m_sortList = b;
}

void FEMeshAdaptorCriterion::SetMaxElements(int m)
{
	m_maxelem = m;
}

// return a list of elements that satisfy the criterion
std::vector<int> FEMeshAdaptorCriterion::GetElementList()
{
	FEModel& fem = *GetFEModel();
	FEMesh& mesh = fem.GetMesh();

	int nselected = 0;
	int nelem = 0;
	vector< pair<int, double> > elem;
	for (int i = 0; i < mesh.Domains(); ++i)
	{
		FEDomain& dom = mesh.Domain(i);
		int NE = dom.Elements();
		for (int j = 0; j < NE; ++j, ++nelem)
		{
			FEElement& el = dom.ElementRef(j);
			if (el.isActive())
			{
				double elemVal = 0;
				bool bselect = Check(el, elemVal);
				if (bselect)
				{
					elem.push_back(pair<int, double>(nelem, elemVal));
					nselected++;
				}
			}
		}
	}

	std::vector<int> selectedElement;
	if (nselected > 0)
	{
		// sort the list
		if (m_sortList) {
			std::sort(elem.begin(), elem.end(), [](pair<int, double>& e1, pair<int, double>& e2) {
				return e1.second > e2.second;
			});
		}

		int nelem = elem.size();
		if ((m_maxelem > 0) && (nelem > m_maxelem)) nelem = m_maxelem;

		selectedElement.resize(nelem);
		for (int i = 0; i < nelem; ++i)
		{
			selectedElement[i] = elem[i].first;
		}
	}

	return selectedElement;
}

bool FEMeshAdaptorCriterion::Check(FEElement& el, double& elemVal)
{
	return false;
}

BEGIN_FECORE_CLASS(FEMaxVolumeCriterion, FEMeshAdaptorCriterion)
	ADD_PARAMETER(m_maxVolume, "max_vol");
END_FECORE_CLASS();

FEMaxVolumeCriterion::FEMaxVolumeCriterion(FEModel* fem) : FEMeshAdaptorCriterion(fem)
{
	m_maxVolume = 0.0;
}

bool FEMaxVolumeCriterion::Check(FEElement& el, double& elemVal)
{
	FESolidDomain* dom = dynamic_cast<FESolidDomain*>(el.GetMeshPartition());
	if (dom == nullptr) return false;

	elemVal = dom->Volume(dynamic_cast<FESolidElement&>(el));

	return (elemVal >= m_maxVolume);
}

BEGIN_FECORE_CLASS(FEMaxVariableCriterion, FEMeshAdaptorCriterion)
	ADD_PARAMETER(m_maxValue, "max_value");
	ADD_PARAMETER(m_dof, "dof");
END_FECORE_CLASS();

FEMaxVariableCriterion::FEMaxVariableCriterion(FEModel* fem) : FEMeshAdaptorCriterion(fem)
{
	m_maxValue = 0.0;
	m_dof = -1;
}

bool FEMaxVariableCriterion::Check(FEElement& el, double& elemVal)
{
	if (m_dof == -1) return false;

	FESolidDomain* dom = dynamic_cast<FESolidDomain*>(el.GetMeshPartition());
	if (dom == nullptr) return false;

	FEMesh& mesh = *dom->GetMesh();
	double maxVal = -1e99;
	for (int i = 0; i < el.Nodes(); ++i)
	{
		double vi = mesh.Node(el.m_node[i]).get(m_dof);
		if (vi > maxVal) maxVal = vi;
	}
	elemVal = maxVal;
	return (elemVal >= m_maxValue);
}

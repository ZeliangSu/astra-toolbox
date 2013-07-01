/*
-----------------------------------------------------------------------
Copyright 2012 iMinds-Vision Lab, University of Antwerp

Contact: astra@ua.ac.be
Website: http://astra.ua.ac.be


This file is part of the
All Scale Tomographic Reconstruction Antwerp Toolbox ("ASTRA Toolbox").

The ASTRA Toolbox is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

The ASTRA Toolbox is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with the ASTRA Toolbox. If not, see <http://www.gnu.org/licenses/>.

-----------------------------------------------------------------------
$Id$
*/

#ifdef ASTRA_CUDA

#include "astra/CudaEMAlgorithm.h"

#include "../cuda/2d/em.h"

using namespace std;

namespace astra {

// type of the algorithm, needed to register with CAlgorithmFactory
std::string CCudaEMAlgorithm::type = "EM_CUDA";

//----------------------------------------------------------------------------------------
// Constructor
CCudaEMAlgorithm::CCudaEMAlgorithm() 
{
	m_bIsInitialized = false;
	CCudaReconstructionAlgorithm2D::_clear();
}

//----------------------------------------------------------------------------------------
// Destructor
CCudaEMAlgorithm::~CCudaEMAlgorithm() 
{
	// The actual work is done by ~CCudaReconstructionAlgorithm2D
}

//---------------------------------------------------------------------------------------
// Initialize - Config
bool CCudaEMAlgorithm::initialize(const Config& _cfg)
{
	ASTRA_ASSERT(_cfg.self);
	ConfigStackCheck<CAlgorithm> CC("CudaEMAlgorithm", this, _cfg);

	m_bIsInitialized = CCudaReconstructionAlgorithm2D::initialize(_cfg);

	if (!m_bIsInitialized)
		return false;

	m_pAlgo = new astraCUDA::EM();
	m_bAlgoInit = false;

	return true;
}

//---------------------------------------------------------------------------------------
// Initialize - C++
bool CCudaEMAlgorithm::initialize(CProjector2D* _pProjector,
                                     CFloat32ProjectionData2D* _pSinogram, 
                                     CFloat32VolumeData2D* _pReconstruction,
                                     int _iGPUindex, int _iDetectorSuperSampling,
                                     int _iPixelSuperSampling)
{
	m_bIsInitialized = CCudaReconstructionAlgorithm2D::initialize(_pProjector, _pSinogram, _pReconstruction, _iGPUindex, _iDetectorSuperSampling, _iPixelSuperSampling);

	if (!m_bIsInitialized)
		return false;

	m_pAlgo = new astraCUDA::EM();
	m_bAlgoInit = false;

	return true;
}


} // namespace astra

#endif // ASTRA_CUDA

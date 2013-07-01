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

#include "astra/AsyncAlgorithm.h"
#include "astra/AstraObjectFactory.h"

#ifndef USE_PTHREAD
#include <boost/bind.hpp>
#endif

namespace astra {

CAsyncAlgorithm::CAsyncAlgorithm()
{
	m_bInitialized = false;
#ifndef USE_PTHREADS
	m_pThread = 0;
#endif
	m_bThreadStarted = false;
}

CAsyncAlgorithm::CAsyncAlgorithm(CAlgorithm* _pAlg)
{
	m_pAlg = _pAlg;
	m_bInitialized = (m_pAlg != 0);
#ifndef USE_PTHREADS
	m_pThread = 0;
#endif
	m_bThreadStarted = false;
	m_bDone = false;
	m_bAutoFree = false;
}

bool CAsyncAlgorithm::initialize(const Config& _cfg)
{
	if (m_bInitialized && m_bThreadStarted) {
#ifndef USE_PTHREADS
		m_pThread->join();
		delete m_pThread;
#else
		pthread_join(m_thread, 0);
#endif
	}
#ifndef USE_PTHREADS
	m_pThread = 0;
#endif
	m_bThreadStarted = false;
	m_pAlg = 0;
	m_bDone = false;

	m_pAlg = CAlgorithmFactory::getSingleton().create(_cfg);
	if (m_pAlg && !m_pAlg->isInitialized()) {
		if (m_bAutoFree)
			delete m_pAlg;
		m_pAlg = 0;
	}
	m_bInitialized = (m_pAlg != 0);
	m_bAutoFree = true;
	return m_bInitialized;
}

bool CAsyncAlgorithm::initialize(CAlgorithm* _pAlg)
{
	if (m_bInitialized && m_bThreadStarted) {
#ifndef USE_PTHREADS
		m_pThread->join();
		delete m_pThread;
#else
		pthread_join(m_thread, 0);
#endif
	}
#ifndef USE_PTHREADS
	m_pThread = 0;
#endif
	m_bThreadStarted = false;
	m_bDone = false;

	m_pAlg = _pAlg;
	m_bInitialized = (m_pAlg != 0);
	m_bAutoFree = false;
	return m_bInitialized;
}

CAsyncAlgorithm::~CAsyncAlgorithm()
{
	if (m_bInitialized && m_bThreadStarted) {
#ifndef USE_PTHREADS
		m_pThread->join();
		delete m_pThread;
#else
		pthread_join(m_thread, 0);
#endif
	}
#ifndef USE_PTHREADS
	m_pThread = 0;
#endif
	m_bThreadStarted = false;

	if (m_bInitialized && m_bAutoFree) {
		delete m_pAlg;
		m_pAlg = 0;
	}
}

#ifdef USE_PTHREADS
void* runAsync_pthreads(void* data)
{
	CAsyncAlgorithm::AsyncThreadInfo *info = (CAsyncAlgorithm::AsyncThreadInfo*)data;
	info->m_pAlg->run(info->m_iIterations);
	*info->m_pDone = true;
	return 0;
}
#endif

void CAsyncAlgorithm::run(int _iNrIterations)
{
	if (!m_bInitialized)
		return;

#ifndef USE_PTHREADS
	m_pThread = new boost::thread(
	         boost::bind(&CAsyncAlgorithm::runWrapped,
	                     this, _iNrIterations));
#else
	m_ThreadInfo.m_iIterations = _iNrIterations;
	m_ThreadInfo.m_pAlg = m_pAlg;
	m_ThreadInfo.m_pDone = &this->m_bDone;
	pthread_create(&m_thread, 0, runAsync_pthreads, &this->m_ThreadInfo);
#endif
}

void CAsyncAlgorithm::runWrapped(int _iNrIterations)
{
	m_pAlg->run(_iNrIterations);
	m_bDone = true;
}

void CAsyncAlgorithm::timedJoin(int _milliseconds)
{
#ifndef USE_PTHREADS
	if (m_pThread) {
		boost::posix_time::milliseconds rel(_milliseconds);
		bool res = m_pThread->timed_join(rel);
		if (res) {
			delete m_pThread;
			m_pThread = 0;
			m_bThreadStarted = false;
		}
	}
#else
	if (m_bThreadStarted) {
		struct timespec abstime;
		clock_gettime(CLOCK_REALTIME, &abstime);
		abstime.tv_sec += _milliseconds / 1000;
		abstime.tv_nsec += (_milliseconds % 1000) * 1000000L;
		int err = pthread_timedjoin_np(m_thread, 0, &abstime);
		if (err == 0) {
			m_bThreadStarted = false;
		}
	}
#endif
}

void CAsyncAlgorithm::signalAbort()
{
	if (m_pAlg)
		m_pAlg->signalAbort();
}

}

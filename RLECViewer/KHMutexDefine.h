//--------------------------------------------------------------------------------------
//
// Mutex class define.
//
//--------------------------------------------------------------------------------------
#ifndef _KHMUTEX_H
#define _KHMUTEX_H
#include <assert.h>

class CKHMutex
{
public:
	CKHMutex() : m_dwHolder(0), m_dwHolderCount(0)
	{
		::InitializeCriticalSection(&m_CritSec);
	}

	~CKHMutex() { ::DeleteCriticalSection(&m_CritSec); }

	void	Lock()	 
	{ 
		//
		// We already have this mutex. Just refcount and return
		if (::GetCurrentThreadId() == m_dwHolder)
		{
			m_dwHolderCount++;
			return;
		}
		::EnterCriticalSection(&m_CritSec);
		assert(m_dwHolder == 0);
		m_dwHolder = ::GetCurrentThreadId();
		m_dwHolderCount++;
		assert(m_dwHolderCount == 1);
	}

	void	Unlock() 
	{ 
		if (::GetCurrentThreadId() != m_dwHolder)
			return;
		
		assert(m_dwHolderCount > 0);
		m_dwHolderCount--;
		if (m_dwHolderCount == 0)
		{
			m_dwHolder = 0;
			::LeaveCriticalSection(&m_CritSec);
		}
	}
private:
	CRITICAL_SECTION	m_CritSec;
	DWORD				m_dwHolder;
	DWORD				m_dwHolderCount;
};

class CKHMutexLocker
{
public:
	
	CKHMutexLocker(CKHMutex * pinMutex) : m_pMutex(pinMutex) 
	{ 
		if (m_pMutex != NULL) m_pMutex->Lock(); 
	}

	~CKHMutexLocker()	{ if (m_pMutex != NULL) m_pMutex->Unlock(); }
	
	void Lock() 		{ if (m_pMutex != NULL) m_pMutex->Lock(); }
	void Unlock() 		{ if (m_pMutex != NULL) m_pMutex->Unlock(); }
	
private:
	CKHMutex *	m_pMutex;
};

#endif // _KHMUTEX_H
//
// MutexClass.cpp: implementation file
//
// Copyright (C) Walter E. Capers.  All rights reserved
//
// This source is free to use as you like.  If you make
// any changes please keep me in the loop.  Email them to
// walt.capers@comcast.net.
//
// PURPOSE:
//
//  To implement mutexes as a C++ object
//
// REVISIONS
// =======================================================
// Date: 10.25.07        
// Name: Walter E. Capers
// Description: File creation
//
// Date:
// Name:
// Description:
//
//
#include "Thread.h"

#include <iostream>
using namespace std;

CMutexClass::CMutexClass(void)
:m_bCreated(TRUE)
{
#if defined(WINDOWS)
   m_mutex = CreateMutex(NULL,FALSE,NULL);
   if( !m_mutex ) m_bCreated = FALSE;
#elif defined(EMSCRIPTEN)
   m_mutex = SDL_CreateMutex();
#else
   pthread_mutexattr_t mattr;

   pthread_mutexattr_init( &mattr );
   pthread_mutex_init(&m_mutex,&mattr);

#endif
   memset(&m_owner,0,sizeof(ThreadId_t));

}

CMutexClass::~CMutexClass(void)
{
#if defined(WINDOWS)
	WaitForSingleObject(m_mutex,INFINITE);
	CloseHandle(m_mutex);
#elif defined(EMSCRIPTEN)
	SDL_DestroyMutex(m_mutex);
#else
	pthread_mutex_lock(&m_mutex);
	pthread_mutex_unlock(&m_mutex); 
	pthread_mutex_destroy(&m_mutex);
#endif
}

/**
 *
 * Lock
 * the same thread can not lock the same mutex
 * more than once
 *
 **/
void
CMutexClass::Lock()
{
	ThreadId_t id = CThread::ThreadId();
	try {
#if !defined(EMSCRIPTEN)
		// Emscripten has no threads so will always be equal here
		if(CThread::ThreadIdsEqual(&m_owner,&id) )
		    throw "\n\tthe same thread can not acquire a mutex twice!\n"; // the mutex is already locked by this thread
#endif

#if defined(WINDOWS)
		WaitForSingleObject(m_mutex,INFINITE);
#elif defined(EMSCRIPTEN)
		SDL_LockMutex(m_mutex);
#else
		pthread_mutex_lock(&m_mutex);
#endif
		m_owner = CThread::ThreadId();
	}
	catch( char *psz )
	{
#ifdef WINDOWS
		MessageBoxA(NULL,&psz[2],"Fatal exception CMutexClass::Lock",MB_ICONHAND);
		exit(-1);
#else
		cerr << "Fatal exception CMutexClass::Lock : " << psz;
#endif


	}

}

/**
 *
 * Unlock
 * releases a mutex.  only the thread that acquires
 * the mutex can release it.
 *
 **/
void 
CMutexClass::Unlock()
{
	ThreadId_t id = CThread::ThreadId();
	try 
	{
	  if( ! CThread::ThreadIdsEqual(&id,&m_owner) )
		throw "\n\tonly the thread that acquires a mutex can release it!"; 

	   memset(&m_owner,0,sizeof(ThreadId_t));
#if defined(WINDOWS)
	   ReleaseMutex(m_mutex);
#elif defined(EMSCRIPTEN)
	   SDL_UnlockMutex(m_mutex);
#else
	   pthread_mutex_unlock(&m_mutex);
#endif
	}
	catch ( char *psz)
	{
#ifdef WINDOWS
		MessageBoxA(NULL,&psz[2],"Fatal exception CMutexClass::Unlock",MB_ICONHAND);
		exit(-1);
#else
		cerr << "Fatal exception CMutexClass::Unlock : " << psz;
#endif

	}
}


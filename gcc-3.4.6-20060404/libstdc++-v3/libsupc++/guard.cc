// Copyright (C) 2002 Free Software Foundation, Inc.
//  
// This file is part of GCC.
//
// GCC is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.

// GCC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with GCC; see the file COPYING.  If not, write to
// the Free Software Foundation, 59 Temple Place - Suite 330,
// Boston, MA 02111-1307, USA. 

// As a special exception, you may use this file as part of a free software
// library without restriction.  Specifically, if other files instantiate
// templates or use macros or inline functions from this file, or you compile
// this file and link it with other files to produce an executable, this
// file does not by itself cause the resulting executable to be covered by
// the GNU General Public License.  This exception does not however
// invalidate any other reasons why the executable file might be covered by
// the GNU General Public License.

// Written by Mark Mitchell, CodeSourcery LLC, <mark@codesourcery.com>

#include <cxxabi.h>
#include <exception>
#include <bits/c++config.h>
#include <bits/gthr.h>

// The IA64/generic ABI uses the first byte of the guard variable.
// The ARM EABI uses the least significant bit.

// Thread-safe static local initialization support.
#ifdef __GTHREADS
namespace
{
  // static_mutex is a single mutex controlling all static initializations.
  // This is a static class--the need for a static initialization function
  // to pass to __gthread_once precludes creating multiple instances, though
  // I suppose you could achieve the same effect with a template.
  class static_mutex
  {
    static __gthread_recursive_mutex_t mutex;

#ifdef __GTHREAD_RECURSIVE_MUTEX_INIT_FUNCTION
    static void init();
#endif

  public:
    static void lock();
    static void unlock();
  };

  __gthread_recursive_mutex_t static_mutex::mutex
#ifdef __GTHREAD_RECURSIVE_MUTEX_INIT
  = __GTHREAD_RECURSIVE_MUTEX_INIT
#endif
  ;

#ifdef __GTHREAD_RECURSIVE_MUTEX_INIT_FUNCTION
  void static_mutex::init()
  {
    __GTHREAD_RECURSIVE_MUTEX_INIT_FUNCTION (&mutex);
  }
#endif

  void static_mutex::lock()
  {
#ifdef __GTHREAD_RECURSIVE_MUTEX_INIT_FUNCTION
    static __gthread_once_t once = __GTHREAD_ONCE_INIT;
    __gthread_once (&once, init);
#endif
    __gthread_recursive_mutex_lock (&mutex);
  }

  void static_mutex::unlock ()
  {
    __gthread_recursive_mutex_unlock (&mutex);
  }
}
#endif

namespace __gnu_cxx
{
  // 6.7[stmt.dcl]/4: If control re-enters the declaration (recursively)
  // while the object is being initialized, the behavior is undefined.

  // Since we already have a library function to handle locking, we might
  // as well check for this situation and throw an exception.
  // We use the second byte of the guard variable to remember that we're
  // in the middle of an initialization.
  class recursive_init: public std::exception
  {
  public:
    recursive_init() throw() { }
    virtual ~recursive_init() throw ();
  };

  recursive_init::~recursive_init() throw() { }
}

namespace __cxxabiv1 
{
  static int
  acquire_1 (__guard *g)
  {
    if (!*(char *)g)
      {
	if (((char *)g)[1]++)
	  {
#ifdef __EXCEPTIONS
	    throw __gnu_cxx::recursive_init();
#else
	    abort ();
#endif
	  }
	return 1;
      }
    return 0;
  }
  
  extern "C"
  int __cxa_guard_acquire (__guard *g) 
  {
#ifdef __GTHREADS
    if (__gthread_active_p ())
      {
	// Simple wrapper for exception safety.
	struct mutex_wrapper
	{
	  bool unlock;
	  mutex_wrapper (): unlock(true)
	  {
	    static_mutex::lock ();
	  }
	  ~mutex_wrapper ()
	  {
	    if (unlock)
	      static_mutex::unlock ();
	  }
	} mw;

	if (acquire_1 (g))
	  {
	    mw.unlock = false;
	    return 1;
	  }

	return 0;
      }
#endif

    return acquire_1 (g);
  }

  extern "C"
  void __cxa_guard_abort (__guard *g)
  {
    ((char *)g)[1]--;
#ifdef __GTHREADS
    if (__gthread_active_p ())
      static_mutex::unlock ();
#endif
  }

  extern "C"
  void __cxa_guard_release (__guard *g)
  {
    ((char *)g)[1]--;
    *(char *)g = 1;
#ifdef __GTHREADS
    if (__gthread_active_p ())
      static_mutex::unlock ();
#endif
  }
}

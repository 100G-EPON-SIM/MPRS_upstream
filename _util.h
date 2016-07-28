/**********************************************************
 * Author: Glen Kramer (kramer@cs.ucdavis.edu)
 *         University of California @ Davis
 *
 * Filename:    _types.h
 *
 * Description: This file contains type definitions and
 *              general macros
 *
 *********************************************************/

#ifndef _UTIL_H_V001_INCLUDED_
#define _UTIL_H_V001_INCLUDED_

#include <stdlib.h>
#include <new.h>

///////////////////////////////////////////////////////////
// METHOD:       insufficient_memory_handle( size_t )
// PURPOSE:      Terminates application if operator new fails
// ARGUMENTS:
// RETURN VALUE:
///////////////////////////////////////////////////////////
int insufficient_memory_handle( size_t )
{
   perror("Memory allocation failed. Terminating application...");
   exit( 100 );
}

inline void InitAllocator(void) { _set_new_handler( insufficient_memory_handle ); }



#endif // _UTIL_H_V001_INCLUDED_

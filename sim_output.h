/**********************************************************
 * Filename:    sim_output.h
 *
 * Description: This file contains routines for output to 
 *              screen and file
 * 
 * Author: Glen Kramer (kramer@cs.ucdavis.edu)
 *         University of California @ Davis
 *
 ********************************************************************/

#ifndef _SIM_OUTPUT_INCLUDED_
#define _SIM_OUTPUT_INCLUDED_


//#include <fstream.h>  // old style for VC++ 6.0
#include <fstream>      // new style for VC++.NET
#include <conio.h>

using namespace std;

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
// Output routines
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
#define REAL_STREAM( n )                                        \
ofstream LOG_##n ;                                              \
inline void OPEN_##n##_STREAM( char* d, size_t sz )             \
{ strcat_s( d, sz, "_" #n ".csv" );                             \
  LOG_##n.open( d );                                            \
  LOG_##n.precision( 12 );                                      \
  LOG_##n << d << endl; }                                       \
inline void CLOSE_##n##_STREAM( void )   { LOG_##n.close(); } 

#define DUMMY_STREAM( n )                                       \
inline void OPEN_##n##_STREAM( char*, size_t )   {}             \
inline void CLOSE_##n##_STREAM( void )   {}


////////////////////////////////////////////////////////////////////////
// Protocol warnings output
////////////////////////////////////////////////////////////////////////
#if defined ( WARNING_OUTPUT_FILE )
    REAL_STREAM( WARN );
    #define WARN_FILE_OUT( msg )    LOG_WARN << "WARNING: " << msg << endl
#else
    DUMMY_STREAM( WARN );
    #define WARN_FILE_OUT( msg )           
#endif
    
#if defined ( WARNING_OUTPUT_SCREEN )
    #define WARN_SCREEN_OUT( msg )  cerr << "WARNING: " << msg << endl 
#else
    #define WARN_SCREEN_OUT( msg )           
#endif

#if defined ( STOP_ON_WARNING )
    #include <signal.h>
    #define STOP_WARN          { clog << "Press any key to continue ..." << endl; if( _getch() == 0x03 ) raise(SIGINT); } 
#else
    #define STOP_WARN           
#endif

#define MSG_WARN( msg )  { WARN_SCREEN_OUT( msg ); WARN_FILE_OUT( msg ); STOP_WARN; }  


////////////////////////////////////////////////////////////////////////
// Configuration output
////////////////////////////////////////////////////////////////////////
#if defined ( CONFIGURATION_OUTPUT_FILE )
    REAL_STREAM( CONF );
    #define CONF_FILE_OUT( msg )    LOG_CONF << msg << endl
#else
    DUMMY_STREAM( CONF );
    #define CONF_FILE_OUT( msg )           
#endif
    
#if defined ( CONFIGURATION_OUTPUT_SCREEN )
    #define CONF_SCREEN_OUT( msg )  clog << msg << endl 
#else
    #define CONF_SCREEN_OUT( msg )           
#endif

#define MSG_CONF( msg )     { CONF_SCREEN_OUT( msg );  CONF_FILE_OUT( msg ); }  

////////////////////////////////////////////////////////////////////////
// Information output
////////////////////////////////////////////////////////////////////////
#if defined ( INFORMATION_OUTPUT_FILE )
    REAL_STREAM( INFO );
    #define INFO_FILE_OUT( msg )    LOG_INFO << "INFO: " << msg << endl
#else
    DUMMY_STREAM( INFO );
    #define INFO_FILE_OUT( msg )           
#endif
    
#if defined ( INFORMATION_OUTPUT_SCREEN )
    #define INFO_SCREEN_OUT( msg )  clog << "INFO: " << msg << endl 
#else
    #define INFO_SCREEN_OUT( msg )           
#endif

#define MSG_INFO( msg )     { INFO_SCREEN_OUT( msg );  INFO_FILE_OUT( msg ); }  


////////////////////////////////////////////////////////////////////////
// Result #1 output
////////////////////////////////////////////////////////////////////////
#if defined ( RESULT_1_OUTPUT_FILE )
    REAL_STREAM( OUT1 );
    #define RSLT1_FILE_OUT( msg )        LOG_OUT1 << msg
#else
    DUMMY_STREAM( OUT1 );
    #define RSLT1_FILE_OUT( msg )           
#endif
    
#if defined ( RESULT_1_OUTPUT_SCREEN )
    #define RSLT1_SCREEN_OUT( msg )      cout << msg 
#else
    #define RSLT1_SCREEN_OUT( msg )           
#endif

#define MSG_OUT1( msg )     { RSLT1_SCREEN_OUT( msg );  RSLT1_FILE_OUT( msg ); } 

////////////////////////////////////////////////////////////////////////
// Result #2 output
////////////////////////////////////////////////////////////////////////
#if defined ( RESULT_2_OUTPUT_FILE )
    REAL_STREAM( OUT2 );
    #define RSLT2_FILE_OUT( msg )        LOG_OUT2 << msg
#else
    DUMMY_STREAM( OUT2 );
    #define RSLT1_FILE_OUT( msg )           
#endif
    
#if defined ( RESULT_2_OUTPUT_SCREEN )
    #define RSLT2_SCREEN_OUT( msg )      cout << msg 
#else
    #define RSLT2_SCREEN_OUT( msg )           
#endif

#define MSG_OUT2( msg )     { RSLT2_SCREEN_OUT( msg );  RSLT2_FILE_OUT( msg ); } 


////////////////////////////////////////////////////////////////////////
// Macro _FILE_ATTRIBUTES( file_type, ver ) outputs file attributes.  
// Example:
//      _FILE_ATTRIBUTES( Configuration, 001 );
//      _FILE_ATTRIBUTES( Simulation, 040 );
////////////////////////////////////////////////////////////////////////
#define _FILE_ATTRIBUTES( file_type, ver )                  \
inline void   file_type##_FileAttributes( void )            \
{                                                           \
    MSG_CONF( "===============================" );          \
    MSG_CONF( #file_type "," #ver );                        \
    MSG_CONF( "File," __FILE__ );                           \
    MSG_CONF( "Last modified," __TIMESTAMP__ );             \
    MSG_CONF( "Last compiled," __DATE__ " " __TIME__ );     \
    MSG_CONF( "===============================" );          \
}

#endif
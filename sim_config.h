/**********************************************************
 * Filename:    sim_config.h
 *
 * Description: Common Configuration Parameters
 *
 * --------------------------------------------------------
 * Date:  
 * --------------------------------------------------------
 * Changes: 
 *********************************************************/
#ifndef _SIMULATION_H_INCLUDED_ 
#define _SIMULATION_H_INCLUDED_ 

#include <time.h>

///////////////////////////////////////////////////////////
//  Output options
///////////////////////////////////////////////////////////
//#define STOP_ON_WARNING

//#define WARNING_OUTPUT_FILE 
#define WARNING_OUTPUT_SCREEN

//#define CONFIGURATION_OUTPUT_FILE 
//#define CONFIGURATION_OUTPUT_SCREEN

#define INFORMATION_OUTPUT_FILE 
#define INFORMATION_OUTPUT_SCREEN

#define RESULT_1_OUTPUT_FILE 
//#define RESULT_1_OUTPUT_SCREEN

#define RESULT_2_OUTPUT_FILE 
//#define RESULT_2_OUTPUT_SCREEN

#define SHOW_64B_PACKETS_ONLY

#define SHOW_HISTOGRAM

//#define CHECK_DOWNSTREAM
#define CHECK_UPSTREAM

//#define SPARSE_TRAFFIC

//#define DEBUG_ENABLE_RS_TX_RX
//#define DEBUG_ENABLE_RS_TX_TX
//#define DEBUG_ENABLE_DATA_PATH_1
//#define DEBUG_ENABLE_MAC_RX

#define FOR_ALL( N, M )     for( int32s M = 0; M < N; M++ )



#include "sim_output.h"
#include "data_path.h"



////////////////////////////////////////////////////////////////
// FUNCTION:     int Simulation( int argc, char* argv[] )
// PURPOSE:      
// ARGUMENTS:    
// RETURN VALUE: 
////////////////////////////////////////////////////////////////
int Simulation( int, char* [] )
{
	//////////////////////////////////////////////////////////////////
    // Seed the random-number generator with the current time so that
    // the numbers will be different every time we run.
	//////////////////////////////////////////////////////////////////
    srand( (unsigned)time( NULL ) );

    ////////////////////////////////////////////////////////////
    // Run simulation
    ////////////////////////////////////////////////////////////
#ifdef CHECK_DOWNSTREAM
    ClearStats();
    DownstreamTiming();
#endif


#ifdef CHECK_UPSTREAM
    ClearStats();
    UpstreamTiming();
#endif

    return 0;
}

#endif _SIMULATION_H_INCLUDED_ 
#include <time.h>
#include <iostream> 
#include "stdafx.h"

#include "_util.h"
#include "sim_config.h"

using namespace std;

/********************************************************************/
/********************************************************************/
int main(int argc, char* argv[])
{
	const size_t BUFFER_SIZE = 32;
	CHAR buffer[BUFFER_SIZE] = { '\0' };

	////////////////////////////////////////////////////////////
	// set 'insufficient memory' handle, so that we don't need 
	// to check if 'new' operator failed or not 
	////////////////////////////////////////////////////////////
	InitAllocator();

	////////////////////////////////////////////////////////////
	// Get timestamp for file name _MMDDYY_HHMMSS_
	////////////////////////////////////////////////////////////
	time_t sim_start_time = time(NULL);
	struct tm parsed_time;
	//struct tm* newtime    = localtime( &sim_start_time );  // deprecated in VC__ 2005
	localtime_s(&parsed_time, &sim_start_time);

	int32s pos = _snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
		"%s_%02i%02i%02i_%02i%02i%02i",
		(argc > 1 ? argv[1] : ""),
		parsed_time.tm_mon + 1,
		parsed_time.tm_mday,
		parsed_time.tm_year - 100,
		parsed_time.tm_hour,
		parsed_time.tm_min,
		parsed_time.tm_sec);

	if (pos < 0 || pos > BUFFER_SIZE - 10)
		pos = BUFFER_SIZE - 10;

	////////////////////////////////////////////////////////////
	// Initialize output streams
	////////////////////////////////////////////////////////////
	buffer[pos] = '\0';  OPEN_WARN_STREAM(buffer, BUFFER_SIZE);
	buffer[pos] = '\0';  OPEN_CONF_STREAM(buffer, BUFFER_SIZE);
	buffer[pos] = '\0';  OPEN_INFO_STREAM(buffer, BUFFER_SIZE);
	buffer[pos] = '\0';  OPEN_OUT1_STREAM(buffer, BUFFER_SIZE);
	buffer[pos] = '\0';  OPEN_OUT2_STREAM(buffer, BUFFER_SIZE);


	////////////////////////////////////////////////////////////
	// Print banner
	// Start simulation
	////////////////////////////////////////////////////////////

	ctime_s(buffer, BUFFER_SIZE, &sim_start_time); // reuse buffer for start time
	MSG_INFO(">>>>> Simulation started on " << buffer);

	////////////////////////////////////////////////////////////
	int ret = Simulation(argc, argv);
	////////////////////////////////////////////////////////////

	MSG_INFO("<<<<< Elapsed time: " << (int32s)(time(NULL) - sim_start_time) << " sec.");

	////////////////////////////////////////////////////////////
	// Close output streams
	////////////////////////////////////////////////////////////
	CLOSE_WARN_STREAM();
	CLOSE_CONF_STREAM();
	CLOSE_INFO_STREAM();
	CLOSE_OUT1_STREAM();
	CLOSE_OUT2_STREAM();

	return ret;
}

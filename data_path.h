#include "stats.h"

#include "FSM_misc.h"
#include "FSM_ID.h"
#include "FSM_DD.h"
#include "FSM_FEC.h"
#include "FSM_II.h"

#include "FSM_NGEPON_MACC.h"
#include "FSM_NGEPON_MPCP.h"
#include "FSM_NGEPON_MAC.h"
#include "FSM_NGEPON_RS.h"
#include "FSM_NGEPON_25GMII.h"

#include <ostream>

const int32s TEST_FRAMES = 10000;
int64s frame_bytes = 0;

using namespace std;

///////////////////////////////////////////////////////////////////
// Callback function to return packet sizes
///////////////////////////////////////////////////////////////////

int16s PacketSize(void) 
{
    double p = (double)rand() / RAND_MAX;

    if (p <= 0.25)	return MIN_PACKET_BYTES;
    return (int16s) (((double)rand() / RAND_MAX) * (MAX_PACKET_BYTES - MIN_PACKET_BYTES) + MIN_PACKET_BYTES);
}

/////////////////////////////////////////////////////////////////////
// objects for collecting statistics
/////////////////////////////////////////////////////////////////////
#define DISTRIB_BINS 1400
Distrib< DISTRIB_BINS > DelayHistogram[ DELAY_ARRAY_SIZE + 1];

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
// Macros
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////

#define ALL_MODULES(header, val)                      \
{                                                       \
    MSG_OUT2(header << ",");                          \
    for (n=0; n <= DELAY_ARRAY_SIZE; n++)              \
        MSG_OUT2("," << DelayHistogram[n].##val);     \
    MSG_OUT2(endl);                                   \
}

//#define HEADER_STRING   "CLIENT,MPCP_TX,MAC_TX,XGMII_TX,IDLE_DEL,66B_ENCODER,SCRAMBLER,DATA_DET,FEC_DECODER,DESCRAMBLER,66B_DECODER,IDLE_INS,XGMII_RX,MAC_RX,MPCP_RX,TOTAL"
#define HEADER_STRING   "CLIENT,MPCP_TX,MAC_TX,RS_TX,25GMII_TX,25GMII_RX,RS_RX,MAC_RX,TOTAL"

/////////////////////////////////////////////////////////////
// void CollectStats(const _frm_t& frame) 
/////////////////////////////////////////////////////////////
void CollectStats(const _frm_t& frame)
{ 
    frame_bytes += frame.GetFrameSize();

#ifdef SHOW_64B_PACKETS_ONLY
    if (frame.GetFrameSize() != MPCP_PACKET_BYTES + PREAMBLE_BYTES)
        return;
#endif

    int16s delay, total_delay = 0;
    MSG_OUT1(frame.GetFrameSize() - PREAMBLE_BYTES << ",,");

    FOR_ALL(DELAY_ARRAY_SIZE, dly_ndx)
    {
        delay = frame.GetDelay(dly_ndx);
        DelayHistogram[ dly_ndx ].Sample(delay);
        MSG_OUT1(delay << ",");

        //////////////////////////////////////////////////////////
        // calculates total delay after messages were timestamped, 
        // i.e., excluding the MAC Client and MPCP delay
        //////////////////////////////////////////////////////////
        if (dly_ndx >= DLY_NGEPON_MAC_TX)
            total_delay += delay;
    }
    
    DelayHistogram[ DELAY_ARRAY_SIZE ].Sample(total_delay);
    MSG_OUT1(total_delay << endl);
}
 
/////////////////////////////////////////////////////////////
// void OutputStats(void)
/////////////////////////////////////////////////////////////
void OutputStats(void)
{ 
    MSG_INFO("Throughput: " << static_cast<double>(frame_bytes)/timestamp_t::GetClock());
    MSG_OUT2("Throughput,"  << static_cast<double>(frame_bytes)/timestamp_t::GetClock() << endl);
    
    int32s n;
    MSG_OUT2("Delay (byte times),," << HEADER_STRING << endl);
    ALL_MODULES("Total frames",  GetCount());
    ALL_MODULES("Min delay",     GetMin()  );
    ALL_MODULES("Max delay",     GetMax()  );
    ALL_MODULES("Max drift",     GetRange());
    MSG_OUT2(endl);

    /////////////////////////////////////////////////////////////
    // output histograms 
    /////////////////////////////////////////////////////////////
#ifdef SHOW_HISTOGRAM
    MSG_OUT2("Delay (byte times),," << HEADER_STRING << endl);
    FOR_ALL(DISTRIB_BINS, bin)
        ALL_MODULES(bin, GetBinNorm(bin));
#endif
}

/////////////////////////////////////////////////////////////
// void ClearStats(void)
/////////////////////////////////////////////////////////////
void ClearStats(void)
{ 
    timestamp_t::ResetClock();
    frame_bytes = 0;
    FOR_ALL(DELAY_ARRAY_SIZE + 1, n)             
        DelayHistogram[n].Clear();
}

/////////////////////////////////////////////////////////////////////
// void DownstreamTiming(void)
/////////////////////////////////////////////////////////////////////
void DownstreamTiming(void)
{

}

/////////////////////////////////////////////////////////////////////
// void UpstreamTiming(void)
/////////////////////////////////////////////////////////////////////
void UpstreamTiming(void)
{
    /////////////////////////////////////////////////////////////////////
    // instances of finite state machines
    /////////////////////////////////////////////////////////////////////
	fsm_ngepon_macc_t< PacketSize >		FSM_MAC_CLIENT(true);		// defined in FSM_misc.h
    fsm_ngepon_mpcp_tx_t				FSM_MPCP_TX;				// defined in FSM_NGEPON_MPCP.h
    fsm_ngepon_mac_tx_t					FSM_MAC_TX;					// defined in FSM_NGEPON_MAC.h
	fsm_ngepon_rs_tx_t					FSM_RS_TX;					// defined in FSM_NGEPON_RS.h
    fsm_ngepon_25gmii_tx_t				FSM_25GMII_TX;				// defined in FSM_XGMII.h
	fsm_ngepon_25gmii_rx_t				FSM_25GMII_RX;				// defined in FSM_XGMII.h
	fsm_ngepon_rs_rx_t					FSM_RS_RX;					// defined in FSM_NGEPON_RS.h
	fsm_ngepon_mac_rx_t					FSM_MAC_RX;					// defined in FSM_NGEPON_MAC.h
	fsm_ngepon_mpcp_rx_t				FSM_MPCP_RX;				// defined in FSM_NGEPON_MPCP.h
    //fsm_onu_idle_deletion_t			FSM_ONU_IDLE_DELETION;  // defined in FSM_ID.h
    //fsm_64b66b_encoder_t			FSM_64B66B_ENCODER;     // defined in FSM_misc.h
    //fsm_scrambler_t					FSM_SCRAMBLER;          // defined in FSM_misc.h
    //fsm_onu_data_detector_t			FSM_ONU_DATA_DETECTOR;  // defined in FSM_DD.h
    //fsm_fec_decoder_t				FSM_FEC_DECODER;        // defined in FSM_FEC.h
    //fsm_descrambler_t				FSM_DESCRAMBLER;        // defined in FSM_misc.h
    //fsm_66b64b_decoder_t			FSM_66B64B_DECODER;     // defined in FSM_misc.h
    //fsm_idle_insertion_t			FSM_IDLE_INSERTION;	    // defined in FSM_II.h
    //fsm_xgmii_rx_t					FSM_XGMII_RX;           // defined in FSM_XGMII.h
    //fsm_mac_rx_t					FSM_MAC_RX;             // defined in FSM_MAC.h
    //fsm_mpcp_rx_t					FSM_MPCP_RX;		    // defined in FSM_MPCP.h

	int32u VectorCount36b = 0;


    MSG_OUT1("Frame size,," << HEADER_STRING << endl);

    /////////////////////////////////////////////////////////////////////
    // data propagation through upstream path
    /////////////////////////////////////////////////////////////////////
    for (int32s frame_count = 0; frame_count < TEST_FRAMES;)
    {
		
		/////////////////////////////////////////////////////////////////
		// This section operates over byte clock. A single vector out of 
		// MAC becomes available every 4 byte clock cycles
		/////////////////////////////////////////////////////////////////
		for (int16s byte_ndx = 0; byte_ndx < COLUMN_BYTES; byte_ndx++)
		{
			// increase local clock refereces (1 byte resolution)
			timestamp_t::IncrementClock();
			FSM_MAC_CLIENT.IncrementMACClientClock();
			FSM_MPCP_TX.IncrementByteClock();
			
			// transfer data from MAC Client into MPCP layer for further transmission 
			if (FSM_MPCP_TX.ChannelReady() && FSM_MAC_CLIENT.FrameAvailable() && FSM_MAC_TX.MacReady())
			{
				FSM_MPCP_TX << (_frm_t)FSM_MAC_CLIENT;
				FSM_MPCP_TX.grantStart = FSM_MAC_CLIENT.GrantStart();
				FSM_RS_TX.CbCtrlRequest(0, 300); // @TODO@ dynamic bandwidth granting could be implemented for testing purposes
			}

			// If frame is available at MPCP, pass it to MAC 
			if (FSM_MPCP_TX.OutputReady())
				FSM_MAC_TX << (_frm_t)FSM_MPCP_TX;

		}

		/////////////////////////////////////////////////////////////////
		// The section below operates over 36-bit columns on rising and 
		// falling edges of the 25GMII clock, moving data from MAC into
		// RS and from RS into 25GMII 
		/////////////////////////////////////////////////////////////////

		// pass data from MAC to RS only if RS needs data 
		if (FSM_RS_TX.IsReadyForMoreData(0))
			FSM_RS_TX << (_36b_t)FSM_MAC_TX;

		// pass data from RS into 25GMII unconditionally
		_36b_t TempVectorDataPath1 = (_36b_t)FSM_RS_TX;
		VectorCount36b++;
		#ifdef DEBUG_ENABLE_DATA_PATH_1
			std::cout << "Data path 1 column type: " << BlockName(TempVectorDataPath1.C_TYPE()) << ", sequence " << TempVectorDataPath1.GetSeqNumber() << ", nbr: " << VectorCount36b << std::endl;
		#endif // DEBUG_ENABLE_DATA_PATH_1
		if (VectorCount36b > 1000)
			VectorCount36b = VectorCount36b;
		FSM_25GMII_TX << TempVectorDataPath1;

		// std::cout << "Point 0001:" << tempVector1.GetSeqNumber() << std::endl;

		/////////////////////////////////////////////////////////////////
		// The section below operates over 72-bit columns 
		// One vector is passed on every falling edge of the clock.
		/////////////////////////////////////////////////////////////////
		if (FSM_25GMII_TX.OutputReady()) 
		{
			FSM_25GMII_RX << (_72b_t)FSM_25GMII_TX;
		}

		/////////////////////////////////////////////////////////////////
		// The section below operates over 36-bit columns on rising and 
		// falling edges of the 25GMII clock
		/////////////////////////////////////////////////////////////////
		FSM_MAC_RX << (_36b_t)FSM_25GMII_RX;

		if (FSM_MAC_RX.OutputReady()) // if a complete MAC frame available...        
		{
			frame_count++;
			if (frame_count%1000 == 0)
				std::cout << "Packet counter: " << frame_count << std::endl;
			FSM_MPCP_RX << (_frm_t)FSM_MAC_RX;
			CollectStats((_frm_t)FSM_MPCP_RX);
		}

    }

    OutputStats();
}

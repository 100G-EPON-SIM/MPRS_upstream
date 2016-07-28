#include "stats.h"

#include "FSM_misc.h"
#include "FSM_MPCP.h"
#include "FSM_MAC.h"
#include "FSM_XGMII.h"
#include "FSM_ID.h"
#include "FSM_DD.h"
#include "FSM_FEC.h"
#include "FSM_II.h"

const int32s TEST_FRAMES = 10000;
int64s frame_bytes = 0;

///////////////////////////////////////////////////////////////////
// Callback function to return packet sizes
///////////////////////////////////////////////////////////////////


int16s PacketSize(void) 
{
    double p = (double)rand() / RAND_MAX;

    if ( p <= 0.25 )	return MIN_PACKET_BYTES;
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

#define ALL_MODULES( header, val )                      \
{                                                       \
    MSG_OUT2( header << "," );                          \
    for( n=0; n <= DELAY_ARRAY_SIZE; n++ )              \
        MSG_OUT2( "," << DelayHistogram[n].##val );     \
    MSG_OUT2( endl );                                   \
}

#define HEADER_STRING   "CLIENT,MPCP_TX,MAC_TX,XGMII_TX,IDLE_DEL,66B_ENCODER,SCRAMBLER,DATA_DET,FEC_DECODER,DESCRAMBLER,66B_DECODER,IDLE_INS,XGMII_RX,MAC_RX,MPCP_RX,TOTAL"

/////////////////////////////////////////////////////////////
// void CollectStats( const _frm_t& frame ) 
/////////////////////////////////////////////////////////////
void CollectStats( const _frm_t& frame )
{ 
    frame_bytes += frame.GetFrameSize();

#ifdef SHOW_64B_PACKETS_ONLY
    if( frame.GetFrameSize() != MPCP_PACKET_BYTES + PREAMBLE_BYTES )
        return;
#endif

    int16s delay, total_delay = 0;
    MSG_OUT1( frame.GetFrameSize() - PREAMBLE_BYTES << ",," );

    FOR_ALL( DELAY_ARRAY_SIZE, dly_ndx )
    {
        delay = frame.GetDelay( dly_ndx );
        DelayHistogram[ dly_ndx ].Sample( delay );
        MSG_OUT1( delay << "," );

        //////////////////////////////////////////////////////////
        // calculates total delay after messages were timestamped, 
        // i.e., excluding the MAC Client and MPCP delay
        //////////////////////////////////////////////////////////
        if( dly_ndx >= DLY_MAC_TX )
            total_delay += delay;
    }
    
    DelayHistogram[ DELAY_ARRAY_SIZE ].Sample( total_delay );
    MSG_OUT1( total_delay << endl );
}
 
/////////////////////////////////////////////////////////////
// void OutputStats( void )
/////////////////////////////////////////////////////////////
void OutputStats( void )
{ 
    MSG_INFO( "Throughput: " << static_cast<double>(frame_bytes)/timestamp_t::GetClock() );
    MSG_OUT2( "Throughput,"  << static_cast<double>(frame_bytes)/timestamp_t::GetClock() << endl );
    
    int32s n;
    MSG_OUT2( "Delay (byte times),," << HEADER_STRING << endl );
    ALL_MODULES( "Total frames",  GetCount() );
    ALL_MODULES( "Min delay",     GetMin()   );
    ALL_MODULES( "Max delay",     GetMax()   );
    ALL_MODULES( "Max drift",     GetRange() );
    MSG_OUT2( endl );

    /////////////////////////////////////////////////////////////
    // output histograms 
    /////////////////////////////////////////////////////////////
#ifdef SHOW_HISTOGRAM
    MSG_OUT2( "Delay (byte times),," << HEADER_STRING << endl );
    FOR_ALL( DISTRIB_BINS, bin )
        ALL_MODULES( bin, GetBinNorm( bin ));
#endif
}

/////////////////////////////////////////////////////////////
// void ClearStats( void )
/////////////////////////////////////////////////////////////
void ClearStats( void )
{ 
    timestamp_t::ResetClock();
    frame_bytes = 0;
    FOR_ALL( DELAY_ARRAY_SIZE + 1, n )             
        DelayHistogram[n].Clear();
}


/////////////////////////////////////////////////////////////////////
// void DownstreamTiming( void )
/////////////////////////////////////////////////////////////////////
void DownstreamTiming( void )
{
    /////////////////////////////////////////////////////////////////////
    // instances of finite state machines
    /////////////////////////////////////////////////////////////////////
	fsm_mac_client_t< PacketSize >	FSM_MAC_CLIENT;		    // defined in FSM_misc.h
    fsm_mpcp_tx_t					FSM_MPCP_TX;			// defined in FSM_MPCP.h
    fsm_mac_tx_t					FSM_MAC_TX;             // defined in FSM_MAC.h
    fsm_xgmii_tx_t					FSM_XGMII_TX;           // defined in FSM_XGMII.h
    fsm_olt_idle_deletion_t			FSM_OLT_IDLE_DELETION;  // defined in FSM_ID.h
    fsm_64b66b_encoder_t			FSM_64B66B_ENCODER;     // defined in FSM_misc.h
    fsm_scrambler_t					FSM_SCRAMBLER;          // defined in FSM_misc.h
    fsm_olt_data_detector_t			FSM_OLT_DATA_DETECTOR;  // defined in FSM_DD.h
    fsm_fec_decoder_t				FSM_FEC_DECODER;        // defined in FSM_FEC.h
    fsm_descrambler_t				FSM_DESCRAMBLER;        // defined in FSM_misc.h
    fsm_66b64b_decoder_t			FSM_66B64B_DECODER;     // defined in FSM_misc.h
    fsm_idle_insertion_t			FSM_IDLE_INSERTION;		// defined in FSM_II.h
    fsm_xgmii_rx_t					FSM_XGMII_RX;           // defined in FSM_XGMII.h
    fsm_mac_rx_t					FSM_MAC_RX;             // defined in FSM_MAC.h
    fsm_mpcp_rx_t					FSM_MPCP_RX;			// defined in FSM_MPCP.h


    MSG_OUT1( "Frame size,," << HEADER_STRING << endl );

    /////////////////////////////////////////////////////////////////////
    // data propagation through downstream path
    /////////////////////////////////////////////////////////////////////
    for( int32s frame_count = 0; frame_count < TEST_FRAMES; )
    {
        for( int16s byte_ndx = 0; byte_ndx < COLUMN_BYTES; byte_ndx++ )
        {
            /////////////////////////////////////////////////////////////////
            // This section operates over byte clock (0.8 ns). 
            // One XGMII transfer is ready after every 4 byte clocks
            /////////////////////////////////////////////////////////////////
            timestamp_t::IncrementClock();
			FSM_MAC_CLIENT.IncrementMACClientClock();
            FSM_MPCP_TX.IncrementByteClock();

			if( FSM_MPCP_TX.ChannelReady() && FSM_MAC_CLIENT.FrameAvailable() )
			    FSM_MPCP_TX << (_frm_t)FSM_MAC_CLIENT;

            // If frame is available at MPCP, pass it to MAC 
            if( FSM_MPCP_TX.OutputReady() )
                FSM_MAC_TX << (_frm_t) FSM_MPCP_TX;
        }

        /////////////////////////////////////////////////////////////////////
        // The section below operates over 36-bit columns on rising and 
        // falling edges of the XGMII clock
        /////////////////////////////////////////////////////////////////////
        FSM_XGMII_TX << (_36b_t) FSM_MAC_TX;

        if( FSM_XGMII_TX.OutputReady() ) // if a complete 72-bit vector available
        {
            /////////////////////////////////////////////////////////////////////
            // This section operates over 72-bit vectors. 
            // One vector is passed on every falling edge of the clock.
            /////////////////////////////////////////////////////////////////////
            FSM_OLT_IDLE_DELETION << (_72b_t) FSM_XGMII_TX;

            if( FSM_OLT_IDLE_DELETION.OutputReady() ) //gaps appear due to idle removal
            {
                FSM_64B66B_ENCODER      << (_72b_t) FSM_OLT_IDLE_DELETION;
                FSM_SCRAMBLER           << (_72b_t) FSM_64B66B_ENCODER;
                FSM_OLT_DATA_DETECTOR   << (_72b_t) FSM_SCRAMBLER;
            }
            
            /////////////////////////////////////////////////////////////////////
            // Transmission to the ONU ... 
            // Assume that the output of OLT's DATA Detector is connected to the
            // input of ONU's FEC Decoder, i.e., ignore constant delay in PMA, PMD,
            // and propagation delay in media
            //
            // The following is the receiving side...
            /////////////////////////////////////////////////////////////////////

            FSM_FEC_DECODER << (_72b_t) FSM_OLT_DATA_DETECTOR;

            if( FSM_FEC_DECODER.OutputReady() ) //gaps appear due to parity removal
            {
                FSM_DESCRAMBLER     << (_72b_t) FSM_FEC_DECODER;
                FSM_66B64B_DECODER  << (_72b_t) FSM_DESCRAMBLER;
                FSM_IDLE_INSERTION  << (_72b_t) FSM_66B64B_DECODER;
            }
        
            FSM_XGMII_RX << (_72b_t) FSM_IDLE_INSERTION;
        }
        /////////////////////////////////////////////////////////////////////
        // The section below operates over 36-bit columns on rising and 
        // falling edges of the XGMII clock
        /////////////////////////////////////////////////////////////////////
        FSM_MAC_RX << (_36b_t) FSM_XGMII_RX;

        if( FSM_MAC_RX.OutputReady() ) // if a complete MAC frame available...        
        {
            frame_count++;
            FSM_MPCP_RX << (_frm_t) FSM_MAC_RX;
            CollectStats( (_frm_t) FSM_MPCP_RX );
        }
    }

    OutputStats();
}


/////////////////////////////////////////////////////////////////////
// void UpstreamTiming( void )
/////////////////////////////////////////////////////////////////////
void UpstreamTiming( void )
{
    /////////////////////////////////////////////////////////////////////
    // instances of finite state machines
    /////////////////////////////////////////////////////////////////////
	fsm_mac_client_t< PacketSize >	FSM_MAC_CLIENT( true ); // defined in FSM_misc.h
    fsm_mpcp_tx_t					FSM_MPCP_TX;			// defined in FSM_MPCP.h
    fsm_mac_tx_t					FSM_MAC_TX;             // defined in FSM_MAC.h
    fsm_xgmii_tx_t					FSM_XGMII_TX;           // defined in FSM_XGMII.h
    fsm_onu_idle_deletion_t			FSM_ONU_IDLE_DELETION;  // defined in FSM_ID.h
    fsm_64b66b_encoder_t			FSM_64B66B_ENCODER;     // defined in FSM_misc.h
    fsm_scrambler_t					FSM_SCRAMBLER;          // defined in FSM_misc.h
    fsm_onu_data_detector_t			FSM_ONU_DATA_DETECTOR;  // defined in FSM_DD.h
    fsm_fec_decoder_t				FSM_FEC_DECODER;        // defined in FSM_FEC.h
    fsm_descrambler_t				FSM_DESCRAMBLER;        // defined in FSM_misc.h
    fsm_66b64b_decoder_t			FSM_66B64B_DECODER;     // defined in FSM_misc.h
    fsm_idle_insertion_t			FSM_IDLE_INSERTION;	    // defined in FSM_II.h
    fsm_xgmii_rx_t					FSM_XGMII_RX;           // defined in FSM_XGMII.h
    fsm_mac_rx_t					FSM_MAC_RX;             // defined in FSM_MAC.h
    fsm_mpcp_rx_t					FSM_MPCP_RX;		    // defined in FSM_MPCP.h


    MSG_OUT1( "Frame size,," << HEADER_STRING << endl );

    /////////////////////////////////////////////////////////////////////
    // data propagation through upstream path
    /////////////////////////////////////////////////////////////////////
    for( int32s frame_count = 0; frame_count < TEST_FRAMES; )
    {
        for( int16s byte_ndx = 0; byte_ndx < COLUMN_BYTES; byte_ndx++ )
        {
			/////////////////////////////////////////////////////////////////
            // This section operates over byte clock (0.8 ns). 
            // One XGMII transfer is ready after every 4 byte clocks
            /////////////////////////////////////////////////////////////////
            timestamp_t::IncrementClock();
			FSM_MAC_CLIENT.IncrementMACClientClock();
            FSM_MPCP_TX.IncrementByteClock();

			if( FSM_MPCP_TX.ChannelReady() && FSM_MAC_CLIENT.FrameAvailable() )
            {
			    FSM_MPCP_TX << (_frm_t)FSM_MAC_CLIENT;
                FSM_MPCP_TX.grantStart = FSM_MAC_CLIENT.GrantStart();
            }

            // If frame is available at MPCP, pass it to MAC 
            if( FSM_MPCP_TX.OutputReady() )
                FSM_MAC_TX << (_frm_t) FSM_MPCP_TX;
        }

        /////////////////////////////////////////////////////////////////
        // The section below operates over 36-bit columns on rising and 
        // falling edges of the XGMII clock
        /////////////////////////////////////////////////////////////////
        FSM_XGMII_TX << (_36b_t) FSM_MAC_TX;

        if( FSM_XGMII_TX.OutputReady() ) // if a complete 72-bit vector available
        {
            ////////////////////////////////////////////////////////////////
            // This section operates over 72-bit vectors. 
            // One vector is passed on every falling edge of the clock.
            ////////////////////////////////////////////////////////////////
            FSM_ONU_IDLE_DELETION << (_72b_t) FSM_XGMII_TX;

            if( FSM_ONU_IDLE_DELETION.OutputReady() ) //gaps appear due to idle removal
            {
                FSM_64B66B_ENCODER      << (_72b_t) FSM_ONU_IDLE_DELETION;
                FSM_SCRAMBLER           << (_72b_t) FSM_64B66B_ENCODER;
                FSM_ONU_DATA_DETECTOR   << (_72b_t) FSM_SCRAMBLER;
            }
            
            //////////////////////////////////////////////////////////////
            // Transmission to the OLT ... 
            // Assume that the output of ONU's DATA Detector is connected 
			// to the input of OLT's FEC Decoder, i.e., ignore constant
			// delay in PMA, PMD, and propagation delay in media
            // The following is the receiving side...
            ///////////////////////////////////////////////////////////////

            FSM_FEC_DECODER << (_72b_t) FSM_ONU_DATA_DETECTOR;

            if( FSM_FEC_DECODER.OutputReady() ) //gaps appear due to parity removal
            {
                FSM_DESCRAMBLER     << (_72b_t) FSM_FEC_DECODER;
                FSM_66B64B_DECODER  << (_72b_t) FSM_DESCRAMBLER;
                FSM_IDLE_INSERTION  << (_72b_t) FSM_66B64B_DECODER;
            }
        
            FSM_XGMII_RX << (_72b_t) FSM_IDLE_INSERTION;
        }
        /////////////////////////////////////////////////////////////////
        // The section below operates over 36-bit columns on rising and 
        // falling edges of the XGMII clock
        /////////////////////////////////////////////////////////////////
        FSM_MAC_RX << (_36b_t) FSM_XGMII_RX;

        if( FSM_MAC_RX.OutputReady() ) // if a complete MAC frame available...        
        {
            frame_count++;
            FSM_MPCP_RX << (_frm_t) FSM_MAC_RX;
            CollectStats( (_frm_t) FSM_MPCP_RX );
        }
    }

    OutputStats();
}

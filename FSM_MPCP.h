/**********************************************************
 * Filename:    FSM_MPCP.h
 *
 * Description: MPCP state machine
 *
 *********************************************************/


#ifndef _FSM_MPCP_H_INCLUDED_
#define _FSM_MPCP_H_INCLUDED_

#include "FSM_base.h"

/////////////////////////////////////////////////////////////////////
// MPCP TX state machine 
/////////////////////////////////////////////////////////////////////
class fsm_mpcp_tx_t: public fsm_base_t< DLY_MPCP_TX, _frm_t >
{
    private:
        clk_t   initiate_timer;    // Timer to keep track when channel
								   // will be ready for next transfer .
        bool	frameAvailable;	   // Indicator of a waiting frame to transfer.
        int16s  byte_time;         

        /////////////////////////////////////////////////////////////
        // FEC_Overhead() calculates the FEC overhead and returns the
		// sum of length and overhead i.e. the total time after which
		// channel will be rady again for another transfer
        /////////////////////////////////////////////////////////////
        clk_t FEC_Overhead( int16s length )
        {
            //////////////////////////////////////////////////////////
			// length is rounded up to column boundary to account for
			// increased IPG as discussed in section xxx.xx
            //////////////////////////////////////////////////////////
            length = (int16s)(COLUMN_BYTES * BLOCKS_ROUND_UP<COLUMN_BYTES>( length ));
            return length + FEC_PARITY_BYTES * ((byte_time + length ) / FEC_PAYLOAD_BYTES );
        }

        /////////////////////////////////////////////////////////////
        //  ReceiveUnit() receieves a frame from MAC Client only when 
		//  Channel is ready and a frame is available at MAC Client
        /////////////////////////////////////////////////////////////
        void ReceiveUnit( _frm_t frame ) 
        {
			if( !ChannelReady() )
                MSG_WARN( "MPCP has not finished sending previous frame" );
			
            output_block   = frame; 
            frameAvailable = true;
        }

        ////////////////////////////////////////////////////////////////////
        // Transfer from MPCP to MAC can be done when 1) channel is ready
		// (i.e. initiate_timer == 0), 2) a frame is available, 
		// 3) byte_time is alligned with column boundary and 
		// 4) byte_time < FEC_PAYLOAD_BYTES (to ensure that frame is not 
		// transferred to MAC when Data Detector is transmitting FEC 
		// parity code, otherwise this frame can be delayed by upto 32
		// bytes time).  
        ///////////////////////////////////////////////////////////////////
        _frm_t TransmitUnit( void )
        {
            if( !OutputReady() )
                MSG_WARN( "MPCP Frame is not ready" );

            //////////////////////////////////////////////////////////////
            // if this is the beginning of a burst, account for 2 idle
			// blocks at the beginning of FEC-protected portion of a burst
            //////////////////////////////////////////////////////////////
            if( grantStart ) 
            {		
               byte_time = 16;  
               grantStart = false;
            }					
          
            frameAvailable = false;

            //////////////////////////////////////////////////////////
            // MAC_CLIENT returns frame size excluding IPG and 
			// preamble but includes E_HEADER_BYTES and CHECKSUM_BYTES
            //////////////////////////////////////////////////////////
            int16s data_tx = output_block.GetFrameSize() - E_HEADER_BYTES - CHECKSUM_BYTES;

            ////////////////////////////////////////////////
            // TAIL_GAURD includes PREAMBLE, E_HEADER_BYTES,
			// CHECKSUM_BYTES and IPG
            ////////////////////////////////////////////////
            initiate_timer = FEC_Overhead( data_tx + TAIL_GUARD ); 

			////////////////////////////////////////////////////
            // Transfer a frame to the MAC 
            ////////////////////////////////////////////////////
            return output_block;
        }

 
    public:

        bool grantStart;
        ///////////////////////////////////////////////////////
        //  
        ///////////////////////////////////////////////////////
        fsm_mpcp_tx_t()
        {
            byte_time        = 0;
            initiate_timer   = 0;
            frameAvailable	 = false;
            grantStart       = false;
        }

        ///////////////////////////////////////////////////////
        //  
        ///////////////////////////////////////////////////////
        inline void IncrementByteClock( void ) 
        {
            byte_time ++;

            if( byte_time == FEC_CODEWORD_BYTES )
                byte_time = 0;

            if( initiate_timer > 0 )
                initiate_timer--;
        }

        /////////////////////////////////////////////////////////////
        // Transfer from MPCP to MAC can be done when 1) channel is ready
		// (i.e. initiate_timer == 0), 2) a frame is available, 
		// 3) byte_time is alligned with column boundary and 
		// 4) byte_time < FEC_PAYLOAD_BYTES (to ensure that frame is not 
		// transferred to MAC when Data Detector is transmitting FEC 
		// parity code, otherwise this frame can be delayed by upto 32
		// bytes time).  
		//
		//
		// At the beginning of a burst, the control multiplexor at the
		// ONU resets the byte_time to 16, to account for the FEC
		// codeword reset. If the next frame is the first frame in a burst,
		// condition (4) will become satisfied automatically when the
		// byte_time is reset.
        /////////////////////////////////////////////////////////////
        inline bool OutputReady( void ) 
        {
            bool alignmentCorrect = ( byte_time & 0x0003 ) == 0  && ( byte_time < FEC_PAYLOAD_BYTES || grantStart );
            return initiate_timer == 0 && frameAvailable && alignmentCorrect;
        }

		/////////////////////////////////////////////////////////////
        // As soon as the initiate_timer goes to zero and no frame is 
		// waiting for transfer( in MPCP ) to MAC, the channel becomes
		// ready and MAC Client can transfer a frame to MPCP (if there
		// is a frame available at MAC Client ).
        /////////////////////////////////////////////////////////////
        inline bool ChannelReady( void ) 
        {
            return initiate_timer == 0 && !frameAvailable;
        }
};

/////////////////////////////////////////////////////////////////////
// MPCP RX state machine 
/////////////////////////////////////////////////////////////////////
class fsm_mpcp_rx_t: public fsm_base_t< DLY_MPCP_RX, _frm_t >
{
    private:
        /////////////////////////////////////////////////////////////
        void ReceiveUnit( _frm_t in_blk )
        {
            output_block = in_blk;
            output_ready = true;
        }
};

#endif //_FSM_MPCP_H_INCLUDED_
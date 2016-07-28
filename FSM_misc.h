/**********************************************************
 * Filename:    FSM_misc.h
 *
 * Description: 
 *
 *********************************************************/


#ifndef _MISC_FSMS_H_INCLUDED_
#define _MISC_FSMS_H_INCLUDED_


#include "FSM_base.h"

////////////////////////////////////////////////////////////////////
// Mac Client: can generate back to back or sparse traffic. 
// MAC Client represents all functions located above MPCP Control
// Multiplexor function. Thus, it is client's responsibility to
// delay frames until the grant start time.
////////////////////////////////////////////////////////////////////
template< int16s (*pf_packet_size)( void ) > class fsm_mac_client_t: public fsm_base_t< DLY_MAC_CLIENT, _frm_t, _frm_t >
{
    private:
		int16s	frame_ready_counter;   // Timer to keep track when next frame will be
		                               // available for transfer to MPCP. 

		int32s  frame_count;		   // Keep track of number of transferred frame to 
									   // MPCP in a burst; only relevant in Busrt Mode.

		bool	frame_waiting;         // True if next frame is already scheduled; false
									   // otherwise.

		bool    burst_mode;            // Indicates wheather Burst Mode is ON of OFF
	
	    /////////////////////////////////////////////////////////////
        //  
        /////////////////////////////////////////////////////////////
		void ReceiveUnit( _frm_t ){};

		/////////////////////////////////////////////////////////////
        //  
        /////////////////////////////////////////////////////////////
		_frm_t TransmitUnit( void )
        {
				
			if( frame_ready_counter > 0 )
			{
				MSG_WARN( "MAC Client frame is not available" );
                exit(0);
            }

			frame_count++;
			frame_waiting = false;

			///////////////////////////////////////////////////////////////////
			// this function returns packet size excluding preamble and IPG
			///////////////////////////////////////////////////////////////////
			return _frm_t( timestamp_t::GetClock(), pf_packet_size() );
		}

		

	public:
        fsm_mac_client_t( bool brst_md = false )
        {
            burst_mode	    = brst_md;
			frame_count     = 0;
			frame_waiting   = false;
           
            //////////////////////////////////////////////////////////////////
            // At the begining, a frame will be ready after BURST_GAP_BYTES if 
            // burst_mode is ON (i.e. ONU), or MIN_IPG_BYTES, otherwise (OLT).
            //////////////////////////////////////////////////////////////////
            frame_ready_counter = burst_mode? BURST_GAP_BYTES: MIN_IPG_BYTES;
        }

        inline bool GrantStart( void ) const      { return frame_count == 1; }
		//////////////////////////////////////////////////////////////////////
		// frame_ready_counter is set to either a random positive value or 
		// 0 in FrameAvailable() as soon as the value reaches 0, 
		// frameAvailable() returns true
		//////////////////////////////////////////////////////////////////////
		inline void IncrementMACClientClock( void )
		{
			if( frame_ready_counter > 0 )
				frame_ready_counter--;
		}

		/////////////////////////////////////////////////////////////////////
		// This function will be called when MPCP channel becomes available, 
        // i.e., when MPCP "thinks" the frame transmission is finished. When 
        // this function is called first time after MPCP channel becomes 
        // available, it may make a next frame from client available right 
        // away (back-to-back traffic) or with some delay (either sparse 
        // traffic or to recreate gap between bursts at the ONU).
		/////////////////////////////////////////////////////////////////////
		inline bool FrameAvailable( void )
		{
			/////////////////////////////////////////////////////////////////
			// Frame_ready_counter should be set only after a frame is 
			// transfered to MPCP layer. Frame_waiting is set to false at 
			// TransmitUnit() i.e. after transfering a frame to MPCP.
			//////////////////////////////////////////////////////////////////
			if( !frame_waiting )
			{
				frame_waiting = true;
				//////////////////////////////////////////////////////////////
				// if sparse traffic is desirable, next frame will be 
                // available after some random delay
				//////////////////////////////////////////////////////////////
				#ifdef SPARSE_TRAFFIC
					frame_ready_counter += (int16s)(( rand() * FEC_CODEWORD_BYTES ) / RAND_MAX );
				#endif

				///////////////////////////////////////////////////////////////
				// if previous frame was the last frame of a burst then client 
                // creates a bigger gap  
				///////////////////////////////////////////////////////////////
				if( burst_mode && frame_count >= BURST_FRAMES )
				{
					frame_count = 0;
					frame_ready_counter += BURST_GAP_BYTES;
				}
			}

            return ( frame_ready_counter <= 0 );
		}
};





/////////////////////////////////////////////////////////////////////
// 64B/66B encoder state machine 
/////////////////////////////////////////////////////////////////////
class fsm_64b66b_encoder_t: public fsm_base_t< DLY_66B_ENCODER, _72b_t, _66b_t >
{
    private:
        void ReceiveUnit( _72b_t in_blk ) 
        { 
            output_block = _66b_t( in_blk ); 
            output_ready = true;
        }
};

/////////////////////////////////////////////////////////////////////
// 66B/64B decoder state machine 
/////////////////////////////////////////////////////////////////////
class fsm_66b64b_decoder_t: public fsm_base_t< DLY_66B_DECODER, _66b_t, _72b_t >
{
    private:
        void ReceiveUnit( _66b_t in_blk ) 
        { 
            output_block = static_cast<_72b_t>( in_blk );
            output_ready = true;
        }
};

/////////////////////////////////////////////////////////////////////
// Scramber state machine 
/////////////////////////////////////////////////////////////////////
class fsm_scrambler_t:   public fsm_base_t< DLY_SCRAMBLER, _66b_t > 
{
    private:
        /////////////////////////////////////////////////////////////
		//
		/////////////////////////////////////////////////////////////
        void ReceiveUnit( _66b_t in_blk )
        {
            output_block = in_blk;
            output_ready = true;
        }
};


class fsm_descrambler_t: public fsm_base_t< DLY_DESCRAMBLER, _66b_t > 
{
    private:
        /////////////////////////////////////////////////////////////
		//
		/////////////////////////////////////////////////////////////
        void ReceiveUnit( _66b_t in_blk )
        {
            output_block = in_blk;
            output_ready = true;
        }
};


#endif //_MISC_FSMS_H_INCLUDED_
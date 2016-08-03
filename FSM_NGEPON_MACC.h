/**********************************************************
 * Filename:    FSM_misc.h
 *
 * Description: 
 *
 *********************************************************/


#ifndef _FSM_NGEPON_MACC_H_INCLUDED_
#define _FSM_NGEPON_MACC_H_INCLUDED_


#include "FSM_base.h"

////////////////////////////////////////////////////////////////////
// Mac Client: can generate back to back or sparse traffic. 
// MAC Client represents all functions located above MPCP Control
// Multiplexor function. Thus, it is client's responsibility to
// delay frames until the grant start time.
////////////////////////////////////////////////////////////////////
template< int16s (*pf_packet_size) (void) > class fsm_ngepon_macc_t: public fsm_base_t< DLY_NGEPON_MACC, _frm_t, _frm_t >
{
    private:

		int16s	frame_ready_counter;   // Timer to keep track when next frame will be available for transfer to MPCP. 
		int32s  frame_count;		   // Keep track of number of transferred frame to MPCP in a burst; only relevant in Burst Mode.
		bool	frame_waiting;         // True if next frame is already scheduled; false otherwise.
		bool    burst_mode;            // Indicates wheather Burst Mode is ON of OFF

	public:
	
	    /////////////////////////////////////////////////////////////
        // Function to receive data (no special processing is needed)
        /////////////////////////////////////////////////////////////
		void ReceiveUnit (_frm_t)
		{
		};

		/////////////////////////////////////////////////////////////
        // Function to transmit (generate) traffic. 
        /////////////////////////////////////////////////////////////
		_frm_t TransmitUnit (void)
        {
			// log error condition if there are no frames pending transmission 	
			if (this->frame_ready_counter > 0)
			{
				MSG_WARN ("MAC Client frame is not available");
                exit(0);
            }

			// update number of transmitted frames and unlock MAC Client status
			this->frame_count++;
			this->frame_waiting = false;

			// this function returns packet size excluding preamble and IPG
			return _frm_t (timestamp_t::GetClock(), pf_packet_size());
		}

		fsm_ngepon_macc_t (bool brst_md = false)
        {
            // intialize variables
			this->burst_mode	  = brst_md;
			this->frame_count     = 0;
			this->frame_waiting   = false;
           
            //////////////////////////////////////////////////////////////////
            // At the begining, a frame will be ready after BURST_GAP_BYTES if 
            // burst_mode is ON (i.e. ONU), or MIN_IPG_BYTES, otherwise (OLT).
            //////////////////////////////////////////////////////////////////
            this->frame_ready_counter = this->burst_mode? BURST_GAP_BYTES : MIN_IPG_BYTES;
        }

        inline bool GrantStart (void) const      
		{ 
			return (this->frame_count == 1); 
		}

		//////////////////////////////////////////////////////////////////////
		// frame_ready_counter is set to either a random positive value or 
		// 0 in FrameAvailable() as soon as the value reaches 0, 
		// frameAvailable() returns true
		//////////////////////////////////////////////////////////////////////
		inline void IncrementMACClientClock (void)
		{
			if (this->frame_ready_counter > 0)
				this->frame_ready_counter--;
		}

		/////////////////////////////////////////////////////////////////////
		// This function will be called when MPCP channel becomes available, 
        // i.e., when MPCP "thinks" the frame transmission is finished. When 
        // this function is called first time after MPCP channel becomes 
        // available, it may make a next frame from client available right 
        // away (back-to-back traffic) or with some delay (either sparse 
        // traffic or to recreate gap between bursts at the ONU).
		/////////////////////////////////////////////////////////////////////
		inline bool FrameAvailable (void)
		{
			/////////////////////////////////////////////////////////////////
			// Frame_ready_counter should be set only after a frame is 
			// transfered to MPCP layer. Frame_waiting is set to false at 
			// TransmitUnit() i.e. after transfering a frame to MPCP.
			//////////////////////////////////////////////////////////////////
			if (this->frame_waiting == false)
			{
				this->frame_waiting = true;
				
				//////////////////////////////////////////////////////////////
				// if sparse traffic is desirable, next frame will be 
                // available after some random delay
				//////////////////////////////////////////////////////////////
				#ifdef SPARSE_TRAFFIC
				this->frame_ready_counter += (int16s)( (rand() * FEC_CODEWORD_BYTES) / RAND_MAX);
				#endif

				///////////////////////////////////////////////////////////////
				// if previous frame was the last frame of a burst then client 
                // creates a bigger gap  
				///////////////////////////////////////////////////////////////
				if (this->burst_mode && this->frame_count >= BURST_FRAMES)
				{
					this->frame_count = 0;
					this->frame_ready_counter += BURST_GAP_BYTES;
				}
			}

            return  (this->frame_ready_counter <= 0);
		}
};

#endif //_FSM_NGEPON_MACC_H_INCLUDED_
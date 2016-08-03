/**********************************************************
 * Filename:    FSM_MPCP.h
 *
 * Description: MPCP state machine
 *
 *********************************************************/


#ifndef _FSM_NGEPON_MPCP_H_INCLUDED_
#define _FSM_NGEPON_MPCP_H_INCLUDED_

#include "FSM_base.h"

/////////////////////////////////////////////////////////////////////
// MPCP TX state machine 
/////////////////////////////////////////////////////////////////////
class fsm_ngepon_mpcp_tx_t: public fsm_base_t< DLY_NGEPON_MPCP_TX, _frm_t >
{
    private:
        clk_t   initiate_timer;    // Timer to keep track when channel will be ready for next transfer .
        bool	frameAvailable;	   // Indicator of a waiting frame to transfer.
        int16s  byte_time;         

        /////////////////////////////////////////////////////////////
        //  ReceiveUnit() receieves a frame from MAC Client only when 
		//  Channel is ready and a frame is available at MAC Client
        /////////////////////////////////////////////////////////////
        void ReceiveUnit (_frm_t frame) 
        {
			// log a warning message, MPCP has not finished sending previous frame
			if (this->ChannelReady() == false)
                MSG_WARN ("MPCP has not finished sending previous frame");
			
            this->output_block   = frame; 
            this->frameAvailable = true;
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
        _frm_t TransmitUnit (void)
        {
			// log a warning message, MPCP output frame is not ready 
            if (this->OutputReady() == false)
                MSG_WARN ("MPCP Frame is not ready");

            //////////////////////////////////////////////////////////////
            // if this is the beginning of a burst, account for 2 idle
			// blocks at the beginning of FEC-protected portion of a burst
            //////////////////////////////////////////////////////////////
            if (this->grantStart == true) 
            {		
               this->byte_time = 16;  
               this->grantStart = false;
            }					
          
            this->frameAvailable = false;

            //////////////////////////////////////////////////////////
            // MAC_CLIENT returns frame size excluding IPG and 
			// preamble but includes E_HEADER_BYTES and CHECKSUM_BYTES
			// TAIL_GAURD includes PREAMBLE, E_HEADER_BYTES,
			// CHECKSUM_BYTES and IPG
            //////////////////////////////////////////////////////////
			initiate_timer = output_block.GetFrameSize() - E_HEADER_BYTES - CHECKSUM_BYTES + TAIL_GUARD;

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
        fsm_ngepon_mpcp_tx_t()
        {
            byte_time        = 0;
            initiate_timer   = 0;
            frameAvailable	 = false;
            grantStart       = false;
        }

        ///////////////////////////////////////////////////////
        //  
        ///////////////////////////////////////////////////////
        inline void IncrementByteClock (void) 
        {
            this->byte_time ++;

            if (this->byte_time == FEC_CODEWORD_BYTES)
				this->byte_time = 0;

            if (this->initiate_timer > 0)
				this->initiate_timer--;
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
        inline bool OutputReady (void) 
        {
            bool alignmentCorrect =  (byte_time & 0x0003) == 0  &&  (byte_time < FEC_PAYLOAD_BYTES || grantStart);
            // return initiate_timer == 0 && frameAvailable && alignmentCorrect;
			return initiate_timer == 0 && frameAvailable;
        }

		/////////////////////////////////////////////////////////////
        // As soon as the initiate_timer goes to zero and no frame is 
		// waiting for transfer (in MPCP) to MAC, the channel becomes
		// ready and MAC Client can transfer a frame to MPCP (if there
		// is a frame available at MAC Client).
        /////////////////////////////////////////////////////////////
        inline bool ChannelReady (void) 
        {
            return initiate_timer == 0 && !frameAvailable;
        }
};

/////////////////////////////////////////////////////////////////////
// MPCP RX state machine 
/////////////////////////////////////////////////////////////////////
class fsm_ngepon_mpcp_rx_t: public fsm_base_t< DLY_NGEPON_MPCP_RX, _frm_t >
{
    private:
        /////////////////////////////////////////////////////////////
        void ReceiveUnit (_frm_t in_blk)
        {
            output_block = in_blk;
            output_ready = true;
        }
};

#endif //_FSM_NGEPON_MPCP_H_INCLUDED_
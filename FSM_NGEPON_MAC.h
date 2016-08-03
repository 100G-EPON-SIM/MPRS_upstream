/**********************************************************
 * Filename:    FSM_NGEPON_MAC.h
 *
 * Description: 
 *
 *********************************************************/


#ifndef _FSM_NGEPON_MAC_H_INCLUDED_
#define _FSM_NGEPON_MAC_H_INCLUDED_

#include "FSM_base.h"

#define IDLE_COLUMN _36b_t (C_BLOCK)


/////////////////////////////////////////////////////////////////////
// MAC TX state machine 
/////////////////////////////////////////////////////////////////////
class fsm_ngepon_mac_tx_t: public fsm_base_t< DLY_NGEPON_MAC_TX, _frm_t, _36b_t >
{
    private:
        
		timestamp_t timestamp;
        bool        transmitting;
		int32s	    tx_sequence;
        int16s      frame_bytes;
        int16s      data_columns;
        int16s      idle_deficit;
        int16s      IPG_required;

	public:
    
		//////////////////////////////////////////////////////////////////////////////
		//  
		//////////////////////////////////////////////////////////////////////////////
		inline int16s CalculateIPGBytes (int16s packet_size)
		{
			int16s temp = MIN_IPG_BYTES + this->idle_deficit;
			this->idle_deficit = (packet_size + this->idle_deficit) & 0x03;
			return (temp - this->idle_deficit);
		}

        /////////////////////////////////////////////////////////////
		// This function accepts a new frame from MPCP 
        // for transmission. This function is called by MPCP
        /////////////////////////////////////////////////////////////
        void ReceiveUnit (_frm_t frame)
        {
			// log error condition
            if (this->MacReady() == false)
            {
				MSG_WARN ("Frame passed to a busy MAC");
                return;
            }
           
			// save the frame size and its timestamp
            this->frame_bytes  = frame.GetFrameSize() + PREAMBLE_BYTES;
            this->timestamp = static_cast<timestamp_t> (frame);
		}

        /////////////////////////////////////////////////////////////
        // Transmit 36-bit column towards RS
        /////////////////////////////////////////////////////////////
        _36b_t TransmitUnit (void)
        {
            /////////////////////////////////////////////////////////
            // If already transmitting a frame, continue...
            /////////////////////////////////////////////////////////
            if (this->transmitting == true)
            {
                if (this->data_columns == 1) // if this is the last data block...
                {
                    /////////////////////////////////////////////////
                    // Find out how many idle columns should be trasmitted
                    // after the frame. This is an implementation of
                    // Deficit Idle Counter. 
                    //
                    // T-block may may include up to 3 idle characters;
                    // IPG_required shows the number of additional idle
                    // columns to be sent. IPG_required can take values
                    // of 2 or 3 only. 
                    /////////////////////////////////////////////////
                    this->IPG_required = this->CalculateIPGBytes (this->frame_bytes) / COLUMN_BYTES;
                    this->frame_bytes = 0;

                    /////////////////////////////////////////////////
                    // Transmit T block 
                    // (this block may include up to 3 idles)
                    /////////////////////////////////////////////////
                    this->data_columns = 0;
                    this->transmitting = false;

					// @TODO@ - MAC needs to distinguish T1_BLOCK, T2_BLOCK, and T3_BLOCK sequences 
					// to make sure that RS can proeprly encode them into 25GMII
                    return _36b_t (this->timestamp, T_BLOCK, ++this->tx_sequence);
                }
                
                this->data_columns--;
                return _36b_t (this->timestamp, D_BLOCK, ++this->tx_sequence);
            }

            /////////////////////////////////////////////////////////
            // enforce minimum IPG 
            /////////////////////////////////////////////////////////
            if (this->IPG_required > 0)
            {   
				///////////////////////////////////////////////////////////////////////////////////////
				//if MAC is transmitting IPG and a new frame is available, then it should 
				//latch the frame in transmit buffer
				///////////////////////////////////////////////////////////////////////////////////////
				
                this->data_columns = (int16s)BLOCKS_ROUND_UP< COLUMN_BYTES > (this->frame_bytes);
				this->IPG_required--;
                return IDLE_COLUMN;  
            }

            /////////////////////////////////////////////////////////
            // if data is latched for transmission, start sending the next frame
            /////////////////////////////////////////////////////////
            if (this->data_columns > 0)
            {
				this->transmitting = true;
				this->data_columns--;
                return _36b_t (this->timestamp, S_BLOCK, ++this->tx_sequence);
            }

            /////////////////////////////////////////////////////////
            // if another frame arrived, latch it in transmit buffer
            /////////////////////////////////////////////////////////
            if (this->frame_bytes > 0)
            {
				this->data_columns = (int16s)BLOCKS_ROUND_UP< COLUMN_BYTES > (this->frame_bytes);
            }

            /////////////////////////////////////////////////////////
            // if no data available, send idles
            /////////////////////////////////////////////////////////
            return IDLE_COLUMN;  
        }

		fsm_ngepon_mac_tx_t()
        {
			// initialize all variables 
			this->timestamp     = 0;
			this->transmitting  = false;
			this->tx_sequence   = 0;
			this->data_columns  = 0;
			this->idle_deficit  = 0;
			this->IPG_required  = 0;
        }

		/////////////////////////////////////////////////////////////
        // MAC is ready to accept a new frame from the MAC client
		// Note that MAC may not be ready to start transmission due 
		// to asserted CRS or IPG being enforced 
        /////////////////////////////////////////////////////////////
		inline bool MacReady (void) const 
		{ 
			return this->frame_bytes <= 0;
		}
		
};

/////////////////////////////////////////////////////////////////////
// MAC RX state machine 
/////////////////////////////////////////////////////////////////////
class fsm_ngepon_mac_rx_t: public fsm_base_t< DLY_NGEPON_MAC_RX, _36b_t, _frm_t >
{
    private:
        
		clk_t   timestamp;
		bool    receiving;
		int32s	rx_sequence;
		int32u  BlockCountIn;

	public:

        /////////////////////////////////////////////////////////////
        // Receive 36-bit column 
        /////////////////////////////////////////////////////////////
        void ReceiveUnit (_36b_t col)
        { 
			// increase number of received blocks
			this->BlockCountIn++;

			// update block sequence only for specific block types
			if (col.GetSeqNumber() != -1)
				this->rx_sequence++;
			
			#ifdef DEBUG_ENABLE_MAC_RX
				if (!(col.IsType(C_BLOCK) || col.IsType(Y_BLOCK) || col.IsType(X_BLOCK)))
					std::cout << "MAC RX, column type: " << BlockName(col.C_TYPE()) << ", sequence [expected: " << this->rx_sequence << ", received: " << col.GetSeqNumber() << "], count: " << this->BlockCountIn << std::endl;
			#endif // DEBUG_ENABLE_MAC_RX

			if (col.IsType(E_BLOCK) || col.IsType(X_BLOCK) || col.IsType(Y_BLOCK) || col.IsType(P_BLOCK))
				return;

            if (col.IsType(C_BLOCK))
            {
                if (this->receiving == true)
                {
					this->receiving = false;
					this->output_ready = true;
                }
				return;
            }

			// log a warning message, column out of sequence was received
			if (this->rx_sequence != col.GetSeqNumber())
			{
				MSG_WARN (BlockName(col.C_TYPE()) << "-column received out of sequence [expected: " << this->rx_sequence << ", received: " << col.GetSeqNumber() << "]");
				// synchronize sequence numbers 
				this->rx_sequence = col.GetSeqNumber();
			}

			// log a warning message, data is still in MAC
			if (this->output_ready == true)
			{
				MSG_WARN("Received MAC frame is being overwritten");
			}
			
			// log a warning message, unexpected column type was received
			if ((col.IsType(D_BLOCK) || col.IsType(T_BLOCK)) && this->receiving == false)
            {
                MSG_WARN("Unexpected " << BlockName (col.C_TYPE()) << " column");
            }

			// log a warning message, S column received in the middle of a MAC frame
            else if (col.IsType(S_BLOCK) && this->receiving == true)
            {
			    MSG_WARN("S column received in the middle of a MAC frame");
            }

			this->receiving = true;
			this->output_block.AddColumn (col);
        }

		fsm_ngepon_mac_rx_t()
        {
			// initialize internal variables 
            this->timestamp     = 0;
			this->receiving     = false;
			this->rx_sequence   = 0;
			this->BlockCountIn	= 0;
        }
};

#endif //_FSM_NGEPON_MAC_H_INCLUDED_
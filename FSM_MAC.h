/**********************************************************
 * Filename:    FSM_MAC.h
 *
 * Description: 
 *
 *********************************************************/


#ifndef _FSM_MAC_H_INCLUDED_
#define _FSM_MAC_H_INCLUDED_

#include "FSM_base.h"

#define IDLE_COLUMN _36b_t( C_BLOCK )


/////////////////////////////////////////////////////////////////////
// MAC TX state machine 
/////////////////////////////////////////////////////////////////////
class fsm_mac_tx_t: public fsm_base_t< DLY_MAC_TX, _frm_t, _36b_t >
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
		inline int16s CalculateIPGBytes( int16s packet_size )
		{
			int16s temp = MIN_IPG_BYTES + idle_deficit;
			idle_deficit = (packet_size + idle_deficit) & 0x03;
			return temp - idle_deficit;
		}

        /////////////////////////////////////////////////////////////
		// This function accepts a new frame from MPCP 
        // for transmission. This function is called by MPCP
        /////////////////////////////////////////////////////////////
        void ReceiveUnit( _frm_t frame )
        {
            if( !MacReady() )
            {
				MSG_WARN( "Frame passed to a busy MAC" );
                return;
            }
           
            frame_bytes  = frame.GetFrameSize() + PREAMBLE_BYTES;
            timestamp    = static_cast<timestamp_t>( frame );
		}

        /////////////////////////////////////////////////////////////
        // Transmit 36-bit column 
        /////////////////////////////////////////////////////////////
        _36b_t TransmitUnit( void )
        {
            /////////////////////////////////////////////////////////
            // If already transmitting a frame, continue...
            /////////////////////////////////////////////////////////
            if( transmitting )
            {
                if( data_columns == 1 ) // if this is the last data block...
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
                    IPG_required = CalculateIPGBytes( frame_bytes ) / COLUMN_BYTES;
                    frame_bytes = 0;

                    /////////////////////////////////////////////////
                    // Transmit T block 
                    // (this block may include up to 3 idles )
                    /////////////////////////////////////////////////
                    data_columns = 0;
                    transmitting = false;

					// @TODO@ - MAC needs to distinguish T1_BLOCK, T2_BLOCK, and T3_BLOCK sequences 
					// to make sure that RS can proeprly encode them into 25GMII
                    return _36b_t( timestamp, T_BLOCK, ++tx_sequence );
                }
                
                data_columns--;
                return _36b_t( timestamp, D_BLOCK, ++tx_sequence );
            }

            /////////////////////////////////////////////////////////
            // enforce minimum IPG 
            /////////////////////////////////////////////////////////
            if( IPG_required > 0 )
            {   
				///////////////////////////////////////////////////////////////////////////////////////
				//if MAC is transmitting IPG and a new frame is available, then it should 
				//latch the frame in transmit buffer
				///////////////////////////////////////////////////////////////////////////////////////
				
                data_columns = (int16s)BLOCKS_ROUND_UP< COLUMN_BYTES >( frame_bytes );
                IPG_required--;
                return IDLE_COLUMN;  
            }

            /////////////////////////////////////////////////////////
            // if data is latched for transmission, start sending the next frame
            /////////////////////////////////////////////////////////
            if( data_columns > 0 )
            {
                transmitting = true;
                data_columns--;
                return _36b_t( timestamp, S_BLOCK, ++tx_sequence );
            }

            /////////////////////////////////////////////////////////
            // if another frame arrived, latch it in transmit buffer
            /////////////////////////////////////////////////////////
            if( frame_bytes > 0 )
            {
                data_columns = (int16s)BLOCKS_ROUND_UP< COLUMN_BYTES >( frame_bytes );
            }

            /////////////////////////////////////////////////////////
            // if no data available, send idles
            /////////////////////////////////////////////////////////
            return IDLE_COLUMN;  
        }


    public:
        fsm_mac_tx_t()
        {
            timestamp     = 0;
            transmitting  = false;
			tx_sequence   = 0;
            data_columns  = 0;
            idle_deficit  = 0;
            IPG_required  = 0;
        }

		/////////////////////////////////////////////////////////////
        // MAC is ready to accept a new frame from the MAC client
		// Note that MAC may not be ready to start transmission due 
		// to asserted CRS or IPG being enforced 
        /////////////////////////////////////////////////////////////
		inline bool MacReady( void ) const { return frame_bytes <= 0; }

		
};



/////////////////////////////////////////////////////////////////////
// MAC RX state machine 
/////////////////////////////////////////////////////////////////////
class fsm_mac_rx_t: public fsm_base_t< DLY_MAC_RX, _36b_t, _frm_t >
{
    private:
        clk_t   timestamp;
		bool    receiving;
		int32s	rx_sequence;

        /////////////////////////////////////////////////////////////
        // Receive 36-bit column 
        /////////////////////////////////////////////////////////////
        void ReceiveUnit( _36b_t col )
        { 
            if( col.IsType( E_BLOCK | P_BLOCK ))
				return;

            if( col.IsType( C_BLOCK ))
            {
                if( receiving )
                {
				    receiving = false;
                    output_ready = true;
                }
				return;
            }

			if( ++ rx_sequence != col.GetSeqNumber() )
			{
				MSG_WARN( BlockName(col.C_TYPE()) << "-column received out of sequence [expected: " << rx_sequence << ", received: " << col.GetSeqNumber() << "]" );
				rx_sequence = col.GetSeqNumber();
			}

            if( output_ready )
			    MSG_WARN( "Received MAC frame is being overwritten" );
			
			if( col.IsType( D_BLOCK | T_BLOCK ) && !receiving )
            {
                MSG_WARN( "Unexpected " << BlockName( col.C_TYPE() ) << " column" );
            }
            else if( col.IsType( S_BLOCK ) && receiving )
            {
			    MSG_WARN( "S column received in the middle of a MAC frame" );
            }

            receiving = true;
            output_block.AddColumn( col );
        }

    public:
        fsm_mac_rx_t()
        {
            timestamp     = 0;
			receiving     = false;
			rx_sequence   = 0;
        }
};

#endif //_FSM_MAC_H_INCLUDED_
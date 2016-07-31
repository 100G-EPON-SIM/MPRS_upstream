
/**********************************************************
 * Filename:    FSM_25GMII.h
 *
 * Description: Defines 25GMII process. It's main purpose
 * in this simulation is to combine two columns into one
 * 72-bit vector in the transmit direction, and to break
 * one 72-bit vector into two 36-bit columns in the
 * receive direction.
 *
 *********************************************************/


#ifndef _FSM_25GMII_H_INCLUDED_
#define _FSM_25GMII_H_INCLUDED_

#include "FSM_base.h"

class fsm_25GMII_tx_t: public fsm_base_t< DLY_25GMII_TX, _36b_t, _72b_t >
{

	private:

		_72b_t vector;				// internal storage only
		int16s column_count;		// internal storage only

		/////////////////////////////////////////////////////////////
		// 
		/////////////////////////////////////////////////////////////
		void ReceiveUnit( _36b_t column )
		{
			if( output_ready )
				MSG_WARN( "Overwritting vector in 25GMII TX" );

			vector[ column_count ] = column;
			column_count++;

			if( column_count == 2 )
			{
				output_ready = true;
				column_count = 0;
			}
		}

		/////////////////////////////////////////////////////////////
		// 
		/////////////////////////////////////////////////////////////
		_72b_t TransmitUnit( void )
		{
			if( !output_ready )
				MSG_WARN( "25GMII TX passed incomplete vector" );

			output_ready = false;
			return vector;
		}
	
public:
    fsm_25GMII_tx_t()
    {
		column_count = 0;
    }
};




class fsm_25GMII_rx_t: public fsm_base_t< DLY_25GMII_RX, _72b_t, _36b_t >
{
private:
    _72b_t vector;
    int16s last_index;

    /////////////////////////////////////////////////////////////
    // 
    /////////////////////////////////////////////////////////////
    void ReceiveUnit( _72b_t vctr )
    {
        vector = vctr;
    }

	/////////////////////////////////////////////////////////////
    // 
    /////////////////////////////////////////////////////////////
    _36b_t TransmitUnit( void )
    {
		return vector[ last_index ^= 0x0001 ];
    }

	
public:
    fsm_25GMII_rx_t()
    {
        last_index = 0;  
        output_ready = true;
    }
};


#endif // _FSM_25GMII_H_INCLUDED_

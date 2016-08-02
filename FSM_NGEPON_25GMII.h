
/**********************************************************
 * Filename:    FSM_NGEPON_25GMII.h
 *
 * Description: Defines 25GMII process. It's main purpose
 * in this simulation is to combine two columns into one
 * 72-bit vector in the transmit direction, and to break
 * one 72-bit vector into two 36-bit columns in the
 * receive direction.
 *
 *********************************************************/


#ifndef _FSM_NGEPON_25GMII_H_INCLUDED_
#define _FSM_NGEPON_25GMII_H_INCLUDED_

#include "FSM_base.h"

 /////////////////////////////////////////////////////////////////////
 // 25GMII state machine, Transmit Direction 
 /////////////////////////////////////////////////////////////////////
class fsm_ngepon_25gmii_tx_t: public fsm_base_t< DLY_NGEPON_25GMII_TX, _36b_t, _72b_t >
{

	private:

		_72b_t vector;				// internal storage only
		int16s column_count;		// internal storage only

	public:

		/////////////////////////////////////////////////////////////
		// This function accepts a 36-bit vector and stores it locally
		// until 2 consecutive transfers are received, at which time 
		// a single 72-bit vector is made available to PCS  
		////////////////////////////////////////////////////////////
		void ReceiveUnit (_36b_t column)
		{
			// if there is output data available, log a warning
			if (this->output_ready == true)
				MSG_WARN ("Overwritting vector in 25GMII TX");

			// store received data locally 
			this->vector[this->column_count] = column;
			this->column_count++;

			// signal data available 
			if (column_count == 2)
			{
				this->output_ready = true;
				this->column_count = 0;
			}
		}

		/////////////////////////////////////////////////////////////
		// This function sends a single 72-bit vector when data is 
		// available
		////////////////////////////////////////////////////////////
		_72b_t TransmitUnit (void)
		{
			// if there is no complete data available, log a warning
			if (this->output_ready == false)
				MSG_WARN ("25GMII TX passed incomplete vector");

			// return putput data 
			this->output_ready = false;
			return vector;
		}
	
		fsm_ngepon_25gmii_tx_t()
		{
			// initialize internal variables
			this->column_count = 0;
		}

};

/////////////////////////////////////////////////////////////////////
// 25GMII state machine, Receive Direction 
/////////////////////////////////////////////////////////////////////
class fsm_ngepon_25gmii_rx_t: public fsm_base_t< DLY_NGEPON_25GMII_RX, _72b_t, _36b_t >
{

	private:

		_72b_t vector;
		int16s last_index;

	public:

		/////////////////////////////////////////////////////////////
		// This function accepts a 72-bit vector from PCS and stores 
		// it internally to be transmitted into RS one 36-bit vector
		// at a time 
		/////////////////////////////////////////////////////////////
		void ReceiveUnit (_72b_t vctr)
		{
			vector = vctr;
		}

		/////////////////////////////////////////////////////////////
		// This function transmits the next 36-bit vector from the 
		// internal buffer
		/////////////////////////////////////////////////////////////
		_36b_t TransmitUnit (void)
		{
			return vector[last_index ^= 0x0001];
		}

		fsm_ngepon_25gmii_rx_t()
		{
			// initialize internal variables 	
			this->last_index = 0;
			this->output_ready = true;
		}
};


#endif // _FSM_NGEPON_25GMII_H_INCLUDED_

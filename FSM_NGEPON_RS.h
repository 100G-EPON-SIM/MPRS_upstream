/**********************************************************
 * Filename:    FSM_NGEPON_RS.h
 *********************************************************/

#ifndef _FSM_NGEPON_RS_H_INCLUDED_
#define _FSM_NGEPON_RS_H_INCLUDED_

#include "FSM_base.h"

/////////////////////////////////////////////////////////////////////
// RS state machine, Transmit Direction 
/////////////////////////////////////////////////////////////////////
class fsm_ngepon_rs_tx_t: public fsm_base_t< DLY_NGEPON_RS_TX, _frm_t, _36b_t >
{
    private:

		// based on kramer_3ca_

		int8u			EntryWriteIndex;	// 3-bit index to the next available slot in TX_DATA_CTRL buffer
		int8u			EntryReadIndex[3];	// 3-bit index to the next entry in TX_DATA_CTRL buffer to be transmitted. This variable
											// is a shared semaphore variable that can be accessed (serially) by 4 output processes.
		int8u			WordWriteIndex;		// Index to 32-bit block within a codeword to be written next
		int8u			WordReadIndex;		// Index to 32-bit block within a codeword to be read next
		int8u			LinkIndex;			// stores the lane number
		int32u			CodeWordsLeft;		// Number of codewords that remain to be transmitted on a given lane. This variable can be 
											// updated asynchronously via the Channel Bonding Control Process, resulting in a grant being extended.
		bool			InStateTransferParityPlaceholder;
		bool			InStateTransferPayloadWord;
		_36b_t			TX_DATA_CTRL[8][8];	// Channel Bonding Tx Data & Control buffer (TX_DATA and TX_CTRL) concatenated into a single 36-bit wide vector construct
		_36b_t			TX_DATA_CTRL_ENTRY;	// concatenation of TX_DATA_ENTRY and TX_CTRL_ENTRY
	
	public:

        /////////////////////////////////////////////////////////////
		// This function accepts a new frame from MAC for transmission
		// This function is called every time MAC has 32 bits ready for 
		// transmission; MAC always sends 36-bit wide vectors, 
		// comprising TxData<31:0> and TxCtrl<3:0>
        /////////////////////////////////////////////////////////////
        void ReceiveUnit(_36b_t frame)
        {

			// state WAIT (empty) 

			if (this->EntryWriteIndex - this->EntryReadIndex[this->LinkIndex] < 4)
			{
				// state INITIATE_CODEWORD_RX
				this->EntryWriteIndex++;
				this->TX_DATA_CTRL[this->EntryWriteIndex][0] = _36b_t(0x1122, this->EntryWriteIndex);
				// this->TX_DATA_CTRL[this->EntryWriteIndex][0] = _36b_t(this->LLID, this->EntryWriteIndex); @TODO@ define LLID variable associated with MAC
				this->WordWriteIndex = 1;

			}

			// state WAIT_FOR_WORD (empty)

			// state RECEIVE_WORD
			if (this->WordWriteIndex < PAYLOAD_SIZE)
			{
				this->TX_DATA_CTRL[this->EntryWriteIndex][this->WordWriteIndex] = frame;
				this->WordWriteIndex++;
			}

		}


		_36b_t TransmitUnit(void)
		{
			if (!this->InStateTransferParityPlaceholder)
			{
				// state TRANSFER_IDLE
				if (this->CodeWordsLeft == 0)
				{
					return _36b_t(C_BLOCK);
				}

				if (this->WordReadIndex < PAYLOAD_SIZE)
				{
					// state SELECT_BUFFER_ENTRY
					this->CodeWordsLeft--;
					this->TX_DATA_CTRL_ENTRY = this->TX_DATA_CTRL[this->LinkIndex][this->EntryReadIndex[this->LinkIndex]];
					this->EntryReadIndex[this->LinkIndex]++;
					this->WordReadIndex = 0;

					// state TRANSFER_PAYLOAD_WORD
					_36b_t TempTransferVector = this->TX_DATA_CTRL_ENTRY;
					this->WordReadIndex++;
					return TempTransferVector;
				}
				else
				{
					this->InStateTransferParityPlaceholder = true;
					this->InStateTransferPayloadWord = false;
				}

				// state PAYLOAD_COMPLETED
				this->WordReadIndex = 0;
			}

			if (!this->InStateTransferPayloadWord)
			{
				// state TRANSFER_PARITY_PLACEHOLDER
				if (this->WordReadIndex < PARITY_SIZE)
				{
					_36b_t TempTransferVector = _36b_t(PP_BLOCK);
					this->WordReadIndex++;
					return TempTransferVector;
				}
				else
				{
					this->InStateTransferParityPlaceholder = false;
					this->InStateTransferPayloadWord = true;
				}
			}


		}


    public:
		fsm_ngepon_rs_tx_t()
        {
			this->EntryWriteIndex = 0;
			for (int8u iVar0=0; iVar0<3;iVar0++)
				this->EntryReadIndex[iVar0] = 0;
			this->WordWriteIndex = 0;

			// reset boolean flags
			this->InStateTransferParityPlaceholder = false;
			this->InStateTransferPayloadWord = false;
        }

};

#endif //_FSM_NGEPON_RS_H_INCLUDED_
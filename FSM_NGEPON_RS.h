/**********************************************************
 * Filename:    FSM_NGEPON_RS.h
 *********************************************************/

#ifndef _FSM_NGEPON_RS_H_INCLUDED_
#define _FSM_NGEPON_RS_H_INCLUDED_

#include "FSM_base.h"

/////////////////////////////////////////////////////////////////////
// RS state machine, Transmit Direction 
/////////////////////////////////////////////////////////////////////
class fsm_ngepon_rs_tx_t: public fsm_base_t< DLY_NGEPON_RS_TX, _36b_t, _36b_t >
{
    private:

		// based on kramer_3ca_3c_0716


		int8u			EntryWriteIndex[8];						// 3-bit index to the next available slot in TX_DATA/TX_CTRL buffer, indexed [LinkIndex][0...2]
		int8u			EntryReadIndex[8];						// 3-bit index to the next entry in TX_DATA/TX_CTRL buffer to be transmitted, indexed [LinkIndex][0...2]. This variable
																// is a shared semaphore variable that can be accessed (serially) by 4 output processes.
		int8u			WordWriteIndex[8];						// Index to 32-bit block within a codeword to be written next, indexed [LinkIndex]
		int8u			WordReadIndex[8];						// Index to 32-bit block within a codeword to be read next, indexed [LinkIndex]
		int32u			CodeWordsLeft[8];						// Number of codewords that remain to be transmitted on a given lane, indexed [LinkIndex]. This variable can be 
																// updated asynchronously via the Channel Bonding Control Process, resulting in a grant being extended.
		bool			InStateTransferParityPlaceholder;
		bool			InStateTransferPayloadWord;
		bool			InStateReceiveWord;						// this extra flag controls whether the input process SD stays in RECEIVE_WORD state (when true) or in INITIATE_CODEWORD_RX state (when false)
		_36b_t			TX_DATA_CTRL[8][8][PAYLOAD_SIZE];		// Channel Bonding Tx Data & Control buffer (TX_DATA and TX_CTRL) concatenated into a single 36-bit wide vector construct, indexed [LinkIndex][0..7][0..PAYLOAD_SIZE]
		int8u			TX_DATA_CTRL_ENTRY;						// pointer to concatenation of TX_DATA_ENTRY and TX_CTRL_ENTRY
	
	public:

        /////////////////////////////////////////////////////////////
		// This function accepts a new vector from MAC for transmission
		// This function is called every time MAC has 32-bit data vector
		// ready for transmission; MAC always sends 36-bit blocks, 
		// comprising TxData<31:0> and TxCtrl<3:0>
        /////////////////////////////////////////////////////////////
        void ReceiveUnit(_36b_t frame)
        {

			// recover LinkIndex value for this transmission 
			int8u LinkIndex = 0; 
			// @TODO@ add more dynamic support for more LLIDs if needed
			// local value, does not matter for simulation (max 8 LLIDs per simulation for now) 

			// state WAIT (empty) 

			if (this->EntryWriteIndex[LinkIndex] - this->EntryReadIndex[LinkIndex] < 4 && this->InStateReceiveWord == false)
			{
				// state INITIATE_CODEWORD_RX
				this->EntryWriteIndex[LinkIndex]++;
				this->TX_DATA_CTRL[LinkIndex][this->EntryWriteIndex[LinkIndex]][0] = _36b_t(LinkIndex, this->EntryWriteIndex[LinkIndex]);
				this->WordWriteIndex[LinkIndex] = 1;
				this->InStateReceiveWord = true;
			}

			// state WAIT_FOR_WORD (empty)

			// state RECEIVE_WORD
			this->TX_DATA_CTRL[LinkIndex][this->EntryWriteIndex[LinkIndex]][this->WordWriteIndex[LinkIndex]] = frame;
			this->WordWriteIndex[LinkIndex]++;
			this->InStateReceiveWord = this->WordWriteIndex[LinkIndex] < PAYLOAD_SIZE;

		}

		/////////////////////////////////////////////////////////////
		// This function sends out a new vector from RS into 25GMII
		// This function is called every 32 clock cycles, transferring
		// 32-bit data vector TXD <31:0> and 
		// 4-bit control vector TXC <4:0>
		// represented by a single _36b_t block 
		/////////////////////////////////////////////////////////////
		_36b_t TransmitUnit(void)
		{

			// recover LinkIndex value for this transmission 
			int8u LinkIndex = 0;
			// @TODO@ add more dynamic support for more LLIDs if needed
			// local value, does not matter for simulation (max 8 LLIDs per simulation for now) 

			if (this->InStateTransferParityPlaceholder == false && this->InStateTransferPayloadWord == false)
			{
				// state TRANSFER_IDLE
				if (this->CodeWordsLeft[LinkIndex] == 0)
				{
					return _36b_t(C_BLOCK);
				}

				// state SELECT_BUFFER_ENTRY
				this->CodeWordsLeft[LinkIndex]--;
				this->TX_DATA_CTRL_ENTRY = this->EntryReadIndex[LinkIndex];
				this->EntryReadIndex[LinkIndex]++;
				this->WordReadIndex[LinkIndex] = 0;
				this->InStateTransferPayloadWord = true; // push to next state 
			}

			if (this->InStateTransferPayloadWord == true)
			{
				// state TRANSFER_PAYLOAD_WORD
				_36b_t TempTransferVector = TX_DATA_CTRL[LinkIndex][this->TX_DATA_CTRL_ENTRY][this->WordReadIndex[LinkIndex]];
				this->WordReadIndex[LinkIndex]++;
				if (this->WordReadIndex[LinkIndex] >= PAYLOAD_SIZE)
				{
					// set local flags 
					this->InStateTransferPayloadWord = false;
					this->InStateTransferParityPlaceholder = true;
					// state PAYLOAD_COMPLETED
					this->WordReadIndex[LinkIndex] = 0;
				}
				return TempTransferVector;
			}

			if (this->InStateTransferParityPlaceholder == true)
			{
				// state TRANSFER_PARITY_PLACEHOLDER
				this->WordReadIndex[LinkIndex]++;
				if (this->WordReadIndex[LinkIndex] >= PARITY_SIZE)
				{
					// set local flags 
					this->InStateTransferParityPlaceholder = false;
					this->InStateTransferPayloadWord = false;
				}
				return _36b_t(PP_BLOCK);
			}

			return _36b_t(C_BLOCK);
		}

		fsm_ngepon_rs_tx_t()
        {
			// initialize all indexes
			for (int8u iVar0 = 0; iVar0 < 8; iVar0++)
			{
				this->EntryReadIndex[iVar0] = 0;
				this->EntryWriteIndex[iVar0] = 0;
				this->WordWriteIndex[iVar0] = 0;
				this->WordReadIndex[iVar0] = 0;
				this->CodeWordsLeft[iVar0] = 0;
			}

			// initialize data storage
			for (int8u iVar0 = 0; iVar0 < 8; iVar0++)
				for (int8u iVar1 = 0; iVar1 < 8; iVar1++)
					for (int8u iVar2 = 0; iVar2 < PAYLOAD_SIZE; iVar2++)
						TX_DATA_CTRL[iVar0][iVar1][iVar2] = _36b_t();

			// reset boolean flags
			this->InStateTransferParityPlaceholder = false;
			this->InStateTransferPayloadWord = false;
			this->InStateReceiveWord = false;
        }

};

#endif //_FSM_NGEPON_RS_H_INCLUDED_
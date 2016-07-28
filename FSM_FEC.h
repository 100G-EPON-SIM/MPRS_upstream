/**********************************************************
* Filename:    FSM_FEC.h
*
* Description: ONU FEC decoder state machine
*
*********************************************************/

#ifndef _FSM_FEC_H_INCLUDED_
#define _FSM_FEC_H_INCLUDED_

#include "_queue.h"
#include "FSM_base.h"

class fsm_fec_decoder_t: public fsm_base_t< DLY_FEC_DECODER, _66b_t > 
{
    typedef Queue< _66b_t, FEC_DSIZE > fifo_t;
private:
	int32s  parity_count;
	fifo_t  FIFO[2];
    fifo_t* fifo_in;
    fifo_t* fifo_out;

    /////////////////////////////////////////////////////////////
    // 
    /////////////////////////////////////////////////////////////
    void ReceiveUnit( _66b_t block )
    {
        /////////////////////////////////////////////////////////
        // process FEC codeword after 4 parity blocks are received
        /////////////////////////////////////////////////////////
        if( parity_count == FEC_PSIZE )
        {
            /////////////////////////////////////////////////////////
            // At this point, FIFO_IN contains 27 "corrected" blocks
            // and FIFO_OUT is empty. Transfer corrected blocks to
            // the output FIFO (i.e., swap the fifo's)
            /////////////////////////////////////////////////////////
            SWAP< fifo_t* >( fifo_in, fifo_out );
            output_ready = true;
            parity_count = 0;
        }

        /////////////////////////////////////////////////////////
        // ignore N, L, and Z blocks with Idles
        /////////////////////////////////////////////////////////
        if( block.SyncHeader() == SH_NONE )
            return;

        if( block.SyncHeader() == SH_PRTY )
            parity_count ++;
        else
        {
            if( fifo_in->IsFull() )
                fifo_in->Get();
            fifo_in->Add( block );
        }
	}

    /////////////////////////////////////////////////////////////
    // 
    /////////////////////////////////////////////////////////////
	_66b_t TransmitUnit( void )
    {
        output_ready = fifo_out->GetSize() > 1;
        return fifo_out->Get();
	}

	
public:

	fsm_fec_decoder_t()
    {
        parity_count = 0;
        output_ready = false;
        fifo_in      = &FIFO[0];
        fifo_out     = &FIFO[1];
	}
};

#endif // _FSM_FEC_H_INCLUDED_

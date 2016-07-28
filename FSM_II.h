
/**********************************************************
 * Filename:    FSM_II.h
 *
 * Description: Defines Idle Insertion process
 *
 *********************************************************/


#ifndef _FSM_IDLE_INSERTION_H_INCLUDED_
#define _FSM_IDLE_INSERTION_H_INCLUDED_

#include "_queue.h"
#include "FSM_base.h"

#define IDLE_VECTOR _72b_t( 0, C_BLOCK )

const int32s FIFO_II_SIZE = BLK_ROUNDUP( MAX_FRAME_BYTES, FEC_PAYLOAD_BYTES ) * FEC_PSIZE + 1;



class fsm_idle_insertion_t: public fsm_base_t< DLY_IDLE_INS, _72b_t >
{
private:
    Queue< _72b_t, FIFO_II_SIZE > FIFO_II;

    /////////////////////////////////////////////////////////////
    // 
    /////////////////////////////////////////////////////////////
    void ReceiveUnit( _72b_t vector )
    {
		if( FIFO_II.IsFull() )
		{
			MSG_WARN( "Attempt to receive into a full FIFO_II" );
			return;
		}

		if( vector.IsType( S_BLOCK | C_BLOCK ))
		{
			while( FIFO_II.GetSize() < FIFO_II_SIZE - 1 )
				FIFO_II.Add( IDLE_VECTOR );
		}
			
		FIFO_II.Add( vector );
    }

	/////////////////////////////////////////////////////////////
    // 
    /////////////////////////////////////////////////////////////
    _72b_t TransmitUnit( void )
    {
        /////////////////////////////////////////////////////////
        // In the ONU FIFO may get empty because of gaps between  
        // bursts. In this case, an idle should be generated.
        /////////////////////////////////////////////////////////
        return FIFO_II.IsEmpty()? IDLE_VECTOR : FIFO_II.Get(); 
    }

	
public:
    fsm_idle_insertion_t()
    {
		while( FIFO_II.GetSize() < FIFO_II_SIZE - 1 )
			FIFO_II.Add( IDLE_VECTOR );

        output_ready = true;
    }
};

#endif // _FSM_IDLE_INSERTION_H_INCLUDED_


/**********************************************************
 * Filename:    FSM_ID.h
 *
 * Description: Defines Idle Deletion process
 *
 *********************************************************/


#ifndef _FSM_IDLE_DELETION_H_INCLUDED_
#define _FSM_IDLE_DELETION_H_INCLUDED_

#include "_queue.h"
#include "FSM_base.h"

class fsm_olt_idle_deletion_t: public fsm_base_t< DLY_IDLE_DEL, _72b_t >
{
protected:
	int32s  VectorCount;
	int32s  IdleCount;
	int32s  DeleteCount;

    /////////////////////////////////////////////////////////////
    // 
    /////////////////////////////////////////////////////////////
    void ReceiveUnit( _72b_t vector )
    {
		bool idle_or_error = vector.IsType( C_BLOCK | E_BLOCK );

		if( idle_or_error && /*IdleCount >= MIN_IPG_VECTORS &&*/ DeleteCount > 0 )
		{
			DeleteCount--;
			output_ready = false;
			return;
		}

		if( idle_or_error )
			IdleCount++;
		else
			IdleCount = 0;

		output_block = vector;
		output_ready = true;
		VectorCount++;

		if( VectorCount == FEC_DSIZE )
		{
			VectorCount  = 0;
			DeleteCount += FEC_PSIZE;
		}
    }

public:
    fsm_olt_idle_deletion_t()
    {
		VectorCount = 0;
		IdleCount   = 0;
		DeleteCount = 0;
    }
};




class fsm_onu_idle_deletion_t: public fsm_olt_idle_deletion_t
{
private:
	bool	HalfShift;
	_36b_t	tx_next;

    _72b_t AlignVector( _72b_t vector )
	{
		////////////////////////////////////////////////////////////
		// Between bursts...
		/////////////////////////////////////////////////////////////
		if( IdleCount > DELAY_BOUND )
		{
			HalfShift   = false;      // Reset half-shift 
            VectorCount = 2;
            DeleteCount = 0;

            ////////////////////////////////////////////////////////////
		    // Start shifting 
		    /////////////////////////////////////////////////////////////
            if( vector[1].IsType( S_BLOCK ) )
		    {
                VectorCount = 1;
			    tx_next   = _36b_t();
                HalfShift = true;
                IdleCount = 0;
		    }
		}

		////////////////////////////////////////////////////////////
		// Continue shifting till the end of the burst
		/////////////////////////////////////////////////////////////
		if( HalfShift )
		{
			_36b_t tx_temp;

			tx_temp   = vector[1];
			vector[1] = vector[0];
			vector[0] = tx_next;
			tx_next   = tx_temp;
		}
		return vector;
	}

    /////////////////////////////////////////////////////////////
    // 
    /////////////////////////////////////////////////////////////
    inline void ReceiveUnit( _72b_t vector )
    {
		fsm_olt_idle_deletion_t::ReceiveUnit( AlignVector( vector ) );
	}

public:
	fsm_onu_idle_deletion_t() 
	{
		HalfShift = false;
	}
};

#endif // _FSM_IDLE_DELETION_H_INCLUDED_


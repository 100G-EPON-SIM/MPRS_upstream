/**********************************************************
* Filename:    FSM_DD.h
*
* Description: OLT Data Detector state machine
*
*********************************************************/

#ifndef _FSM_DD_H_INCLUDED_
#define _FSM_DD_H_INCLUDED_

#include "_queue.h"
#include "FSM_base.h"

const int32s FIFO_DD_OLT_SIZE = BLK_ROUNDUP( FEC_PAYLOAD_BYTES + MAX_FRAME_BYTES, FEC_PAYLOAD_BYTES ) * FEC_PSIZE + 1;
const int32s FIFO_DD_ONU_SIZE = DELAY_BOUND + 45;


#define SP              _66b_t( 0, N_BLOCK )
#define PARITY_BLOCK    _66b_t( 0, P_BLOCK )
#define IDLE_BLOCK      _66b_t( 0, C_BLOCK )
#define ZERO_BLOCK      _66b_t( 0, Z_BLOCK )
#define BURST_DELIMITER _66b_t( 0, L_BLOCK )

/////////////////////////////////////////////////////////////
// OLT DATA DETECTOR
/////////////////////////////////////////////////////////////
class fsm_olt_data_detector_t: public fsm_base_t< DLY_DATA_DET, _66b_t >
{
    typedef fsm_olt_data_detector_t olt_dd;
    typedef _66b_t (olt_dd::*pf_state_t)(void);

private:
    pf_state_t  active_state;
    int32s      protected_block_count;
    int32s      parity_block_count;

    Queue< _66b_t, FIFO_DD_OLT_SIZE >   FIFO_DD;

    /////////////////////////////////////////////////////////////
    // state FEC_IS_ON
    /////////////////////////////////////////////////////////////
    inline _66b_t state_FEC_IS_ON( void )
    {
        // pass FIFO_DD[0] to FEC Encoder
        // FEC_encoder( FIFO[0] )

        protected_block_count++;
        parity_block_count = 0;

        // state transition condition 
        if( protected_block_count == FEC_DSIZE )
            active_state = &olt_dd::state_TRANSMIT_PARITY;

        //////////////////////////////////////////////////////
        // remove element[0] from FIFO_DD and send it
        //////////////////////////////////////////////////////
        if( FIFO_DD.IsEmpty() )
		{
			MSG_WARN( "Attempt to transmit from an empty FIFO_DD" );
			return _66b_t();
		}
		return FIFO_DD.Get();
    }

    /////////////////////////////////////////////////////////////
    // state TRANSMIT_PARITY
    /////////////////////////////////////////////////////////////
    inline _66b_t state_TRANSMIT_PARITY( void )
    {
        parity_block_count++;
        protected_block_count = 0;

        // state transition condition 
        if( parity_block_count == FEC_PSIZE )
            active_state = &olt_dd::state_FEC_IS_ON;

        return PARITY_BLOCK;
    }

    /////////////////////////////////////////////////////////////
    // 
    /////////////////////////////////////////////////////////////
    void ReceiveUnit( _66b_t block )
    {
        //////////////////////////////////////////////////////
        // add new block to FIFO_DD
        //////////////////////////////////////////////////////
        if( FIFO_DD.IsFull() )
        {
			MSG_WARN( "Attempt to receive into a full OLT's FIFO_DD." );
            return;
        }

        FIFO_DD.Add( block );
    }

	/////////////////////////////////////////////////////////////
    // 
    /////////////////////////////////////////////////////////////
	inline _66b_t TransmitUnit( void ) { return (this->*active_state)(); }

public:

    fsm_olt_data_detector_t() 
    {
        active_state            = &olt_dd::state_FEC_IS_ON;
        protected_block_count   = 0;
        parity_block_count      = 0;
        output_ready            = true;
    }
};



/////////////////////////////////////////////////////////////
// ONU DATA DETECTOR
/////////////////////////////////////////////////////////////
class fsm_onu_data_detector_t: public fsm_base_t< DLY_DATA_DET, _66b_t >
{
    typedef fsm_onu_data_detector_t onu_dd;
    typedef _66b_t (onu_dd::*pf_state_t)(void);

private:
    pf_state_t  active_state;
    int32s      idle_block_count;
    int32s      sync_block_count;
    int32s      protected_block_count;
    int32s      parity_block_count;
    int32s      terminator_block_count;
    bool        transmitting;

    Queue< _66b_t, FIFO_DD_ONU_SIZE >   FIFO_DD;

    /////////////////////////////////////////////////////////////
    // state LASER_IS_OFF
    /////////////////////////////////////////////////////////////
    inline _66b_t state_LASER_IS_OFF( void )
    {
        sync_block_count = 0;

        // state transition condition
        if( transmitting )
        {
            // initiate the laser
            // PMD_SIGNAL.request( true );
            active_state = &onu_dd::state_TRANSMIT_BURST_PREAMBLE;
        }
        return SP;
    }

    /////////////////////////////////////////////////////////////
    // state TRANSMIT_BURST_PREAMBLE
    /////////////////////////////////////////////////////////////
    inline _66b_t state_TRANSMIT_BURST_PREAMBLE( void )
    {
        sync_block_count ++;

        // state transition condition
        if( sync_block_count == SYNC_LENGTH )
            active_state = &onu_dd::state_TRANSMIT_BURST_DELIMITER;

        return SP;
    }

    /////////////////////////////////////////////////////////////
    // state TRANSMIT_BURST_DELIMITER
    /////////////////////////////////////////////////////////////
    inline _66b_t state_TRANSMIT_BURST_DELIMITER( void )
    {
        protected_block_count = 0;
        // state transition condition (UCT)
        active_state = &onu_dd::state_FEC_IS_ON;

        return BURST_DELIMITER;
    }

    /////////////////////////////////////////////////////////////
    // state FEC_IS_ON
    /////////////////////////////////////////////////////////////
    inline _66b_t state_FEC_IS_ON( void )
    {
        // pass FIFO_DD[0] to FEC Encoder
        // FEC_encoder( FIFO[0] )

        protected_block_count++;
        parity_block_count = 0;

        // state transition condition 
        if( protected_block_count == FEC_DSIZE )
            active_state = &onu_dd::state_TRANSMIT_PARITY;

        //////////////////////////////////////////////////////
        // remove element[0] from FIFO_DD and send it
        //////////////////////////////////////////////////////
        if( FIFO_DD.IsEmpty() )
		{
			MSG_WARN( "Attempt to transmit from an empty FIFO_DD" );
			return _66b_t();
		}
		return FIFO_DD.Get();
    }

    /////////////////////////////////////////////////////////////
    // state TRANSMIT_PARITY
    /////////////////////////////////////////////////////////////
    inline _66b_t state_TRANSMIT_PARITY( void )
    {
        parity_block_count++;
        protected_block_count = 0;

        // state transition condition 
        if( parity_block_count == FEC_PSIZE )
            active_state = (idle_block_count > DELAY_BOUND)? &onu_dd::state_LASER_OFF: 
                                                             &onu_dd::state_FEC_IS_ON;

        return PARITY_BLOCK;
    }

    /////////////////////////////////////////////////////////////
    // state LASER_OFF
    /////////////////////////////////////////////////////////////
    inline _66b_t state_LASER_OFF( void )
    {
        // terminate the laser
        // PMD_SIGNAL.request( false );
        transmitting = false;
        terminator_block_count = 0;

        // state transition condition (UCT) 
        active_state = &onu_dd::state_TRANSMIT_BURST_TERMINATOR;

        return state_TRANSMIT_BURST_TERMINATOR();
    }

    /////////////////////////////////////////////////////////////
    // state TRANSMIT_BURST_TERMINATOR
    /////////////////////////////////////////////////////////////
    inline _66b_t state_TRANSMIT_BURST_TERMINATOR( void )
    {
        terminator_block_count++;

        // state transition condition (UCT) 
        if( terminator_block_count == TERMINATOR_LENGTH )
            active_state = &onu_dd::state_LASER_IS_OFF;

        return ZERO_BLOCK;
    }

    /////////////////////////////////////////////////////////////
    // 
    /////////////////////////////////////////////////////////////
    void ReceiveUnit( _66b_t block )
    {
        if( block.SyncHeader() == SH_CTRL )
        {
            idle_block_count++;
            //////////////////////////////////////////////////////
            // between bursts, keep only 3 control blocks in the FIFO
            // when data block arrives, these 3 blocks will be
            // FIFO_DD[ 0 ] = 1st protected IDLE
            // FIFO_DD[ 1 ] = 2nd protected IDLE
            // FIFO_DD[ 2 ] = block containing /S/ - added below
            //////////////////////////////////////////////////////
            while( !transmitting && FIFO_DD.GetSize() > 2 )
                FIFO_DD.Get();
        }
        else
        {
            idle_block_count = -1;
            transmitting = true;
        }
        //////////////////////////////////////////////////////
        // add new block to FIFO
        //////////////////////////////////////////////////////
        if( FIFO_DD.IsFull() )
			MSG_WARN( "Attempt to receive into a full FIFO_DD." );

        FIFO_DD.Add( block );
    }

	/////////////////////////////////////////////////////////////
    // 
    /////////////////////////////////////////////////////////////
	inline _66b_t TransmitUnit( void ) { return (this->*active_state)(); }

public:
    fsm_onu_data_detector_t() 
    {
        active_state            = &onu_dd::state_LASER_IS_OFF;
        idle_block_count        = -1;
        sync_block_count        = 0;
        protected_block_count   = 0;
        parity_block_count      = 0;
        terminator_block_count  = 0;
        transmitting            = false;
        output_ready            = true;
    }
};


#endif // _FSM_DD_H_INCLUDED_


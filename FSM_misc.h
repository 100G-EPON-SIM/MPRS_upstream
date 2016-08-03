/**********************************************************
 * Filename:    FSM_misc.h
 *
 * Description: 
 *
 *********************************************************/


#ifndef _MISC_FSMS_H_INCLUDED_
#define _MISC_FSMS_H_INCLUDED_


#include "FSM_base.h"

/////////////////////////////////////////////////////////////////////
// 64B/66B encoder state machine 
/////////////////////////////////////////////////////////////////////
class fsm_64b66b_encoder_t: public fsm_base_t< DLY_66B_ENCODER, _72b_t, _66b_t >
{
    private:
        void ReceiveUnit( _72b_t in_blk ) 
        { 
            output_block = _66b_t( in_blk ); 
            output_ready = true;
        }
};

/////////////////////////////////////////////////////////////////////
// 66B/64B decoder state machine 
/////////////////////////////////////////////////////////////////////
class fsm_66b64b_decoder_t: public fsm_base_t< DLY_66B_DECODER, _66b_t, _72b_t >
{
    private:
        void ReceiveUnit( _66b_t in_blk ) 
        { 
            output_block = static_cast<_72b_t>( in_blk );
            output_ready = true;
        }
};

/////////////////////////////////////////////////////////////////////
// Scramber state machine 
/////////////////////////////////////////////////////////////////////
class fsm_scrambler_t:   public fsm_base_t< DLY_SCRAMBLER, _66b_t > 
{
    private:
        /////////////////////////////////////////////////////////////
		//
		/////////////////////////////////////////////////////////////
        void ReceiveUnit( _66b_t in_blk )
        {
            output_block = in_blk;
            output_ready = true;
        }
};


class fsm_descrambler_t: public fsm_base_t< DLY_DESCRAMBLER, _66b_t > 
{
    private:
        /////////////////////////////////////////////////////////////
		//
		/////////////////////////////////////////////////////////////
        void ReceiveUnit( _66b_t in_blk )
        {
            output_block = in_blk;
            output_ready = true;
        }
};


#endif //_MISC_FSMS_H_INCLUDED_
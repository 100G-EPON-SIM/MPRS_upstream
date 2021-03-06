/**********************************************************
 * Filename:    FSM_base.h
 *
 * Description: Base class for state machines
 *
 *********************************************************/


#ifndef _FSM_BASE_H_INCLUDED_
#define _FSM_BASE_H_INCLUDED_

#include "_types.h"
#include <iostream>

using namespace std;

/////////////////////////////////////////////////////////////////////
// constants
/////////////////////////////////////////////////////////////////////
const int16s MIN_IPG_BYTES      = 12;		// in bytes
const int16s PREAMBLE_BYTES     = 8;		// in bytes 
const int16s CHECKSUM_BYTES     = 4;		// in bytes
const int16s E_HEADER_BYTES     = 14;		// in bytes
const int16s EPD_BYTES          = 3;		// in bytes

const int16s MIN_PACKET_BYTES   = 64;
const int16s MPCP_PACKET_BYTES  = MIN_PACKET_BYTES;
const int16s MAX_PACKET_BYTES   = 2000;
const int16s MAX_FRAME_BYTES    = MAX_PACKET_BYTES + PREAMBLE_BYTES + MIN_IPG_BYTES;

const int16s TAIL_GUARD         = 38; //42; // per D1.992
                                /*PREAMBLE_BYTES
                                + E_HEADER_BYTES
                                + CHECKSUM_BYTES
                                + MIN_IPG_BYTES; // total = 38
                                */

const int16s COLUMN_BYTES       = 4;				// 4 bytes per column
const int16s VECTOR_BYTES       = COLUMN_BYTES * 2;	// 8 bytes per column

const int16s MIN_IPG_VECTORS    = MIN_IPG_BYTES / VECTOR_BYTES;	// in vectors

// FEC Framing
const int16s FEC_DSIZE			= 27;
const int16s FEC_PSIZE			=  4;
const int16s FEC_PAYLOAD_BYTES  = FEC_DSIZE * VECTOR_BYTES;
const int16s FEC_PARITY_BYTES   = FEC_PSIZE * VECTOR_BYTES;
const int16u PAYLOAD_SIZE		= FEC_DSIZE * 2;
const int16u PARITY_SIZE		= FEC_PSIZE * 2;
const int16s FEC_CODEWORD_BYTES = FEC_PAYLOAD_BYTES + FEC_PARITY_BYTES;

const int16s TQ_SIZE_BYTES      = 20;

// burst mode parameters
const int32s SYNC_LENGTH        = 60;
const int32s DELAY_BOUND        = SYNC_LENGTH + 5;
const int32s BURST_FRAMES       = 8;
const int32s BURST_GAP_BYTES    = FEC_CODEWORD_BYTES + 2 * DELAY_BOUND * VECTOR_BYTES;
const int32s TERMINATOR_LENGTH  = 3; // 3 blocks (of zeroes) to terminate a burst

//const int32s DELAY_BOUND        = 271 + 1;          // 271 - default value per Clause 92.2.2.1.2;
//const int32s SYNC_LENGTH        = DELAY_BOUND - 4;  // minus delimiter, two idles, and /S/
//const int32s BURST_FRAMES       = 10;               // Send 10 frames per burst



/////////////////////////////////////////////////////////////////////
template <int32s BLOCK_SIZE> int32s BLOCKS_ROUND_UP( int32s payload )
{
    return (payload + BLOCK_SIZE - 1) / BLOCK_SIZE;
}

/////////////////////////////////////////////////////////////////////
#define BLK_ROUNDUP( VAL, BLK )	(((VAL) + (BLK) - 1) / (BLK))


/////////////////////////////////////////////////////////////////////
// type definitions
/////////////////////////////////////////////////////////////////////
enum blk_t
{
    C_BLOCK = 0x0001, // 'C' - control block
    S_BLOCK = 0x0002, // 'S' - start of frame
    D_BLOCK = 0x0004, // 'D' - data block
    T_BLOCK = 0x0008, // 'T' - terminating character
	// @TODO@ - MAC needs to distinguish T1, T2, T3 sequences to make sure that RS can proeprly encode them into 25GMII
	T1_BLOCK = 0x0009, // 'T' - terminating character, 1 IDLE
	T2_BLOCK = 0x000A, // 'T' - terminating character, 2 IDLEs
	T3_BLOCK = 0x000B, // 'T' - terminating character, 3 IDLEs
    E_BLOCK = 0x001F, // 'E' - errored block
    P_BLOCK = 0x0020, // 'P' - parity block
	X_BLOCK = 0x0021, // 'X' - parity placeholder block, added in 802.3ca
	Y_BLOCK = 0x0022, // 'Y' - codeword delimiter block, added in 802.3ca
   
    Z_BLOCK = 0x0040, // 'Z' - zero block (end of burst delineation)
    L_BLOCK = 0x0080, // 'L' - burst deLimiter
    N_BLOCK = 0x0100  // 'N' - burst sync pattern (0x555...)
};

enum hdr_t
{
    SH_DATA,
    SH_CTRL,
    SH_PRTY,
    SH_NONE  // used to represent sync patter, burst delimiter, or zero-blocks
};

typedef int64s clk_t;

////////////////////////////////////////////////////////////
// Get type name
/////////////////////////////////////////////////////////////
char BlockName( blk_t bt ) 
{
    switch( bt )
    {
        case C_BLOCK: return 'C';
        case S_BLOCK: return 'S';
        case D_BLOCK: return 'D';
        case T_BLOCK: return 'T';
		case T1_BLOCK: return '1';
		case T2_BLOCK: return '2';
		case T3_BLOCK: return '3';
        case E_BLOCK: return 'E';
        case P_BLOCK: return 'P';
        case Z_BLOCK: return 'Z';
        case L_BLOCK: return 'L';
        case N_BLOCK: return 'N';
		case X_BLOCK: return 'X';
		case Y_BLOCK: return 'Y';
        default:	  return '-'; 
    }
}

/////////////////////////////////////////////////////////////////////
// Array of timestamps
/////////////////////////////////////////////////////////////////////

const int16s DLY_NGEPON_MACC			= 0;
const int16s DLY_NGEPON_MPCP_TX			= 1;
const int16s DLY_NGEPON_MAC_TX		    = 2;
const int16s DLY_NGEPON_RS_TX			= 3;
const int16s DLY_NGEPON_25GMII_TX		= 4;
const int16s DLY_NGEPON_25GMII_RX		= 5;
const int16s DLY_NGEPON_RS_RX			= 6;
const int16s DLY_NGEPON_MAC_RX		    = 7;	
const int16s DLY_NGEPON_MPCP_RX			= 8;

const int16s DLY_IDLE_DEL	    = 30;	
const int16s DLY_66B_ENCODER	=31;	
const int16s DLY_SCRAMBLER	    = 32;	
const int16s DLY_DATA_DET	    = 33;	
const int16s DLY_FEC_DECODER	= 34;	
const int16s DLY_DESCRAMBLER	= 35;	
const int16s DLY_66B_DECODER	= 36;	
const int16s DLY_IDLE_INS	    = 37;	

const int32s DELAY_ARRAY_SIZE   = 8;


class timestamp_t
{
private:
    static clk_t    _global_clock;
    clk_t           _timestamp;
    int16s          _delay[ DELAY_ARRAY_SIZE ];
	int16s			_frame_size;

public:
    timestamp_t( clk_t stamp = 0 ) 
    {
        // initialize local variables
		this->_timestamp = stamp;
		// initialide delay array 
		for (int16u iVar0 = 0; iVar0 < DELAY_ARRAY_SIZE; iVar0++)
			_delay[iVar0] = 0;
    }

    /////////////////////////////////////////////////////////////
    // 
    /////////////////////////////////////////////////////////////
    static void   IncrementClock( void )       { _global_clock++;      }
    static clk_t  GetClock( void )             { return _global_clock; }
    static void   ResetClock( clk_t clk = 0 )  { _global_clock = clk;  }
    inline clk_t  GetTimestamp( void )   const { return _timestamp;    }
    inline int16s GetDelay( int32s ndx ) const { return _delay[ ndx ]; }

    /////////////////////////////////////////////////////////////
    // Measure delay in the current block 
    /////////////////////////////////////////////////////////////
    inline void MeasureDelay( int32s ndx )
    {
        _delay[ ndx ] = static_cast<int16s>( _global_clock - _timestamp );
        if( ndx == 0 && _delay[ ndx ] < 0 )
        {
            cout << _global_clock <<"," <<  _timestamp << endl;
        }
        _timestamp = _global_clock;
    }
};

clk_t timestamp_t::_global_clock = 0;

/////////////////////////////////////////////////////////////////////
// 36-bit column representing one XGMII transfer
/////////////////////////////////////////////////////////////////////
class _36b_t: public timestamp_t
{
    private:
        blk_t  _block_type;
        int32s _seq_number;
		int16u _LLID;
		int8u _EntryWriteIndex;

    public:
        _36b_t( blk_t blk = C_BLOCK ): timestamp_t( 0 )
        {
            _block_type = blk;
            _seq_number = -1;
        }

		_36b_t(int16u LLID, int8u EntryWriteIndex) : timestamp_t(0)
		{
			_block_type = X_BLOCK;
			_seq_number = -1;
			_LLID = LLID;
			_EntryWriteIndex = EntryWriteIndex;
		}
        
        _36b_t( timestamp_t stamp, blk_t blk, int32s seq ): timestamp_t( stamp )
        {
            _block_type = blk;
            _seq_number = seq;
        }

        inline int32s GetSeqNumber( void )	const { return _seq_number; }
        inline blk_t  C_TYPE( void )		const { return _block_type; }
        /////////////////////////////////////////////////////////////
        // check type of vector
        /////////////////////////////////////////////////////////////
        inline bool IsType( int32s blk_type_field ) const
        {
            // return (( C_TYPE() & blk_type_field ) != 0 );
			return (_block_type == blk_type_field);
        }
        
        
};

/////////////////////////////////////////////////////////////////////
// 72-bit vector consisting of two XGMII transfers
/////////////////////////////////////////////////////////////////////
class _72b_t
{
    private:
        _36b_t _column[2];

    public:
        /////////////////////////////////////////////////////////////
        _72b_t( _36b_t col0, _36b_t col1 ) 
        {
            _column[0] = col0;
            _column[1] = col1;
        }
        /////////////////////////////////////////////////////////////
        _72b_t( clk_t stamp = 0, blk_t blk = C_BLOCK, int32s seq = -1 )
        {
            _column[0] = _column[1] = _36b_t( stamp, blk, seq );
        }
        /////////////////////////////////////////////////////////////
        // Subscript operator for accessing individual columns
        /////////////////////////////////////////////////////////////
        inline _36b_t& operator[]( int index )	{ return _column[ index & 0x01 ]; }

        /////////////////////////////////////////////////////////////
        // get type of vector
        /////////////////////////////////////////////////////////////
        inline bool Match( int32s t0, int32s t1 ) const 
        { 
            return _column[0].IsType( t0 ) && _column[1].IsType( t1 ); 
        }

        inline bool Match( int32s t0 ) const 
        { 
            return Match( t0, t0 ); 
        }

        /////////////////////////////////////////////////////////////
        // get type of vector
        /////////////////////////////////////////////////////////////
        inline blk_t T_TYPE( void ) const
        {
            if( Match( S_BLOCK, D_BLOCK ))  return S_BLOCK;
            if( Match( C_BLOCK, S_BLOCK ))  return S_BLOCK;
            if( Match( D_BLOCK, T_BLOCK ))  return T_BLOCK;
            if( Match( T_BLOCK, C_BLOCK ))  return T_BLOCK;
            if( Match( D_BLOCK ))           return D_BLOCK;
            if( Match( C_BLOCK ))           return C_BLOCK;
            if( Match( P_BLOCK ))           return P_BLOCK;
            if( Match( L_BLOCK ))           return L_BLOCK;
            if( Match( N_BLOCK ))           return N_BLOCK;
            if( Match( Z_BLOCK ))           return Z_BLOCK;

            return E_BLOCK;
        }

        /////////////////////////////////////////////////////////////
        // check type of vector
        /////////////////////////////////////////////////////////////
        inline bool IsType( int32s blk_type_field ) const
        {
            return (( T_TYPE() & blk_type_field ) != 0 );
        }
         /////////////////////////////////////////////////////////////
        // check type of vector
        /////////////////////////////////////////////////////////////
        inline void MeasureDelay( int16s location )
        {
            _column[0].MeasureDelay( location );
            _column[1].MeasureDelay( location );
        }
};


/////////////////////////////////////////////////////////////////////
// 66-bit block 
/////////////////////////////////////////////////////////////////////
class _66b_t: public _72b_t
{
    private:
        hdr_t sync_header;

    public:
        /////////////////////////////////////////////////////////////
        _66b_t( clk_t stamp = 0, blk_t blk = C_BLOCK, int32s seq = -1 ): _72b_t( stamp, blk, seq )
        {
            if( IsType( D_BLOCK ))			            sync_header = SH_DATA;
            else if( IsType( P_BLOCK ))		            sync_header = SH_PRTY;
            else if( IsType( C_BLOCK|S_BLOCK|T_BLOCK )) sync_header = SH_CTRL;
            else							            sync_header = SH_NONE;
        }
        /////////////////////////////////////////////////////////////
        _66b_t( _72b_t vector ): _72b_t( vector )
        {
            if( IsType( D_BLOCK ))			            sync_header = SH_DATA;
            else if( IsType( P_BLOCK ))		            sync_header = SH_PRTY;
            else if( IsType( C_BLOCK|S_BLOCK|T_BLOCK )) sync_header = SH_CTRL;
            else							            sync_header = SH_NONE;
        }
        /////////////////////////////////////////////////////////////
        inline hdr_t SyncHeader( void ) const { return sync_header; }
};


/////////////////////////////////////////////////////////////////////
// MAC Frame
/////////////////////////////////////////////////////////////////////
class _frm_t: public timestamp_t
{
    private:
        int16s _frame_size;

    public:
        /////////////////////////////////////////////////////////////
        _frm_t( clk_t stamp = 0, int16s frm_size = 0 ): timestamp_t( stamp )
        {
            _frame_size = frm_size;
        }

        /////////////////////////////////////////////////////////////
        inline void AddColumn( _36b_t col )
        {
            if( col.IsType( S_BLOCK ))
            {
                _frame_size = COLUMN_BYTES;
                *(timestamp_t*)this = (timestamp_t)col;
            }
            else if( col.IsType(D_BLOCK) || col.IsType(T_BLOCK))
            {
                _frame_size += COLUMN_BYTES;
            }
        }

        inline int16s GetFrameSize( void )  const { return _frame_size; }
};

/////////////////////////////////////////////////////////////////////
// Finite State Machine base class 
/////////////////////////////////////////////////////////////////////
template< int16s L, class in_t, class out_t = in_t > class fsm_base_t
{
    protected:
        out_t   output_block;
        bool    output_ready;
        
        /////////////////////////////////////////////////////////////
        virtual void    ReceiveUnit( in_t in_blk ) = 0;
        /////////////////////////////////////////////////////////////
        virtual out_t   TransmitUnit( void )  
        { 
            output_ready = false;
            return output_block; 
        }
       
        
    public:
        fsm_base_t() 
        {
            output_ready = false;
        }
        /////////////////////////////////////////////////////////////
        inline void operator<<( in_t in_blk )	
        {
            ReceiveUnit( in_blk );
        }
        /////////////////////////////////////////////////////////////
        inline operator out_t()	
        {
            out_t out_blk1 = TransmitUnit();
			out_blk1.MeasureDelay(L);
            return out_blk1; 
        }
        /////////////////////////////////////////////////////////////
        bool OutputReady( void ) const { return output_ready; }
};


#endif //_FSM_BASE_H_INCLUDED_
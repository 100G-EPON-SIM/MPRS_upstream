/////////////////////////////////////////////////////////////////////////
// Filename:  stats.h 
//
// Description: Contains declarations for classes to collect and analyze 
//              statistical variables
//                 class Stats, 
//                 class Distrib
//                 class AutoCorr
//
// Author:      Glen Kramer (kramer@cs.ucdavis.edu)
//              University of California, Davis
//
/////////////////////////////////////////////////////////////////////////



#ifndef _STATS_H_INCLUDED_
#define _STATS_H_INCLUDED_

#include <string.h>
#include "_types.h"

#define INVALID_VAL     0		// value returned when asking for AVG or VAR of an empty set 
typedef DOUBLE stat_t;


//////////////////////////////////////////////////////////////////////
// class Stats 
// Calculates average value, maximum value, and variance of a series.
//////////////////////////////////////////////////////////////////////
class Stats  
{
private:
	stat_t _Min;    // smallest element in the set
    stat_t _Max;    // largest element in the set
    stat_t _Sqr;    // weighted square of all elements collected
    stat_t _Tot;    // weighted sum of all elements collected
    stat_t _Cnt;    // number of elements collected (sum of weights)

public:
    Stats()                 { Clear(); }
    virtual ~Stats()        {}
    //////////////////////////////////////////////////////////////////
    inline void Sample( stat_t sample, stat_t weight = 1 )
    {
		if( _Max < sample || _Cnt == 0 ) _Max = sample;
		if( _Min > sample || _Cnt == 0 ) _Min = sample;
        _Tot += sample * weight;
        _Sqr += sample * sample * weight;
        _Cnt += weight;
    }
    //////////////////////////////////////////////////////////////////
	inline void Clear( void )			{ _Min = _Max = _Sqr = _Tot = _Cnt = 0.0; }

    inline stat_t GetTotal(void) const  { return _Tot; }
    inline stat_t GetCount(void) const  { return _Cnt; }
    inline stat_t GetMin(void)   const  { return _Min; }
	inline stat_t GetMax(void)   const  { return _Max; }
    inline stat_t GetRange(void) const  { return _Max - _Min; }
    inline stat_t GetAvg(void)   const  { return _Cnt? _Tot / _Cnt : INVALID_VAL; }
    inline stat_t GetVar(void)   const  { return _Cnt? (_Sqr - _Tot*_Tot/_Cnt)/_Cnt: INVALID_VAL; }
    //////////////////////////////////////////////////////////////////
    inline Stats operator+ ( Stats st ) { return st += *this; }
    //////////////////////////////////////////////////////////////////
    inline Stats& operator+= ( const Stats& st )
    {
       if( _Max < st._Max )
           _Max = st._Max;

        _Tot += st._Tot;
        _Sqr += st._Sqr;
        _Cnt += st._Cnt;
        return *this;
    }
};


//////////////////////////////////////////////////////////////////////
// class Distrib 
// Builds a histogram of a series 
//////////////////////////////////////////////////////////////////////
template < int32s BINS > class Distrib : public Stats
{
private:
    stat_t _MinVal;
    stat_t _BinSize;
    stat_t _Dstrb[ BINS ];  
    
    //////////////////////////////////////////////////////////////////
    // Returns a bin number to which the current sample 
    // should be added 
    //////////////////////////////////////////////////////////////////
    inline int32s _CalcBin( stat_t sample ) const
	{
		return static_cast<int32s>((sample - _MinVal) / _BinSize);
	}
    
    //////////////////////////////////////////////////////////////////
    // Verifies that the bin index is not outside allocated
    // array. If it is, the nearest bin index is returned.
    //////////////////////////////////////////////////////////////////
	inline int32s _VerifyBin( int32s bin ) const
	{
		if( bin <  0 )		    return 0;
		if( bin >= BINS )		return BINS - 1;
		return bin;
	}

public:
    Distrib( stat_t min_val = 0, stat_t bin_size = 1.0 ) : Stats()        
	{ 
        _MinVal  = min_val;
        _BinSize = bin_size;
		Clear(); 
	}
    virtual ~Distrib()         {}

    //////////////////////////////////////////////////////////////////
    inline void SetMaxValue( stat_t max_val )
    {
        _BinSize = ( max_val - _MinVal ) / BINS;
    }

    //////////////////////////////////////////////////////////////////
    inline void Clear( void )     
    { 
        Stats::Clear();
        memset( _Dstrb, 0, BINS * sizeof( stat_t ));
    }
   //////////////////////////////////////////////////////////////////
    inline void Sample( stat_t sample, stat_t weight = 1.0 )
    {
        Stats::Sample( sample, weight );
		_Dstrb[ _VerifyBin( _CalcBin( sample )) ] += weight;
    }
    //////////////////////////////////////////////////////////////////
    inline Distrib< BINS > operator+ ( Distrib< BINS > d )
    {
        return d += *this;
    }
    //////////////////////////////////////////////////////////////////
    inline Distrib< BINS > operator+= (const Distrib< BINS >& d )
    {
        Stats::operator += ( static_cast<Stats>( d ));
        for( int16s i = 0; i < BINS; i++ )
            _Dstrb[i] += d._Dstrb[i];
        return *this;
    }
    //////////////////////////////////////////////////////////////////
    inline stat_t GetBin( int32s bin )       const { return _Dstrb[ _VerifyBin( bin ) ]; }
    inline stat_t GetBinNorm( int32s bin )   const { return GetCount()? GetBin(bin) / GetCount(): INVALID_VAL; }
	inline stat_t GetBinFloor( int32s bin )  const { return _MinVal + _VerifyBin( bin ) * _BinSize; }
	inline stat_t GetBinCeil( int32s bin )   const { return GetBinFloor( bin ) + _BinSize;     }
    inline stat_t GetBinCenter( int32s bin ) const { return GetBinFloor( bin ) + _BinSize / 2; }

    //////////////////////////////////////////////////////////////////
    // Returns bin index such that pcnt (%) of all samples 
    // are in this bin or below. 
    //////////////////////////////////////////////////////////////////
    int32s GetPercentileBin( DOUBLE pcnt ) const 
    {
        int32s n = 0;
        stat_t sum = 0, limit = pcnt * GetCount();

        while( sum < limit && n < BINS )
            sum += _Dstrb[ n++ ];
        return n - 1;
    }

    //////////////////////////////////////////////////////////////////
    // Returns a value such that pcnt (%) of all samples are below 
    // this value. The returned value is rounded up to the next bin 
    // boundary. 
    //////////////////////////////////////////////////////////////////
    inline stat_t GetPercentileValue( DOUBLE pcnt ) const 
    {
        return GetBinCeil( GetPercentileBin( pcnt ));
    }

    //////////////////////////////////////////////////////////////////
    // Returns a percentage of samples <= value 
    //////////////////////////////////////////////////////////////////
    DOUBLE GetRank( stat_t val ) const 
    {
        DOUBLE rank = 0;

        for( int32s bin  = _VerifyBin( _CalcBin( val )); bin >= 0; bin-- )
            rank += _Dstrb[ b ];

        return rank / GetCount();
    }
	//////////////////////////////////////////////////////////////////
};

//////////////////////////////////////////////////////////////////////
// class AutoCorr 
// Calculates auto-correlation of a series 
//////////////////////////////////////////////////////////////////////
template < int32s OFFSET > class AutoCorr : public Stats
{
private:

    stat_t _History[ OFFSET ];  // circular buffer
    stat_t _Prod;
    int32s _Fill;
    int32s _Tail;
    

public:
    AutoCorr() : Stats()
    {
        _Prod = 0;
        _Fill = 0;
        _Tail = 0;
    }
    //////////////////////////////////////////////////////////////////
    inline void Sample( stat_t sample )
    {
        if( _Fill < OFFSET )
            _Fill++;

        else
        {
            _Prod += sample * _History[ _Tail ];
            Stats::Sample( sample );
        }

        _History[ _Tail++ ] = sample;

        if( _Tail >= OFFSET ) 
            _Tail -= OFFSET;
    }

    //////////////////////////////////////////////////////////////////
    inline stat_t GetAutoCorr(void) const 
    {
        stat_t avg = GetAvg();

        if( GetCount() <= 0 )
            return INVALID_VAL;

        return ( _Prod / GetCount() - avg * avg ) / GetVar();
    }
};

#endif // _STATS_H_INCLUDED_

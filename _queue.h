/**********************************************************
 * Author: Glen Kramer (kramer@cs.ucdavis.edu)
 *         University of California @ Davis
 *
 * Filename:    _queue.h
 *
 * Description: Queue
 * 
 *********************************************************/
#ifndef _QUEUE_H_V001_
#define _QUEUE_H_V001_

#include "_types.h"

template< class item_t, int32s SIZE > class Queue
{
  private:
    item_t  qArray[ SIZE ];
    int32s  qHead;
    int32s  qSize;
    int32s  qLimit;

    ////////////////////////////////////////////////////////////////
    inline int32s  qMap( int32s index ) const
    {
        if((index += qHead) >= qLimit )   index -= qLimit;
        return index;
    }
    ////////////////////////////////////////////////////////////////


  public:
    Queue( int32s queue_limit = SIZE )                        
    { 
        qHead   = 0;
        qSize   = 0; 
        SetLimit( queue_limit );
    }

    ////////////////////////////////////////////////////////////////
    inline void SetLimit( int32s queue_limit )
    {
        qLimit  = MIN<int32s>( queue_limit, SIZE );
        if( qSize > qLimit )
            qSize = qLimit;
    }

    ////////////////////////////////////////////////////////////////
    inline bool     IsEmpty(void)   const   { return qSize <= 0;    }
    inline bool     IsFull(void)    const   { return qSize >= qLimit; }
    inline int32s   GetSize(void)   const   { return qSize; }
    inline void     Clear  (void)           { qSize = 0;    }
    ////////////////////////////////////////////////////////////////
    inline item_t Peek( int32s index = 0 )  const
    {
        return qArray[ qMap( index ) ];
    }
    ////////////////////////////////////////////////////////////////
    inline void Add( item_t item )
    {
        if( !IsFull() ) qArray[ qMap( qSize++ ) ] = item;
    }
    ////////////////////////////////////////////////////////////////
    inline item_t Get(void)
    {
        int32s index = qHead;
        if( !IsEmpty() ) 
        {
            qHead = qMap( 1 );
            qSize--;
        }
        return qArray[ index ];
    }
    ////////////////////////////////////////////////////////////////
    inline void Set( item_t item, int32s index = 0 )
    {
        if( index < qSize )  qArray[ qMap( index ) ] = item;
    }
};


#endif  /* _QUEUE_H_V001_ */



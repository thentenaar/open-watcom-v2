/****************************************************************************
*
*                            Open Watcom Project
*
* Copyright (c) 2002-2020 The Open Watcom Contributors. All Rights Reserved.
*    Portions Copyright (c) 1983-2002 Sybase, Inc. All Rights Reserved.
*
*  ========================================================================
*
*    This file contains Original Code and/or Modifications of Original
*    Code as defined in and that are subject to the Sybase Open Watcom
*    Public License version 1.0 (the 'License'). You may not use this file
*    except in compliance with the License. BY USING THIS FILE YOU AGREE TO
*    ALL TERMS AND CONDITIONS OF THE LICENSE. A copy of the License is
*    provided with the Original Code and Modifications, and is also
*    available at www.sybase.com/developer/opensource.
*
*    The Original Code and all software distributed under the License are
*    distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
*    EXPRESS OR IMPLIED, AND SYBASE AND ALL CONTRIBUTORS HEREBY DISCLAIM
*    ALL SUCH WARRANTIES, INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF
*    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR
*    NON-INFRINGEMENT. Please see the License for the specific language
*    governing rights and limitations under the License.
*
*  ========================================================================
*
* Description:  Convert hex string to binary
*
****************************************************************************/

#include "ftnstd.h"
#include <string.h>
#include <ctype.h>
#include "hexcnv.h"


static const char __FAR HexDigits[ 16 ] = {
    '0', '1', '2', '3', '4', '5', '6', '7',
    '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
};


char    Hex( char data ) {
//========================

    if( isdigit( (unsigned char)data ) == 0 ) {
        data += 9;
    }
    data &= 0x0f;
    return( data );
}


uint    HSToB( const char *src, uint src_len, char *dst ) {
//=========================================================

    uint        length;

    length = 0;
    if( ( src_len % 2 ) != 0 ) {
        if( isxdigit( (unsigned char)src[0] ) == 0 )
            return( 0 );
        if( dst != NULL ) {
            *dst++ = Hex( *src );
        }
        length++;
        src++;
        src_len--;
    }
    while( src_len != 0 ) {
        if( isxdigit( (unsigned char)src[0] ) == 0 || isxdigit( (unsigned char)src[1] ) == 0 )
            return( length );
        if( dst != NULL ) {
            *dst++ = Hex( src[0] ) * 0x10 + Hex( src[1] );
        }
        length++;
        src += 2;
        src_len -= 2;
    }
    return( length );
}


char    *BToHS( char *mem, int length, char *fmt_buf ) {
//==============================================================

    char        data;

    for(;;) {
        if( --length < 0 )
            break;
        data = *mem;
        ++mem;
        *fmt_buf = HexDigits[( (unsigned char)data >> 4 ) & 0x0f];
        ++fmt_buf;
        if( --length < 0 )
            break;
        *fmt_buf = HexDigits[(unsigned char)data & 0x0f];
        ++fmt_buf;
    }
    *fmt_buf = NULLCHAR;
    return( fmt_buf );
}

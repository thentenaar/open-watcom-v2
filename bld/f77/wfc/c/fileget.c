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
* Description:  ISO C I/O support for reading from files
*
****************************************************************************/


#include "ftnstd.h"
#include <string.h>
#if defined( __WINDOWS__ )
    #include <conio.h>
#endif
#include "fileio.h"
#include "fileget.h"
#include "fileseek.h"
#include "fileerr.h"


#define READ_ERROR      ((size_t)-1)    // read error indicator

size_t readbytes( b_file *io, char *buff, size_t len )
//====================================================
{
    size_t      bytes_read;

    if( len == 0 )
        return( 0 );
    bytes_read = fread( buff, 1, len, io->fp );
    if( bytes_read != len && ferror( io->fp ) ) {
        FSetSysErr( io );
        return( READ_ERROR );
    }
    if( bytes_read == 0 ) {
        FSetEof( io );
        return( READ_ERROR );
    }
    io->phys_offset += (unsigned long)bytes_read;
    return( bytes_read );
}


static  int     FillBuffer( b_file *io )
//======================================
{
    size_t      bytes_read;

    bytes_read = readbytes( io, io->buffer, io->buff_size );
    if( bytes_read == READ_ERROR )
        return( -1 );
    // setup the buffer
    io->b_curs = 0;
    io->attrs |= READ_AHEAD;
    io->high_water = 0;
    io->read_len = bytes_read;
    return( 0 );
}


static size_t SysRead( b_file *io, char *b, size_t len )
//======================================================
{
    size_t      bytes_read;
    size_t      amt;
    size_t      offs_in_b;
    size_t      max_valid;

    if( io->attrs & BUFFERED ) {
        // determine the maximum valid position in the buffer
        max_valid = io->read_len;
        if( max_valid < io->high_water ) {
            max_valid = io->high_water;
        }
        // if we're beyond that position then we must fill the buffer
        if( io->b_curs >= max_valid ) {
            if( FillBuffer( io ) < 0 )
                return( READ_ERROR );
            max_valid = io->read_len;
        }
        amt = max_valid - io->b_curs;
        if( amt > len ) {
            amt = len;
        }
        memcpy( b, &io->buffer[io->b_curs], amt );
        offs_in_b = amt;
        io->b_curs += amt;
        len -= amt;
        if( len ) {
            // flush the buffer
            io->b_curs = 0;
            io->read_len = 0;
            io->high_water = 0;
            io->attrs &= ~READ_AHEAD;
            if( len > io->buff_size ) {
                // read a multiple of io->buff_size bytes
                amt = len - len % io->buff_size;
                bytes_read = readbytes( io, b + offs_in_b, amt );
                if( bytes_read == READ_ERROR )
                    return( READ_ERROR );
                offs_in_b += bytes_read;
                if( bytes_read < amt )
                    return( offs_in_b );
                len -= amt;
            }
            if( len ) {
                // first fill the buffer
                bytes_read = readbytes( io, io->buffer, io->buff_size );
                if( bytes_read == READ_ERROR )
                    return( READ_ERROR );
                io->attrs |= READ_AHEAD;
                io->read_len = bytes_read;
                // then grab our bytes from it
                if( len > bytes_read ) {
                    len = bytes_read;
                }
                memcpy( b + offs_in_b, io->buffer, len );
                io->b_curs = len;
                offs_in_b += len;
            }
        }
        return( offs_in_b );
    } else {
        return( readbytes( io, b, len ) );
    }
}


size_t FGetRecText( b_file *io, char *b, size_t len )
//===================================================
// Get a record from a TEXT file.
{
    char        ch;
    size_t      read;

    FSetIOOk( io );
    if( io->attrs & BUFFERED ) {
        char            *ptr;
        char            *stop;
        bool            seen_cr;
        bool            trunc;
        size_t          max_valid;

        // determine maximum valid position in the buffer
        max_valid = io->read_len;
        if( max_valid < io->high_water ) {
            max_valid = io->high_water;
        }
        stop = io->buffer + max_valid;
        ptr = io->buffer + io->b_curs;
        read = 0;
        seen_cr = false;
        trunc = false;
        for( ;; ) {
            if( ptr >= stop ) {
                io->b_curs = ptr - io->buffer;
                if( FillBuffer( io ) < 0 ) {
                    // we have to do this so that io->b_curs is set properly
                    // on end of file
                    ptr = io->buffer + io->b_curs;
                    if( read > 0 && io->stat == FILEIO_EOF ) {
                        FSetIOOk( io );
                    }
                    break;
                }
                stop = io->buffer + io->read_len;
                ptr = io->buffer + io->b_curs;
            }
            ch = *ptr;
            ++ptr;
            if( ch == CHAR_LF )
                break;
            if( !seen_cr ) {
                if( ch == CHAR_CTRL_Z ) {
                    --ptr; // give back char so we don't read past EOF
                    if( read == 0 )
                        FSetEof( io );
                    break;
                }
                if( ch == CHAR_CR ) {
                    seen_cr = true;
                } else if( read < len ) {
                    b[read] = ch;
                    ++read;
                } else {
                    trunc = true;
                }
            } else {
                --ptr;  // give back the char
                seen_cr = false;
                if( read < len ) {
                    b[read] = CHAR_CR;
                    ++read;
                } else {
                    trunc = true;
                }
            }
        }
        io->b_curs = ptr - io->buffer;
        if( trunc ) {
            FSetTrunc( io );
        }
        return( read );
    } else {    // device (CON)
        read = 0;
        len = readbytes( io, b, len );
        if( len == READ_ERROR )
            return( 0 );
        for( ;; ) {
            if( read == len )
                break;
#if defined( __UNIX__ ) || defined( __NETWARE__ )
            if( *b == CHAR_LF )
                return( read );
#else
            if( *b == CHAR_CR ) {
                ++b;
                if( read == len - 1 )
                    break;
                if( *b == CHAR_LF )
                    return( read );
                --b;
            } else if( *b == CHAR_CTRL_Z ) {
                FSetEof( io );
                return( read );
            }
#endif
            ++b;
            ++read;
        }
        FSetTrunc( io );
        return( read );
    }
}


size_t FGetRecFixed( b_file *io, char *b, size_t len )
//====================================================
// Get a record from a file with "fixed" records.
{
    FSetIOOk( io );
    len = SysRead( io, b, len );
    if( len == READ_ERROR )
        return( 0 );
    return( len );
}

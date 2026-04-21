
// Code modified for use by Clonk engine.

// skstream.cpp
// Copyright (C) 1996, 1995 by John C. Wang <jcwang@csie.ntu.edu.tw>
// All Rights Reserved.
//
// You may distribute this file with your product in either of two ways:
// IN SOURCE CODE FORM: You must include this file in its entirety.
// IN OBJECT CODE FORM: You must give proper acknowledgements to the author(s)
//   of this program. This may take the form of credits on the start-up screen.
//
// IN ANYCASE, THIS SOFTWARE IS DISTRIBUTED WITHOUT ANY KIND OF EXPLICIT OR
// IMPLICIT WARRANTY AND THE AUTHOR(S) ARE NOT RESPONSIBLE FOR ANY EVENT THAT
// OCCURS DUE TO THE USE OR MISUSE OF THIS SOFTWARE.
//
// History:
// [JCW 95-Dec-04] created.
// [JCW 95-Dec-20] comments added for distribution 95a.
// [JCW 96-Jan-01] removed UDP capabilities from skstream.
// [JCW 96-Mar-14] exceptions made optional.
// [JCW 96-Oct-20] protected skstream::init and ::shutdown changed.
//                 char( 255 ) == EOF bug fixed.

#include "skstream.h"
#include <assert.h>

#include <stdio.h>


// Machine dependent macros
//  Don't forget to modify 
#ifdef _OS2
// Add definitions here
#endif

#ifdef _LINUX
// Add definitions here
#endif

#ifdef _UNIX
#define INVALID_SOCKET -1
// Add definitions here
#endif

// Machine independent macros
#ifdef _DEBUG
	#define dassert(x) assert(x)
	#define debug(x) x
#else
	#define dassert(x)
	#define debug(x)
#endif //def _DEBUG

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE !FALSE
#endif

//
// skstream
//

int skstream::init( void )
{

	_socket = INVALID_SOCKET; // initialization added - matthes

	lasterror = 0;
	// platform dependent initialization
	const WORD wMinVer = 0x0101;	// request WinSock v1.1 (at least)
	WSADATA wsaData;
	if( 0 == WSAStartup( wMinVer, &wsaData ) )
		return TRUE ;
	return FALSE ;
}

int skstream::shutdown( void )
{
	// platform dependent shutdown
	if( 0 == WSACleanup() )
		return TRUE ;
	return FALSE ;
}

skstream::skstream( void ) : _sockbuf( _socket ), iostream( &_sockbuf )
{
	if( init() )
		attach( INVALID_SOCKET ) ;
}

skstream::skstream( const char *addr, const service port, const role side )
 : _sockbuf( _socket ), iostream( &_sockbuf )
{
	if( init() )
	{
		attach( INVALID_SOCKET ) ;
		open( addr, port, side ) ;
	}
}

skstream::skstream( SOCKET sock ) : _sockbuf( _socket ), iostream( &_sockbuf )
{
	if( init() )
		attach( sock ) ;
}

skstream::~skstream( void )
{
	close() ;
	shutdown() ;
}

int skstream::is_open( void ) const
	{
	return ( INVALID_SOCKET != getsocket() ) ;
	}

void skstream::open( const char *addr, int port, const role side )
	{

	// Stream already open
	if (is_open()) close();
	
	// Create socket
	if( INVALID_SOCKET == ( _socket = ::socket( PF_INET, SOCK_STREAM, 0 ) ) )
		// Cannot create socket
		{ lasterror=1; return ; }

	// Bind to local address (any) and port (server only)
	SOCKADDR_IN sa;
	sa.sin_family = AF_INET;
	sa.sin_addr.S_un.S_addr = INADDR_ANY;
	sa.sin_port = (side==skstream::client) ? 0 : htons((unsigned short) port);
	if	(SOCKET_ERROR == ::bind( _socket, (sockaddr *)&sa, sizeof(sa) ))
		{
		// Cannot bind to the chosen port
		close(); lasterror=2; return;
		}

	// Client: connect
	if	(side == skstream::client)
		{
		// 3(cli). Connect
		SOCKADDR_IN sa ;
		sa.sin_family = AF_INET ;

		DWORD S_addr;
		hostent *he ;
		char host[MAXGETHOSTSTRUCT]; memset(&host, 0, MAXGETHOSTSTRUCT);
		
		// Test convert address
		if ( (S_addr=inet_addr(addr)) != INADDR_NONE) 
			{
			// Get host by address
			he = ::gethostbyaddr( (char*) &S_addr, sizeof S_addr, AF_INET );
			// Else, compose own hostent
			if (he == NULL)
				{
				he = (HOSTENT*) host;
				he->h_addrtype = AF_INET;
				he->h_length = 4;
				he->h_addr_list = (char**)&(he->h_addr_list) + 4;
				he->h_addr_list[0] = (char*)(he->h_addr_list) + 4;
				memcpy(he->h_addr_list[0], &S_addr, 4);
				}
			}
		else
			{
			// Get host by name
			he = ::gethostbyname( addr );
			}

		// Host not found
		if (he == NULL)
			{
			// Cannot resolve remote server ip
			close() ;
			lasterror=3; return;
			}

		sa.sin_addr.S_un.S_addr = *(unsigned long *)( he->h_addr_list[ 0 ] ) ;
		sa.sin_port = htons( port ) ;

		if( SOCKET_ERROR == ::connect( _socket, (sockaddr *)&sa, sizeof( sa ) ) )
			{
			// Connection error
			close() ;
			lasterror=4; 
			return;
			}
		}

	// Server: listen
	else
		{
		
		// Listen
		if (SOCKET_ERROR == ::listen( _socket, 5 ))	// max backlog
			{
			// Error listening
			close(); lasterror=5; return;
			}

		// Accept
		SOCKET skAcceptedSocket;
		if	( INVALID_SOCKET == ( skAcceptedSocket = ::accept( _socket, NULL, NULL ) ) )
			{
			// Accepting error
			close(); lasterror=6; return;
			}

		// Close listening socket, return accepted socket
		if	( SOCKET_ERROR == ::closesocket( _socket ) )
			{
			// Cannot close service port, resource may be occupied
			close(); lasterror=7; return;
			}
		_socket = skAcceptedSocket;

		}

	// Success
	}

void skstream::close( void )
{
	if( is_open() )
	{
		_sockbuf.sync() ;
		if( SOCKET_ERROR == ::closesocket( _socket ) )
			// Cannot close. Leave _socket as it was
			return ;
	}

	_socket = INVALID_SOCKET ;
}

void skstream::attach( SOCKET sock )
{
	_socket = sock ;
}

SOCKET skstream::getsocket() const
{
	return _socket ;
}

char *skstream::getpeername( char *buf, int size ) const
{
	SOCKADDR_IN sa ;
	int sasize = sizeof( sa ) ;
	if( SOCKET_ERROR ==::getpeername( getsocket(), (sockaddr *)&sa, &sasize ) )
		// Cannot get peer name
		return NULL ;
	strncpy( buf, ::inet_ntoa( sa.sin_addr ), size - 1 ) ;
	return buf ;
}

unsigned short skstream::getport( void ) const
{
	SOCKADDR_IN sa ;
	int sasize = sizeof( sa ) ;
	if( SOCKET_ERROR ==::getpeername( getsocket(), (sockaddr *)&sa, &sasize ) )
		// Cannot get peer port
		return ntohs( IPPORT_RESERVED ) ;
	return ntohs( sa.sin_port ) ;
}

//
//	sockbuf
//
sockbuf::sockbuf( SOCKET &sock ) : _socket( sock ), streambuf( NULL, 0 )
{
	const int insize = 0x4000 ;		// allocate 16k buffer each for input and output
	const int outsize = 0x4000 ;
	const int bufsize = insize + outsize ;

	_buffer = new char [ bufsize ] ;

	if( this != setbuf( _buffer, bufsize ) )
		_buffer = NULL ;
	else
	{
		setp( _buffer, _buffer + insize ) ;
		setg( _buffer + insize, _buffer + bufsize, _buffer + bufsize ) ;
	}
}

sockbuf::sockbuf( SOCKET &sock, char *buf, int length )
 : _socket( sock ), streambuf( buf, length )
{
	_buffer = NULL ;

	setp( buf, buf + length / 2 ) ;
	setg( buf + length / 2, buf + length, buf + length ) ;
}

sockbuf::~sockbuf()
{
	delete[] _buffer ;
	_buffer = NULL ;
}

int sockbuf::overflow( int nCh )
{
	// in case of error, user finds out by testing fail()
	if( _socket == INVALID_SOCKET )
		// Invalid socket
		return EOF ;

	if( pptr() - pbase() <= 0 )
		// nothing to send
		return 0 ;

	int size ;
	if( SOCKET_ERROR == ( size = ::send( _socket, pbase(), pptr() - pbase(), 0 ) ) )
		// (TCP) Cannot send
		return EOF ;

	if( size == 0 )
		// remote site has closed this connection
		return EOF ;

	if( nCh != EOF )	// size >= 1 at this point
	{
		size-- ;
		*( pbase() + size ) = nCh ;
	}

	// move remaining pbase() + size .. pptr() - 1 => pbase() .. pptr() - size - 1
	for( char *p = pbase() + size; p < pptr(); p++ )
		*( p - size ) = *p ;
	const int newlen = ( pptr() - pbase() ) - size ;

	setp( pbase(), epptr() ) ;
	pbump( newlen ) ;

	return 0 ;
}

int sockbuf::underflow()
{
	// if get area not empty, return first character
	// else fill up get area and return 1st character

	// in case of error, user finds out by testing eof()

	if( _socket == INVALID_SOCKET )
		// Invalid socket!
		return EOF ;

	if( egptr() - gptr() > 0 )
		return (int)(unsigned char)(*gptr()) ;

	// fill up from eback to egptr
	int size ;
	if( SOCKET_ERROR == ( size = ::recv( _socket, eback(), egptr() - eback(), 0 ) ) )
		// (TCP) Receive error
		return EOF ;

	if( size == 0 )
		// remote site has closed this connection
		return EOF ;

	// move rcvd data from eback() .. eback() + size to egptr() - size .. egptr()
	const int delta = egptr() - ( eback() + size ) ;
	for( char *p = eback() + size - 1; p >= eback(); p-- )
	{
		dassert( p + delta >= eback() ) ;
		dassert( p + delta < egptr() ) ;
		*( p + delta ) = *p ;
	}

	setg( eback(), egptr() - size, egptr() ) ;

	return (int)(unsigned char)(*gptr()) ;
}

int sockbuf::sync()
{
	if( EOF == overflow() )
		return EOF ;	// ios will set the fail bit
	else
	{
		// empty put and get areas
		setp( pbase(), epptr() ) ;
		setg( eback(), egptr(), egptr() ) ;
		
		return 0 ;
	}
}

int skstream::getlasterror()
{
	return lasterror;
}

void skstream::create(int port)
	{
	// Create socket
	if ( INVALID_SOCKET == ( _socket = ::socket( PF_INET, SOCK_STREAM, 0 ) ) )
		// Cannot create socket
		{ lasterror=1; return ; }

	// Bind to local address (any) and port (create is for server only)
	SOCKADDR_IN sa;
	sa.sin_family = AF_INET;
	sa.sin_addr.S_un.S_addr = INADDR_ANY;
	sa.sin_port = htons((unsigned short) port);
	if	(SOCKET_ERROR == ::bind( _socket, (sockaddr *)&sa, sizeof(sa) ))
		{
		// Cannot bind to the chosen port
		close(); lasterror=2; return;
		}
	}

bool skstream::has_socket()
	{
	return (_socket!=INVALID_SOCKET);
	}

void skstream::listen(skstream **ppnew)
	{
	
	*ppnew=NULL;
	
	// Listen
	if (SOCKET_ERROR == ::listen( _socket, 5 ))	// max backlog
		{
		// Error listening
		close(); lasterror=5; return;
		}
	
	// Accept
	SOCKET skAcceptedSocket;
	if	( INVALID_SOCKET == ( skAcceptedSocket = ::accept( _socket, NULL, NULL ) ) )
		{
		// Accepting error
		close(); lasterror=6; return;
		}
	
	// Create new skstream with accepted socket
	*ppnew = new skstream(skAcceptedSocket);

	}

BOOL skstream::is_readable()
	{
	TIMEVAL timeout = {0,0};
	FD_SET fdset_readable;
	fdset_readable.fd_count = 1;
	fdset_readable.fd_array[0] = _socket;

	int iResult = select( 0, &fdset_readable, NULL, NULL, &timeout );
	
	if (iResult==SOCKET_ERROR) return FALSE;
	if (iResult==0) return FALSE;
	return TRUE;
	}

BOOL skstream::is_writeable()
	{
	TIMEVAL timeout = {0,0};
	FD_SET fdset_writeable;
	fdset_writeable.fd_count = 1;
	fdset_writeable.fd_array[0] = _socket;

	int iResult = select( 0, NULL, &fdset_writeable, NULL, &timeout );
	
	if (iResult==SOCKET_ERROR) return FALSE;
	if (iResult==0) return FALSE;
	return TRUE;
	}

#include "skstream.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

#ifndef _WIN32
  #include <sys/ioctl.h>
  #include <fcntl.h>
  #define closesocket close
#endif

#ifdef _DEBUG
    #define dassert(x) assert(x)
    #define debug(x) x
#else
    #define dassert(x)
    #define debug(x)
#endif

//
// skstream
//

inline int skstream::init( void )
{
    _socket = INVALID_SOCKET;
    lasterror = 0;
#ifdef _WIN32
    const WORD wMinVer = 0x0101;
    WSADATA wsaData;
    if( 0 == WSAStartup( wMinVer, &wsaData ) )
            return TRUE ;
    return FALSE ;
#else
    return TRUE;
#endif
}

inline int skstream::shutdown( void )
{
#ifdef _WIN32
    if( 0 == WSACleanup() )
            return TRUE ;
    return FALSE ;
#else
    return TRUE;
#endif
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
    struct sockaddr_in sa;
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = INADDR_ANY;
    sa.sin_port = (side==skstream::client) ? 0 : htons((unsigned short) port);
    
    // allow address reuse
    int opt = 1;
    setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, (const char *)&opt, sizeof(opt));
    
    if (SOCKET_ERROR == ::bind( _socket, (struct sockaddr *)&sa, sizeof(sa) ))
    {
        // Cannot bind to the chosen port
        close(); lasterror=2; return;
    }

    // Client: connect
    if (side == skstream::client)
    {
        struct sockaddr_in sa_client;
        memset(&sa_client, 0, sizeof(sa_client));
        sa_client.sin_family = AF_INET;
        
        struct hostent *he = NULL;
        uint32_t S_addr = inet_addr(addr);

        if (S_addr != INADDR_NONE) 
        {
#ifdef _WIN32
            he = ::gethostbyaddr( (char*) &S_addr, sizeof S_addr, AF_INET );
#else
            he = ::gethostbyaddr( (const void*) &S_addr, sizeof S_addr, AF_INET );
#endif
            if (he == NULL)
            {
                sa_client.sin_addr.s_addr = S_addr;
            }
        }
        else
        {
            he = ::gethostbyname( addr );
        }
        
        if (he != NULL) {
            memcpy(&sa_client.sin_addr.s_addr, he->h_addr_list[0], sizeof(sa_client.sin_addr.s_addr));
        } else if (S_addr == INADDR_NONE) {
            close() ;
            lasterror=3; return;
        }

        sa_client.sin_port = htons( port ) ;

        if( SOCKET_ERROR == ::connect( _socket, (struct sockaddr *)&sa_client, sizeof( sa_client ) ) )
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
        if (SOCKET_ERROR == ::listen( _socket, 5 ))     // max backlog
        {
            // Error listening
            close(); lasterror=5; return;
        }

        // Accept
        SOCKET skAcceptedSocket;
        if ( INVALID_SOCKET == ( skAcceptedSocket = ::accept( _socket, NULL, NULL ) ) )
        {
            // Accepting error
            close(); lasterror=6; return;
        }

        // Close listening socket, return accepted socket
        if ( SOCKET_ERROR == ::closesocket( _socket ) )
        {
            // Cannot close service port, resource may be occupied
            close(); lasterror=7; return;
        }
        _socket = skAcceptedSocket;
    }
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
    struct sockaddr_in sa ;
#ifdef _WIN32
    int sasize = sizeof( sa ) ;
#else
    socklen_t sasize = sizeof( sa ) ;
#endif
    if( SOCKET_ERROR ==::getpeername( getsocket(), (struct sockaddr *)&sa, &sasize ) )
            // Cannot get peer name
            return NULL ;
    strncpy( buf, ::inet_ntoa( sa.sin_addr ), size - 1 ) ;
    return buf ;
}

unsigned short skstream::getport( void ) const
{
    struct sockaddr_in sa ;
#ifdef _WIN32
    int sasize = sizeof( sa ) ;
#else
    socklen_t sasize = sizeof( sa ) ;
#endif
    if( SOCKET_ERROR ==::getpeername( getsocket(), (struct sockaddr *)&sa, &sasize ) )
            // Cannot get peer port
            return 0; // fallback if getsockname fails
    return ntohs( sa.sin_port ) ;
}

//
//      sockbuf
//
sockbuf::sockbuf( SOCKET &sock ) : _socket( sock ), streambuf() // removed streambuf(NULL, 0) since modern C++ streambuf default ctor doesn't take args
{
    const int insize = 0x4000 ;             // allocate 16k buffer each for input and output
    const int outsize = 0x4000 ;
    const int bufsize = insize + outsize ;

    _buffer = new char [ bufsize ] ;

    if( this != setbuf( _buffer, bufsize ) ) {
        delete[] _buffer;
        _buffer = NULL ;
    }
    else
    {
        setp( _buffer, _buffer + insize ) ;
        setg( _buffer + insize, _buffer + bufsize, _buffer + bufsize ) ;
    }
}

sockbuf::sockbuf( SOCKET &sock, char *buf, int length )
 : _socket( sock ), streambuf() 
{
    _buffer = NULL ;

    setp( buf, buf + length / 2 ) ;
    setg( buf + length / 2, buf + length, buf + length ) ;
}

sockbuf::~sockbuf()
{
    if (_buffer) delete[] _buffer ;
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

    if( nCh != EOF )        // size >= 1 at this point
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
            return EOF ;    // ios will set the fail bit
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
    struct sockaddr_in sa;
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = INADDR_ANY;
    sa.sin_port = htons((unsigned short) port);
    
    // allow address reuse
    int opt = 1;
    setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, (const char *)&opt, sizeof(opt));
    
    if (SOCKET_ERROR == ::bind( _socket, (struct sockaddr *)&sa, sizeof(sa) ))
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
    if (SOCKET_ERROR == ::listen( _socket, 5 ))     // max backlog
            {
            // Error listening
            close(); lasterror=5; return;
            }

    // Accept
    SOCKET skAcceptedSocket;
    if ( INVALID_SOCKET == ( skAcceptedSocket = ::accept( _socket, NULL, NULL ) ) )
            {
            // Accepting error
            close(); lasterror=6; return;
            }

    // Create new skstream with accepted socket
    *ppnew = new skstream(skAcceptedSocket);
}

BOOL skstream::is_readable()
{
    if (_socket == INVALID_SOCKET) return FALSE;
    struct timeval timeout = {0,0};
    fd_set fdset_readable;
    FD_ZERO(&fdset_readable);
    FD_SET(_socket, &fdset_readable);

    int iResult = select( _socket + 1, &fdset_readable, NULL, NULL, &timeout );

    if (iResult==SOCKET_ERROR) return FALSE;
    if (iResult==0) return FALSE;
    return TRUE;
}

BOOL skstream::is_writeable()
{
    if (_socket == INVALID_SOCKET) return FALSE;
    struct timeval timeout = {0,0};
    fd_set fdset_writeable;
    FD_ZERO(&fdset_writeable);
    FD_SET(_socket, &fdset_writeable);

    int iResult = select( _socket + 1, NULL, &fdset_writeable, NULL, &timeout );

    if (iResult==SOCKET_ERROR) return FALSE;
    if (iResult==0) return FALSE;
    return TRUE;
}

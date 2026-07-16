#include "skstream.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

#ifndef _WIN32
  #include <sys/ioctl.h>
  #include <fcntl.h>
  #include <errno.h>
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

skstream::skstream( void ) : iostream( &_sockbuf ), _socket( INVALID_SOCKET ), _sockbuf( _socket )
{
    if( init() )
            attach( INVALID_SOCKET ) ;
}

skstream::skstream( const char *addr, const service port, const role side )
 : iostream( &_sockbuf ), _socket( INVALID_SOCKET ), _sockbuf( _socket )
{
    if( init() )
    {
            attach( INVALID_SOCKET ) ;
            open( addr, port, side ) ;
    }
}

skstream::skstream( SOCKET sock ) : iostream( &_sockbuf ), _socket( INVALID_SOCKET ), _sockbuf( _socket )
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

void skstream::set_nonblocking()
{
    // NO-OP for now, keeping code in blocking mode to match Clonk logic
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
sockbuf::sockbuf( SOCKET &sock ) : streambuf(), _socket( sock ) 
{
    const int insize = 0x4000 ;             // allocate 16k buffer each for input and output
    const int outsize = 0x4000 ;
    const int bufsize = insize + outsize ;

    _buffer = new char [ bufsize ] ;

    setp( _buffer, _buffer + insize ) ;
    setg( _buffer + insize, _buffer + bufsize, _buffer + bufsize ) ;
}

sockbuf::sockbuf( SOCKET &sock, char *buf, int length )
 : streambuf(), _socket( sock ) 
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
    if( _socket == INVALID_SOCKET ) return EOF ;

    int nData = pptr() - pbase();
    if( nData > 0 )
    {
        int nSent = ::send( _socket, pbase(), nData, 0 );
        if( nSent == SOCKET_ERROR || nSent == 0 )
        {
            return EOF;
        }
        
        // Move unsent data to the front
        if (nSent < nData)
            memmove(pbase(), pbase() + nSent, nData - nSent);
        setp(pbase(), epptr());
        pbump(nData - nSent);
    }

    if( nCh != EOF )
    {
        if (pptr() < epptr()) {
            *pptr() = (char)nCh;
            pbump(1);
        } else {
            return EOF;
        }
    }

    return (nCh == EOF) ? 0 : nCh;
}

int sockbuf::underflow()
{
    if( _socket == INVALID_SOCKET ) return EOF ;

    if( gptr() < egptr() )
        return (int)(unsigned char)(*gptr());

    // fill up from eback to egptr
    int capacity = egptr() - eback();
    int nRead = ::recv( _socket, eback(), capacity, 0 );
    if( nRead == SOCKET_ERROR || nRead == 0 )
    {
        return EOF ;
    }

    memmove(egptr() - nRead, eback(), nRead);
    setg(eback(), egptr() - nRead, egptr());

    return (int)(unsigned char)(*gptr());
}

int sockbuf::sync()
{
    return (overflow(EOF) == EOF) ? -1 : 0;
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
    if (rdbuf()->in_avail() > 0) return TRUE;
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

// No system headers here: this header is used together with the engine's Compat.h, which must not
// meet <windows.h>. The socket headers are included by skstream.cpp.
#include <stdint.h>

#ifndef _WIN32
  typedef int SOCKET;
#else
  typedef uintptr_t SOCKET; // UINT_PTR of winsock2.h
#endif
typedef int BOOL;

// Code modified for use by Clonk engine.

#ifndef __SOCKET_STREAM__
#define __SOCKET_STREAM__

#include <iostream>
#include <streambuf>
using namespace std;

//
// sockbuf
//
class sockbuf : public streambuf
{
public:
	sockbuf( SOCKET & );
	sockbuf( SOCKET &, char *, int );

	virtual ~sockbuf();

	virtual int overflow(int =EOF) override;
	virtual int underflow() override;

    virtual int sync() override;

protected:
	char *_buffer ;

	// sockbuf specific
	SOCKET &_socket ;
} ;

//
// skstream
//
class skstream : public iostream {
public:
	BOOL is_readable();
	BOOL is_writeable();
	void listen(skstream **ppnew);
	bool has_socket();
	void create(int port);
	int getlasterror();
	// constants
	enum service
	{
		ftp         =      21, //tcp
		telnet      =      23, //tcp
		smtp        =      25, //tcp       mail
		time        =      37, //tcp       timserver
		name        =      42, //tcp       nameserver
		nameserver  =      53, //tcp       domain        # name-domain server
		finger      =      79, //tcp
		http        =      80, //tcp
		pop         =     109, //tcp       postoffice
		pop2        =     109, //tcp                     # Post Office
		pop3        =     110, //tcp       postoffice
		nntp        =     119, //tcp       usenet        # Network News Transfer

		clonk4			=		11111,
		clonk4i			=		11112

	} ;

	enum role {	server, client, };

	// methods
	skstream( void );
	skstream( const char *addr, const service, const role = client ) ;
	skstream( SOCKET );
	~skstream( void );

	void open( const char *addr, int service, const role = client ) ;
	void close( void ) ;
	int is_open( void ) const ;
	void attach( SOCKET = 0 );
	void set_nonblocking();
	SOCKET getsocket() const;

	char *getpeername( char *, int ) const ;
	unsigned short getport( void ) const ;
protected:
	int lasterror;
	SOCKET _socket ;
	sockbuf _sockbuf ;

	// platform dependent library housekeeping
	// 
	inline int init( void ) ;
	inline int shutdown( void ) ;
};

#endif // ndef __SOCKET_STREAM__

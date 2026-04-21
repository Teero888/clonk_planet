
// Code modified for use by Clonk engine.

// skstream.h
// Copyright (C) 1995, 1996 by John C. Wang. All Rights Reserved.
//
// You may distribute this file with your product in either of two ways:
// IN SOURCE CODE FORM: You must include this file in its entirety.
// IN OBJECT CODE FORM: You must give proper acknowledgements to the author(s)
//   of this program. This may take the form of credits on the start-up screen.
//
// IN ANYCASE, THIS SOFTWARE IS DISTRIBUTED WITHOUT ANY KIND OF EXPLICIT OR
// IMPLICIT WARRANTIES AND THE AUTHORS ARE NOT RESPONSIBLE FOR ANY EVENTS THAT
// OCCURS AS A RESULT OF USING THIS SOFTWARE.
//
// History:
// [JCW 95-Dec-04] created
// [JCW 95-Dec-20] comments added for distribution 95a
// [JCW 96-Jan-01] removed UDP capabilities from skstream
// [JCW 96-Mar-14] exceptions made optional
// [JCW 96-Oct-20] protected skstream::init and ::shutdown changed

#ifndef __SOCKET_STREAM__
#define __SOCKET_STREAM__

#include <iostream.h>
#include <winsock.h>


//
// sockbuf
//
class sockbuf : public streambuf
{
public:
	sockbuf( SOCKET & );
	sockbuf( SOCKET &, char *, int );

	virtual ~sockbuf();

	virtual int overflow(int =EOF) ;
	virtual int underflow() ;

    virtual int sync();

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
	void attach( SOCKET = NULL );
	SOCKET getsocket() const ;

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

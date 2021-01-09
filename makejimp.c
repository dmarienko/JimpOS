/* 
  _           _   Image maker
   | . |   | |_|  * 11/12/2001 - need add argv[] checking
  \  | | | | |    * 05/02/2002 - added argc checking, porting to QNX 
                  * 04-nov-2002 - replaced write( 2, ... ); to fprintf( stderr, ... );
*/

#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>

#define  JIMP_NAME	"__jimp"

long __filelength( char *cf ) {
    struct stat buf;
    if( stat( cf, &buf ) == -1 ) return 0; 
    return buf.st_size;
}

int main( int argc, char *argv[] ) {
	long fi_len, res;
	char buf[1024], *b2;
	int fi, fo;

   /* Logo */	
	fprintf( stderr, " - JIMP image making tool, ver 0.0.3 \n" );

	if( argc < 3 ) {
	  fprintf( stderr, "makejimp> Too few arguments \n Usage: makejimp boot.img kernel.img\n" );
	  exit(1);
	}

 /* Unlink jimp image if exist */
	unlink( JIMP_NAME );

	fo = open( JIMP_NAME, O_CREAT | O_RDWR,  S_IRGRP | S_IWGRP | S_IRUSR | S_IWUSR | S_IROTH | S_IWOTH );
	if( fo < 0 ){ fprintf( stderr, "makejimp> __jimp  not  opened \n" ); exit(1);}

  /* Read boot image */
	fi = open( argv[1], O_RDONLY ); /*| O_BINARY );*/
	fi_len = __filelength( argv[1] );
	if( fi_len > 512 ) { fprintf( stderr, "makejimp> boot.img have size more than 512 b\n" ); exit(1);}  
	memset( buf, 0, 512 );

	res = read( fi, buf, fi_len );
	if( res != fi_len ){ perror("makejimp> Bad read operation"); exit(1); }

  /* Insert boot id key */
	buf[510] = 0x55;
	buf[511] = 0xAA;

	if( 512 != write( fo, buf, 512 )) { perror("makejimp> Error writing"); exit(1); }
	close( fi );

  /* Read kernel image */
	fi = open( argv[2], O_RDONLY ); /* | O_BINARY );*/
	if( fi < 0 ){ fprintf( stderr, "makejimp> kernel image not found\n" ); exit(1);}
	fi_len = __filelength( argv[2] );

	b2 = (char*) calloc( fi_len, sizeof( char ) );
	if( !b2 ){ fprintf( stderr, "makejimp> Cannot allocate memory for head.img\n" ); exit(1); }

	res = read( fi, b2, fi_len );
	write( fo, b2, fi_len );

	close( fo );
	cfree( b2 );
	fprintf( stderr, "makejimp> ok\n" );
	exit(0);
	return 0;
}


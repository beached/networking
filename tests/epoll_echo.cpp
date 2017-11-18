// The MIT License (MIT)
//
// Copyright (c) 2017 Darrell Wright
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files( the "Software" ), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <cstdint>
#include <cstdlib>
#include <iostream>

#include <daw/daw_static_array.h>
#include <daw/daw_bits.h>

#include "socket.h"

int main( int, char ** ) {
	daw::net::tcp_socket srv_socket{};
	try {
		srv_socket.bind( "0.0.0.0", 12345 );
		set_non_blocking( srv_socket );
		srv_socket.listen( 5 );

		epoll_event_t event{srv_socket.socket( ), EPOLLIN | EPOLLET};
		epoll efd{ };
		efd.add( srv_socket.socket( ), event );
		daw::static_array<epoll_event_t, 64> events{ };
		while( true ) {
			try {
				for( auto const & cur_event: efd.wait( events ) ) {
					if( daw::are_set( cur_event->events, EPOLLERR, EPOLLHUP ) || !are_set( cur_event->events, EPOLLIN ) ) {
						// Error on this fd, socket is not read ready.
						std::cerr << "epoll error\n";
						continue;
					}
					
					if( srv_socket.socket( ) == cur_event->data.fd ) {
						// New incoming connection
						while( true ) {
							auto const client = srv_socket.accept( );
						}
					}
				}
			} catch( std::exception &ex ) {
				std::cerr << ex.what( ) << '\n';
				exit( EXIT_FAILURE );
			}
		}
	} catch( std::exception &ex ) {
		std::cerr << ex.what( ) << '\n';
		exit( EXIT_FAILURE );
	}
	return EXIT_SUCCESS;
}


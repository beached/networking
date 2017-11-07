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

#include "socket.h"

int main( int, char ** ) {
	daw::net::tcp_socket srv_socket{};
	try {
		srv_socket.bind( "0.0.0.0", 12345 );
		srv_socket.listen( 5 );
	} catch( std::exception &ex ) {
		std::cerr << ex.what( ) << '\n';
		exit( EXIT_FAILURE );
	}
	while( true ) {
		try {
			auto client = srv_socket.accept( );
			client->send( daw::string_view{""}, 0 );
			daw::static_array_t<char, 128> buff_raw{0};
			auto buff = daw::make_span( buff_raw );
			int how_many = 0;
			while( ( how_many = client->receive( buff, 0 ) ) > 0 ) {
				client->send( buff.subset( 0, how_many ), 0 );
				buff_raw.fill( 0 );
			}
		} catch( std::exception &ex ) {
			std::cerr << ex.what( ) << '\n';
			exit( EXIT_FAILURE );
		}
	}
	return EXIT_SUCCESS;
}


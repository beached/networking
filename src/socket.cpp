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

#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "socket.h"

namespace daw {
	namespace net {
		tcp_socket::tcp_socket( int family, int flags ) : tcp_socket{} {
			m_info.ai_family = family;
			m_info.ai_socktype = SOCK_STREAM;
			m_info.ai_flags = flags;

			if( family == AF_UNSPEC ) {
				return;
			}

			m_socket = socket( m_info.ai_family, m_info.ai_socktype, 0 );
			daw::exception::daw_throw_on_true( m_socket < 0, std::string{strerror( errno )} );

			option_socket_created( true );
		}

		tcp_socket::tcp_socket( int socket, addrinfo info, bool bound, bool connected )
		  : m_info{}, m_srvinfo{new addrinfo{std::move( info )}}, m_socket{socket}, m_options{} {

			option_bound( bound );
			option_connected( connected );
		}

		tcp_socket::~tcp_socket( ) noexcept {
			try {
				if( !option_closed( ) ) {
					close( );
				}
			} catch( ... ) {
				// TODO: determine best course of action if ignoring is not it
			}
			freeaddrinfo( m_srvinfo );
		}

		void tcp_socket::bind( daw::string_view address, uint16_t port ) {
			daw::exception::daw_throw_on_true( option_bound( ) && option_connected( ), "Already bound" );

			set_info( address, port );

			for( auto result = m_srvinfo; result != nullptr; result = m_srvinfo->ai_next ) {
				if( !option_socket_created( ) ) {
					try {
						open_socket( result );
					} catch( ... ) { continue; }
				}
				if(::bind( m_socket, result->ai_addr, result->ai_addrlen ) == 0 ) {
					option_bound( true );
					return;
				}
			}
			daw::exception::daw_throw( "Can't bind to port" );
		}

		void tcp_socket::connect( daw::string_view address, uint16_t port ) {
			daw::exception::daw_throw_on_true( option_connected( ), "Already connected" );

			set_info( address, htons( port ) );

			for( auto result = m_srvinfo; result != nullptr; result = m_info.ai_next ) {
				if( !option_socket_created( ) ) {
					try {
						open_socket( result );
					} catch( ... ) { continue; }
				}
				if(::connect( m_socket, result->ai_addr, result->ai_addrlen ) == 0 ) {
					option_connected( true );
					return;
				}
			}
			daw::exception::daw_throw( "Can't connect to host" );
		}

		void tcp_socket::listen( int max_queue ) {
			auto const status = ::listen( m_socket, max_queue );
			daw::exception::daw_throw_on_true( status != 0, std::string{strerror( errno )} );
		}

		std::shared_ptr<tcp_socket> tcp_socket::accept( ) {

			union {
				sockaddr addr;
				sockaddr_in in;
				sockaddr_in6 in6;
				sockaddr_storage s;
			} address;

			socklen_t address_size = sizeof( sockaddr_storage );

			auto new_sock = ::accept( m_socket, reinterpret_cast<sockaddr *>( &address.s ), &address_size );
			daw::exception::daw_throw_on_true( new_sock < 0, std::string{strerror( errno )} );

			addrinfo info{};

			if( address.s.ss_family == AF_INET ) {
				info.ai_family = AF_INET;
				info.ai_addr = new sockaddr{address.addr};
			} else {
				info.ai_family = AF_INET6;
				info.ai_addr = new sockaddr{address.addr};
			}

			return std::make_shared<tcp_socket>( new_sock, info, true, false );
		}

		void tcp_socket::send( daw::array_view<char> data, int flags ) {
			while( !data.empty( ) ) {
				auto const status = ::send( m_socket, data.data( ), data.size( ), flags );
				daw::exception::daw_throw_on_true( status < 0, std::string{strerror( errno )} );
				data.remove_prefix( static_cast<size_t>( status ) );
			}
		}

		daw::span<char> tcp_socket::receive( daw::span<char> data, int flags ) {
			auto const status = ::recv( m_socket, data.data( ), data.size( ), flags );
			daw::exception::daw_throw_on_true( status < 0, std::string{strerror( errno )} );
			return data.subset( 0, status );
		}

		void tcp_socket::close( ) {
			auto const status = ::close( m_socket );
			daw::exception::daw_throw_on_true( status < 0, std::string{strerror( errno )} );
			option_closed( true );
		}

		void tcp_socket::set_info( daw::string_view address, uint16_t port ) {
			auto const status = getaddrinfo( address.c_str( ), std::to_string( port ).c_str( ), &m_info, &m_srvinfo );
			if( status != 0 ) {
				daw::exception::daw_throw( "getaddrinfo returned non-zero" + std::string{gai_strerror( status )} );
			}
		}

		void tcp_socket::open_socket( addrinfo *info ) {
			m_socket = socket( info->ai_family, info->ai_socktype, info->ai_protocol );
			daw::exception::daw_throw_on_true( m_socket < 0, std::string{strerror( errno )} );
		}

		void set_non_blocking( tcp_socket const &socket ) {
			auto flags = fcntl( socket.raw_socket( ), F_GETFL, 0 );
			daw::exception::daw_throw_on_true( flags < 0, std::string{strerror( errno )} );
			flags |= O_NONBLOCK;
			auto const status = fcntl( socket.raw_socket( ), F_SETFL, flags );
			daw::exception::daw_throw_on_true( status < 0, std::string{strerror( errno )} );
		}
	} // namespace net
} // namespace daw

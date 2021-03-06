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

#pragma once

#include <memory>
#include <netdb.h>

#ifdef major
#undef major
#endif
#ifdef minor
#undef minor
#endif

#include <daw/daw_array_view.h>
#include <daw/daw_span.h>
#include <daw/daw_string_view.h>
namespace daw {
	namespace net {

		class tcp_socket {
			addrinfo m_info;
			addrinfo *m_srvinfo;
			int m_socket;
			enum class socket_options { socket_created = 0, bound = 1, connected = 2, closed = 3 };
			std::bitset<4> m_options;

			void set_info( daw::string_view address, uint16_t port );
			void open_socket( addrinfo *info );

			inline void set_info( uint16_t port ) {
				set_info( daw::string_view{}, port );
			}

			inline bool option_socket_created( ) const {
				return m_options[static_cast<size_t>( socket_options::socket_created )];
			}

			inline void option_socket_created( bool val ) {
				m_options[static_cast<size_t>( socket_options::socket_created )] = val;
			}

			inline bool option_bound( ) const {
				return m_options[static_cast<size_t>( socket_options::bound )];
			}

			inline void option_bound( bool val ) {
				m_options[static_cast<size_t>( socket_options::bound )] = val;
			}

			inline bool option_connected( ) const {
				return m_options[static_cast<size_t>( socket_options::connected )];
			}

			inline void option_connected( bool val ) {
				m_options[static_cast<size_t>( socket_options::connected )] = val;
			}

			inline bool option_closed( ) const {
				return m_options[static_cast<size_t>( socket_options::closed )];
			}

			inline void option_closed( bool val ) {
				m_options[static_cast<size_t>( socket_options::closed )] = val;
			}

		public:
			constexpr tcp_socket( ) noexcept : m_info{}, m_srvinfo{nullptr}, m_socket{-1}, m_options{} {
				m_info.ai_family = AF_UNSPEC;
				m_info.ai_socktype = SOCK_STREAM;
			}

			tcp_socket( tcp_socket const &socket ) = delete;
			tcp_socket &operator=( tcp_socket const &socket ) = delete;

			tcp_socket( int socket, addrinfo info, bool bound, bool connected );
			tcp_socket( int family, int flags );
			~tcp_socket( ) noexcept;
			tcp_socket( tcp_socket &&socket ) noexcept = default;
			tcp_socket &operator=( tcp_socket &&socket ) noexcept = default;

			void bind( daw::string_view address, uint16_t port );
			void connect( daw::string_view address, uint16_t port );
			void listen( int max_queue );
			std::shared_ptr<tcp_socket> accept( );
			void send( daw::array_view<char> data, int flags );
			daw::span<char> receive( daw::span<char> data, int flags );
			void close( );

			inline void bind( uint16_t port ) {
				bind( daw::string_view{}, port );
			}

			inline void send( daw::array_view<char> data ) {
				send( data, 0 );
			}

			inline void send( daw::span<char> const data, int flags ) {
				send( daw::array_view<char>{data.cbegin( ), data.size( )}, flags );
			}

			inline void send( daw::span<char> const data ) {
				send( daw::array_view<char>{data.cbegin( ), data.size( )}, 0 );
			}

			inline void send( daw::string_view data, int flags ) {
				send( daw::array_view<char>{data.cbegin( ), data.size( )}, flags );
			}

			inline void send( daw::string_view data ) {
				send( daw::array_view<char>{data.cbegin( ), data.size( )}, 0 );
			}

			inline daw::span<char> receive( daw::span<char> data ) {
				return receive( data, 0 );
			}

			template<typename Buffer>
			inline daw::span<char> receive( Buffer &buff, int flags ) {
				return receive( daw::make_span( buff, 0, buff.size( ) ), flags );
			}

			template<typename Buffer>
			inline daw::span<char> receive( Buffer &buff ) {
				return receive( daw::make_span( buff, 0, buff.size( ) ), 0 );
			}

			constexpr int raw_socket( ) const noexcept {
				return m_socket;
			}
		};

		void set_non_blocking( tcp_socket const &socket );
	} // namespace net
} // namespace daw


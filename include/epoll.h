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

#include <chrono>
#include <cstdint>
#include <sys/epoll.h>
#include <vector>

namespace daw {
	class epoll_event_t {
		epoll_event m_data;

	public:
		constexpr epoll_event_t( ) noexcept : m_data{} {}

		constexpr epoll_event_t( int file_descriptor ) noexcept : m_data{} {
			m_data.data.fd = file_descriptor;
		}

		constexpr epoll_event_t( int file_descriptor, uint32_t events_ ) noexcept : m_data{} {
			m_data.data.fd = file_descriptor;
			m_data.events = events_;
		}

		constexpr epoll_event &get( ) noexcept {
			return m_data;
		}

		constexpr epoll_event const &get( ) const noexcept {
			return m_data;
		}

		constexpr epoll_event &operator*( ) noexcept {
			return m_data;
		}

		constexpr epoll_event const &operator*( ) const noexcept {
			return m_data;
		}

		constexpr epoll_event *operator->( ) noexcept {
			return &m_data;
		}

		constexpr epoll_event const *operator*( ) const noexcept {
			return &m_data;
		}
	};

	enum class epoll_flags { none = 0, close_on_exec = EPOLL_CLOEXEC };
	class epoll {
		int m_handle;

	public:
		inline epoll( flags ) : m_handle{::epoll_create1( static_cast<int>( flags ) )} {
			daw::exception::daw_throw_on_true( status < 0, std::string{strerror( errno )} );
		}

		inline epoll( ) : epoll{epoll_flags::none} {}

		inline ~epoll( ) noexcept {
			if( m_handle >= 0 ) {
				::close( std::exchange( m_handle, -1 ) );
			}
		}

		inline void add( int file_descriptor, epoll_event &event ) {
			auto const status = ::epoll_ctl( m_handle, EPOLL_CTL_ADD, file_descriptor, &event.get( ) );
			daw::exception::daw_throw_on_false( status == 0, std::string{strerror( errno )} );
		}

		inline void mod( int file_descriptor, epoll_event &event ) {
			auto const status = ::epoll_ctl( m_handle, EPOLL_CTL_MOD, file_descriptor, &event.get( ) );
			daw::exception::daw_throw_on_false( status == 0, std::string{strerror( errno )} );
		}

		inline void del( int file_descriptor, epoll_event &event ) {
			auto const status = ::epoll_ctl( m_handle, EPOLL_CTL_MOD, file_descriptor, &event.get( ) );
			daw::exception::daw_throw_on_false( status == 0, std::string{strerror( errno )} );
		}

		inline daw::span<epoll_event_t> wait( daw::span<epoll_event> events, std::chrono::milliseconds timeout ) {
			auto const status = ::epoll_wait( m_handle, &events.data, max_events, timeout.count( ) );
			daw::exception::daw_throw_on_true( status < 0, std::string{strerror( errno )} );
			return events.subset( 0, status );
		}

		inline daw::span<epoll_event_t> wait( daw::span<epoll_event> events ) {
			auto const status = ::epoll_wait( m_handle, &events.data, max_events, -1 );
			daw::exception::daw_throw_on_true( status < 0, std::string{strerror( errno )} );
			return events.subset( 0, status );
		}

		template<typename EventsBuffer>
		inline daw::span<epoll_event_t> wait( EventsBuffer &events, std::chrono::milliseconds timeout ) {
			return wait( daw::make_span( events, 0, events.size( ) ), timeout );
		}

		template<typename EventsBuffer>
		inline daw::span<epoll_event_t> wait( EventsBuffer &events ) {
			return wait( daw::make_span( events, 0, events.size( ) ), -1 );
		}
	}
} // namespace daw


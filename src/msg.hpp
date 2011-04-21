/*
    Copyright (c) 2007-2011 iMatix Corporation
    Copyright (c) 2007-2011 Other contributors as noted in the AUTHORS file

    This file is part of 0MQ.

    0MQ is free software; you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    0MQ is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __ZMQ_MSG_HPP_INCLUDE__
#define __ZMQ_MSG_HPP_INCLUDE__

#include <stddef.h>

#include "config.hpp"
#include "atomic_counter.hpp"

namespace zmq
{

    //  Note that this structure needs to be explicitly constructed
    //  (init functions) and destructed (close function).

    class msg_t
    {
    public:

        //  Mesage flags.
        enum
        {
            more = 1,
            shared = 128
        };

        //  Signature for free function to deallocate the message content.
        typedef void (free_fn_t) (void *data, void *hint);

        bool check ();
        int init ();
        int init_size (size_t size_);
        int init_data (void *data_, size_t size_, free_fn_t *ffn_,
            void *hint_);
        int init_delimiter ();
        int close ();
        int move (msg_t &src_);
        int copy (msg_t &src_);
        void *data ();
        size_t size ();
        unsigned char flags ();
        void set_flags (unsigned char flags_);
        void reset_flags (unsigned char flags_);
        bool is_delimiter ();

        //  After calling this function you can copy the message in POD-style
        //  refs_ times. No need to call copy.
        void add_refs (int refs_);

        //  Removes references previously added by add_refs.
        void rm_refs (int refs_);

    private:

        //  Shared message buffer. Message data are either allocated in one
        //  continuous block along with this structure - thus avoiding one
        //  malloc/free pair or they are stored in used-supplied memory.
        //  In the latter case, ffn member stores pointer to the function to be
        //  used to deallocate the data. If the buffer is actually shared (there
        //  are at least 2 references to it) refcount member contains number of
        //  references.
        struct content_t
        {
            void *data;
            size_t size;
            free_fn_t *ffn;
            void *hint;
            zmq::atomic_counter_t refcnt;
        };

        //  Different message types.
        enum type_t
        {
            type_min = 101,
            type_vsm = 101,
            type_lmsg = 102,
            type_delimiter = 103,
            type_max = 103
        };

        //  Note that fields shared between different message types are not
        //  moved to tha parent class (msg_t). This way we ger tighter packing
        //  of the data. Shared fields can be accessed via 'base' member of
        //  the union.
        union {
            struct {
                unsigned char type;
                unsigned char flags;
            } base;
            struct {
                unsigned char type;
                unsigned char flags;
                unsigned char size;
                unsigned char data [max_vsm_size];
            } vsm;
            struct {
                unsigned char type;
                unsigned char flags;
                content_t *content;
            } lmsg;
            struct {
                unsigned char type;
                unsigned char flags;
            } delimiter;
        } u;
    };

}

#endif

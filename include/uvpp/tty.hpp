#pragma once

#include "handle.hpp"
#include "error.hpp"

namespace uvpp {
class TTY : public Stream<TTY, uv_tty_t>
{
public:

    enum Type
    {
        STDIN,
        STDOUT,
        STDERR
    };

    TTY(Type type, bool readable)
		: type_(type)
    {
        uv_tty_init(uv_default_loop(), get(), 
			type_, readable ? 1 : 0);
    }

    TTY(Loop& l, Type type, bool readable)
		: type_(type)
    {
        uv_tty_init(l.get(), get(), type_, readable ? 1 : 0);
    }
	/*
	UV_EXTERN int uv_tty_set_mode(uv_tty_t*, uv_tty_mode_t mode);
	UV_EXTERN int uv_tty_reset_mode(void);
	UV_EXTERN int uv_tty_get_winsize(uv_tty_t*, int* width, int* height);
	UV_EXTERN void uv_tty_set_vterm_state(uv_tty_vtermstate_t state);
	UV_EXTERN int uv_tty_get_vterm_state(uv_tty_vtermstate_t* state);
	*/

private:
    Type type_;
};
}
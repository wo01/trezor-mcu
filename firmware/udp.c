/*
 * This file is part of the TREZOR project, https://trezor.io/
 *
 * Copyright (C) 2017 Saleem Rashid <trezor@saleemrashid.com>
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdint.h>

#include "usb.h"

#include "messages.h"
#include "timer.h"
#include "debug.h"

static volatile char tiny = 0;

void usbInit(void) {
	emulatorSocketInit();
}

#if DEBUG_LINK
#define _ISDBG ((debug ? 'd' : 'n'))
#else
#define _ISDBG ('n')
#endif

void usbPoll(void) {
	emulatorPoll();

	static uint8_t buffer[64];

	int debug = 0;
	if (emulatorSocketRead(buffer, sizeof(buffer), &debug) > 0) {
		if (!tiny) {
			msg_read_common(_ISDBG, buffer, sizeof(buffer));
		} else {
			msg_read_tiny(buffer, sizeof(buffer));
		}
	}

	debug = 0;
	const uint8_t *data = msg_out_data();

#if DEBUG_LINK
	if (data == NULL) {
		debug = 1;
		data = msg_debug_out_data();
	}
#endif

	if (data != NULL) {
		emulatorSocketWrite(data, 64, debug);
	}
}

char usbTiny(char set) {
	char old = tiny;
	tiny = set;
	return old;
}

void usbSleep(uint32_t millis) {
	uint32_t start = timer_ms();

	while ((timer_ms() - start) < millis) {
		usbPoll();
	}
}

/*
 * Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 */

#അന്തര്ഭാവ <cros_ec.h>
#അന്തര്ഭാവ <dm.h>
#അന്തര്ഭാവ <errno.h>
#അന്തര്ഭാവ <log.h>

ഘടന udevice *board_get_cros_ec_dev(ശൂന്യം)
{
	ഘടന udevice *dev;
	പൂർണ്ണസംഖ്യ ret;

	ret = uclass_get_device(UCLASS_CROS_EC, 0, &dev);
	എങ്കിൽ (ret) {
		debug("%s: Error %d\n", __func__, ret);
		തിരികെ ശൂന്യ;
	}
	തിരികെ dev;
}

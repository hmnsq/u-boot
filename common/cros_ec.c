/*
 * Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 */

#અન્તર્ભાવ <cros_ec.h>
#અન્તર્ભાવ <dm.h>
#અન્તર્ભાવ <errno.h>
#અન્તર્ભાવ <log.h>

સંરચના udevice *board_get_cros_ec_dev(શૂન્ય)
{
	સંરચના udevice *dev;
	પૂર્ણાંક ret;

	ret = uclass_get_device(UCLASS_CROS_EC, 0, &dev);
	જો (ret) {
		debug("%s: Error %d\n", __func__, ret);
		પાછા ખાલી;
	}
	પાછા dev;
}

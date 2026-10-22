/*
 * Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 */

#ਅਨ੍ਤਰ੍ਭਾਵ <cros_ec.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <dm.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <log.h>

ਬਣਤਰ udevice *board_get_cros_ec_dev(ਖਾਲੀ)
{
	ਬਣਤਰ udevice *dev;
	ਪੂਰਨ ret;

	ret = uclass_get_device(UCLASS_CROS_EC, 0, &dev);
	ਜੇ (ret) {
		debug("%s: Error %d\n", __func__, ret);
		ਵਾਪਸ ਨਿਰਾ;
	}
	ਵਾਪਸ dev;
}

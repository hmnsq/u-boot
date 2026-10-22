/*
 * Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 */

#شامل <cros_ec.h>
#شامل <dm.h>
#شامل <errno.h>
#شامل <log.h>

ساخت udevice *board_get_cros_ec_dev(خالی)
{
	ساخت udevice *dev;
	صحیح ret;

	ret = uclass_get_device(UCLASS_CROS_EC, 0, &dev);
	اگر (ret) {
		debug("%s: Error %d\n", __func__, ret);
		واپس تہی;
	}
	واپس dev;
}

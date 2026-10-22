/*
 * Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 */

#அந்தர்பாவ <cros_ec.h>
#அந்தர்பாவ <dm.h>
#அந்தர்பாவ <errno.h>
#அந்தர்பாவ <log.h>

கட்டமைப்பு udevice *board_get_cros_ec_dev(வெறுமை)
{
	கட்டமைப்பு udevice *dev;
	முழு ret;

	ret = uclass_get_device(UCLASS_CROS_EC, 0, &dev);
	எனில் (ret) {
		debug("%s: Error %d\n", __func__, ret);
		திரும்பு வெற்று;
	}
	திரும்பு dev;
}

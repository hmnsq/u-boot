/*
 * Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 */

#অন্তর্ভাব <cros_ec.h>
#অন্তর্ভাব <dm.h>
#অন্তর্ভাব <errno.h>
#অন্তর্ভাব <log.h>

কাঠামো udevice *board_get_cros_ec_dev(শূন্য)
{
	কাঠামো udevice *dev;
	পূর্ণসংখ্যা ret;

	ret = uclass_get_device(UCLASS_CROS_EC, 0, &dev);
	যদি (ret) {
		debug("%s: Error %d\n", __func__, ret);
		ফেরত ফাঁকা;
	}
	ফেরত dev;
}

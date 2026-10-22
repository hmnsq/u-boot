/*
 * Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 */

#అన్తర్భావ <cros_ec.h>
#అన్తర్భావ <dm.h>
#అన్తర్భావ <errno.h>
#అన్తర్భావ <log.h>

నిర్మాణం udevice *board_get_cros_ec_dev(శూన్య)
{
	నిర్మాణం udevice *dev;
	పూర్ణాంకం ret;

	ret = uclass_get_device(UCLASS_CROS_EC, 0, &dev);
	ఒకవేళ (ret) {
		debug("%s: Error %d\n", __func__, ret);
		తిరిగి ఖాళీ;
	}
	తిరిగి dev;
}

/*
 * Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 */

#ଅନ୍ତର୍ଭାଵ <cros_ec.h>
#ଅନ୍ତର୍ଭାଵ <dm.h>
#ଅନ୍ତର୍ଭାଵ <errno.h>
#ଅନ୍ତର୍ଭାଵ <log.h>

ସଂରଚନା udevice *board_get_cros_ec_dev(ଶୂନ୍ୟ)
{
	ସଂରଚନା udevice *dev;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;

	ret = uclass_get_device(UCLASS_CROS_EC, 0, &dev);
	ଯଦି (ret) {
		debug("%s: Error %d\n", __func__, ret);
		ଫେରନ୍ତୁ ଖାଲି;
	}
	ଫେରନ୍ତୁ dev;
}

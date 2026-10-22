/*
 * Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 */

#ಅನ್ತರ್ಭಾವ <cros_ec.h>
#ಅನ್ತರ್ಭಾವ <dm.h>
#ಅನ್ತರ್ಭಾವ <errno.h>
#ಅನ್ತರ್ಭಾವ <log.h>

ಸಂರಚನಾ udevice *board_get_cros_ec_dev(ಶೂನ್ಯ)
{
	ಸಂರಚನಾ udevice *dev;
	ಪೂರ್ಣಾಙ್ಕ ret;

	ret = uclass_get_device(UCLASS_CROS_EC, 0, &dev);
	ಯದಿ (ret) {
		debug("%s: Error %d\n", __func__, ret);
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}
	ಪ್ರತ್ಯಾಗಮ dev;
}

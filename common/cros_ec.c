/*
 * Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 */

#अन्तर्भाव <cros_ec.h>
#अन्तर्भाव <dm.h>
#अन्तर्भाव <errno.h>
#अन्तर्भाव <log.h>

संरचना udevice *board_get_cros_ec_dev(शून्य)
{
	संरचना udevice *dev;
	पूर्णाङ्क ret;

	ret = uclass_get_device(UCLASS_CROS_EC, 0, &dev);
	यदि (ret) {
		debug("%s: Error %d\n", __func__, ret);
		फर्काऊ रिक्त;
	}
	फर्काऊ dev;
}

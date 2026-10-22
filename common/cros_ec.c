/*
 * Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 */

#समावेश <cros_ec.h>
#समावेश <dm.h>
#समावेश <errno.h>
#समावेश <log.h>

काष्ठा udevice *board_get_cros_ec_dev(व्योम)
{
	काष्ठा udevice *dev;
	पूर्णांक ret;

	ret = uclass_get_device(UCLASS_CROS_EC, 0, &dev);
	अगर (ret) {
		debug("%s: Error %d\n", __func__, ret);
		वापस शून्य;
	}
	वापस dev;
}

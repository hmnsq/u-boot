// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2018, Linaro Limited
 */

#ضمن <avb_verify.h>
#ضمن <blk.h>
#ضمن <cpu_func.h>
#ضمن <image.h>
#ضمن <malloc.h>
#ضمن <part.h>
#ضمن <tee.h>
#ضمن <tee/optee_ta_avb.h>
#ضمن <u-boot/sha256.h>

#إذا CONFIG_IS_ENABLED(AVB_جذر_مفتاح_BUILTIN)
ساكن ثابت unsigned حرف avb_جذر_pub[1032] = {
	0x0, 0x0, 0x10, 0x0, 0x55, 0xd9, 0x4, 0xad, 0xd8, 0x4,
	0xaf, 0xe3, 0xd3, 0x84, 0x6c, 0x7e, 0xd, 0x89, 0x3d, 0xc2,
	0x8c, 0xd3, 0x12, 0x55, 0xe9, 0x62, 0xc9, 0xf1, 0xf, 0x5e,
	0xcc, 0x16, 0x72, 0xab, 0x44, 0x7c, 0x2c, 0x65, 0x4a, 0x94,
	0xb5, 0x16, 0x2b, 0x0, 0xbb, 0x6, 0xef, 0x13, 0x7, 0x53,
	0x4c, 0xf9, 0x64, 0xb9, 0x28, 0x7a, 0x1b, 0x84, 0x98, 0x88,
	0xd8, 0x67, 0xa4, 0x23, 0xf9, 0xa7, 0x4b, 0xdc, 0x4a, 0xf,
	0xf7, 0x3a, 0x18, 0xae, 0x54, 0xa8, 0x15, 0xfe, 0xb0, 0xad,
	0xac, 0x35, 0xda, 0x3b, 0xad, 0x27, 0xbc, 0xaf, 0xe8, 0xd3,
	0x2f, 0x37, 0x34, 0xd6, 0x51, 0x2b, 0x6c, 0x5a, 0x27, 0xd7,
	0x96, 0x6, 0xaf, 0x6b, 0xb8, 0x80, 0xca, 0xfa, 0x30, 0xb4,
	0xb1, 0x85, 0xb3, 0x4d, 0xaa, 0xaa, 0xc3, 0x16, 0x34, 0x1a,
	0xb8, 0xe7, 0xc7, 0xfa, 0xf9, 0x9, 0x77, 0xab, 0x97, 0x93,
	0xeb, 0x44, 0xae, 0xcf, 0x20, 0xbc, 0xf0, 0x80, 0x11, 0xdb,
	0x23, 0xc, 0x47, 0x71, 0xb9, 0x6d, 0xd6, 0x7b, 0x60, 0x47,
	0x87, 0x16, 0x56, 0x93, 0xb7, 0xc2, 0x2a, 0x9a, 0xb0, 0x4c,
	0x1, 0xc, 0x30, 0xd8, 0x93, 0x87, 0xf0, 0xed, 0x6e, 0x8b,
	0xbe, 0x30, 0x5b, 0xf6, 0xa6, 0xaf, 0xdd, 0x80, 0x7c, 0x45,
	0x5e, 0x8f, 0x91, 0x93, 0x5e, 0x44, 0xfe, 0xb8, 0x82, 0x7,
	0xee, 0x79, 0xca, 0xbf, 0x31, 0x73, 0x62, 0x58, 0xe3, 0xcd,
	0xc4, 0xbc, 0xc2, 0x11, 0x1d, 0xa1, 0x4a, 0xbf, 0xfe, 0x27,
	0x7d, 0xa1, 0xf6, 0x35, 0xa3, 0x5e, 0xca, 0xdc, 0x57, 0x2f,
	0x3e, 0xf0, 0xc9, 0x5d, 0x86, 0x6a, 0xf8, 0xaf, 0x66, 0xa7,
	0xed, 0xcd, 0xb8, 0xed, 0xa1, 0x5f, 0xba, 0x9b, 0x85, 0x1a,
	0xd5, 0x9, 0xae, 0x94, 0x4e, 0x3b, 0xcf, 0xcb, 0x5c, 0xc9,
	0x79, 0x80, 0xf7, 0xcc, 0xa6, 0x4a, 0xa8, 0x6a, 0xd8, 0xd3,
	0x31, 0x11, 0xf9, 0xf6, 0x2, 0x63, 0x2a, 0x1a, 0x2d, 0xd1,
	0x1a, 0x66, 0x1b, 0x16, 0x41, 0xbd, 0xbd, 0xf7, 0x4d, 0xc0,
	0x4a, 0xe5, 0x27, 0x49, 0x5f, 0x7f, 0x58, 0xe3, 0x27, 0x2d,
	0xe5, 0xc9, 0x66, 0xe, 0x52, 0x38, 0x16, 0x38, 0xfb, 0x16,
	0xeb, 0x53, 0x3f, 0xe6, 0xfd, 0xe9, 0xa2, 0x5e, 0x25, 0x59,
	0xd8, 0x79, 0x45, 0xff, 0x3, 0x4c, 0x26, 0xa2, 0x0, 0x5a,
	0x8e, 0xc2, 0x51, 0xa1, 0x15, 0xf9, 0x7b, 0xf4, 0x5c, 0x81,
	0x9b, 0x18, 0x47, 0x35, 0xd8, 0x2d, 0x5, 0xe9, 0xad, 0xf,
	0x35, 0x74, 0x15, 0xa3, 0x8e, 0x8b, 0xcc, 0x27, 0xda, 0x7c,
	0x5d, 0xe4, 0xfa, 0x4, 0xd3, 0x5, 0xb, 0xba, 0x3a, 0xb2,
	0x49, 0x45, 0x2f, 0x47, 0xc7, 0xd, 0x41, 0x3f, 0x97, 0x80,
	0x4d, 0x3f, 0xc1, 0xb5, 0xbb, 0x70, 0x5f, 0xa7, 0x37, 0xaf,
	0x48, 0x22, 0x12, 0x45, 0x2e, 0xf5, 0xf, 0x87, 0x92, 0xe2,
	0x84, 0x1, 0xf9, 0x12, 0xf, 0x14, 0x15, 0x24, 0xce, 0x89,
	0x99, 0xee, 0xb9, 0xc4, 0x17, 0x70, 0x70, 0x15, 0xea, 0xbe,
	0xc6, 0x6c, 0x1f, 0x62, 0xb3, 0xf4, 0x2d, 0x16, 0x87, 0xfb,
	0x56, 0x1e, 0x45, 0xab, 0xae, 0x32, 0xe4, 0x5e, 0x91, 0xed,
	0x53, 0x66, 0x5e, 0xbd, 0xed, 0xad, 0xe6, 0x12, 0x39, 0xd,
	0x83, 0xc9, 0xe8, 0x6b, 0x6c, 0x2d, 0xa5, 0xee, 0xc4, 0x5a,
	0x66, 0xae, 0x8c, 0x97, 0xd7, 0xd, 0x6c, 0x49, 0xc7, 0xf5,
	0xc4, 0x92, 0x31, 0x8b, 0x9, 0xee, 0x33, 0xda, 0xa9, 0x37,
	0xb6, 0x49, 0x18, 0xf8, 0xe, 0x60, 0x45, 0xc8, 0x33, 0x91,
	0xef, 0x20, 0x57, 0x10, 0xbe, 0x78, 0x2d, 0x83, 0x26, 0xd6,
	0xca, 0x61, 0xf9, 0x2f, 0xe0, 0xbf, 0x5, 0x30, 0x52, 0x5a,
	0x12, 0x1c, 0x0, 0xa7, 0x5d, 0xcc, 0x7c, 0x2e, 0xc5, 0x95,
	0x8b, 0xa3, 0x3b, 0xf0, 0x43, 0x2e, 0x5e, 0xdd, 0x0, 0xdb,
	0xd, 0xb3, 0x37, 0x99, 0xa9, 0xcd, 0x9c, 0xb7, 0x43, 0xf7,
	0x35, 0x44, 0x21, 0xc2, 0x82, 0x71, 0xab, 0x8d, 0xaa, 0xb4,
	0x41, 0x11, 0xec, 0x1e, 0x8d, 0xfc, 0x14, 0x82, 0x92, 0x4e,
	0x83, 0x6a, 0xa, 0x6b, 0x35, 0x5e, 0x5d, 0xe9, 0x5c, 0xcc,
	0x8c, 0xde, 0x39, 0xd1, 0x4a, 0x5b, 0x5f, 0x63, 0xa9, 0x64,
	0xe0, 0xa, 0xcb, 0xb, 0xb8, 0x5a, 0x7c, 0xc3, 0xb, 0xe6,
	0xbe, 0xfe, 0x8b, 0xf, 0x7d, 0x34, 0x8e, 0x2, 0x66, 0x74,
	0x1, 0x6c, 0xca, 0x76, 0xac, 0x7c, 0x67, 0x8, 0x2f, 0x3f,
	0x1a, 0xa6, 0x2c, 0x60, 0xb3, 0xff, 0xda, 0x8d, 0xb8, 0x12,
	0xc, 0x0, 0x7f, 0xcc, 0x50, 0xa1, 0x5c, 0x64, 0xa1, 0xe2,
	0x5f, 0x32, 0x65, 0xc9, 0x9c, 0xbe, 0xd6, 0xa, 0x13, 0x87,
	0x3c, 0x2a, 0x45, 0x47, 0xc, 0xca, 0x42, 0x82, 0xfa, 0x89,
	0x65, 0xe7, 0x89, 0xb4, 0x8f, 0xf7, 0x1e, 0xe6, 0x23, 0xa5,
	0xd0, 0x59, 0x37, 0x79, 0x92, 0xd7, 0xce, 0x3d, 0xfd, 0xe3,
	0xa1, 0xb, 0xcf, 0x6c, 0x85, 0xa0, 0x65, 0xf3, 0x5c, 0xc6,
	0x4a, 0x63, 0x5f, 0x6e, 0x3a, 0x3a, 0x2a, 0x8b, 0x6a, 0xb6,
	0x2f, 0xbb, 0xf8, 0xb2, 0x4b, 0x62, 0xbc, 0x1a, 0x91, 0x25,
	0x66, 0xe3, 0x69, 0xca, 0x60, 0x49, 0xb, 0xf6, 0x8a, 0xbe,
	0x3e, 0x76, 0x53, 0xc2, 0x7a, 0xa8, 0x4, 0x17, 0x75, 0xf1,
	0xf3, 0x3, 0x62, 0x1b, 0x85, 0xb2, 0xb0, 0xef, 0x80, 0x15,
	0xb6, 0xd4, 0x4e, 0xdf, 0x71, 0xac, 0xdb, 0x2a, 0x4, 0xd4,
	0xb4, 0x21, 0xba, 0x65, 0x56, 0x57, 0xe8, 0xfa, 0x84, 0xa2,
	0x7d, 0x13, 0xe, 0xaf, 0xd7, 0x9a, 0x58, 0x2a, 0xa3, 0x81,
	0x84, 0x8d, 0x9, 0xa0, 0x6a, 0xc1, 0xbb, 0xd9, 0xf5, 0x86,
	0xac, 0xbd, 0x75, 0x61, 0x9, 0xe6, 0x8c, 0x3d, 0x77, 0xb2,
	0xed, 0x30, 0x20, 0xe4, 0x0, 0x1d, 0x97, 0xe8, 0xbf, 0xc7,
	0x0, 0x1b, 0x21, 0xb1, 0x16, 0xe7, 0x41, 0x67, 0x2e, 0xec,
	0x38, 0xbc, 0xe5, 0x1b, 0xb4, 0x6, 0x23, 0x31, 0x71, 0x1c,
	0x49, 0xcd, 0x76, 0x4a, 0x76, 0x36, 0x8d, 0xa3, 0x89, 0x8b,
	0x4a, 0x7a, 0xf4, 0x87, 0xc8, 0x15, 0xf, 0x37, 0x39, 0xf6,
	0x6d, 0x80, 0x19, 0xef, 0x5c, 0xa8, 0x66, 0xce, 0x1b, 0x16,
	0x79, 0x21, 0xdf, 0xd7, 0x31, 0x30, 0xc4, 0x21, 0xdd, 0x34,
	0x5b, 0xd2, 0x1a, 0x2b, 0x3e, 0x5d, 0xf7, 0xea, 0xca, 0x5,
	0x8e, 0xb7, 0xcb, 0x49, 0x2e, 0xa0, 0xe3, 0xf4, 0xa7, 0x48,
	0x19, 0x10, 0x9c, 0x4, 0xa7, 0xf4, 0x28, 0x74, 0xc8, 0x6f,
	0x63, 0x20, 0x2b, 0x46, 0x24, 0x26, 0x19, 0x1d, 0xd1, 0x2c,
	0x31, 0x6d, 0x5a, 0x29, 0xa2, 0x6, 0xa6, 0xb2, 0x41, 0xcc,
	0xa, 0x27, 0x96, 0x9, 0x96, 0xac, 0x47, 0x65, 0x78, 0x68,
	0x51, 0x98, 0xd6, 0xd8, 0xa6, 0x2d, 0xa0, 0xcf, 0xec, 0xe2,
	0x74, 0xf2, 0x82, 0xe3, 0x97, 0xd9, 0x7e, 0xd4, 0xf8, 0xb,
	0x70, 0x43, 0x3d, 0xb1, 0x7b, 0x97, 0x80, 0xd6, 0xcb, 0xd7,
	0x19, 0xbc, 0x63, 0xb, 0xfd, 0x4d, 0x88, 0xfe, 0x67, 0xac,
	0xb8, 0xcc, 0x50, 0xb7, 0x68, 0xb3, 0x5b, 0xd6, 0x1e, 0x25,
	0xfc, 0x5f, 0x3c, 0x8d, 0xb1, 0x33, 0x7c, 0xb3, 0x49, 0x1,
	0x3f, 0x71, 0x55, 0xe, 0x51, 0xba, 0x61, 0x26, 0xfa, 0xea,
	0xe5, 0xb5, 0xe8, 0xaa, 0xcf, 0xcd, 0x96, 0x9f, 0xd6, 0xc1,
	0x5f, 0x53, 0x91, 0xad, 0x5, 0xde, 0x20, 0xe7, 0x51, 0xda,
	0x5b, 0x95, 0x67, 0xed, 0xf4, 0xee, 0x42, 0x65, 0x70, 0x13,
	0xb, 0x70, 0x14, 0x1c, 0xc9, 0xe0, 0x19, 0xca, 0x5f, 0xf5,
	0x1d, 0x70, 0x4b, 0x6c, 0x6, 0x74, 0xec, 0xb5, 0x2e, 0x77,
	0xe1, 0x74, 0xa1, 0xa3, 0x99, 0xa0, 0x85, 0x9e, 0xf1, 0xac,
	0xd8, 0x7e,
};
#endif /* AVB_ROOT_KEY_BUILTIN */

ثابت حرف *str_avb_io_error(AvbIOResult res)
{
	بدل (res) {
	حالة AVB_IO_RESULT_OK:
		أرجع "Requested operation was successful";
	حالة AVB_IO_RESULT_ERROR_IO:
		أرجع "Underlying hardware encountered an I/O error";
	حالة AVB_IO_RESULT_ERROR_OOM:
		أرجع "Unable to allocate memory";
	حالة AVB_IO_RESULT_ERROR_NO_SUCH_PARTITION:
		أرجع "Requested partition does not exist";
	حالة AVB_IO_RESULT_ERROR_RANGE_OUTSIDE_PARTITION:
		أرجع "Bytes requested is outside the range of partition";
	حالة AVB_IO_RESULT_ERROR_NO_SUCH_قيمة:
		أرجع "Named persistent value does not exist";
	حالة AVB_IO_RESULT_ERROR_INVALID_قيمة_SIZE:
		أرجع "Named persistent value size is not supported";
	حالة AVB_IO_RESULT_ERROR_INSUFFICIENT_SPACE:
		أرجع "Buffer is too small for the requested operation";
	افتراضي:
		أرجع "Unknown AVB error";
	}
}

ثابت حرف *str_avb_slot_error(AvbSlotVerifyResult res)
{
	بدل (res) {
	حالة AVB_SLOT_VERIFY_RESULT_OK:
		أرجع "Verification passed successfully";
	حالة AVB_SLOT_VERIFY_RESULT_ERROR_OOM:
		أرجع "Allocation of memory failed";
	حالة AVB_SLOT_VERIFY_RESULT_ERROR_IO:
		أرجع "I/O error occurred while trying to load data";
	حالة AVB_SLOT_VERIFY_RESULT_ERROR_VERIFICATION:
		أرجع "Digest didn't match or signature checks failed";
	حالة AVB_SLOT_VERIFY_RESULT_ERROR_ROLLBACK_INDEX:
		أرجع "Rollback index is less than its stored value";
	حالة AVB_SLOT_VERIFY_RESULT_ERROR_PUBLIC_مفتاح_REJECTED:
		أرجع "Public keys are not accepted";
	حالة AVB_SLOT_VERIFY_RESULT_ERROR_INVALID_METADATA:
		أرجع "Metadata is invalid or inconsistent";
	حالة AVB_SLOT_VERIFY_RESULT_ERROR_UNSUPPORTED_VERSION:
		أرجع "Metadata requires a newer version of libavb";
	حالة AVB_SLOT_VERIFY_RESULT_ERROR_INVALID_ARGUMENT:
		أرجع "Invalid arguments are used";
	افتراضي:
		أرجع "Unknown AVB slot verification error";
	}
}
/**
 * ============================================================================
 * Boot states support (GREEN, YELLOW, ORANGE, RED) and dm_verity
 * ============================================================================
 */
حرف *avb_set_state(AvbOps *ops, تعداد avb_boot_state boot_state)
{
	بنية AvbOpsData *data;
	حرف *cmdline = عدم;

	إذا (!ops)
		أرجع عدم;

	data = (بنية AvbOpsData *)ops->user_data;
	إذا (!data)
		أرجع عدم;

	data->boot_state = boot_state;
	بدل (boot_state) {
	حالة AVB_GREEN:
		cmdline = "androidboot.verifiedbootstate=green";
		اكسر;
	حالة AVB_YELLOW:
		cmdline = "androidboot.verifiedbootstate=yellow";
		اكسر;
	حالة AVB_ORANGE:
		cmdline = "androidboot.verifiedbootstate=orange";
		اكسر;
	حالة AVB_RED:
		اكسر;
	}

	أرجع cmdline;
}

حرف *append_cmd_سطر(حرف *cmdline_orig, حرف *cmdline_new)
{
	حرف *cmd_سطر;

	إذا (!cmdline_new)
		أرجع cmdline_orig;

	إذا (cmdline_orig)
		cmd_سطر = cmdline_orig;
	وإلا
		cmd_سطر = " ";

	cmd_سطر = avb_strdupv(cmd_سطر, " ", cmdline_new, عدم);

	أرجع cmd_سطر;
}

ساكن صحيح avb_find_dm_وسائط(حرف **وسائط, حرف *str)
{
	صحيح i;

	إذا (!str)
		أرجع -1;

	لكل (i = 0; i < AVB_MAX_وسائط && وسائط[i]; ++i) {
		إذا (strstr(وسائط[i], str))
			أرجع i;
	}

	أرجع -1;
}

ساكن حرف *avb_set_enforce_option(ثابت حرف *cmdline, ثابت حرف *option)
{
	حرف *cmdarg[AVB_MAX_وسائط];
	حرف *newargs = عدم;
	صحيح i = 0;
	صحيح total_وسائط;

	memset(cmdarg, 0, حجم(cmdarg));
	cmdarg[i++] = strtok((حرف *)cmdline, " ");

	افعل {
		cmdarg[i] = strtok(عدم, " ");
		إذا (!cmdarg[i])
			اكسر;

		إذا (++i >= AVB_MAX_وسائط) {
			printf("%s: Can't handle more then %d args\n",
			       __func__, i);
			أرجع عدم;
		}
	} طالما (true);

	total_وسائط = i;
	i = avb_find_dm_وسائط(&cmdarg[0], VERITY_TABLE_OPT_LOGGING);
	إذا (i >= 0) {
		cmdarg[i] = (حرف *)option;
	} وإلا {
		i = avb_find_dm_وسائط(&cmdarg[0], VERITY_TABLE_OPT_RESTART);
		إذا (i < 0) {
			printf("%s: No verity options found\n", __func__);
			أرجع عدم;
		}

		cmdarg[i] = (حرف *)option;
	}

	لكل (i = 0; i <= total_وسائط; i++)
		newargs = append_cmd_سطر(newargs, cmdarg[i]);

	أرجع newargs;
}

حرف *avb_set_ignore_corruption(ثابت حرف *cmdline)
{
	حرف *newargs = عدم;

	newargs = avb_set_enforce_option(cmdline, VERITY_TABLE_OPT_LOGGING);
	إذا (newargs)
		newargs = append_cmd_سطر(newargs,
					  "androidboot.veritymode=eio");

	أرجع newargs;
}

حرف *avb_set_enforce_verity(ثابت حرف *cmdline)
{
	حرف *newargs;

	newargs = avb_set_enforce_option(cmdline, VERITY_TABLE_OPT_RESTART);
	إذا (newargs)
		newargs = append_cmd_سطر(newargs,
					  "androidboot.veritymode=enforcing");
	أرجع newargs;
}

/**
 * ============================================================================
 * IO(mmc) auxiliary functions
 * ============================================================================
 */
ساكن unsigned long mmc_read_and_flush(بنية mmc_part *part,
					lbaint_t start,
					lbaint_t sectors,
					فراغ *buffer)
{
	unsigned long blks;
	فراغ *tmp_buf;
	size_t buf_size;
	bool unaligned = is_buf_unaligned(buffer);

	إذا (start < part->info.start) {
		printf("%s: partition start out of bounds\n", __func__);
		أرجع 0;
	}
	إذا ((start + sectors) > (part->info.start + part->info.size)) {
		sectors = part->info.start + part->info.size - start;
		printf("%s: read sector aligned to partition bounds (" LBAF ")\n",
		       __func__, sectors);
	}

	/*
	 * Reading fails on unaligned buffers, so we have to
	 * use aligned temporary buffer and then copy to destination
	 */
	إذا (unaligned) {
		debug("%s: handling unaligned read buffer, addr = 0x%p\n",
		      __func__, buffer);
		tmp_buf = get_sector_buf();
		buf_size = get_sector_buf_size();
		إذا (sectors > buf_size / part->info.blksz)
			sectors = buf_size / part->info.blksz;
	} وإلا {
		tmp_buf = buffer;
	}

	blks = blk_dread(part->mmc_blk,
			 start, sectors, tmp_buf);
	/* flush cache after read */
	flush_cache((ulong)tmp_buf, sectors * part->info.blksz);

	إذا (unaligned)
		memcpy(buffer, tmp_buf, sectors * part->info.blksz);

	أرجع blks;
}

ساكن unsigned long mmc_write(بنية mmc_part *part, lbaint_t start,
			       lbaint_t sectors, فراغ *buffer)
{
	فراغ *tmp_buf;
	size_t buf_size;
	bool unaligned = is_buf_unaligned(buffer);

	إذا (start < part->info.start) {
		printf("%s: partition start out of bounds\n", __func__);
		أرجع 0;
	}
	إذا ((start + sectors) > (part->info.start + part->info.size)) {
		sectors = part->info.start + part->info.size - start;
		printf("%s: sector aligned to partition bounds (" LBAF ")\n",
		       __func__, sectors);
	}
	إذا (unaligned) {
		tmp_buf = get_sector_buf();
		buf_size = get_sector_buf_size();
		debug("%s: handling unaligned read buffer, addr = 0x%p\n",
		      __func__, buffer);
		إذا (sectors > buf_size / part->info.blksz)
			sectors = buf_size / part->info.blksz;

		memcpy(tmp_buf, buffer, sectors * part->info.blksz);
	} وإلا {
		tmp_buf = buffer;
	}

	أرجع blk_dwrite(part->mmc_blk,
			  start, sectors, tmp_buf);
}

ساكن بنية mmc_part *get_partition(AvbOps *ops, ثابت حرف *partition)
{
	صحيح ret;
	u8 dev_رقم;
	صحيح part_رقم = 0;
	بنية mmc_part *part;
	بنية blk_desc *mmc_blk;

	part = malloc(حجم(بنية mmc_part));
	إذا (!part)
		أرجع عدم;

	dev_رقم = get_boot_device(ops);
	part->mmc = find_mmc_device(dev_رقم);
	إذا (!part->mmc) {
		printf("%s: no MMC device at slot %x\n", __func__, dev_رقم);
		اذهب err;
	}

	ret = mmc_init(part->mmc);
	إذا (ret) {
		printf("%s: MMC initialization failed, err = %d\n",
		       __func__, ret);
		اذهب err;
	}

	إذا (IS_MMC(part->mmc)) {
		ret = mmc_switch_part(part->mmc, part_رقم);
		إذا (ret) {
			printf("%s: MMC part switch failed, err = %d\n",
			       __func__, ret);
			اذهب err;
		}
	}

	mmc_blk = mmc_get_blk_desc(part->mmc);
	إذا (!mmc_blk) {
		printf("%s: failed to obtain block descriptor\n", __func__);
		اذهب err;
	}

	ret = part_get_info_by_name(mmc_blk, partition, &part->info);
	إذا (ret < 0) {
		printf("%s: can't find partition '%s'\n", __func__, partition);
		اذهب err;
	}

	part->dev_رقم = dev_رقم;
	part->mmc_blk = mmc_blk;

	أرجع part;
err:
	حرر(part);
	أرجع عدم;
}

ساكن AvbIOResult mmc_byte_io(AvbOps *ops,
			       ثابت حرف *partition,
			       s64 offset,
			       size_t رقم_bytes,
			       فراغ *buffer,
			       size_t *out_رقم_read,
			       تعداد mmc_io_type io_type)
{
	ulong ret;
	بنية mmc_part *part;
	u64 start_offset, start_sector, sectors, residue;
	u8 *tmp_buf;
	size_t io_cnt = 0;
	AvbIOResult io_ret = AVB_IO_RESULT_OK;

	إذا (!partition || !buffer || io_type > IO_WRITE)
		أرجع AVB_IO_RESULT_ERROR_IO;

	part = get_partition(ops, partition);
	إذا (!part)
		أرجع AVB_IO_RESULT_ERROR_NO_SUCH_PARTITION;

	إذا (!part->info.blksz) {
		io_ret = AVB_IO_RESULT_ERROR_IO;
		اذهب out;
	}

	start_offset = calc_offset(part, offset);
	طالما (رقم_bytes) {
		start_sector = start_offset / part->info.blksz;
		sectors = رقم_bytes / part->info.blksz;
		/* handle non block-aligned reads */
		إذا (start_offset % part->info.blksz ||
		    رقم_bytes < part->info.blksz) {
			tmp_buf = get_sector_buf();
			إذا (start_offset % part->info.blksz) {
				residue = part->info.blksz -
					(start_offset % part->info.blksz);
				إذا (residue > رقم_bytes)
					residue = رقم_bytes;
			} وإلا {
				residue = رقم_bytes;
			}

			إذا (io_type == IO_READ) {
				ret = mmc_read_and_flush(part,
							 part->info.start +
							 start_sector,
							 1, tmp_buf);

				إذا (ret != 1) {
					printf("%s: read error (%ld, %lld)\n",
					       __func__, ret, start_sector);
					io_ret = AVB_IO_RESULT_ERROR_IO;
					اذهب out;
				}
				/*
				 * if this is not aligned at sector start,
				 * we have to adjust the tmp buffer
				 */
				tmp_buf += (start_offset % part->info.blksz);
				memcpy(buffer, (فراغ *)tmp_buf, residue);
			} وإلا {
				ret = mmc_read_and_flush(part,
							 part->info.start +
							 start_sector,
							 1, tmp_buf);

				إذا (ret != 1) {
					printf("%s: read error (%ld, %lld)\n",
					       __func__, ret, start_sector);
					io_ret = AVB_IO_RESULT_ERROR_IO;
					اذهب out;
				}
				memcpy((فراغ *)tmp_buf +
					start_offset % part->info.blksz,
					buffer, residue);

				ret = mmc_write(part, part->info.start +
						start_sector, 1, tmp_buf);
				إذا (ret != 1) {
					printf("%s: write error (%ld, %lld)\n",
					       __func__, ret, start_sector);
					io_ret = AVB_IO_RESULT_ERROR_IO;
					اذهب out;
				}
			}

			io_cnt += residue;
			buffer += residue;
			start_offset += residue;
			رقم_bytes -= residue;
			استمر;
		}

		إذا (sectors) {
			إذا (io_type == IO_READ) {
				ret = mmc_read_and_flush(part,
							 part->info.start +
							 start_sector,
							 sectors, buffer);
			} وإلا {
				ret = mmc_write(part,
						part->info.start +
						start_sector,
						sectors, buffer);
			}

			إذا (!ret) {
				printf("%s: sector read error\n", __func__);
				io_ret = AVB_IO_RESULT_ERROR_IO;
				اذهب out;
			}

			io_cnt += ret * part->info.blksz;
			buffer += ret * part->info.blksz;
			start_offset += ret * part->info.blksz;
			رقم_bytes -= ret * part->info.blksz;
		}
	}

	/* Set counter for read operation */
	إذا (io_type == IO_READ && out_رقم_read)
		*out_رقم_read = io_cnt;

out:
	حرر(part);
	أرجع io_ret;
}

/**
 * ============================================================================
 * AVB 2.0 operations
 * ============================================================================
 */

/**
 * read_from_partition() - reads @num_bytes from  @offset from partition
 * identified by a string name
 *
 * @ops: contains AVB ops handlers
 * @partition_name: partition name, NUL-terminated UTF-8 string
 * @offset: offset from the beginning of partition
 * @num_bytes: amount of bytes to read
 * @buffer: destination buffer to store data
 * @out_num_read:
 *
 * @return:
 *      AVB_IO_RESULT_OK, if partition was found and read operation succeed
 *      AVB_IO_RESULT_ERROR_IO, if i/o error occurred from the underlying i/o
 *            subsystem
 *      AVB_IO_RESULT_ERROR_NO_SUCH_PARTITION, if there is no partition with
 *      the given name
 */
ساكن AvbIOResult read_from_partition(AvbOps *ops,
				       ثابت حرف *partition_name,
				       s64 offset_from_partition,
				       size_t رقم_bytes,
				       فراغ *buffer,
				       size_t *out_رقم_read)
{
	أرجع mmc_byte_io(ops, partition_name, offset_from_partition,
			   رقم_bytes, buffer, out_رقم_read, IO_READ);
}

/**
 * write_to_partition() - writes N bytes to a partition identified by a string
 * name
 *
 * @ops: AvbOps, contains AVB ops handlers
 * @partition_name: partition name
 * @offset_from_partition: offset from the beginning of partition
 * @num_bytes: amount of bytes to write
 * @buf: data to write
 * @out_num_read:
 *
 * @return:
 *      AVB_IO_RESULT_OK, if partition was found and read operation succeed
 *      AVB_IO_RESULT_ERROR_IO, if input/output error occurred
 *      AVB_IO_RESULT_ERROR_NO_SUCH_PARTITION, if partition, specified in
 *            @partition_name was not found
 */
ساكن AvbIOResult write_to_partition(AvbOps *ops,
				      ثابت حرف *partition_name,
				      s64 offset_from_partition,
				      size_t رقم_bytes,
				      ثابت فراغ *buffer)
{
	أرجع mmc_byte_io(ops, partition_name, offset_from_partition,
			   رقم_bytes, (فراغ *)buffer, عدم, IO_WRITE);
}

/**
 * validate_vmbeta_public_key() - checks if the given public key used to sign
 * the vbmeta partition is trusted
 *
 * @ops: AvbOps, contains AVB ops handlers
 * @public_key_data: public key for verifying vbmeta partition signature
 * @public_key_length: length of public key
 * @public_key_metadata:
 * @public_key_metadata_length:
 * @out_key_is_trusted:
 *
 * @return:
 *      AVB_IO_RESULT_OK, if partition was found and read operation succeed
 */
ساكن AvbIOResult validate_vbmeta_public_مفتاح(AvbOps *ops,
					      ثابت u8 *public_مفتاح_data,
					      size_t public_مفتاح_length,
					      ثابت u8
					      *public_مفتاح_metadata,
					      size_t
					      public_مفتاح_metadata_length,
					      bool *out_مفتاح_is_trusted)
{
	u8 مفتاح_digest[SHA256_SUM_LEN];
	u8 trusted_digest[SHA256_SUM_LEN];
	AvbIOResult rc;

	إذا (!public_مفتاح_length || !public_مفتاح_data || !out_مفتاح_is_trusted)
		أرجع AVB_IO_RESULT_ERROR_IO;

	/* Default deny: only flip to trusted on a positive digest match. */
	*out_مفتاح_is_trusted = false;

	/* Digest of the key embedded in the (untrusted) vbmeta. */
	sha256_csum_wd(public_مفتاح_data, public_مفتاح_length, مفتاح_digest,
		       CHUNKSZ_SHA256);

	/* Digest of the trusted root key from the configured source. */
	rc = avb_read_جذر_مفتاح_digest(ops, trusted_digest);
	إذا (rc != AVB_IO_RESULT_OK)
		أرجع rc;

	إذا (avb_safe_memcmp(مفتاح_digest, trusted_digest, حجم(مفتاح_digest)) == 0)
		*out_مفتاح_is_trusted = true;

	أرجع AVB_IO_RESULT_OK;
}

#ifdef CONFIG_OPTEE_TA_AVB
ساكن صحيح get_open_session(بنية AvbOpsData *ops_data)
{
	بنية udevice *tee = عدم;

	طالما (!ops_data->tee) {
		ثابت بنية tee_optee_ta_uuid uuid = TA_AVB_UUID;
		بنية tee_open_session_arg arg;
		صحيح rc;

		tee = tee_find_device(tee, عدم, عدم, عدم);
		إذا (!tee)
			أرجع -ENODEV;

		memset(&arg, 0, حجم(arg));
		tee_optee_ta_uuid_to_octets(arg.uuid, &uuid);
		rc = tee_open_session(tee, &arg, 0, عدم);
		إذا (rc || arg.ret)
			استمر;

		ops_data->tee = tee;
		ops_data->session = arg.session;
	}

	أرجع 0;
}

ساكن AvbIOResult invoke_func(بنية AvbOpsData *ops_data, u32 func,
			       ulong رقم_param, بنية tee_param *param)
{
	بنية tee_invoke_arg arg;

	إذا (get_open_session(ops_data))
		أرجع AVB_IO_RESULT_ERROR_IO;

	memset(&arg, 0, حجم(arg));
	arg.func = func;
	arg.session = ops_data->session;

	إذا (tee_invoke_func(ops_data->tee, &arg, رقم_param, param))
		أرجع AVB_IO_RESULT_ERROR_IO;
	بدل (arg.ret) {
	حالة TEE_SUCCESS:
		أرجع AVB_IO_RESULT_OK;
	حالة TEE_ERROR_OUT_OF_MEMORY:
		أرجع AVB_IO_RESULT_ERROR_OOM;
	حالة TEE_ERROR_STORAGE_NO_SPACE:
		أرجع AVB_IO_RESULT_ERROR_INSUFFICIENT_SPACE;
	حالة TEE_ERROR_ITEM_NOT_FOUND:
		أرجع AVB_IO_RESULT_ERROR_NO_SUCH_قيمة;
	حالة TEE_ERROR_TARGET_DEAD:
		/*
		 * The TA has paniced, close the session to reload the TA
		 * for the next request.
		 */
		tee_close_session(ops_data->tee, ops_data->session);
		ops_data->tee = عدم;
		أرجع AVB_IO_RESULT_ERROR_IO;
	افتراضي:
		أرجع AVB_IO_RESULT_ERROR_IO;
	}
}
#endif

/**
 * read_rollback_index() - gets the rollback index corresponding to the
 * location of given by @out_rollback_index.
 *
 * @ops: contains AvbOps handlers
 * @rollback_index_slot:
 * @out_rollback_index: used to write a retrieved rollback index.
 *
 * @return
 *       AVB_IO_RESULT_OK, if the roolback index was retrieved
 */
ساكن AvbIOResult read_rollback_index(AvbOps *ops,
				       size_t rollback_index_slot,
				       u64 *out_rollback_index)
{
#ifndef CONFIG_OPTEE_TA_AVB
	/* For now we always return 0 as the stored rollback index. */
	debug("%s: rollback protection is not implemented\n", __func__);

	إذا (out_rollback_index)
		*out_rollback_index = 0;

	أرجع AVB_IO_RESULT_OK;
#وإلا
	AvbIOResult rc;
	بنية tee_param param[2];

	إذا (rollback_index_slot >= TA_AVB_MAX_ROLLBACK_LOCATIONS)
		أرجع AVB_IO_RESULT_ERROR_NO_SUCH_قيمة;

	memset(param, 0, حجم(param));
	param[0].attr = TEE_PARAM_ATTR_TYPE_قيمة_مدخل;
	param[0].u.قيمة.a = rollback_index_slot;
	param[1].attr = TEE_PARAM_ATTR_TYPE_قيمة_OUTPUT;

	rc = invoke_func(ops->user_data, TA_AVB_CMD_READ_ROLLBACK_INDEX,
			 ARRAY_SIZE(param), param);
	إذا (rc)
		أرجع rc;

	*out_rollback_index = (u64)param[1].u.قيمة.a << 32 |
			      (u32)param[1].u.قيمة.b;
	أرجع AVB_IO_RESULT_OK;
#endif
}

/**
 * write_rollback_index() - sets the rollback index corresponding to the
 * location of given by @out_rollback_index.
 *
 * @ops: contains AvbOps handlers
 * @rollback_index_slot:
 * @rollback_index: rollback index to write.
 *
 * @return
 *       AVB_IO_RESULT_OK, if the roolback index was retrieved
 */
ساكن AvbIOResult write_rollback_index(AvbOps *ops,
					size_t rollback_index_slot,
					u64 rollback_index)
{
#ifndef CONFIG_OPTEE_TA_AVB
	/* For now this is a no-op. */
	debug("%s: rollback protection is not implemented\n", __func__);

	أرجع AVB_IO_RESULT_OK;
#وإلا
	بنية tee_param param[2];

	إذا (rollback_index_slot >= TA_AVB_MAX_ROLLBACK_LOCATIONS)
		أرجع AVB_IO_RESULT_ERROR_NO_SUCH_قيمة;

	memset(param, 0, حجم(param));
	param[0].attr = TEE_PARAM_ATTR_TYPE_قيمة_مدخل;
	param[0].u.قيمة.a = rollback_index_slot;
	param[1].attr = TEE_PARAM_ATTR_TYPE_قيمة_مدخل;
	param[1].u.قيمة.a = (u32)(rollback_index >> 32);
	param[1].u.قيمة.b = (u32)rollback_index;

	أرجع invoke_func(ops->user_data, TA_AVB_CMD_WRITE_ROLLBACK_INDEX,
			   ARRAY_SIZE(param), param);
#endif
}

/**
 * read_is_device_unlocked() - gets whether the device is unlocked
 *
 * @ops: contains AVB ops handlers
 * @out_is_unlocked: device unlock state is stored here, true if unlocked,
 *       false otherwise
 *
 * @return:
 *       AVB_IO_RESULT_OK: state is retrieved successfully
 *       AVB_IO_RESULT_ERROR_IO: an error occurred
 */
ساكن AvbIOResult read_is_device_unlocked(AvbOps *ops, bool *out_is_unlocked)
{
#ifndef CONFIG_OPTEE_TA_AVB
	/* For now we always return that the device is unlocked. */
	debug("%s: device locking is not implemented\n", __func__);

	*out_is_unlocked = true;

	أرجع AVB_IO_RESULT_OK;
#وإلا
	AvbIOResult rc;
	بنية tee_param param = { .attr = TEE_PARAM_ATTR_TYPE_قيمة_OUTPUT };

	rc = invoke_func(ops->user_data, TA_AVB_CMD_READ_LOCK_STATE, 1, &param);
	إذا (rc)
		أرجع rc;
	*out_is_unlocked = !param.u.قيمة.a;
	أرجع AVB_IO_RESULT_OK;
#endif
}

/**
 * get_unique_guid_for_partition() - gets the GUID for a partition identified
 * by a string name
 *
 * @ops: contains AVB ops handlers
 * @partition: partition name (NUL-terminated UTF-8 string)
 * @guid_buf: buf, used to copy in GUID string. Example of value:
 *      527c1c6d-6361-4593-8842-3c78fcd39219
 * @guid_buf_size: @guid_buf buffer size
 *
 * @return:
 *      AVB_IO_RESULT_OK, on success (GUID found)
 *      AVB_IO_RESULT_ERROR_IO, if incorrect buffer size (@guid_buf_size) was
 *             provided
 *      AVB_IO_RESULT_ERROR_NO_SUCH_PARTITION, if partition was not found
 */
ساكن AvbIOResult get_unique_guid_for_partition(AvbOps *ops,
						 ثابت حرف *partition,
						 حرف *guid_buf,
						 size_t guid_buf_size)
{
	بنية mmc_part *part;
	size_t uuid_size;

	part = get_partition(ops, partition);
	إذا (!part)
		أرجع AVB_IO_RESULT_ERROR_NO_SUCH_PARTITION;

	uuid_size = حجم(part->info.uuid);
	إذا (uuid_size > guid_buf_size) {
		حرر(part);
		أرجع AVB_IO_RESULT_ERROR_IO;
	}

	memcpy(guid_buf, part->info.uuid, uuid_size);
	guid_buf[uuid_size - 1] = 0;

	حرر(part);
	أرجع AVB_IO_RESULT_OK;
}

/**
 * get_size_of_partition() - gets the size of a partition identified
 * by a string name
 *
 * @ops: contains AVB ops handlers
 * @partition: partition name (NUL-terminated UTF-8 string)
 * @out_size_num_bytes: returns the value of a partition size
 *
 * @return:
 *      AVB_IO_RESULT_OK, on success (GUID found)
 *      AVB_IO_RESULT_ERROR_INSUFFICIENT_SPACE, out_size_num_bytes is NULL
 *      AVB_IO_RESULT_ERROR_NO_SUCH_PARTITION, if partition was not found
 */
ساكن AvbIOResult get_size_of_partition(AvbOps *ops,
					 ثابت حرف *partition,
					 u64 *out_size_رقم_bytes)
{
	بنية mmc_part *part;

	إذا (!out_size_رقم_bytes)
		أرجع AVB_IO_RESULT_ERROR_INSUFFICIENT_SPACE;

	part = get_partition(ops, partition);
	إذا (!part)
		أرجع AVB_IO_RESULT_ERROR_NO_SUCH_PARTITION;

	*out_size_رقم_bytes = part->info.blksz * part->info.size;
	حرر(part);

	أرجع AVB_IO_RESULT_OK;
}

#ifdef CONFIG_OPTEE_TA_AVB
ساكن AvbIOResult read_persistent_قيمة(AvbOps *ops,
					 ثابت حرف *name,
					 size_t buffer_size,
					 u8 *out_buffer,
					 size_t *out_رقم_bytes_read)
{
	AvbIOResult rc;
	بنية tee_shm *shm_name;
	بنية tee_shm *shm_buf;
	بنية tee_param param[2];
	بنية udevice *tee;
	size_t name_size = strlen(name) + 1;

	إذا (get_open_session(ops->user_data))
		أرجع AVB_IO_RESULT_ERROR_IO;

	tee = ((بنية AvbOpsData *)ops->user_data)->tee;

	rc = tee_shm_alloc(tee, name_size,
			   TEE_SHM_ALLOC, &shm_name);
	إذا (rc)
		أرجع AVB_IO_RESULT_ERROR_OOM;

	rc = tee_shm_alloc(tee, buffer_size,
			   TEE_SHM_ALLOC, &shm_buf);
	إذا (rc) {
		rc = AVB_IO_RESULT_ERROR_OOM;
		اذهب free_name;
	}

	memcpy(shm_name->addr, name, name_size);

	memset(param, 0, حجم(param));
	param[0].attr = TEE_PARAM_ATTR_TYPE_MEMREF_مدخل;
	param[0].u.memref.shm = shm_name;
	param[0].u.memref.size = name_size;
	param[1].attr = TEE_PARAM_ATTR_TYPE_MEMREF_INOUT;
	param[1].u.memref.shm = shm_buf;
	param[1].u.memref.size = buffer_size;

	rc = invoke_func(ops->user_data, TA_AVB_CMD_READ_PERSIST_قيمة,
			 2, param);
	إذا (rc)
		اذهب out;

	إذا (param[1].u.memref.size > buffer_size) {
		rc = AVB_IO_RESULT_ERROR_NO_SUCH_قيمة;
		اذهب out;
	}

	*out_رقم_bytes_read = param[1].u.memref.size;

	memcpy(out_buffer, shm_buf->addr, *out_رقم_bytes_read);

out:
	tee_shm_free(shm_buf);
free_name:
	tee_shm_free(shm_name);

	أرجع rc;
}

ساكن AvbIOResult write_persistent_قيمة(AvbOps *ops,
					  ثابت حرف *name,
					  size_t قيمة_size,
					  ثابت u8 *قيمة)
{
	AvbIOResult rc;
	بنية tee_shm *shm_name;
	بنية tee_shm *shm_buf;
	بنية tee_param param[2];
	بنية udevice *tee;
	size_t name_size = strlen(name) + 1;

	إذا (get_open_session(ops->user_data))
		أرجع AVB_IO_RESULT_ERROR_IO;

	tee = ((بنية AvbOpsData *)ops->user_data)->tee;

	إذا (!قيمة_size)
		أرجع AVB_IO_RESULT_ERROR_NO_SUCH_قيمة;

	rc = tee_shm_alloc(tee, name_size,
			   TEE_SHM_ALLOC, &shm_name);
	إذا (rc)
		أرجع AVB_IO_RESULT_ERROR_OOM;

	rc = tee_shm_alloc(tee, قيمة_size,
			   TEE_SHM_ALLOC, &shm_buf);
	إذا (rc) {
		rc = AVB_IO_RESULT_ERROR_OOM;
		اذهب free_name;
	}

	memcpy(shm_name->addr, name, name_size);
	memcpy(shm_buf->addr, قيمة, قيمة_size);

	memset(param, 0, حجم(param));
	param[0].attr = TEE_PARAM_ATTR_TYPE_MEMREF_مدخل;
	param[0].u.memref.shm = shm_name;
	param[0].u.memref.size = name_size;
	param[1].attr = TEE_PARAM_ATTR_TYPE_MEMREF_مدخل;
	param[1].u.memref.shm = shm_buf;
	param[1].u.memref.size = قيمة_size;

	rc = invoke_func(ops->user_data, TA_AVB_CMD_WRITE_PERSIST_قيمة,
			 2, param);
	إذا (rc)
		اذهب out;

out:
	tee_shm_free(shm_buf);
free_name:
	tee_shm_free(shm_name);

	أرجع rc;
}
#endif

/**
 * ============================================================================
 * AVB root key digest providers (selected via CONFIG_AVB_ROOT_KEY_*)
 * ============================================================================
 */
#إذا CONFIG_IS_ENABLED(AVB_جذر_مفتاح_BUILTIN)
AvbIOResult avb_read_جذر_مفتاح_digest(AvbOps *ops, uint8_t *digest)
{
	sha256_csum_wd(avb_جذر_pub, حجم(avb_جذر_pub), digest,
		       CHUNKSZ_SHA256);

	أرجع AVB_IO_RESULT_OK;
}
#وإلاإذا CONFIG_IS_ENABLED(AVB_جذر_مفتاح_TEE)
/*
 * Read the trusted root key digest from OP-TEE secure storage as a named
 * persistent value, using the existing TA_AVB_CMD_READ_PERSIST_VALUE command
 * (see OP-TEE ta/avb/entry.c). The digest must be provisioned into the TEE
 * beforehand under CONFIG_AVB_ROOT_KEY_TEE_NAME; a missing value returns
 * AVB_IO_RESULT_ERROR_NO_SUCH_VALUE and verification fails closed.
 */
AvbIOResult avb_read_جذر_مفتاح_digest(AvbOps *ops, uint8_t *digest)
{
	size_t رقم_read = 0;
	AvbIOResult rc;

	rc = read_persistent_قيمة(ops, CONFIG_AVB_جذر_مفتاح_TEE_NAME,
				   SHA256_SUM_LEN, digest, &رقم_read);
	إذا (rc != AVB_IO_RESULT_OK)
		أرجع rc;

	إذا (رقم_read != SHA256_SUM_LEN)
		أرجع AVB_IO_RESULT_ERROR_INVALID_قيمة_SIZE;

	أرجع AVB_IO_RESULT_OK;
}
#وإلا /* CONFIG_AVB_ROOT_KEY_BOARD */
/*
 * Weak, fail-closed default. A board/SoC selecting CONFIG_AVB_ROOT_KEY_BOARD
 * must provide a strong avb_read_root_key_digest() (e.g. reading a hash fused
 * into OTP/eFuse). If it does not, verification fails rather than silently
 * trusting a wrong key.
 */
__weak AvbIOResult avb_read_جذر_مفتاح_digest(AvbOps *ops, uint8_t *digest)
{
	printf("%s: board root key provider not implemented\n", __func__);

	أرجع AVB_IO_RESULT_ERROR_NO_SUCH_قيمة;
}
#endif

/**
 * ============================================================================
 * AVB2.0 AvbOps alloc/initialisation/free
 * ============================================================================
 */
AvbOps *avb_ops_alloc(صحيح boot_device)
{
	بنية AvbOpsData *ops_data;

	ops_data = avb_calloc(حجم(بنية AvbOpsData));
	إذا (!ops_data)
		أرجع عدم;

	ops_data->ops.user_data = ops_data;

	ops_data->ops.read_from_partition = read_from_partition;
	ops_data->ops.write_to_partition = write_to_partition;
	ops_data->ops.validate_vbmeta_public_مفتاح = validate_vbmeta_public_مفتاح;
	ops_data->ops.read_rollback_index = read_rollback_index;
	ops_data->ops.write_rollback_index = write_rollback_index;
	ops_data->ops.read_is_device_unlocked = read_is_device_unlocked;
	ops_data->ops.get_unique_guid_for_partition =
		get_unique_guid_for_partition;
#ifdef CONFIG_OPTEE_TA_AVB
	ops_data->ops.write_persistent_قيمة = write_persistent_قيمة;
	ops_data->ops.read_persistent_قيمة = read_persistent_قيمة;
#endif
	ops_data->ops.get_size_of_partition = get_size_of_partition;
	ops_data->mmc_dev = boot_device;

	أرجع &ops_data->ops;
}

فراغ avb_ops_free(AvbOps *ops)
{
	بنية AvbOpsData *ops_data;

	إذا (!ops)
		أرجع;

	ops_data = ops->user_data;

	إذا (ops_data) {
#ifdef CONFIG_OPTEE_TA_AVB
		إذا (ops_data->tee)
			tee_close_session(ops_data->tee, ops_data->session);
#endif
		avb_free(ops_data);
	}
}

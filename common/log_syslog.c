// SPDX-License-Identifier: GPL-2.0+
/*
 * Log to syslog.
 *
 * Copyright (c) 2020, Heinrich Schuchardt <xypron.glpk@gmx.de>
 */

#অন্তর্ভাব <env.h>
#অন্তর্ভাব <log.h>
#অন্তর্ভাব <net.h>
#অন্তর্ভাব <asm/global_data.h>

DECLARE_GLOBAL_তথ্য_PTR;

#define বাফার_আকার_2 480

স্থির শূন্য append(অক্ষর **buf, অক্ষর *buf_end, ধ্রুবক অক্ষর *fmt, ...)
{
	va_list args;
	size_t আকার_2 = buf_end - *buf;

	va_start(args, fmt);
	vsnprintf(*buf, আকার_2, fmt, args);
	va_end(args);
	*buf += strlen(*buf);
}

স্থির পূর্ণসংখ্যা log_syslog_emit(কাঠামো log_device *ldev, কাঠামো log_rec *rec)
{
	পূর্ণসংখ্যা ret;
	পূর্ণসংখ্যা fmt = gd->log_fmt;
	অক্ষর বার্তা[বাফার_আকার_2];
	অক্ষর *বার্তা_end = বার্তা + বাফার_আকার_2;
	অক্ষর *ptr = বার্তা;
	অক্ষর *iphdr;
	অক্ষর *log_বার্তা;
	পূর্ণসংখ্যা eth_hdr_আকার;
	কাঠামো in_addr bcast_ip;
	unsigned পূর্ণসংখ্যা log_level;
	অক্ষর *log_hostname;

	/* Setup packet buffers */
	ret = net_init();
	যদি (ret)
		ফেরত ret;
	/* Disable hardware and put it into the reset state */
	eth_halt();
	/* Set current device according to environment variables */
	eth_set_current();
	/* Get hardware ready for send and receive operations */
	ret = eth_init();
	যদি (ret < 0) {
		eth_halt();
		যাও out;
	}

	memset(বার্তা, 0, বাফার_আকার_2);

	/* Set ethernet header */
	eth_hdr_আকার = net_set_ether((uchar *)ptr, net_bcast_ethaddr, PROT_IP);
	ptr += eth_hdr_আকার;
	iphdr = ptr;
	ptr += IP_UDP_HDR_আকার;
	log_বার্তা = ptr;

	/*
	 * The syslog log levels defined in RFC 5424 match the U-Boot ones up to
	 * level 7 (debug).
	 */
	log_level = rec->level;
	যদি (log_level > 7)
		log_level = 7;
	/* Leave high bits as 0 to write a 'kernel message' */

	/* Write log message to buffer */
	append(&ptr, বার্তা_end, "<%u>", log_level);
	log_hostname = env_get("log_hostname");
	যদি (log_hostname)
		append(&ptr, বার্তা_end, "%s ", log_hostname);
	append(&ptr, বার্তা_end, "uboot: ");
	যদি (fmt & BIT(LOGF_LEVEL))
		append(&ptr, বার্তা_end, "%s.",
		       log_get_level_name(rec->level));
	যদি (fmt & BIT(LOGF_CAT))
		append(&ptr, বার্তা_end, "%s,",
		       log_get_cat_name(rec->cat));
	যদি (fmt & BIT(LOGF_FILE))
		append(&ptr, বার্তা_end, "%s:", rec->file);
	যদি (fmt & BIT(LOGF_সারি))
		append(&ptr, বার্তা_end, "%d-", rec->সারি);
	যদি (fmt & BIT(LOGF_FUNC))
		append(&ptr, বার্তা_end, "%s()", rec->func ?: "?");
	যদি (fmt & BIT(LOGF_বার্তা))
		append(&ptr, বার্তা_end, "%s%s",
		       fmt != BIT(LOGF_বার্তা) ? " " : "", rec->বার্তা);
	/* Consider trailing 0x00 */
	ptr++;

	debug("log message: '%s'\n", log_বার্তা);

	/* Broadcast message */
	bcast_ip.s_addr = 0xFFFFFFFFL;
	net_set_udp_header((uchar *)iphdr, bcast_ip, 514, 514, ptr - log_বার্তা);
	net_send_packet((uchar *)বার্তা, ptr - বার্তা);

out:
	ফেরত ret;
}

LOG_DRIVER(syslog) = {
	.name	= "syslog",
	.emit	= log_syslog_emit,
};

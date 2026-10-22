// SPDX-License-Identifier: GPL-2.0+
/*
 * Log to syslog.
 *
 * Copyright (c) 2020, Heinrich Schuchardt <xypron.glpk@gmx.de>
 */

#অন্তর্ভাৱ <env.h>
#অন্তর্ভাৱ <log.h>
#অন্তর্ভাৱ <net.h>
#অন্তর্ভাৱ <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

#define BUFFER_SIZE 480

স্থির শূন্য append(ৱর্ণ **buf, ৱর্ণ *buf_end, ধ্রুৱ ৱর্ণ *fmt, ...)
{
	va_list args;
	size_t size = buf_end - *buf;

	va_start(args, fmt);
	vsnprintf(*buf, size, fmt, args);
	va_end(args);
	*buf += strlen(*buf);
}

স্থির পূর্ণাঙ্ক log_syslog_emit(সংরচনা log_device *ldev, সংরচনা log_rec *rec)
{
	পূর্ণাঙ্ক ret;
	পূর্ণাঙ্ক fmt = gd->log_fmt;
	ৱর্ণ বাৰ্তা[BUFFER_SIZE];
	ৱর্ণ *বাৰ্তা_end = বাৰ্তা + BUFFER_SIZE;
	ৱর্ণ *ptr = বাৰ্তা;
	ৱর্ণ *iphdr;
	ৱর্ণ *log_বাৰ্তা;
	পূর্ণাঙ্ক eth_hdr_size;
	সংরচনা in_addr bcast_ip;
	unsigned পূর্ণাঙ্ক log_level;
	ৱর্ণ *log_hostname;

	/* Setup packet buffers */
	ret = net_init();
	যদি (ret)
		ঘূৰাই ret;
	/* Disable hardware and put it into the reset state */
	eth_halt();
	/* Set current device according to environment variables */
	eth_set_current();
	/* Get hardware ready for send and receive operations */
	ret = eth_init();
	যদি (ret < 0) {
		eth_halt();
		গচ্ছ out;
	}

	memset(বাৰ্তা, 0, BUFFER_SIZE);

	/* Set ethernet header */
	eth_hdr_size = net_set_ether((uchar *)ptr, net_bcast_ethaddr, PROT_IP);
	ptr += eth_hdr_size;
	iphdr = ptr;
	ptr += IP_UDP_HDR_SIZE;
	log_বাৰ্তা = ptr;

	/*
	 * The syslog log levels defined in RFC 5424 match the U-Boot ones up to
	 * level 7 (debug).
	 */
	log_level = rec->level;
	যদি (log_level > 7)
		log_level = 7;
	/* Leave high bits as 0 to write a 'kernel message' */

	/* Write log message to buffer */
	append(&ptr, বাৰ্তা_end, "<%u>", log_level);
	log_hostname = env_get("log_hostname");
	যদি (log_hostname)
		append(&ptr, বাৰ্তা_end, "%s ", log_hostname);
	append(&ptr, বাৰ্তা_end, "uboot: ");
	যদি (fmt & BIT(LOGF_LEVEL))
		append(&ptr, বাৰ্তা_end, "%s.",
		       log_get_level_name(rec->level));
	যদি (fmt & BIT(LOGF_CAT))
		append(&ptr, বাৰ্তা_end, "%s,",
		       log_get_cat_name(rec->cat));
	যদি (fmt & BIT(LOGF_FILE))
		append(&ptr, বাৰ্তা_end, "%s:", rec->file);
	যদি (fmt & BIT(LOGF_সাৰি))
		append(&ptr, বাৰ্তা_end, "%d-", rec->সাৰি);
	যদি (fmt & BIT(LOGF_FUNC))
		append(&ptr, বাৰ্তা_end, "%s()", rec->func ?: "?");
	যদি (fmt & BIT(LOGF_বাৰ্তা))
		append(&ptr, বাৰ্তা_end, "%s%s",
		       fmt != BIT(LOGF_বাৰ্তা) ? " " : "", rec->বাৰ্তা);
	/* Consider trailing 0x00 */
	ptr++;

	debug("log message: '%s'\n", log_বাৰ্তা);

	/* Broadcast message */
	bcast_ip.s_addr = 0xFFFFFFFFL;
	net_set_udp_header((uchar *)iphdr, bcast_ip, 514, 514, ptr - log_বাৰ্তা);
	net_send_packet((uchar *)বাৰ্তা, ptr - বাৰ্তা);

out:
	ঘূৰাই ret;
}

LOG_DRIVER(syslog) = {
	.name	= "syslog",
	.emit	= log_syslog_emit,
};

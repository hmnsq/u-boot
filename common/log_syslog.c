// SPDX-License-Identifier: GPL-2.0+
/*
 * Log to syslog.
 *
 * Copyright (c) 2020, Heinrich Schuchardt <xypron.glpk@gmx.de>
 */

#شامل <env.h>
#شامل <log.h>
#شامل <net.h>
#شامل <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

#define BUFFER_SIZE 480

ایستا تهی append(نویسه **buf, نویسه *buf_end, ثابت نویسه *fmt, ...)
{
	va_list args;
	size_t size = buf_end - *buf;

	va_start(args, fmt);
	vsnprintf(*buf, size, fmt, args);
	va_end(args);
	*buf += strlen(*buf);
}

ایستا صحیح log_syslog_emit(ساختار log_device *ldev, ساختار log_rec *rec)
{
	صحیح ret;
	صحیح fmt = gd->log_fmt;
	نویسه پیام[BUFFER_SIZE];
	نویسه *پیام_end = پیام + BUFFER_SIZE;
	نویسه *ptr = پیام;
	نویسه *iphdr;
	نویسه *log_پیام;
	صحیح eth_hdr_size;
	ساختار in_addr bcast_ip;
	unsigned صحیح log_level;
	نویسه *log_hostname;

	/* Setup packet buffers */
	ret = net_init();
	اگر (ret)
		بازگشت ret;
	/* Disable hardware and put it into the reset state */
	eth_halt();
	/* Set current device according to environment variables */
	eth_set_current();
	/* Get hardware ready for send and receive operations */
	ret = eth_init();
	اگر (ret < 0) {
		eth_halt();
		برو out;
	}

	memset(پیام, 0, BUFFER_SIZE);

	/* Set ethernet header */
	eth_hdr_size = net_set_ether((uchar *)ptr, net_bcast_ethaddr, PROT_IP);
	ptr += eth_hdr_size;
	iphdr = ptr;
	ptr += IP_UDP_HDR_SIZE;
	log_پیام = ptr;

	/*
	 * The syslog log levels defined in RFC 5424 match the U-Boot ones up to
	 * level 7 (debug).
	 */
	log_level = rec->level;
	اگر (log_level > 7)
		log_level = 7;
	/* Leave high bits as 0 to write a 'kernel message' */

	/* Write log message to buffer */
	append(&ptr, پیام_end, "<%u>", log_level);
	log_hostname = env_get("log_hostname");
	اگر (log_hostname)
		append(&ptr, پیام_end, "%s ", log_hostname);
	append(&ptr, پیام_end, "uboot: ");
	اگر (fmt & BIT(LOGF_LEVEL))
		append(&ptr, پیام_end, "%s.",
		       log_get_level_name(rec->level));
	اگر (fmt & BIT(LOGF_CAT))
		append(&ptr, پیام_end, "%s,",
		       log_get_cat_name(rec->cat));
	اگر (fmt & BIT(LOGF_FILE))
		append(&ptr, پیام_end, "%s:", rec->file);
	اگر (fmt & BIT(LOGF_سطر))
		append(&ptr, پیام_end, "%d-", rec->سطر);
	اگر (fmt & BIT(LOGF_FUNC))
		append(&ptr, پیام_end, "%s()", rec->func ?: "?");
	اگر (fmt & BIT(LOGF_پیام))
		append(&ptr, پیام_end, "%s%s",
		       fmt != BIT(LOGF_پیام) ? " " : "", rec->پیام);
	/* Consider trailing 0x00 */
	ptr++;

	debug("log message: '%s'\n", log_پیام);

	/* Broadcast message */
	bcast_ip.s_addr = 0xFFFFFFFFL;
	net_set_udp_header((uchar *)iphdr, bcast_ip, 514, 514, ptr - log_پیام);
	net_send_packet((uchar *)پیام, ptr - پیام);

out:
	بازگشت ret;
}

LOG_DRIVER(syslog) = {
	.name	= "syslog",
	.emit	= log_syslog_emit,
};

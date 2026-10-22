// SPDX-License-Identifier: GPL-2.0+
/*
 * Log to syslog.
 *
 * Copyright (c) 2020, Heinrich Schuchardt <xypron.glpk@gmx.de>
 */

#ضمن <env.h>
#ضمن <log.h>
#ضمن <net.h>
#ضمن <asm/global_data.h>

DECLARE_GLOBAL_داده_PTR;

#تعریف_کن میانگیر_اندازه_2 480

ساكن فراغ append(حرف **buf, حرف *buf_انتها, ثابت حرف *fmt, ...)
{
	va_list آرگومانها;
	size_t اندازه = buf_انتها - *buf;

	va_شروع(آرگومانها, fmt);
	vsnprintf(*buf, اندازه, fmt, آرگومانها);
	va_انتها(آرگومانها);
	*buf += طول_رشته(*buf);
}

ساكن صحيح log_syslog_emit(بنية log_device *ldev, بنية log_rec *rec)
{
	صحيح ret;
	صحيح fmt = gd->log_fmt;
	حرف پیام[میانگیر_اندازه_2];
	حرف *پیام_انتها = پیام + میانگیر_اندازه_2;
	حرف *ptr = پیام;
	حرف *iphdr;
	حرف *log_پیام;
	صحيح eth_hdr_اندازه;
	بنية درون_addr bcast_ip;
	بیعلامت صحيح log_level;
	حرف *log_hostname;

	/* Setup packet buffers */
	ret = net_init();
	إذا (ret)
		أرجع ret;
	/* Disable hardware and put it into the reset state */
	eth_halt();
	/* Set current device according to environment variables */
	eth_مجموعه_current();
	/* Get hardware ready for send and receive operations */
	ret = eth_init();
	إذا (ret < 0) {
		eth_halt();
		اذهب بیرون;
	}

	memset(پیام, 0, میانگیر_اندازه_2);

	/* Set ethernet header */
	eth_hdr_اندازه = net_مجموعه_ether((uchar *)ptr, net_bcast_ethaddr, PROT_IP);
	ptr += eth_hdr_اندازه;
	iphdr = ptr;
	ptr += IP_UDP_HDR_اندازه;
	log_پیام = ptr;

	/*
	 * The syslog log levels defined in RFC 5424 match the U-Boot ones up to
	 * level 7 (debug).
	 */
	log_level = rec->level;
	إذا (log_level > 7)
		log_level = 7;
	/* Leave high bits as 0 to write a 'kernel message' */

	/* Write log message to buffer */
	append(&ptr, پیام_انتها, "<%u>", log_level);
	log_hostname = env_get("log_hostname");
	إذا (log_hostname)
		append(&ptr, پیام_انتها, "%s ", log_hostname);
	append(&ptr, پیام_انتها, "uboot: ");
	إذا (fmt & بیت(LOGF_LEVEL))
		append(&ptr, پیام_انتها, "%s.",
		       log_get_level_name(rec->level));
	إذا (fmt & بیت(LOGF_CAT))
		append(&ptr, پیام_انتها, "%s,",
		       log_get_cat_name(rec->cat));
	إذا (fmt & بیت(LOGF_FILE))
		append(&ptr, پیام_انتها, "%s:", rec->file);
	إذا (fmt & بیت(LOGF_سطر))
		append(&ptr, پیام_انتها, "%d-", rec->سطر);
	إذا (fmt & بیت(LOGF_FUNC))
		append(&ptr, پیام_انتها, "%s()", rec->func ?: "?");
	إذا (fmt & بیت(LOGF_پیام))
		append(&ptr, پیام_انتها, "%s%s",
		       fmt != بیت(LOGF_پیام) ? " " : "", rec->پیام);
	/* Consider trailing 0x00 */
	ptr++;

	debug("log message: '%s'\n", log_پیام);

	/* Broadcast message */
	bcast_ip.s_addr = 0xFFFFFFFFL;
	net_مجموعه_udp_سرآیند((uchar *)iphdr, bcast_ip, 514, 514, ptr - log_پیام);
	net_send_packet((uchar *)پیام, ptr - پیام);

بیرون:
	أرجع ret;
}

LOG_DRIVER(syslog) = {
	.name	= "syslog",
	.emit	= log_syslog_emit,
};

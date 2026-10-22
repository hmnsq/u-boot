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

DECLARE_GLOBAL_معلومات_PTR;

#تعریف_کړه بفر_اندازه_2 480

ساكن فراغ append(حرف **buf, حرف *buf_پای_ټکی, ثابت حرف *fmt, ...)
{
	va_list دلیلونه;
	size_t اندازه = buf_پای_ټکی - *buf;

	va_پیل(دلیلونه, fmt);
	vsnprintf(*buf, اندازه, fmt, دلیلونه);
	va_پای_ټکی(دلیلونه);
	*buf += مزي_اوږدوالی(*buf);
}

ساكن صحيح log_syslog_emit(بنية log_device *ldev, بنية log_rec *rec)
{
	صحيح ret;
	صحيح fmt = gd->log_fmt;
	حرف پیغام[بفر_اندازه_2];
	حرف *پیغام_پای_ټکی = پیغام + بفر_اندازه_2;
	حرف *ptr = پیغام;
	حرف *iphdr;
	حرف *log_پیغام;
	صحيح eth_hdr_اندازه;
	بنية دننه_addr bcast_ip;
	بینښه صحيح log_level;
	حرف *log_hostname;

	/* Setup packet buffers */
	ret = net_init();
	إذا (ret)
		أرجع ret;
	/* Disable hardware and put it into the reset state */
	eth_halt();
	/* Set current device according to environment variables */
	eth_سېټ_current();
	/* Get hardware ready for send and receive operations */
	ret = eth_init();
	إذا (ret < 0) {
		eth_halt();
		اذهب بهر;
	}

	memset(پیغام, 0, بفر_اندازه_2);

	/* Set ethernet header */
	eth_hdr_اندازه = net_سېټ_ether((uchar *)ptr, net_bcast_ethaddr, PROT_IP);
	ptr += eth_hdr_اندازه;
	iphdr = ptr;
	ptr += IP_UDP_HDR_اندازه;
	log_پیغام = ptr;

	/*
	 * The syslog log levels defined in RFC 5424 match the U-Boot ones up to
	 * level 7 (debug).
	 */
	log_level = rec->level;
	إذا (log_level > 7)
		log_level = 7;
	/* Leave high bits as 0 to write a 'kernel message' */

	/* Write log message to buffer */
	append(&ptr, پیغام_پای_ټکی, "<%u>", log_level);
	log_hostname = env_get("log_hostname");
	إذا (log_hostname)
		append(&ptr, پیغام_پای_ټکی, "%s ", log_hostname);
	append(&ptr, پیغام_پای_ټکی, "uboot: ");
	إذا (fmt & بټ(LOGF_LEVEL))
		append(&ptr, پیغام_پای_ټکی, "%s.",
		       log_get_level_name(rec->level));
	إذا (fmt & بټ(LOGF_CAT))
		append(&ptr, پیغام_پای_ټکی, "%s,",
		       log_get_cat_name(rec->cat));
	إذا (fmt & بټ(LOGF_FILE))
		append(&ptr, پیغام_پای_ټکی, "%s:", rec->file);
	إذا (fmt & بټ(LOGF_کرښه))
		append(&ptr, پیغام_پای_ټکی, "%d-", rec->کرښه);
	إذا (fmt & بټ(LOGF_FUNC))
		append(&ptr, پیغام_پای_ټکی, "%s()", rec->func ?: "?");
	إذا (fmt & بټ(LOGF_پیغام))
		append(&ptr, پیغام_پای_ټکی, "%s%s",
		       fmt != بټ(LOGF_پیغام) ? " " : "", rec->پیغام);
	/* Consider trailing 0x00 */
	ptr++;

	debug("log message: '%s'\n", log_پیغام);

	/* Broadcast message */
	bcast_ip.s_addr = 0xFFFFFFFFL;
	net_سېټ_udp_سرلیک((uchar *)iphdr, bcast_ip, 514, 514, ptr - log_پیغام);
	net_send_packet((uchar *)پیغام, ptr - پیغام);

بهر:
	أرجع ret;
}

LOG_DRIVER(syslog) = {
	.name	= "syslog",
	.emit	= log_syslog_emit,
};

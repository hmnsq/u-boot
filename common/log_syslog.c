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

DECLARE_GLOBAL_انکڑے_PTR;

#تعریف_کرو بفر_ناپ_2 480

ساكن فراغ append(حرف **buf, حرف *buf_سرا, ثابت حرف *fmt, ...)
{
	va_list دلیلاں;
	size_t ناپ = buf_سرا - *buf;

	va_شروع(دلیلاں, fmt);
	vsnprintf(*buf, ناپ, fmt, دلیلاں);
	va_سرا(دلیلاں);
	*buf += لڑی_لمبائی(*buf);
}

ساكن صحيح log_syslog_emit(بنية log_device *ldev, بنية log_rec *rec)
{
	صحيح ret;
	صحيح fmt = gd->log_fmt;
	حرف پیغام[بفر_ناپ_2];
	حرف *پیغام_سرا = پیغام + بفر_ناپ_2;
	حرف *ptr = پیغام;
	حرف *iphdr;
	حرف *log_پیغام;
	صحيح eth_hdr_ناپ;
	بنية وچ_addr bcast_ip;
	بنانشانی صحيح log_level;
	حرف *log_hostname;

	/* Setup packet buffers */
	ret = net_init();
	إذا (ret)
		أرجع ret;
	/* Disable hardware and put it into the reset state */
	eth_halt();
	/* Set current device according to environment variables */
	eth_سیٹ_current();
	/* Get hardware ready for send and receive operations */
	ret = eth_init();
	إذا (ret < 0) {
		eth_halt();
		اذهب باہرلا;
	}

	memset(پیغام, 0, بفر_ناپ_2);

	/* Set ethernet header */
	eth_hdr_ناپ = net_سیٹ_ether((uchar *)ptr, net_bcast_ethaddr, PROT_IP);
	ptr += eth_hdr_ناپ;
	iphdr = ptr;
	ptr += IP_UDP_HDR_ناپ;
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
	append(&ptr, پیغام_سرا, "<%u>", log_level);
	log_hostname = env_get("log_hostname");
	إذا (log_hostname)
		append(&ptr, پیغام_سرا, "%s ", log_hostname);
	append(&ptr, پیغام_سرا, "uboot: ");
	إذا (fmt & ثنائی_ہندسہ(LOGF_LEVEL))
		append(&ptr, پیغام_سرا, "%s.",
		       log_get_level_name(rec->level));
	إذا (fmt & ثنائی_ہندسہ(LOGF_CAT))
		append(&ptr, پیغام_سرا, "%s,",
		       log_get_cat_name(rec->cat));
	إذا (fmt & ثنائی_ہندسہ(LOGF_FILE))
		append(&ptr, پیغام_سرا, "%s:", rec->file);
	إذا (fmt & ثنائی_ہندسہ(LOGF_سطر))
		append(&ptr, پیغام_سرا, "%d-", rec->سطر);
	إذا (fmt & ثنائی_ہندسہ(LOGF_FUNC))
		append(&ptr, پیغام_سرا, "%s()", rec->func ?: "?");
	إذا (fmt & ثنائی_ہندسہ(LOGF_پیغام))
		append(&ptr, پیغام_سرا, "%s%s",
		       fmt != ثنائی_ہندسہ(LOGF_پیغام) ? " " : "", rec->پیغام);
	/* Consider trailing 0x00 */
	ptr++;

	debug("log message: '%s'\n", log_پیغام);

	/* Broadcast message */
	bcast_ip.s_addr = 0xFFFFFFFFL;
	net_سیٹ_udp_سرخی((uchar *)iphdr, bcast_ip, 514, 514, ptr - log_پیغام);
	net_send_packet((uchar *)پیغام, ptr - پیغام);

باہرلا:
	أرجع ret;
}

LOG_DRIVER(syslog) = {
	.name	= "syslog",
	.emit	= log_syslog_emit,
};

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

DECLARE_GLOBAL_ڈیٹا_PTR;

#تعریف_کرو بفر_سائز_2 480

ساکن خالی append(حرف **buf, حرف *buf_انتہا, ثابت حرف *fmt, ...)
{
	va_list دلائل;
	size_t سائز = buf_انتہا - *buf;

	va_آغاز(دلائل, fmt);
	vsnprintf(*buf, سائز, fmt, دلائل);
	va_انتہا(دلائل);
	*buf += لڑی_لمبائی(*buf);
}

ساکن صحیح log_syslog_emit(ساخت log_device *ldev, ساخت log_rec *rec)
{
	صحیح ret;
	صحیح fmt = gd->log_fmt;
	حرف پیغام[بفر_سائز_2];
	حرف *پیغام_انتہا = پیغام + بفر_سائز_2;
	حرف *ptr = پیغام;
	حرف *iphdr;
	حرف *log_پیغام;
	صحیح eth_hdr_سائز;
	ساخت اندر_addr bcast_ip;
	غیرعلامتی صحیح log_level;
	حرف *log_hostname;

	/* Setup packet buffers */
	ret = net_init();
	اگر (ret)
		واپس ret;
	/* Disable hardware and put it into the reset state */
	eth_halt();
	/* Set current device according to environment variables */
	eth_سیٹ_current();
	/* Get hardware ready for send and receive operations */
	ret = eth_init();
	اگر (ret < 0) {
		eth_halt();
		جاؤ برآمد;
	}

	memset(پیغام, 0, بفر_سائز_2);

	/* Set ethernet header */
	eth_hdr_سائز = net_سیٹ_ether((uchar *)ptr, net_bcast_ethaddr, PROT_IP);
	ptr += eth_hdr_سائز;
	iphdr = ptr;
	ptr += IP_UDP_HDR_سائز;
	log_پیغام = ptr;

	/*
	 * The syslog log levels defined in RFC 5424 match the U-Boot ones up to
	 * level 7 (debug).
	 */
	log_level = rec->level;
	اگر (log_level > 7)
		log_level = 7;
	/* Leave high bits as 0 to write a 'kernel message' */

	/* Write log message to buffer */
	append(&ptr, پیغام_انتہا, "<%u>", log_level);
	log_hostname = env_get("log_hostname");
	اگر (log_hostname)
		append(&ptr, پیغام_انتہا, "%s ", log_hostname);
	append(&ptr, پیغام_انتہا, "uboot: ");
	اگر (fmt & ثنائی_ہندسہ(LOGF_LEVEL))
		append(&ptr, پیغام_انتہا, "%s.",
		       log_get_level_name(rec->level));
	اگر (fmt & ثنائی_ہندسہ(LOGF_CAT))
		append(&ptr, پیغام_انتہا, "%s,",
		       log_get_cat_name(rec->cat));
	اگر (fmt & ثنائی_ہندسہ(LOGF_FILE))
		append(&ptr, پیغام_انتہا, "%s:", rec->file);
	اگر (fmt & ثنائی_ہندسہ(LOGF_سطر))
		append(&ptr, پیغام_انتہا, "%d-", rec->سطر);
	اگر (fmt & ثنائی_ہندسہ(LOGF_FUNC))
		append(&ptr, پیغام_انتہا, "%s()", rec->func ?: "?");
	اگر (fmt & ثنائی_ہندسہ(LOGF_پیغام))
		append(&ptr, پیغام_انتہا, "%s%s",
		       fmt != ثنائی_ہندسہ(LOGF_پیغام) ? " " : "", rec->پیغام);
	/* Consider trailing 0x00 */
	ptr++;

	debug("log message: '%s'\n", log_پیغام);

	/* Broadcast message */
	bcast_ip.s_addr = 0xFFFFFFFFL;
	net_سیٹ_udp_سرخی((uchar *)iphdr, bcast_ip, 514, 514, ptr - log_پیغام);
	net_send_packet((uchar *)پیغام, ptr - پیغام);

برآمد:
	واپس ret;
}

LOG_DRIVER(syslog) = {
	.name	= "syslog",
	.emit	= log_syslog_emit,
};

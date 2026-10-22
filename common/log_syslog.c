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

#وصف_ڪريو بفر_سائيز_2 480

ساكن فراغ append(حرف **buf, حرف *buf_پڇاڙي, ثابت حرف *fmt, ...)
{
	va_list دليلون;
	size_t سائيز = buf_پڇاڙي - *buf;

	va_شروعات(دليلون, fmt);
	vsnprintf(*buf, سائيز, fmt, دليلون);
	va_پڇاڙي(دليلون);
	*buf += لڙي_ڊيگهه(*buf);
}

ساكن صحيح log_syslog_emit(بنية log_device *ldev, بنية log_rec *rec)
{
	صحيح ret;
	صحيح fmt = gd->log_fmt;
	حرف نياپو[بفر_سائيز_2];
	حرف *نياپو_پڇاڙي = نياپو + بفر_سائيز_2;
	حرف *ptr = نياپو;
	حرف *iphdr;
	حرف *log_نياپو;
	صحيح eth_hdr_سائيز;
	بنية اندريون_addr bcast_ip;
	بنانشان صحيح log_level;
	حرف *log_hostname;

	/* Setup packet buffers */
	ret = net_init();
	إذا (ret)
		أرجع ret;
	/* Disable hardware and put it into the reset state */
	eth_halt();
	/* Set current device according to environment variables */
	eth_سيٽ_current();
	/* Get hardware ready for send and receive operations */
	ret = eth_init();
	إذا (ret < 0) {
		eth_halt();
		اذهب ٻاهريون;
	}

	memset(نياپو, 0, بفر_سائيز_2);

	/* Set ethernet header */
	eth_hdr_سائيز = net_سيٽ_ether((uchar *)ptr, net_bcast_ethaddr, PROT_IP);
	ptr += eth_hdr_سائيز;
	iphdr = ptr;
	ptr += IP_UDP_HDR_سائيز;
	log_نياپو = ptr;

	/*
	 * The syslog log levels defined in RFC 5424 match the U-Boot ones up to
	 * level 7 (debug).
	 */
	log_level = rec->level;
	إذا (log_level > 7)
		log_level = 7;
	/* Leave high bits as 0 to write a 'kernel message' */

	/* Write log message to buffer */
	append(&ptr, نياپو_پڇاڙي, "<%u>", log_level);
	log_hostname = env_get("log_hostname");
	إذا (log_hostname)
		append(&ptr, نياپو_پڇاڙي, "%s ", log_hostname);
	append(&ptr, نياپو_پڇاڙي, "uboot: ");
	إذا (fmt & ٻٽو_انگ(LOGF_LEVEL))
		append(&ptr, نياپو_پڇاڙي, "%s.",
		       log_get_level_name(rec->level));
	إذا (fmt & ٻٽو_انگ(LOGF_CAT))
		append(&ptr, نياپو_پڇاڙي, "%s,",
		       log_get_cat_name(rec->cat));
	إذا (fmt & ٻٽو_انگ(LOGF_FILE))
		append(&ptr, نياپو_پڇاڙي, "%s:", rec->file);
	إذا (fmt & ٻٽو_انگ(LOGF_سٽ))
		append(&ptr, نياپو_پڇاڙي, "%d-", rec->سٽ);
	إذا (fmt & ٻٽو_انگ(LOGF_FUNC))
		append(&ptr, نياپو_پڇاڙي, "%s()", rec->func ?: "?");
	إذا (fmt & ٻٽو_انگ(LOGF_نياپو))
		append(&ptr, نياپو_پڇاڙي, "%s%s",
		       fmt != ٻٽو_انگ(LOGF_نياپو) ? " " : "", rec->نياپو);
	/* Consider trailing 0x00 */
	ptr++;

	debug("log message: '%s'\n", log_نياپو);

	/* Broadcast message */
	bcast_ip.s_addr = 0xFFFFFFFFL;
	net_سيٽ_udp_سرو((uchar *)iphdr, bcast_ip, 514, 514, ptr - log_نياپو);
	net_send_packet((uchar *)نياپو, ptr - نياپو);

ٻاهريون:
	أرجع ret;
}

LOG_DRIVER(syslog) = {
	.name	= "syslog",
	.emit	= log_syslog_emit,
};

// SPDX-License-Identifier: GPL-2.0+
/*
 * Log to syslog.
 *
 * Copyright (c) 2020, Heinrich Schuchardt <xypron.glpk@gmx.de>
 */

#అన్తర్భావ <env.h>
#అన్తర్భావ <log.h>
#అన్తర్భావ <net.h>
#అన్తర్భావ <asm/global_data.h>

DECLARE_GLOBAL_దత్తాంశం_PTR;

#నిర్వచించు బఫరు_కొలత_2 480

స్థిర శూన్య append(అక్షరం **buf, అక్షరం *buf_చివర, స్థిరరాశి అక్షరం *fmt, ...)
{
	va_list వాదనలు;
	size_t కొలత = buf_చివర - *buf;

	va_ప్రారంభం(వాదనలు, fmt);
	vsnprintf(*buf, కొలత, fmt, వాదనలు);
	va_చివర(వాదనలు);
	*buf += అక్షరశ్రేణి_పొడవు(*buf);
}

స్థిర పూర్ణాంకం log_syslog_emit(నిర్మాణం log_device *ldev, నిర్మాణం log_rec *rec)
{
	పూర్ణాంకం ret;
	పూర్ణాంకం fmt = gd->log_fmt;
	అక్షరం సందేశం[బఫరు_కొలత_2];
	అక్షరం *సందేశం_చివర = సందేశం + బఫరు_కొలత_2;
	అక్షరం *ptr = సందేశం;
	అక్షరం *iphdr;
	అక్షరం *log_సందేశం;
	పూర్ణాంకం eth_hdr_కొలత;
	నిర్మాణం లోపల_addr bcast_ip;
	గుర్తులేని పూర్ణాంకం log_level;
	అక్షరం *log_hostname;

	/* Setup packet buffers */
	ret = net_init();
	ఒకవేళ (ret)
		తిరిగి ret;
	/* Disable hardware and put it into the reset state */
	eth_halt();
	/* Set current device according to environment variables */
	eth_సమితి_current();
	/* Get hardware ready for send and receive operations */
	ret = eth_init();
	ఒకవేళ (ret < 0) {
		eth_halt();
		వెళ్ళు బహిర్గతం;
	}

	memset(సందేశం, 0, బఫరు_కొలత_2);

	/* Set ethernet header */
	eth_hdr_కొలత = net_సమితి_ether((uchar *)ptr, net_bcast_ethaddr, PROT_IP);
	ptr += eth_hdr_కొలత;
	iphdr = ptr;
	ptr += IP_UDP_HDR_కొలత;
	log_సందేశం = ptr;

	/*
	 * The syslog log levels defined in RFC 5424 match the U-Boot ones up to
	 * level 7 (debug).
	 */
	log_level = rec->level;
	ఒకవేళ (log_level > 7)
		log_level = 7;
	/* Leave high bits as 0 to write a 'kernel message' */

	/* Write log message to buffer */
	append(&ptr, సందేశం_చివర, "<%u>", log_level);
	log_hostname = env_get("log_hostname");
	ఒకవేళ (log_hostname)
		append(&ptr, సందేశం_చివర, "%s ", log_hostname);
	append(&ptr, సందేశం_చివర, "uboot: ");
	ఒకవేళ (fmt & బిట్టు(LOGF_LEVEL))
		append(&ptr, సందేశం_చివర, "%s.",
		       log_get_level_name(rec->level));
	ఒకవేళ (fmt & బిట్టు(LOGF_CAT))
		append(&ptr, సందేశం_చివర, "%s,",
		       log_get_cat_name(rec->cat));
	ఒకవేళ (fmt & బిట్టు(LOGF_FILE))
		append(&ptr, సందేశం_చివర, "%s:", rec->file);
	ఒకవేళ (fmt & బిట్టు(LOGF_పంక్తి))
		append(&ptr, సందేశం_చివర, "%d-", rec->పంక్తి);
	ఒకవేళ (fmt & బిట్టు(LOGF_FUNC))
		append(&ptr, సందేశం_చివర, "%s()", rec->func ?: "?");
	ఒకవేళ (fmt & బిట్టు(LOGF_సందేశం))
		append(&ptr, సందేశం_చివర, "%s%s",
		       fmt != బిట్టు(LOGF_సందేశం) ? " " : "", rec->సందేశం);
	/* Consider trailing 0x00 */
	ptr++;

	debug("log message: '%s'\n", log_సందేశం);

	/* Broadcast message */
	bcast_ip.s_addr = 0xFFFFFFFFL;
	net_సమితి_udp_శీర్షిక((uchar *)iphdr, bcast_ip, 514, 514, ptr - log_సందేశం);
	net_send_packet((uchar *)సందేశం, ptr - సందేశం);

బహిర్గతం:
	తిరిగి ret;
}

LOG_DRIVER(syslog) = {
	.name	= "syslog",
	.emit	= log_syslog_emit,
};

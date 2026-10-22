// SPDX-License-Identifier: GPL-2.0+
/*
 * Log to syslog.
 *
 * Copyright (c) 2020, Heinrich Schuchardt <xypron.glpk@gmx.de>
 */

#ଅନ୍ତର୍ଭାଵ <env.h>
#ଅନ୍ତର୍ଭାଵ <log.h>
#ଅନ୍ତର୍ଭାଵ <net.h>
#ଅନ୍ତର୍ଭାଵ <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

#define BUFFER_SIZE 480

ସ୍ଥିର ଶୂନ୍ୟ append(ଅକ୍ଷର **buf, ଅକ୍ଷର *buf_end, ଅଚଳ ଅକ୍ଷର *fmt, ...)
{
	va_list args;
	size_t size = buf_end - *buf;

	va_start(args, fmt);
	vsnprintf(*buf, size, fmt, args);
	va_end(args);
	*buf += strlen(*buf);
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା log_syslog_emit(ସଂରଚନା log_device *ldev, ସଂରଚନା log_rec *rec)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା fmt = gd->log_fmt;
	ଅକ୍ଷର ସନ୍ଦେଶ[BUFFER_SIZE];
	ଅକ୍ଷର *ସନ୍ଦେଶ_end = ସନ୍ଦେଶ + BUFFER_SIZE;
	ଅକ୍ଷର *ptr = ସନ୍ଦେଶ;
	ଅକ୍ଷର *iphdr;
	ଅକ୍ଷର *log_ସନ୍ଦେଶ;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା eth_hdr_size;
	ସଂରଚନା in_addr bcast_ip;
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା log_level;
	ଅକ୍ଷର *log_hostname;

	/* Setup packet buffers */
	ret = net_init();
	ଯଦି (ret)
		ଫେରନ୍ତୁ ret;
	/* Disable hardware and put it into the reset state */
	eth_halt();
	/* Set current device according to environment variables */
	eth_set_current();
	/* Get hardware ready for send and receive operations */
	ret = eth_init();
	ଯଦି (ret < 0) {
		eth_halt();
		ଯାଅ out;
	}

	memset(ସନ୍ଦେଶ, 0, BUFFER_SIZE);

	/* Set ethernet header */
	eth_hdr_size = net_set_ether((uchar *)ptr, net_bcast_ethaddr, PROT_IP);
	ptr += eth_hdr_size;
	iphdr = ptr;
	ptr += IP_UDP_HDR_SIZE;
	log_ସନ୍ଦେଶ = ptr;

	/*
	 * The syslog log levels defined in RFC 5424 match the U-Boot ones up to
	 * level 7 (debug).
	 */
	log_level = rec->level;
	ଯଦି (log_level > 7)
		log_level = 7;
	/* Leave high bits as 0 to write a 'kernel message' */

	/* Write log message to buffer */
	append(&ptr, ସନ୍ଦେଶ_end, "<%u>", log_level);
	log_hostname = env_get("log_hostname");
	ଯଦି (log_hostname)
		append(&ptr, ସନ୍ଦେଶ_end, "%s ", log_hostname);
	append(&ptr, ସନ୍ଦେଶ_end, "uboot: ");
	ଯଦି (fmt & BIT(LOGF_LEVEL))
		append(&ptr, ସନ୍ଦେଶ_end, "%s.",
		       log_get_level_name(rec->level));
	ଯଦି (fmt & BIT(LOGF_CAT))
		append(&ptr, ସନ୍ଦେଶ_end, "%s,",
		       log_get_cat_name(rec->cat));
	ଯଦି (fmt & BIT(LOGF_FILE))
		append(&ptr, ସନ୍ଦେଶ_end, "%s:", rec->file);
	ଯଦି (fmt & BIT(LOGF_ଧାଡ଼ି))
		append(&ptr, ସନ୍ଦେଶ_end, "%d-", rec->ଧାଡ଼ି);
	ଯଦି (fmt & BIT(LOGF_FUNC))
		append(&ptr, ସନ୍ଦେଶ_end, "%s()", rec->func ?: "?");
	ଯଦି (fmt & BIT(LOGF_ସନ୍ଦେଶ))
		append(&ptr, ସନ୍ଦେଶ_end, "%s%s",
		       fmt != BIT(LOGF_ସନ୍ଦେଶ) ? " " : "", rec->ସନ୍ଦେଶ);
	/* Consider trailing 0x00 */
	ptr++;

	debug("log message: '%s'\n", log_ସନ୍ଦେଶ);

	/* Broadcast message */
	bcast_ip.s_addr = 0xFFFFFFFFL;
	net_set_udp_header((uchar *)iphdr, bcast_ip, 514, 514, ptr - log_ସନ୍ଦେଶ);
	net_send_packet((uchar *)ସନ୍ଦେଶ, ptr - ସନ୍ଦେଶ);

out:
	ଫେରନ୍ତୁ ret;
}

LOG_DRIVER(syslog) = {
	.name	= "syslog",
	.emit	= log_syslog_emit,
};

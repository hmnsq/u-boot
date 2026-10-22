// SPDX-License-Identifier: GPL-2.0+
/*
 * Log to syslog.
 *
 * Copyright (c) 2020, Heinrich Schuchardt <xypron.glpk@gmx.de>
 */

#अन्तर्भाव <env.h>
#अन्तर्भाव <log.h>
#अन्तर्भाव <net.h>
#अन्तर्भाव <asm/global_data.h>

DECLARE_GLOBAL_दत्त_PTR;

#परिभाषय बफर_आकार_2 480

स्थिर शून्य append(वर्ण **buf, वर्ण *buf_अन्तः, ध्रुव वर्ण *fmt, ...)
{
	va_list तर्काः;
	size_t आकार = buf_अन्तः - *buf;

	va_start(तर्काः, fmt);
	vsnprintf(*buf, आकार, fmt, तर्काः);
	va_अन्तः(तर्काः);
	*buf += वाक्यदैर्घ्यम्(*buf);
}

स्थिर पूर्णाङ्क log_syslog_emit(संरचना log_device *ldev, संरचना log_rec *rec)
{
	पूर्णाङ्क ret;
	पूर्णाङ्क fmt = gd->log_fmt;
	वर्ण सन्देशः[बफर_आकार_2];
	वर्ण *सन्देशः_अन्तः = सन्देशः + बफर_आकार_2;
	वर्ण *ptr = सन्देशः;
	वर्ण *iphdr;
	वर्ण *log_सन्देशः;
	पूर्णाङ्क eth_hdr_आकार;
	संरचना अन्तर्_addr bcast_ip;
	अचिह्नित पूर्णाङ्क log_level;
	वर्ण *log_hostname;

	/* Setup packet buffers */
	ret = net_init();
	यदि (ret)
		प्रत्यागम ret;
	/* Disable hardware and put it into the reset state */
	eth_halt();
	/* Set current device according to environment variables */
	eth_समुच्चयः_current();
	/* Get hardware ready for send and receive operations */
	ret = eth_init();
	यदि (ret < 0) {
		eth_halt();
		गच्छ बहिर्गमनम्;
	}

	स्मृतिं_पूरय(सन्देशः, 0, बफर_आकार_2);

	/* Set ethernet header */
	eth_hdr_आकार = net_समुच्चयः_ether((uchar *)ptr, net_bcast_ethaddr, PROT_IP);
	ptr += eth_hdr_आकार;
	iphdr = ptr;
	ptr += IP_UDP_HDR_आकार;
	log_सन्देशः = ptr;

	/*
	 * The syslog log levels defined in RFC 5424 match the U-Boot ones up to
	 * level 7 (debug).
	 */
	log_level = rec->level;
	यदि (log_level > 7)
		log_level = 7;
	/* Leave high bits as 0 to write a 'kernel message' */

	/* Write log message to buffer */
	append(&ptr, सन्देशः_अन्तः, "<%u>", log_level);
	log_hostname = env_get("log_hostname");
	यदि (log_hostname)
		append(&ptr, सन्देशः_अन्तः, "%s ", log_hostname);
	append(&ptr, सन्देशः_अन्तः, "uboot: ");
	यदि (fmt & BIT(LOGF_LEVEL))
		append(&ptr, सन्देशः_अन्तः, "%s.",
		       log_get_level_name(rec->level));
	यदि (fmt & BIT(LOGF_CAT))
		append(&ptr, सन्देशः_अन्तः, "%s,",
		       log_get_cat_name(rec->cat));
	यदि (fmt & BIT(LOGF_FILE))
		append(&ptr, सन्देशः_अन्तः, "%s:", rec->file);
	यदि (fmt & BIT(LOGF_पङ्क्तिः))
		append(&ptr, सन्देशः_अन्तः, "%d-", rec->पङ्क्तिः);
	यदि (fmt & BIT(LOGF_FUNC))
		append(&ptr, सन्देशः_अन्तः, "%s()", rec->func ?: "?");
	यदि (fmt & BIT(LOGF_सन्देशः))
		append(&ptr, सन्देशः_अन्तः, "%s%s",
		       fmt != BIT(LOGF_सन्देशः) ? " " : "", rec->सन्देशः);
	/* Consider trailing 0x00 */
	ptr++;

	debug("log message: '%s'\n", log_सन्देशः);

	/* Broadcast message */
	bcast_ip.s_addr = 0xFFFFFFFFL;
	net_समुच्चयः_udp_शीर्षकम्((uchar *)iphdr, bcast_ip, 514, 514, ptr - log_सन्देशः);
	net_send_packet((uchar *)सन्देशः, ptr - सन्देशः);

बहिर्गमनम्:
	प्रत्यागम ret;
}

LOG_DRIVER(syslog) = {
	.name	= "syslog",
	.emit	= log_syslog_emit,
};

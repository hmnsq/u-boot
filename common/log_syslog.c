// SPDX-License-Identifier: GPL-2.0+
/*
 * Log to syslog.
 *
 * Copyright (c) 2020, Heinrich Schuchardt <xypron.glpk@gmx.de>
 */

#समावेश <env.h>
#समावेश <log.h>
#समावेश <net.h>
#समावेश <asm/global_data.h>

DECLARE_GLOBAL_तथ्य_PTR;

#घोषणा बफर_आकार_2 480

अटल व्योम append(अक्षर **buf, अक्षर *buf_end, स्थिर अक्षर *fmt, ...)
{
	बहु_सूची args;
	माप_प्रकार आकार = buf_end - *buf;

	बहु_शुरू(args, fmt);
	vsnprintf(*buf, आकार, fmt, args);
	बहु_पूर्ण(args);
	*buf += म_माप(*buf);
}

अटल पूर्णांक log_syslog_emit(काष्ठा log_device *ldev, काष्ठा log_rec *rec)
{
	पूर्णांक ret;
	पूर्णांक fmt = gd->log_fmt;
	अक्षर संदेश[बफर_आकार_2];
	अक्षर *संदेश_end = संदेश + बफर_आकार_2;
	अक्षर *ptr = संदेश;
	अक्षर *iphdr;
	अक्षर *log_संदेश;
	पूर्णांक eth_hdr_आकार;
	काष्ठा in_addr bcast_ip;
	अचिन्हित पूर्णांक log_level;
	अक्षर *log_hostname;

	/* Setup packet buffers */
	ret = net_init();
	अगर (ret)
		वापस ret;
	/* Disable hardware and put it into the reset state */
	eth_halt();
	/* Set current device according to environment variables */
	eth_set_current();
	/* Get hardware ready for send and receive operations */
	ret = eth_init();
	अगर (ret < 0) {
		eth_halt();
		जाओ out;
	}

	स_रखो(संदेश, 0, बफर_आकार_2);

	/* Set ethernet header */
	eth_hdr_आकार = net_set_ether((uchar *)ptr, net_bcast_ethaddr, PROT_IP);
	ptr += eth_hdr_आकार;
	iphdr = ptr;
	ptr += IP_UDP_HDR_आकार;
	log_संदेश = ptr;

	/*
	 * The syslog log levels defined in RFC 5424 match the U-Boot ones up to
	 * level 7 (debug).
	 */
	log_level = rec->level;
	अगर (log_level > 7)
		log_level = 7;
	/* Leave high bits as 0 to write a 'kernel message' */

	/* Write log message to buffer */
	append(&ptr, संदेश_end, "<%u>", log_level);
	log_hostname = env_get("log_hostname");
	अगर (log_hostname)
		append(&ptr, संदेश_end, "%s ", log_hostname);
	append(&ptr, संदेश_end, "uboot: ");
	अगर (fmt & BIT(LOGF_LEVEL))
		append(&ptr, संदेश_end, "%s.",
		       log_get_level_name(rec->level));
	अगर (fmt & BIT(LOGF_CAT))
		append(&ptr, संदेश_end, "%s,",
		       log_get_cat_name(rec->cat));
	अगर (fmt & BIT(LOGF_FILE))
		append(&ptr, संदेश_end, "%s:", rec->file);
	अगर (fmt & BIT(LOGF_पंक्ति))
		append(&ptr, संदेश_end, "%d-", rec->पंक्ति);
	अगर (fmt & BIT(LOGF_FUNC))
		append(&ptr, संदेश_end, "%s()", rec->func ?: "?");
	अगर (fmt & BIT(LOGF_संदेश))
		append(&ptr, संदेश_end, "%s%s",
		       fmt != BIT(LOGF_संदेश) ? " " : "", rec->संदेश);
	/* Consider trailing 0x00 */
	ptr++;

	debug("log message: '%s'\n", log_संदेश);

	/* Broadcast message */
	bcast_ip.s_addr = 0xFFFFFFFFL;
	net_set_udp_header((uchar *)iphdr, bcast_ip, 514, 514, ptr - log_संदेश);
	net_send_packet((uchar *)संदेश, ptr - संदेश);

out:
	वापस ret;
}

LOG_DRIVER(syslog) = {
	.name	= "syslog",
	.emit	= log_syslog_emit,
};

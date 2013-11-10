/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */
/*
 * Copyright (c) 1997 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the Daedalus Research
 *	Group at the University of California Berkeley.
 * 4. Neither the name of the University nor of the Laboratory may be used
 *    to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * Contributed by Pavlo Ilin
 *
 */

#include "p802_15_4ll.h"

#include <address.h>
#include <mac.h>

#include "p802_15_4pkt.h"


static class LL802_15_4Class : public TclClass
{
public:
	LL802_15_4Class() : TclClass("LL/LL802_15_4") {}
	TclObject* create(int, const char* const* ) {
		return new LL802_15_4;
	}
} class_ll802_14_4;

LL802_15_4::LL802_15_4() : 
	GTS_delivery_(0)
{
	bind("GTS_delivery_", &GTS_delivery_);
}

int LL802_15_4::command(int argc, const char* const* argv)
{
	return LL::command(argc, argv);
}

void LL802_15_4::sendDown(Packet* p)
{
	// hdr_cmn *ch = HDR_CMN(p);
	hdr_ip *ih = HDR_IP(p);
	hdr_lrwpan* wph = HDR_LRWPAN(p);

	nsaddr_t dst = (nsaddr_t)Address::instance().get_nodeaddr(ih->daddr());

	hdr_ll *llh = HDR_LL(p);
	char *mh = (char*)p->access(hdr_mac::offset_);
	llh->seqno_ = ++seqno_;
	llh->lltype() = LL_DATA;

	mac_->hdr_src(mh, mac_->addr());
	mac_->hdr_type(mh, ETHERTYPE_IP);

	if (IP_BROADCAST == (u_int32_t) dst)
	{
		mac_->hdr_dst((char*) HDR_MAC(p), MAC_BROADCAST);
	}
	else
	{
		mac_->hdr_dst((char*) HDR_MAC(p), dst);
	}

	if (GTS_delivery_)
		wph->needGTS = true;

	Scheduler& s = Scheduler::instance();
	// let mac decide when to take a new packet from the queue.
	s.schedule(downtarget_, p, delay_);
}
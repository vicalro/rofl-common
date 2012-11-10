/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "cofdpt.h"

cofdpt::cofdpt(
		crofbase *rofbase,
		int newsd,
		caddress const& ra,
		int domain,
		int type,
		int protocol) :
	dpid(0),
	dpmac(cmacaddr("00:00:00:00:00:00")),
	n_buffers(0),
	n_tables(0),
	capabilities(0),
	flags(0),
	miss_send_len(0),
	rofbase(rofbase),
	features_reply_timeout(DEFAULT_DP_FEATURES_REPLY_TIMEOUT),
	get_config_reply_timeout(DEFAULT_DP_GET_CONFIG_REPLY_TIMEOUT),
	stats_reply_timeout(DEFAULT_DP_STATS_REPLY_TIMEOUT),
	barrier_reply_timeout(DEFAULT_DB_BARRIER_REPLY_TIMEOUT)
{
	WRITELOG(COFDPT, DBG, "cofdpath(%p)::cofdpath() "
			"dpid:%"UINT64DBGFMT" ",
			this, dpid);

	// set initial state
	init_state(COFDPT_STATE_INIT);

	// trigger sending of FEATURES request
	register_timer(COFDPT_TIMER_FEATURES_REQUEST, 1);
}



cofdpt::cofdpt(
		crofbase *rofbase,
		caddress const& ra,
		int domain,
		int type,
		int protocol) :
	dpid(0),
	dpmac(cmacaddr("00:00:00:00:00:00")),
	n_buffers(0),
	n_tables(0),
	capabilities(0),
	flags(0),
	miss_send_len(0),
	rofbase(rofbase),
	features_reply_timeout(DEFAULT_DP_FEATURES_REPLY_TIMEOUT),
	get_config_reply_timeout(DEFAULT_DP_GET_CONFIG_REPLY_TIMEOUT),
	stats_reply_timeout(DEFAULT_DP_STATS_REPLY_TIMEOUT),
	barrier_reply_timeout(DEFAULT_DB_BARRIER_REPLY_TIMEOUT)
{
	WRITELOG(COFDPT, DBG, "cofdpath(%p)::cofdpath() "
			"dpid:%"UINT64DBGFMT" ",
			this, dpid);

	// set initial state
	init_state(COFDPT_STATE_INIT);

	// trigger sending of FEATURES request
	register_timer(COFDPT_TIMER_FEATURES_REQUEST, 1);
}



cofdpt::~cofdpt()
{
	WRITELOG(COFDPT, DBG, "cofdpath(%p)::~cofdpath() "
			"dpid:%"UINT64DBGFMT"  %s",
			this, dpid, this->c_str());

	rofbase->handle_dpath_close(this);

#if 0
	for (std::map<uint8_t, cfttable*>::iterator
			it = flow_tables.begin(); it != flow_tables.end(); ++it)
	{
		delete it->second;
	}
	flow_tables.clear();
#endif

	// remove all cofport instances
	while (not ports.empty())
	{
		delete (ports.begin()->second);
	}
}



void
cofdpt::handle_accepted(
		csocket *socket,
		int newsd,
		caddress const& ra)
{

}



void
cofdpt::handle_connected(
		csocket *socket,
		int sd)
{

}



void
cofdpt::handle_connect_refused(
		csocket *socket,
		int sd)
{

}



void
cofdpt::handle_read(
		csocket *socket,
		int sd)
{

}



void
cofdpt::handle_closed(
		csocket *socket,
		int sd)
{

}



void
cofdpt::handle_timeout(int opaque)
{
	switch (opaque) {
	case COFDPT_TIMER_FEATURES_REQUEST:
		{
			WRITELOG(COFDPT, DBG, "cofdpath(%p): sending -FEATURES-REQUEST-", this);
			rofbase->send_features_request(this);
		}
		break;
	case COFDPT_TIMER_FEATURES_REPLY:
		{
			handle_features_reply_timeout();
		}
		break;
	case COFDPT_TIMER_GET_CONFIG_REQUEST:
		{
			WRITELOG(COFDPT, DBG, "cofdpath(%p): sending -GET-CONFIG-REQUEST-", this);
			rofbase->send_get_config_request(this);
		}
		break;
	case COFDPT_TIMER_GET_CONFIG_REPLY:
		{
			handle_get_config_reply_timeout();
		}
		break;
	case COFDPT_TIMER_STATS_REQUEST:
		{
			WRITELOG(COFDPT, DBG, "cofdpath(%p): sending -STATS-REQUEST-", this);
			rofbase->send_stats_request(this, OFPST_TABLE, 0);
		}
		break;
	case COFDPT_TIMER_STATS_REPLY:
		{
			handle_stats_reply_timeout();
		}
		break;
	case COFDPT_TIMER_BARRIER_REQUEST:
		{
			WRITELOG(COFDPT, DBG, "cofdpath(%p): sending -BARRIER-REQUEST-", this);
			rofbase->send_barrier_request(this);
		}
		break;
	case COFDPT_TIMER_BARRIER_REPLY:
		{
			handle_barrier_reply_timeout();
		}
		break;
	default:
		WRITELOG(COFDPT, DBG, "unknown timer event %d", opaque);
		ciosrv::handle_timeout(opaque);
		break;
	}


}


void
cofdpt::features_request_sent()
{
	register_timer(COFDPT_TIMER_FEATURES_REPLY, features_reply_timeout /* seconds */);
}


void
cofdpt::features_reply_rcvd(
		cofpacket *pack)
{
	try {
		cancel_timer(COFDPT_TIMER_FEATURES_REPLY);

		/* check for existing cofdpath controlling this associated dpid
		 * we assume that a duplicated dpid is caused by loss of and a
		 * reconnected TCP connection, as we cannot determine whether a
		 * second connection attempt is some kind of DoS attack
		 */
		try {
			uint64_t dpid = be64toh(pack->ofh_switch_features->datapath_id);

			cofdpt *dpath = &(rofbase->dpath_find(dpid));

			delete dpath; // clean-up and calls dpath's destructor

		} catch (eOFbaseNotAttached& e) {}



		dpid 			= be64toh(pack->ofh_switch_features->datapath_id);
		n_buffers 		= be32toh(pack->ofh_switch_features->n_buffers);
		n_tables 		= pack->ofh_switch_features->n_tables;
		capabilities 	= be32toh(pack->ofh_switch_features->capabilities);

		int portslen = be16toh(pack->ofh_switch_features->header.length) -
												sizeof(struct ofp_switch_features);


		WRITELOG(COFDPT, DBG, "cofdpath(%p)::features_reply_rcvd() "
				"dpid:%"UINT64DBGFMT" ",
				this, dpid);



		cofport::ports_parse(ports, pack->ofh_switch_features->ports, portslen);

		WRITELOG(COFDPT, DBG, "cofdpath(%p)::features_reply_rcvd() %s", this, this->c_str());


		// dpid as std::string
		cvastring vas;
		s_dpid = std::string(vas("0x%llx", dpid));

		// lower 48bits from dpid as datapath mac address
		dpmac[0] = (dpid & 0x0000ff0000000000ULL) >> 40;
		dpmac[1] = (dpid & 0x000000ff00000000ULL) >> 32;
		dpmac[2] = (dpid & 0x00000000ff000000ULL) >> 24;
		dpmac[3] = (dpid & 0x0000000000ff0000ULL) >> 16;
		dpmac[4] = (dpid & 0x000000000000ff00ULL) >>  8;
		dpmac[5] = (dpid & 0x00000000000000ffULL) >>  0;
		dpmac[0] &= 0xfc;




		if (COFDPT_STATE_INIT == cur_state())
		{
			// next step: send GET-CONFIG request to datapath
			register_timer(COFDPT_TIMER_GET_CONFIG_REQUEST, 0);
		}


	} catch (eOFportMalformed& e) {

		WRITELOG(COFDPT, DBG, "exception: malformed FEATURES reply received");

//		rofbase->send_down_hello_message(this, true /*bye*/);

		delete this;

	}
}


void
cofdpt::handle_features_reply_timeout()
{
	WRITELOG(COFDPT, DBG, "cofdpath(%p)::handle_features_reply_timeout() ", this);

	rofbase->handle_features_reply_timeout(this);

	delete this;
}


void
cofdpt::get_config_request_sent()
{
	register_timer(COFDPT_TIMER_GET_CONFIG_REPLY, get_config_reply_timeout);
}


void
cofdpt::get_config_reply_rcvd(
		cofpacket *pack)
{
	cancel_timer(COFDPT_TIMER_GET_CONFIG_REPLY);

	flags = be16toh(pack->ofh_switch_config->flags);
	miss_send_len = be16toh(pack->ofh_switch_config->miss_send_len);

	rofbase->handle_get_config_reply(this, pack);

	if (cur_state() == COFDPT_STATE_INIT)
	{
		// send stats request during initialization
		register_timer(COFDPT_TIMER_STATS_REQUEST, 0);
	}
}


void
cofdpt::handle_get_config_reply_timeout()
{
	WRITELOG(COFDPT, DBG, "cofdpath(%p)::handle_get_config_reply_timeout() "
			"dpid:%"UINT64DBGFMT" ",
			this, dpid);

	rofbase->handle_get_config_reply_timeout(this);

	delete this;
}


void
cofdpt::stats_request_sent(
		uint32_t xid)
{
	try {
		xidstore[OFPT_STATS_REQUEST].xid_add(this, xid, stats_reply_timeout);

		if (not pending_timer(COFDPT_TIMER_STATS_REPLY))
		{
			register_timer(COFDPT_TIMER_STATS_REPLY, stats_reply_timeout);
		}

	} catch (eXidStoreXidBusy& e) {

		// should never happen, TODO: log error
	}
}


void
cofdpt::stats_reply_rcvd(
		cofpacket *pack)
{
	cancel_timer(COFDPT_TIMER_STATS_REPLY);

	xidstore[OFPT_STATS_REQUEST].xid_rem(be32toh(pack->ofh_header->xid));

	rofbase->handle_stats_reply(this, pack);


	if (cur_state() == COFDPT_STATE_INIT) // enter state running during initialization
	{
		//flow_mod_reset();

		//group_mod_reset();

		new_state(COFDPT_STATE_RUNNING);

		//lldp_emulated_ports(); // skip this for now, we move that to a derived controller at some point in the future

		rofbase->handle_dpath_open(this);
	}
}


void
cofdpt::handle_stats_reply_timeout()
{
	WRITELOG(COFDPT, DBG, "cofdpath(%p)::handle_stats_reply_timeout() "
			"dpid:%"UINT64DBGFMT" ",
			this, dpid);

restart:
	for (cxidstore::iterator
				it = xidstore[OFPT_STATS_REQUEST].begin();
							it != xidstore[OFPT_STATS_REQUEST].end(); ++it)
	{
		cxidtrans& xidt = it->second;

		if (xidt.timeout <= cclock::now())
		{
			rofbase->handle_stats_reply_timeout(this, xidt.xid);

			xidstore[OFPT_STATS_REQUEST].xid_rem(xidt.xid);

			goto restart;
		}
	}

	if (not xidstore.empty())
	{
		reset_timer(COFDPT_TIMER_STATS_REPLY, stats_reply_timeout);
	}
}


void
cofdpt::barrier_request_sent(
		uint32_t xid)
{
	try {
		xidstore[OFPT_BARRIER_REQUEST].xid_add(this, xid, barrier_reply_timeout);

		if (not pending_timer(COFDPT_TIMER_BARRIER_REPLY))
		{
			register_timer(COFDPT_TIMER_BARRIER_REPLY, barrier_reply_timeout);
		}

	} catch (eXidStoreXidBusy& e) {

		// should never happen, TODO: log error
	}
}


void
cofdpt::barrier_reply_rcvd(cofpacket *pack)
{
	cancel_timer(COFDPT_TIMER_BARRIER_REPLY);

	xidstore[OFPT_BARRIER_REQUEST].xid_rem(be32toh(pack->ofh_header->xid));

	rofbase->handle_barrier_reply(this, pack);
}


void
cofdpt::handle_barrier_reply_timeout()
{
restart:
	for (cxidstore::iterator
			it = xidstore[OFPT_BARRIER_REQUEST].begin();
						it != xidstore[OFPT_BARRIER_REQUEST].end(); ++it)
	{
		cxidtrans& xidt = it->second;

		if (xidt.timeout <= cclock::now())
		{
			rofbase->handle_barrier_reply_timeout(this, xidt.xid);

			xidstore[OFPT_BARRIER_REQUEST].xid_rem(xidt.xid);

			goto restart;
		}
	}

	if (not xidstore.empty())
	{
		reset_timer(COFDPT_TIMER_BARRIER_REPLY, barrier_reply_timeout);
	}
}


void
cofdpt::flow_mod_sent(
		cofpacket *pack) throw (eOFdpathNotFound)
{
	try {
		WRITELOG(COFDPT, DBG, "cofdpath(%p)::flow_mod_sent() table_id: %d", this, pack->ofh_flow_mod->table_id);

#if 0
		if (0xff == pack->ofh_flow_mod->table_id)
		{
			std::map<uint8_t, cfttable*>::iterator it;
			for (it = flow_tables.begin(); it != flow_tables.end(); ++it)
			{
				it->second->update_ft_entry(this, pack);
			}
		}
		else
		{
			if (flow_tables.find(pack->ofh_flow_mod->table_id) == flow_tables.end())
			{
				throw eOFdpathNotFound();
			}

			// check for existence of flow_table with id pack->ofh_flow_mod->table_id first?
			flow_tables[pack->ofh_flow_mod->table_id]->update_ft_entry(this, pack);

			WRITELOG(COFDPT, DBG, "cofdpath(%p)::flow_mod_sent() table-id:%d flow-table: %s",
					this, pack->ofh_flow_mod->table_id, flow_tables[pack->ofh_flow_mod->table_id]->c_str());
		}
#endif

	} catch (cerror& e) {
		WRITELOG(CFTTABLE, DBG, "unable to add ftentry to local flow_table instance");
	}
}


void
cofdpt::flow_rmvd_rcvd(
		cofpacket *pack)
{
	rofbase->handle_flow_removed(this, pack);
}


void
cofdpt::flow_mod_reset()
{
	cflowentry fe;
	fe.set_command(OFPFC_DELETE);
	fe.set_table_id(OFPTT_ALL /*all tables*/);

	rofbase->send_flow_mod_message(this, fe); // calls this->flow_mod_sent() implicitly
}


void
cofdpt::group_mod_sent(
		cofpacket *pack)
{
	try {

#if 0
		grp_table.update_gt_entry(this, pack->ofh_group_mod);
#endif

	} catch (cerror& e) {
		WRITELOG(CFTTABLE, DBG, "unable to handle gtentry within local grp_table instance");

	}
}


void
cofdpt::group_mod_reset()
{
	cgroupentry ge;
	ge.set_command(OFPGC_DELETE);
	ge.set_group_id(OFPG_ALL);

	rofbase->send_group_mod_message(this, ge); // calls this->group_mod_sent() implicitly
}


void
cofdpt::table_mod_sent(cofpacket *pack)
{
	// TODO: adjust local flowtable
}


void
cofdpt::port_mod_sent(cofpacket *pack)
{
	if (ports.find(be32toh(pack->ofh_port_mod->port_no)) == ports.end())
	{
		return;
	}

	ports[be32toh(pack->ofh_port_mod->port_no)]->recv_port_mod(
										be32toh(pack->ofh_port_mod->config),
										be32toh(pack->ofh_port_mod->mask),
										be32toh(pack->ofh_port_mod->advertise));
}

void
cofdpt::packet_in_rcvd(cofpacket *pack)
{
	try {
		WRITELOG(COFDPT, DBG, "cofdpath(0x%llx)::packet_in_rcvd() %s", dpid, pack->c_str());

#if 0
		// update forwarding table
		uint32_t in_port = pack->match.get_in_port();
#endif

		// datalen must be at least one Ethernet header in size
		if (pack->packet.length() >= (2 * OFP_ETH_ALEN + sizeof(uint16_t)))
		{
#if 0
			// update local forwarding table
			fwdtable.mac_learning(pack->packet, dpid, in_port);

			WRITELOG(COFDPT, DBG, "cofdpath(0x%llx)::packet_in_rcvd() local fwdtable: %s",
					dpid, fwdtable.c_str());
#endif


#if 0
		rofbase->fwdtable.mac_learning(ether, dpid, in_port);

		WRITELOG(COFDPT, DBG, "cofdpath(0x%llx)::packet_in_rcvd() global fwdtable: %s",
				dpid, rofbase->fwdtable.c_str());
#endif

			// let derived class handle PACKET-IN event
			rofbase->handle_packet_in(this, pack);
		}
	} catch (eOFmatchNotFound& e) {

		WRITELOG(COFDPT, DBG, "cofdpath(0x%llx)::packet_in_rcvd() "
				"no in-port specified in Packet-In message", dpid);
	}
}


void
cofdpt::port_status_rcvd(cofpacket *pack)
{
	WRITELOG(COFDPT, DBG, "cofdpath(0x%016llx)::port_status_rcvd() %s",
			dpid, pack->c_str());

	std::map<uint32_t, cofport*>::iterator it;
	switch (pack->ofh_port_status->reason) {
	case OFPPR_ADD:
		if (ports.find(be32toh(pack->ofh_port_status->desc.port_no)) == ports.end())
		{
			cofport *lport = new cofport(&ports, &(pack->ofh_port_status->desc), sizeof(struct ofp_port));

			// let derived class handle PORT-STATUS message
			rofbase->handle_port_status(this, pack, lport);
		}
		break;
	case OFPPR_DELETE:
		if (ports.find(be32toh(pack->ofh_port_status->desc.port_no)) != ports.end())
		{
			// let derived class handle PORT-STATUS message
			rofbase->handle_port_status(this, pack, ports[be32toh(pack->ofh_port_status->desc.port_no)]);

			delete ports[be32toh(pack->ofh_port_status->desc.port_no)];

			ports.erase(be32toh(pack->ofh_port_status->desc.port_no));
		}
		break;
	case OFPPR_MODIFY:
		if (ports.find(be32toh(pack->ofh_port_status->desc.port_no)) != ports.end())
		{
			ports[be32toh(pack->ofh_port_status->desc.port_no)]->unpack(
																&(pack->ofh_port_status->desc),
																sizeof(struct ofp_port));

			// let derived class handle PORT-STATUS message
			rofbase->handle_port_status(this, pack, ports[be32toh(pack->ofh_port_status->desc.port_no)]);
		}
		break;
	}
}


void
cofdpt::fsp_open(cofmatch const& ofmatch)
{
	cofmatch m(ofmatch);
	croflexp_flowspace rexp(croflexp::OFPRET_FSP_ADD, m);

	cmemory packed(rexp.length());

	rexp.pack(packed.somem(), packed.memlen());

	rofbase->send_experimenter_message(
			this,
			OFPEXPID_ROFL,
			croflexp::OFPRET_FLOWSPACE,
			packed.somem(),
			packed.memlen());

}


void
cofdpt::fsp_close(cofmatch const& ofmatch)
{
	cofmatch m(ofmatch);
	croflexp_flowspace rexp(croflexp::OFPRET_FSP_DELETE, m);

	cmemory packed(rexp.length());

	rexp.pack(packed.somem(), packed.memlen());

	rofbase->send_experimenter_message(
			this,
			OFPEXPID_ROFL,
			croflexp::OFPRET_FLOWSPACE,
			packed.somem(),
			packed.memlen());

}



void
cofdpt::experimenter_message_rcvd(cofpacket *pack)
{
	switch (be32toh(pack->ofh_experimenter->experimenter)) {
	default:
		break;
	}

	// for now: send vendor extensions directly to class derived from crofbase
	rofbase->handle_experimenter_message(this, pack);
}


void
cofdpt::role_reply_rcvd(cofpacket *pack)
{
	rofbase->handle_role_reply(this, pack);
}




const char*
cofdpt::c_str()
{
	cvastring vas;
	info.assign(vas("cofdpath(%p) dpid:0x%llx buffers: %d tables: %d capabilities: 0x%x =>",
			this, dpid, n_buffers, n_tables, capabilities));

	std::map<uint32_t, cofport*>::iterator it;
	for (it = ports.begin(); it != ports.end(); ++it)
	{
		info.append(vas("\n  %s", it->second->c_str()));
	}

	return info.c_str();
}



cofport*
cofdpt::find_cofport(
	uint32_t port_no) throw (eOFdpathNotFound)
{
	std::map<uint32_t, cofport*>::iterator it;
	if (ports.find(port_no) == ports.end())
	{
		throw eOFdpathNotFound();
	}
	return ports[port_no];
}


cofport*
cofdpt::find_cofport(
	std::string port_name) throw (eOFdpathNotFound)
{
	std::map<uint32_t, cofport*>::iterator it;
	if ((it = find_if(ports.begin(), ports.end(),
			cofport_find_by_port_name(port_name))) == ports.end())
	{
		throw eOFdpathNotFound();
	}
	return it->second;
}


cofport*
cofdpt::find_cofport(
	cmacaddr const& maddr) throw (eOFdpathNotFound)
{
	std::map<uint32_t, cofport*>::iterator it;
	if ((it = find_if(ports.begin(), ports.end(),
			cofport_find_by_maddr(maddr))) == ports.end())
	{
		throw eOFdpathNotFound();
	}
	return it->second;
}



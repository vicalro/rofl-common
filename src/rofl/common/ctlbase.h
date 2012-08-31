/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CTLBASE_H
#define CTLBASE_H 1

#include <string>

#include <rofl/common/cfwdelem.h>
#include <rofl/common/openflow/cflowentry.h>
#include <rofl/common/cvastring.h>
#include <rofl/common/cadapt.h>


class eCtlBase 					: public eFwdElemBase {};
class eCtlBaseInval 			: public eCtlBase {};
class eCtlBaseExists 			: public eCtlBase {};
class eCtlBaseNotFound 			: public eCtlBase {};
class eCtlBaseDetached 			: public eCtlBase {};

class eCtlBaseEndpntNotFound 	: public eCtlBase {};
class eCtlBaseNotConnected 		: public eCtlBase {};


class ctlbase :
	public cfwdelem,
	public cadapt_owner
{
/*
 * data structures
 */
protected:

		/*
		 * there is only one layer (n-1) datapath for a transport controller (always)
		 * A transport controller can only use local interfaces (=ports).
		 * A remote port can not be used directly.
		 */

		uint64_t							 lldpid;		// layer (n-1) dpid in use (used in constructor)
		cofdpath							*dpath;			// layer (n-1) datapath in use (once it has connected)
		std::map<uint32_t, cadapt*> 		 n_ports; 		// map of portno's => cadapt mappings




private:

		std::string 						info;



/*
 * methods
 */
public:

	/**
	 *
	 */
	ctlbase(
			uint64_t lldpid, // dpid of layer (n-1) data path to be used
			std::string dpname = std::string("ctlbase0"),
			uint64_t dpid = crandom(8).uint64(),
			uint8_t n_tables = DEFAULT_FE_TABLES_NUM,
			uint32_t n_buffers = DEFAULT_FE_BUFFER_SIZE,
			caddress const& rpc_ctl_addr = caddress(AF_INET, "0.0.0.0", 6643),
			caddress const& rpc_dpt_addr = caddress(AF_INET, "0.0.0.0", 6633));

	/**
	 *
	 */
	virtual
	~ctlbase();


	/**
	 *
	 */
	const char*
	c_str();


	/**
	 *
	 */
	cadapt*
	find_adaptor_by_portno(uint32_t portno)
		throw (eCtlBaseNotFound);




protected:

	/*
	 * methods that must be overwritten by a derived transport controller
	 * These methods replace the versions defined by cfwdelem:
	 * - handle_port_status()
	 * - handle_packet_in()
	 *
	 */



	/**
	 * @name	handle_port_status
	 * @brief	Handle a port-status message from one of the registered adapters.
	 *
	 * This method must be overwritten by the derived transport controller.
	 * It is called by ctlbase when a port-status message was received from one
	 * of the registered adapters.
	 *
	 * @param ofport The ofport structure represents the OF data model for the changing port.
	 * @param reason One of the values from enum ofp_port_reason, e.g. OFPPR_ADD
	 */
	virtual void
	handle_port_status(
			uint8_t reason,
			cofport *ofport) = 0;


	/**
	 * @name	handle_packet_in
	 * @brief 	Handle a packet-in from one of the registered adapters.
	 *
	 * This method must be overwritten by the derived transport controller.
	 * It is called by ctlbase when a packet-in was received from one
	 * of the registered adapters.
	 *
	 * @param buffer_id buffer-id used by storage system on the datapath or OFP_NO_BUFFER
	 * @param total_len The total length of the packet stored in the buffer
	 * @param reason One of the values defined by enumeration ofp_packet_in_reason, e.g. OFPR_NO_MATCH
	 * @param match The C++ representation of a struct ofp_match
	 * @param frame The frame contains the packet data. Note, this may be shorter than total_len!
	 */
	virtual void
	handle_packet_in(
				uint32_t buffer_id,
				uint16_t total_len,
				uint8_t table_id,
				uint8_t reason,
				cofmatch const& match,
				fframe const& frame) = 0;














protected:

	/*
	 *  to be used by derived transport controller
	 *  These methods must be used by a derived transport controller
	 *  for sending messages to the layer (n-1) datapath.
	 *  ctlbase will automatically adapt these messages by
	 *  calling appropriate filter methods on all active adapters.
	 *
	 * - Stats-request
	 * - Packet-Out
	 * - FlowMod
	 * - GroupMod
	 * - TableMod
	 * - PortMod
	 */

	// STATS request/reply
	//

	/** Send OF STATS.request to data path entity.
	 *
	 * @param sw cofswitch instance representing the data path to be addressed
	 * @param type One of the OFPST_* constants
	 * @param flags OFPSF_REQ_* flags
	 * @param body Body of a STATS request
	 * @param bodylen length of STATS request body
	 */
	virtual void
	send_stats_request(
		uint16_t type,
		uint16_t flags,
		uint8_t *body = NULL,
		size_t bodylen = 0);


	// PACKET-OUT message
	//

	/** Send OF PACKET-OUT.message to data path entity.
	 *
	 * @param sw cofswitch instance representing the data path to be addressed
	 * @param buffer_id buffer ID assigned by datapath (-1 if none) in host byte order
	 * @param in_port Packet’s input port (OFPP_NONE if none) in host byte order
	 * @param acvec vector containing cofaction instances
	 * @param data optional packet data to be sent out
	 * @param datalen
	 */
	virtual void
	send_packet_out_message(
		uint32_t buffer_id,
		uint32_t in_port,
		cofaclist const& aclist,
		cpacket *pack = 0) throw (eCtlBaseInval);


	// FLOW-MOD message
	//

	/** Send OF FLOW-MOD.message to data path entity.
	 *
	 * All values in host byte order.
	 *
	 * @param sw cofswitch instance representing the data path to be addressed
	 * @param ofmatch An instance of class cofmatch holding the matching fields
	 * @param cookie Opaque controller-issued identifier
	 * @param command One of OFPFC_*
	 * @param idle_timeout Idle time before discarding (seconds)
	 * @param hard_timeout Max time before discarding (seconds)
	 * @param priority Priority level of flow entry
	 * @param buffer_id Buffered packet to apply to (or -1)
	 * @param out_port For OFPFC_DELETE* commands, require matching entries to include this as an output port. A value of OFPP_NONE indicates no restriction
	 * @param flags One of OFPFF_*
	 * @param acvec Vector of cofaction instances
	 */
	virtual void
	send_flow_mod_message(
		cofmatch& ofmatch,
		uint64_t cookie,
		uint64_t cookie_mask,
		uint8_t table_id,
		uint8_t command,
		uint16_t idle_timeout,
		uint16_t hard_timeout,
		uint16_t priority,
		uint32_t buffer_id,
		uint32_t out_port,
		uint32_t out_group,
		uint16_t flags,
		cofinlist const& inlist);

	virtual void
	send_flow_mod_message(
			cflowentry& flowentry);

	// GROUP-MOD message
	//

	/** Send OF GROUP-MOD.message to data path entity.
	 *
	 * All values in host byte order.
	 *
	 * @param sw cofswitch instance representing the data path to be addressed
	 * @param ofmatch An instance of class cofmatch holding the matching fields
	 * @param cookie Opaque controller-issued identifier
	 * @param command One of OFPFC_*
	 * @param idle_timeout Idle time before discarding (seconds)
	 * @param hard_timeout Max time before discarding (seconds)
	 * @param priority Priority level of flow entry
	 * @param buffer_id Buffered packet to apply to (or -1)
	 * @param out_port For OFPFC_DELETE* commands, require matching entries to include this as an output port. A value of OFPP_NONE indicates no restriction
	 * @param flags One of OFPFF_*
	 * @param acvec Vector of cofaction instances
	 */
#if 0
	virtual void
	send_group_mod_message(
		cofdpath *sw,
		uint16_t command,
		uint8_t type,
		uint32_t group_id,
		std::vector<cofbucket*>& buckets);
#endif

	virtual void
	send_group_mod_message(
			cgroupentry& groupentry);

	// TABLE-MOD message
	//

	/** Send OF GROUP-MOD.message to data path entity.
	 *
	 * All values in host byte order.
	 *
	 * @param sw cofswitch instance representing the data path to be addressed
	 * @param table_id the table id according to OF1.1
	 * @param config table configuration, see openflow.h for OFPTC_TABLE_*
	 */
	virtual void
	send_table_mod_message(
		uint8_t table_id,
		uint32_t config);

	// PORT-MOD message
	//

	/** Send OF PORT-MOD.message to data path entity.
	 *
	 * All values in host byte order.
	 *
	 * @param sw cofswitch instance representing the data path to be addressed
	 * @param ofmatch An instance of class cofmatch holding the matching fields
	 * @param port_no Number of port to modify
	 * @param hw_addr Memory area containing the ports hardware address
	 * @param hw_addr_len Length of memory area
	 * @param config Bitmap of OFPPC_* flags
	 * @param mask Bitmap of OFPPC_* flags to be changed
	 * @param advertise Bitmap of "ofp_port_features"s. Zero all bits to prevent any action taking place.
	 */
	virtual void
	send_port_mod_message(
		uint32_t port_no,
		cmacaddr const& hwaddr,
		uint32_t config,
		uint32_t mask,
		uint32_t advertise) throw (eCtlBaseNotFound);









protected:


	/* methods from cfwdelem overwritten by ctlbase
	 * all remaining methods are overwritten in derived controller classes
	 * cethctl, cipctl, etc.
	 *
	 * only "up" messages are handled in ctlbase
	 */


	/** Handle OF stats reply. To be overwritten by derived class.
	 *
	 * Called upon reception of a STATS.reply from a datapath entity.
	 * The OF packet must be removed from heap by the overwritten method.
	 *
	 * @param sw cofswitch instance from whom a STATS.reply was received
	 * @param pack STATS.reply packet received from datapath
	 */
	virtual void
	handle_stats_reply(cofdpath *sw, cofpacket *pack);


	/** Handle OF packet-in messages. To be overwritten by derived class.
	 *
	 * Called upon reception of a PACKET-IN.message from a datapath entity.
	 * The OF packet must be removed from heap by the overwritten method.
	 *
	 * @param sw cofswitch instance from whom a PACKET-IN.message was received
	 * @param pack PACKET-IN.message packet received from datapath
	 */
	virtual void
	handle_packet_in(cofdpath *sw, cofpacket *pack);


	/** Handle OF error message. To be overwritten by derived class.
	 *
	 * Called upon reception of an ERROR.message from a datapath entity.
	 * The OF packet must be removed from heap by the overwritten method.
	 *
	 * @param sw cofswitch instance from whom an ERROR.message was received
	 * @param pack ERROR.message packet received from datapath
	 */
	virtual void
	handle_error(cofdpath *sw, cofpacket *pack);


	/** Handle OF port-status message. To be overwritten by derived class.
	 *
	 * Called upon reception of a PORT-STATUS.message from a datapath entity.
	 * The OF packet must be removed from heap by the overwritten method.
	 *
	 * @param sw cofswitch instance from whom a PORT-STATUS.message was received
	 * @param pack PORT-STATUS.message packet received from datapath
	 */
	virtual void
	handle_port_status(cofdpath *sw, cofpacket *pack, cofport *port);


	/** Handle new dpath
	 *
	 * Called upon creation of a new cofswitch instance.
	 *
	 * @param sw new cofswitch instance
	 */
	virtual void
	handle_dpath_open(cofdpath *sw);


	/** Handle close event on dpath
	 *
	 * Called upon deletion of a cofswitch instance
	 *
	 * @param sw cofswitch instance to be deleted
	 */
	virtual void
	handle_dpath_close(cofdpath *sw);










public:


	/*
	 *  methods implemented as defined by cadapt_owner interface
	 */


	/**
	 *
	 */
	virtual uint32_t
	get_free_portno()
		throw (eAdaptNotFound);


public: // flowspace related methods


	/**
	 */
	virtual void
	flowspace_open(
			cadapt *adapt,
			cofmatch const& m = cofmatch() /* all wildcard */) throw (eCtlBaseNotConnected);


	/**
	 */
	virtual void
	flowspace_close(
			cadapt *adapt,
			cofmatch const& m = cofmatch() /* all wildcard */);



public: // methods offered to cadapt instances by cadapt_owner

	/*
	 * upwards methods
	 */

	/**
	 */
	virtual void
	send_port_status(
			cadapt *adapt,
			uint8_t reason,
			cofport *ofport);



	/**
	 */
	virtual void
	send_packet_in(
			cadapt *adapt,
			uint32_t buffer_id,
			uint16_t total_len,
			uint8_t table_id,
			uint8_t reason,
			cofmatch& match,
			fframe& frame);




public: // methods offered to cadapt instances by cadapt_owner

	/*
	 * downwards methods
	 */

	/**
	 */
	virtual void
	send_packet_out(
			cadapt *adapt,
			uint32_t buffer_id,
			uint32_t in_port,
			cofaclist& aclist,
			cpacket *pack = 0);


};




#endif


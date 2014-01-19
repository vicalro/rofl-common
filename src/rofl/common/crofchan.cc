/*
 * crofchan.cc
 *
 *  Created on: 31.12.2013
 *      Author: andreas
 */

#include "crofchan.h"

using namespace rofl;
using namespace rofl::openflow;


crofchan::crofchan(
		crofchan_env *env,
		cofhello_elem_versionbitmap const& versionbitmap) :
				env(env),
				versionbitmap(versionbitmap),
				ofp_version(OFP_VERSION_UNKNOWN),
				state(STATE_DISCONNECTED)
{

}



crofchan::~crofchan()
{
	clear();
}



void
crofchan::run_engine(enum crofchan_event_t event)
{
	if (EVENT_NONE != event) {
		events.push_back(event);
	}

	while (not events.empty()) {
		enum crofchan_event_t event = events.front();
		events.pop_front();

		switch (event) {
		case EVENT_DISCONNECTED:	event_disconnected();	return; // might call this object's destructor
		case EVENT_ESTABLISHED: 	event_established(); 	break;
		default: {
			logging::error << "[rofl][chan] unknown event seen, internal error" << std::endl << *this;
		};
		}
	}
}


void
crofchan::event_disconnected()
{
	switch (state) {
	case STATE_DISCONNECTED:
	case STATE_CONNECT_PENDING:
	case STATE_ESTABLISHED: {
		state = STATE_DISCONNECTED;
		env->handle_disconnected(this);

	} break;
	default: {
		logging::error << "[rofl][chan] event -DISCONNECTED- invalid state reached, internal error" << std::endl << *this;
	};
	}
}


void
crofchan::event_established()
{
	switch (state) {
	case STATE_DISCONNECTED:
	case STATE_CONNECT_PENDING:
	case STATE_ESTABLISHED: {
		state = STATE_ESTABLISHED;
		env->handle_established(this);

	} break;
	default: {
		logging::error << "[rofl][chan] event -ESTABLISHED- invalid state reached, internal error" << std::endl << *this;
	};
	}
}


bool
crofchan::is_established() const
{
	if (conns.empty())
		return false;
	// this channel is up, when its main connection is up
	return conns.at(0)->is_established();
}



void
crofchan::clear()
{
	while (not conns.empty()) {
		std::map<uint8_t, crofconn*>::reverse_iterator it = conns.rbegin();
		crofconn *conn = it->second;
		conns.erase(it->first);
		delete conn;
	}
}



void
crofchan::add_conn(
		uint8_t aux_id,
		int domain,
		int type,
		int protocol,
		caddress const& ra)
{
	if (conns.find(aux_id) != conns.end()) {
		throw eRofChanExists();
	}
	if (conns.empty() && (0 != aux_id)) {
		logging::error << "[rofl][chan] first connection must have aux-id:0, found " << (int)aux_id << " instead." << std::endl << *this;
		throw eRofChanInval();
	}

	if ((aux_id > 0) && (ofp_version < rofl::openflow13::OFP_VERSION)) {
		logging::error << "[rofl][chan] no auxiliary connections allowed in OFP version: " << ofp_version << std::endl << *this;
		throw eRofChanInval();
	}

	cofhello_elem_versionbitmap vbitmap;
	if (0 == aux_id) {
		vbitmap = versionbitmap;				// main connection: propose all OFP versions defined for our side
	} else {
		vbitmap.add_ofp_version(ofp_version);	// auxiliary connections: use OFP version negotiated for main connection
	}

	(conns[aux_id] = new crofconn(this, vbitmap))->connect(aux_id, domain, type, protocol, ra);
}


void
crofchan::add_conn(
		crofconn* conn,
		uint8_t aux_id)
{
	if (conns.empty() && (0 != aux_id)) {
		logging::error << "[rofl][chan] first connection must have aux-id:0, found " << (int)aux_id << " instead." << std::endl << *this;
		throw eRofChanInval();
	}

	if ((aux_id > 0) && (ofp_version < rofl::openflow13::OFP_VERSION)) {
		logging::error << "[rofl][chan] no auxiliary connections allowed in OFP version: " << ofp_version << std::endl << *this;
		throw eRofChanInval();
	}



	if (conns.find(aux_id) != conns.end()) {
		drop_conn(aux_id); // drop old connection first
	}
	conns[aux_id] = conn;
	conns[aux_id]->set_env(this);
	handle_connected(conns[aux_id], conns[aux_id]->get_version());
}



crofconn&
crofchan::get_conn(
		uint8_t aux_id)
{
	if (conns.find(aux_id) == conns.end()) {
		throw eRofChanNotFound();
	}
	return *(conns[aux_id]);
}



void
crofchan::drop_conn(
		uint8_t aux_id)
{
	if ((0 != aux_id) && (conns.size() > 1)) {
		logging::warn << "[rofl][chan] dropping main connection, while still auxiliary connections active." << std::endl << *this;
		while (conns.size() > 1) {
			std::map<uint8_t, crofconn*>::reverse_iterator it = conns.rbegin();
			if (0 == it->first)
				continue;
			else
				drop_conn(it->first);
		}
	}

	if (conns.find(aux_id) == conns.end()) {
		return;
	}
	crofconn *conn = conns[aux_id]; // temporary pointer
	conns.erase(aux_id); // remove pointer from map conns
	delete conn; // call destructor => this will lead to a call to crofchan::handle_closed(conn);

	if (conns.empty()) {
		run_engine(EVENT_DISCONNECTED);
	}
}



void
crofchan::handle_connect_refused(
		crofconn *conn)
{
	logging::warn << "[rofl][chan] connection refused." << std::endl << *conn;

	uint8_t aux_id = conn->get_aux_id();

	drop_conn(aux_id);
}



void
crofchan::handle_connected(
		crofconn *conn,
		uint8_t ofp_version)
{
	uint8_t aux_id = conn->get_aux_id();

	if (0 == aux_id) {
		this->ofp_version = ofp_version;
		logging::info << "[rofl][chan] main connection established. Negotiated OFP version:"
				<< (int) ofp_version << std::endl << *conn;
		run_engine(EVENT_ESTABLISHED);

	} else {
		if (this->ofp_version != ofp_version) {
			logging::warn << "[rofl][chan] auxiliary connection with invalid OFP version "
					<< "negotiated, closing connection." << std::endl << *conn;

			drop_conn(aux_id);
		} else {
			logging::debug << "[rofl][chan] auxiliary connection with aux-id:" << (int)aux_id
					<< " established." << std::endl << *conn;
		}
	}
}



void
crofchan::handle_closed(crofconn *conn)
{
	// do nothing upon reception of this closing notification, when there is no entry in map conns
	if (conns.find(conn->get_aux_id()) == conns.end()) {
		logging::warn << "[rofl][chan] internal error: unknown connection closed." << std::endl;
		return;
	}

	/*
	 * main connection
	 */
	if (0 == conn->get_aux_id()) {

		ofp_version = OFP_VERSION_UNKNOWN;
		run_engine(EVENT_DISCONNECTED);

		/*
		 * passive connection (=controller) => drop all connections
		 */
		if (not conn->is_actively_established()) {
			logging::info << "[rofl][chan] passive main connection closed." << std::endl;

			// close all connections
			while (not conns.empty()) {
				uint8_t aux_id = conns.begin()->first;
				delete conns[aux_id];
				conns.erase(aux_id);
			}

		/*
		 * active connection (=datapath) => close all connections and reconnect them
		 */
		} else {
			logging::info << "[rofl][chan] active main connection closed." << std::endl;

restart:
			// remove all passive connections (there should be none, though ...)
			for (std::map<uint8_t, crofconn*>::iterator
					it = conns.begin(); it != conns.end(); ++it) {
				if (not it->second->is_actively_established()) {
					delete it->second;
					conns.erase(it->first);
					goto restart;
				}
			}

			// closing and reconnecting the active connections
			for (std::map<uint8_t, crofconn*>::iterator
					it = conns.begin(); it != conns.end(); ++it) {

				it->second->close();
				it->second->reconnect();
			}
		}

		return;


	/*
	 * auxialiary connection
	 */
	} else {

		/*
		 * passive connection
		 */
		if (not conn->is_actively_established()) {

			logging::info << "[rofl][chan] passive auxiliary connection closed:"
					<< (int)conn->get_aux_id() << std::endl;

			conns.erase(conn->get_aux_id());
			delete conn;
			return;

		/*
		 * active connection (=datapath) => reconnect
		 */
		} else {
			logging::info << "[rofl][chan] active auxiliary connection closed:"
					<< (int)conn->get_aux_id() << std::endl;

			conn->reconnect();
			return;
		}

	}
}



void
crofchan::recv_message(crofconn *conn, cofmsg *msg)
{
	env->recv_message(this, conn->get_aux_id(), msg);
}



void
crofchan::send_message(
		cofmsg *msg,
		uint8_t aux_id)
{
	if (conns.find(aux_id) == conns.end()) {
		throw eRofChanNotFound();
	}
	conns[aux_id]->send_message(msg);
}



uint32_t
crofchan::get_async_xid(crofconn *conn)
{
	return env->get_async_xid(this);
}

uint32_t
crofchan::get_sync_xid(crofconn *conn, uint8_t msg_type, uint16_t msg_sub_type)
{
	return env->get_sync_xid(this, msg_type, msg_sub_type);
}

void
crofchan::release_sync_xid(crofconn *conn, uint32_t xid)
{
	env->release_sync_xid(this, xid);
}



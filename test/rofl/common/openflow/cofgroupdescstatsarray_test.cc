#include <stdlib.h>

#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>

#include "cofgroupdescstatsarray_test.h"


CPPUNIT_TEST_SUITE_REGISTRATION( cofgroupdescstatsarray_test );

#if defined DEBUG
//#undef DEBUG
#endif

void
cofgroupdescstatsarray_test::setUp()
{
}



void
cofgroupdescstatsarray_test::tearDown()
{
}



void
cofgroupdescstatsarray_test::testDefaultConstructor()
{
	rofl::openflow::cofgroupdescstatsarray array;
	CPPUNIT_ASSERT(rofl::openflow::OFP_VERSION_UNKNOWN == array.get_version());
}



void
cofgroupdescstatsarray_test::testCopyConstructor()
{
	rofl::openflow::cofgroupdescstatsarray array(rofl::openflow13::OFP_VERSION);

	array.set_group_desc_stats(0).set_group_id(0);
	array.set_group_desc_stats(0).set_group_type(rofl::openflow13::OFPGT_ALL);
	array.set_group_desc_stats(0).set_buckets().set_bucket(0).set_packet_count(0xa1a2a3a4a5a6a7a8);
	array.set_group_desc_stats(0).set_buckets().set_bucket(0).set_byte_count(0xb1b2b3b4b5b6b7b8);
	array.set_group_desc_stats(0).set_buckets().set_bucket(0).set_watch_port(0xc3c3c4c4);
	array.set_group_desc_stats(0).set_buckets().set_bucket(0).set_watch_group(0xd3d3d4d4);
	array.set_group_desc_stats(0).set_buckets().set_bucket(1).set_byte_count(0xa1a2a3a4a5a6a7a8);
	array.set_group_desc_stats(0).set_buckets().set_bucket(1).set_byte_count(0xb1b2b3b4b5b6b7b8);
	array.set_group_desc_stats(0).set_buckets().set_bucket(1).set_watch_port(0xc3c3c4c4);
	array.set_group_desc_stats(0).set_buckets().set_bucket(1).set_watch_group(0xd3d3d4d4);
	array.set_group_desc_stats(0).set_buckets().set_bucket(2).set_byte_count(0xa1a2a3a4a5a6a7a8);
	array.set_group_desc_stats(0).set_buckets().set_bucket(2).set_byte_count(0xb1b2b3b4b5b6b7b8);
	array.set_group_desc_stats(0).set_buckets().set_bucket(2).set_watch_port(0xc3c3c4c4);
	array.set_group_desc_stats(0).set_buckets().set_bucket(2).set_watch_group(0xd3d3d4d4);

	array.set_group_desc_stats(1).set_group_id(0);
	array.set_group_desc_stats(1).set_group_type(rofl::openflow13::OFPGT_ALL);
	array.set_group_desc_stats(1).set_buckets().set_bucket(0).set_packet_count(0xa1a2a3a4a5a6a7a8);
	array.set_group_desc_stats(1).set_buckets().set_bucket(0).set_byte_count(0xb1b2b3b4b5b6b7b8);
	array.set_group_desc_stats(1).set_buckets().set_bucket(0).set_watch_port(0xc3c3c4c4);
	array.set_group_desc_stats(1).set_buckets().set_bucket(0).set_watch_group(0xd3d3d4d4);
	array.set_group_desc_stats(1).set_buckets().set_bucket(1).set_byte_count(0xa1a2a3a4a5a6a7a8);
	array.set_group_desc_stats(1).set_buckets().set_bucket(1).set_byte_count(0xb1b2b3b4b5b6b7b8);
	array.set_group_desc_stats(1).set_buckets().set_bucket(1).set_watch_port(0xc3c3c4c4);
	array.set_group_desc_stats(1).set_buckets().set_bucket(1).set_watch_group(0xd3d3d4d4);
	array.set_group_desc_stats(1).set_buckets().set_bucket(2).set_byte_count(0xa1a2a3a4a5a6a7a8);
	array.set_group_desc_stats(1).set_buckets().set_bucket(2).set_byte_count(0xb1b2b3b4b5b6b7b8);
	array.set_group_desc_stats(1).set_buckets().set_bucket(2).set_watch_port(0xc3c3c4c4);
	array.set_group_desc_stats(1).set_buckets().set_bucket(2).set_watch_group(0xd3d3d4d4);

	rofl::cmemory marray(array.length());
	array.pack(marray.somem(), marray.memlen());

#ifdef DEBUG
	std::cerr << "array:" << std::endl << array;
	std::cerr << "marray:" << std::endl << marray;
#endif

	rofl::openflow::cofgroupdescstatsarray clone(array);

	rofl::cmemory mclone(clone.length());
	clone.pack(mclone.somem(), mclone.memlen());

#ifdef DEBUG
	std::cerr << "clone:" << std::endl << clone;
	std::cerr << "mclone:" << std::endl << mclone;
#endif

	CPPUNIT_ASSERT(marray == mclone);
}



void
cofgroupdescstatsarray_test::testOperatorPlus()
{
#if 0
	std::vector<rofl::openflow::cofgroupdescstatsarray> array;
	array.push_back(rofl::openflow::cofgroupdescstatsarray(rofl::openflow13::OFP_VERSION));
	array.push_back(rofl::openflow::cofgroupdescstatsarray(rofl::openflow13::OFP_VERSION));

	array[0].set_flow_stats(0).set_version(rofl::openflow13::OFP_VERSION);
	array[0].set_flow_stats(0).set_table_id(1);
	array[0].set_flow_stats(0).set_byte_count(0xa1a2);
	array[0].set_flow_stats(0).set_packet_count(0xb1b2);
	array[0].set_flow_stats(0).set_duration_sec(0xc1c2);
	array[0].set_flow_stats(0).set_duration_nsec(0xd1d2);
	array[0].set_flow_stats(0).set_idle_timeout(0x10);
	array[0].set_flow_stats(0).set_hard_timeout(0x20);
	array[0].set_flow_stats(0).set_match().set_arp_opcode(0x5555);
	array[0].set_flow_stats(0).set_match().set_eth_dst(rofl::cmacaddr("11:22:33:44:55:66"));
	array[0].set_flow_stats(0).set_match().set_eth_src(rofl::cmacaddr("22:22:22:22:22:22"));
	array[0].set_flow_stats(0).set_match().set_eth_type(0x3333);
	array[0].set_flow_stats(0).set_instructions().set_inst_apply_actions().actions.append_action_copy_ttl_in();
	array[0].set_flow_stats(0).set_instructions().set_inst_apply_actions().actions.append_action_output(6);

	array[0].set_flow_stats(1).set_version(rofl::openflow13::OFP_VERSION);
	array[0].set_flow_stats(1).set_table_id(2);
	array[0].set_flow_stats(1).set_byte_count(0xa3a4);
	array[0].set_flow_stats(1).set_packet_count(0xb3b4);
	array[0].set_flow_stats(1).set_duration_sec(0xc3c4);
	array[0].set_flow_stats(1).set_duration_nsec(0xd3d4);
	array[0].set_flow_stats(1).set_idle_timeout(0x30);
	array[0].set_flow_stats(1).set_hard_timeout(0x40);
	array[0].set_flow_stats(1).set_match().set_arp_opcode(0x8888);
	array[0].set_flow_stats(1).set_match().set_eth_dst(rofl::cmacaddr("44:44:44:44:44:44"));

	array[1].set_flow_stats(0).set_version(rofl::openflow13::OFP_VERSION);
	array[1].set_flow_stats(0).set_table_id(4);
	array[1].set_flow_stats(0).set_byte_count(0xa1a2);
	array[1].set_flow_stats(0).set_packet_count(0xb1b2);
	array[1].set_flow_stats(0).set_duration_sec(0xc1c2);
	array[1].set_flow_stats(0).set_duration_nsec(0xd1d2);
	array[1].set_flow_stats(0).set_idle_timeout(0x10);
	array[1].set_flow_stats(0).set_hard_timeout(0x20);
	array[1].set_flow_stats(0).set_match().set_arp_opcode(0x9999);
	array[1].set_flow_stats(0).set_match().set_eth_dst(rofl::cmacaddr("11:22:33:44:55:66"));
	array[1].set_flow_stats(0).set_match().set_eth_src(rofl::cmacaddr("22:22:22:22:22:22"));
	array[1].set_flow_stats(0).set_match().set_eth_type(0x7777);
	array[1].set_flow_stats(0).set_instructions().set_inst_apply_actions().actions.append_action_copy_ttl_in();
	array[1].set_flow_stats(0).set_instructions().set_inst_apply_actions().actions.append_action_output(6);

#ifdef DEBUG
	std::cerr << "array[0]:" << std::endl << array[0];
	std::cerr << "array[1]:" << std::endl << array[1];
#endif

	rofl::openflow::cofgroupdescstatsarray merge(array[0]);
	merge += array[1];

#ifdef DEBUG
	std::cerr << "array[0]+array[1]:" << std::endl << merge;
	std::cerr << "merge.set_flow_stats(0) == array[0].get_flow_stats(0): " << (merge.set_flow_stats(0) == array[0].get_flow_stats(0)) << std::endl;
	std::cerr << "merge.set_flow_stats(1) == array[0].get_flow_stats(1): " << (merge.set_flow_stats(1) == array[0].get_flow_stats(1)) << std::endl;
	std::cerr << "merge.set_flow_stats(2) == array[1].get_flow_stats(0): " << (merge.set_flow_stats(2) == array[1].get_flow_stats(0)) << std::endl;
#endif

	CPPUNIT_ASSERT(merge.set_flow_stats(0) == array[0].get_flow_stats(0));
	CPPUNIT_ASSERT(merge.set_flow_stats(1) == array[0].get_flow_stats(1));
	CPPUNIT_ASSERT(merge.set_flow_stats(2) == array[1].get_flow_stats(0));
#endif
}



void
cofgroupdescstatsarray_test::testPackUnpack()
{
	/*
	 * test memory
	 */
	std::vector<rofl::openflow::cofgroup_desc_stats_reply> stats;
	stats.push_back(rofl::openflow::cofgroup_desc_stats_reply(rofl::openflow13::OFP_VERSION));
	stats.push_back(rofl::openflow::cofgroup_desc_stats_reply(rofl::openflow13::OFP_VERSION));
	stats.push_back(rofl::openflow::cofgroup_desc_stats_reply(rofl::openflow13::OFP_VERSION));

	stats[0].set_version(rofl::openflow13::OFP_VERSION);
	stats[0].set_group_id(0);
	stats[0].set_group_type(rofl::openflow13::OFPGT_ALL);
	stats[0].set_buckets().set_bucket(0).set_packet_count(0x0a);
	stats[0].set_buckets().set_bucket(0).set_byte_count(0x88);
	stats[0].set_buckets().set_bucket(0).set_watch_port(0xc3c3c4c4);
	stats[0].set_buckets().set_bucket(0).set_watch_group(0xd3d3d4d4);

	stats[1].set_version(rofl::openflow13::OFP_VERSION);
	stats[1].set_group_id(0);
	stats[1].set_group_type(rofl::openflow13::OFPGT_ALL);
	stats[1].set_buckets().set_bucket(0).set_packet_count(0x0a);
	stats[1].set_buckets().set_bucket(0).set_byte_count(0x88);
	stats[1].set_buckets().set_bucket(0).set_watch_port(0xc3c3c4c4);
	stats[1].set_buckets().set_bucket(0).set_watch_group(0xd3d3d4d4);
	stats[1].set_buckets().set_bucket(0).set_actions().append_action_dec_mpls_ttl();

	stats[2].set_version(rofl::openflow13::OFP_VERSION);
	stats[2].set_group_id(0);
	stats[2].set_group_type(rofl::openflow13::OFPGT_ALL);
	stats[2].set_buckets().set_bucket(0).set_packet_count(0x0a);
	stats[2].set_buckets().set_bucket(0).set_byte_count(0x88);
	stats[2].set_buckets().set_bucket(0).set_watch_port(0xc3c3c4c4);
	stats[2].set_buckets().set_bucket(0).set_watch_group(0xd3d3d4d4);
	stats[2].set_buckets().set_bucket(0).set_actions().append_action_dec_mpls_ttl();
	stats[2].set_buckets().set_bucket(1).set_packet_count(0x1a);
	stats[2].set_buckets().set_bucket(1).set_byte_count(0x99);
	stats[2].set_buckets().set_bucket(1).set_watch_port(0xe3e3e4e4);
	stats[2].set_buckets().set_bucket(1).set_watch_group(0xf3f3f4f4);
	stats[2].set_buckets().set_bucket(1).set_actions().append_action_group(0xee);


	size_t len = 0;
	for (unsigned int i = 0; i < 3; i++) {
		len += stats[i].length();
	}
	rofl::cmemory mem(len);

	uint8_t *buf = mem.somem();
	for (unsigned int i = 0; i < 3; i++) {
		std::cerr << "stats[" << i << "]=" << stats[i] << std::endl;
		std::cerr << "stats[" << i << "].length()=" << (int)stats[i].length() << std::endl;

		stats[i].pack(buf, stats[i].length());
		buf += stats[i].length();
	}

	/*
	 * array under test
	 */
	rofl::openflow::cofgroupdescstatsarray array(rofl::openflow13::OFP_VERSION);

	for (unsigned int i = 0; i < 3; i++) {
		array.set_group_desc_stats(i) = stats[i];
	}

	rofl::cmemory marray(array.length());
	array.pack(marray.somem(), marray.memlen());

#ifdef DEBUG
	std::cerr << "array:" << std::endl << array;
	std::cerr << "marray:" << std::endl << marray;
	std::cerr << "mem:" << std::endl << mem;
#endif

	CPPUNIT_ASSERT(marray == mem);

	rofl::openflow::cofgroupdescstatsarray clone(rofl::openflow13::OFP_VERSION);
	clone.unpack(marray.somem(), marray.memlen());
#ifdef DEBUG
	std::cerr << "clone:" << std::endl << clone;
#endif
}



void
cofgroupdescstatsarray_test::testAddDropSetGetHas()
{
	rofl::openflow::cofgroupdescstatsarray array(rofl::openflow13::OFP_VERSION);

	try {
		array.get_group_desc_stats(0);
		CPPUNIT_ASSERT(false);
	} catch (rofl::openflow::eGroupDescStatsNotFound& e) {};

	if (array.has_group_desc_stats(0)) {
		CPPUNIT_ASSERT(false);
	}
	array.add_group_desc_stats(0);

	try {
		array.get_group_desc_stats(0);
	} catch (rofl::openflow::eGroupDescStatsNotFound& e) {
		CPPUNIT_ASSERT(false);
	}

	try {
		array.set_group_desc_stats(0);
	} catch (rofl::openflow::eGroupDescStatsNotFound& e) {
		CPPUNIT_ASSERT(false);
	}

	if (not array.has_group_desc_stats(0)) {
		CPPUNIT_ASSERT(false);
	}

	array.drop_group_desc_stats(0);

	try {
		array.get_group_desc_stats(0);
		CPPUNIT_ASSERT(false);
	} catch (rofl::openflow::eGroupDescStatsNotFound& e) {};

	if (array.has_group_desc_stats(0)) {
		CPPUNIT_ASSERT(false);
	}
	array.add_group_desc_stats(0);
}


#include <cppunit/TestAssert.h>
#include "testFetch.hh"
#include "PnSReadNode1.hh"
#include "PnSReadNode.hh"
#include <libmediation/ExecGraph/ExecGraphNode.hpp>
#include <libmediation/ObjectPools/ObjFactory.hpp>
#include <libmediation/ObjectPools/ObjPool.hpp>
#include <libmediation/ObjectPools/SyncObjPool.hpp>
#include <libmediation/HTTPClient/HTTPMgr.hpp>
#include <libmediation/HTTPClient/HTTPClient.hpp>
#include <libmediation/HTTPAccess/HTTPReqMgr2.hpp>
#include <libmediation/ObjectPools/AbstractPool.hpp>
#include <advise-common/BackendConf.hh>
#include <advise-common/OfferDynConf.hh>
#include <advise-common/PnSFieldsConfig.hh>
#include <advise-common/PnSFieldsConfigParser.hh>
#include <httpd.h>
#include <libmediation/Common/ReqCtx.hpp>
#include <libmediation/Apache/ReqCtx.hpp>
#include <apr.h>
#include <apr_pools.h>
#include <apr_tables.h>
#include "PnSUserData.hh"
#include <libmediation/PnS3Parser/PnS3Credential.hpp>
#include <libmediation/PnS3Parser/PnS3CredentialType.hpp>
#include <libmediation/PnS3Parser/PnS3CredTypesConstants.hpp>
#include <libmediation/PnS3Parser/PnS3FieldVisitor.hpp>
#include <advise-auth/CredentialType.hpp>
#include <advise-auth/Credential.hpp>
#include <advise-auth/CredentialSet.hpp>
#include <advise-auth/ContractType.hpp>
#include <advise-auth/IdentityType.hpp>
#include <advise-auth/Identity.hpp>
#include "RequestedArgs.hh"
#include <advise-common/PnSFieldNames.hh>
#include <advise-common/QoSMgr.hh>
#include "PnSRequests.hh"
#include <advise-common/Transaction.hh>
#include <memory>
#include <libmediation/Graph/Graph.hpp>
#include <libmediation/ExecGraph/GraphExecutor.hpp>
#include <libmediation/Common/visibility.hpp>
#include <apr_strings.h>
#include <apr_network_io.h>
#include <arpa/inet.h>
#include <advise-common/ShmSeg.hh>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/creation_tags.hpp>
#include <cstdio>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <advise-auth/Contract.hpp>
#include <advise-common/OAuthReq.hh>
#include "PnSParser.hh"
#include "PnSNode.hh"

using GP::ExecGraphNode;
using GP::ObjFactory;
using GP::ObjPool;
using GP::SyncObjPool;
using GP::HTTPMgr;
using GP::AbstractPool;
using AdvC::BackendConf;
using AdvC::OfferDynConfigs;
using AdvC::PnSFieldsConfig;
using AdvC::PnSFieldsConfigParser;
using OfferMediation::PnSUserData;
using GP::PnS3Credential;
using GP::PnS3CredentialType;
using GP::PnS3CredTypesConstants;
using AdvAuth::IdentityType;
using AdvAuth::Identity;
using AdvAuth::Credential;
using AdvAuth::CredentialSet;
using AdvAuth::Contract;
using OfferMediation::RequestedData;
using AdvC::PnSFieldNames;
using AdvC::QoSMgr;
using OfferMediation::PnSReqs;
using AdvC::Transaction;
using GP::GraphExecutor;
using OfferMediation::PnSReadNode;
using AdvC::OwnedNode;
using AdvC::map_shm;
using AdvC::shm_deleter;
using boost::interprocess::fixed_managed_shared_memory;
using GP::HTTPClient;
using GP::PnS3FieldVisitor;
using AdvC::OAuthReq;
using AdvC::Source;
using AdvC::BackendData;
using AdvC::HTTPAuthdBckData;
using GP::PnS3Data;
using AdvC::AuthdBckCallbacks;
using OfferMediation::PnSParser;
using OfferMediation::PnSBck;

using clock_type = std::chrono::system_clock;
using std::chrono::microseconds;
using std::chrono::seconds;

CPPUNIT_TEST_SUITE_REGISTRATION(TestFetch);

static void
do_fetch(ObjPool<HTTPMgr>& mgr_pool, AbstractPool& p,
	 const BackendConf& bck_cnf, const OfferDynConfigs& pns_confs,
	 request_rec& r, GP::ReqCtx& ctx, PnSUserData& ud1, PnSUserData& ud2,
	 PnSUserData& ud3, const Identity& id1, const Identity& id2,
	 const Identity& id3, const RequestedData& args);

static void
thr_main(ObjPool<HTTPMgr>& mgr_pool, AbstractPool& p,
	 const BackendConf& bck_cnf, const OfferDynConfigs& pns_confs,
	 const Identity& id1, const Identity& id2, const Identity& id3);

static void
iteration(ObjPool<HTTPMgr>& mgr_pool, AbstractPool& p,
	  const BackendConf& bck_cnf, const OfferDynConfigs& pns_confs,
	  request_rec& r, GP::ReqCtx& ctx, PnSUserData& ud1, PnSUserData& ud2,
	  PnSUserData& ud3, const Identity& id1, const Identity& id2,
	  const Identity& id3, const RequestedData& args);

static apr_pool_t *pool;
static bool running = false;
static std::mutex mtx;
static std::condition_variable cnd;

TestFetch::TestFetch() {
}

TestFetch::~TestFetch() {
}

void
TestFetch::setUp() {
	apr_initialize();
	apr_pool_create(&pool, nullptr);
}

void
TestFetch::tearDown() {
	apr_pool_destroy(pool);
	apr_terminate();
}

struct DSO_LOCAL HTTPMgrFct : public ObjFactory<HTTPMgr> {
	virtual HTTPMgr *get_object() const { return new HTTPMgr(); }
};

class DSO_LOCAL Node;
class Node : public OfferMediation::PnSReadNode1 {
	static const OAuthReq route;
public:
	Node(const GP::ExecGraphNode *egn__, GP::HTTPReqMgr2 *mgr__,
	     GP::AbstractPool& p__, const BackendConf& cnf__,
	     const std::string& pns_pred__, const OfferDynConfigs& confs__,
	     const request_rec *r__, GP::ReqCtx& ctx__,
	     GP::PnS3Credential&& cred__, unsigned int ndx__,
	     PnSUserData& data__, const AdvAuth::Identity *id__,
	     const std::string& tag__, const Contract& ct__,
	     const AdvAuth::Credential *c__, const RequestedData& args__,
	     PnSFieldNames& pns_fields_for_id__, const QoSMgr *qos_mgr__,
	     unsigned int clazz__, PnSReqs& pns_reqs__, Transaction *tx__)
		: OfferMediation::PnSReadNode1(
			nullptr, data__, cnf__, egn__, *mgr__, p__, r__, ctx__,
			args__, 1, clazz__, qos_mgr__, route, nullptr, tx__,
			*c__, pns_reqs__, Source(ndx__, 0), pns_fields_for_id__,
			*id__, tag__, ct__, confs__, pns_pred__)
	{}
	OwnedNode *get_owner() const noexcept { return owner; }
	HTTPAuthdBckData<BackendData<PnS3Data>>& get_http_data() {
		return http_bck;
	}
};

const OAuthReq Node::route;

class DSO_LOCAL Mgr;
class Mgr : public GP::HTTPReqMgr2 {
public:
	Mgr(ObjPool<HTTPMgr> *pool__) noexcept
		: GP::HTTPReqMgr2(pool__) {}
	void eos() {
		if (mgr_)
			mgr_->eos();
		GP::HTTPReqMgr2::eos();
	}
};

void
TestFetch::fetch() {
	std::shared_ptr<fixed_managed_shared_memory> mem(
		new fixed_managed_shared_memory(
			boost::interprocess::create_only,
			map_shm::map_shm_name,
			0x04000000), shm_deleter());
	map_shm::shm_ = mem;
	HTTPMgrFct fct;
	ObjPool<HTTPMgr> mgr_pool(&fct, 1, 1);
	AbstractPool p(64);
	BackendConf bck_cnf;
	bck_cnf.set_url("http://pns.com/");
	bck_cnf.set_timeout(800lu);
	bck_cnf.set_connect_timeout(500lu);
	PnSFieldsConfig preds;
	PnSFieldsConfigParser parser(preds.fields_data());
	static const char pred_conf[] =
		"["
		"  ["
		"    {"
		"      \"field\": \"A\","
		"      \"preds\": {"
		"        \"2\": \"\\\"msisdn\\\" == CRED_TYPE\""
		"      }"
		"    },"
		"    {"
		"      \"field\": \"B\","
		"      \"preds\": {"
		"        \"2\": \"\\\"msisdn\\\" == CRED_TYPE || \\\"iacct\\\" == CRED_TYPE\""
		"      }"
		"    },"
		"    {"
		"      \"field\": \"C\","
		"      \"preds\": {"
		"        \"2\": \"\\\"iacct\\\" == CRED_TYPE\""
		"      }"
		"    },"
		"    {"
		"      \"field\": \"D\","
		"      \"preds\": {"
		"        \"2\": \"\\\"msisdn\\\" == CRED_TYPE\""
		"      }"
		"    },"
		"    {"
		"      \"field\": \"Dopt\","
		"      \"options\": {\"limit\": 2},"
		"      \"preds\": {"
		"        \"2\": \"\\\"msisdn\\\" == CRED_TYPE\""
		"      }"
		"    },"
		"    {"
		"      \"field\": \"E\","
		"      \"preds\": {"
		"        \"2\": \"\\\"msisdn\\\" == CRED_TYPE || \\\"iacct\\\" == CRED_TYPE\""
		"      }"
		"    },"
		"    {"
		"      \"field\": \"Eopt\","
		"      \"options\": {\"limit\": 2},"
		"      \"preds\": {"
		"        \"2\": \"\\\"msisdn\\\" == CRED_TYPE || \\\"iacct\\\" == CRED_TYPE\""
		"      }"
		"    },"
		"    {"
		"      \"field\": \"F\","
		"      \"preds\": {"
		"        \"2\": \"\\\"iacct\\\" == CRED_TYPE\""
		"      }"
		"    },"
		"    {"
		"      \"field\": \"Fopt\","
		"      \"options\": {\"limit\": 2},"
		"      \"preds\": {"
		"        \"2\": \"\\\"iacct\\\" == CRED_TYPE\""
		"      }"
		"    },"
		"    {"
		"      \"field\": \"Gopt\","
		"      \"options\": {\"limit\": 2},"
		"      \"preds\": {"
		"        \"2\": \"\\\"msisdn\\\" == CRED_TYPE\""
		"      }"
		"    },"
		"    {"
		"      \"field\": \"Hopt\","
		"      \"options\": {\"limit\": 2},"
		"      \"preds\": {"
		"        \"2\": \"\\\"msisdn\\\" == CRED_TYPE || \\\"iacct\\\" == CRED_TYPE\""
		"      }"
		"    },"
		"    {"
		"      \"field\": \"Iopt\","
		"      \"options\": {\"limit\": 2},"
		"      \"preds\": {"
		"        \"2\": \"\\\"iacct\\\" == CRED_TYPE\""
		"      }"
		"    }"
		"  ]"
		"]";
	parser.parse(pred_conf, sizeof(pred_conf) - 1);
	parser.parse(nullptr, 0);
	request_rec r;
	r.pool = pool;
	apr_sockaddr_t *remote_addr = reinterpret_cast<apr_sockaddr_t *>(
		apr_pcalloc(pool, sizeof(apr_sockaddr_t)));
	remote_addr->pool = pool;
	remote_addr->hostname = apr_pstrdup(pool, "localhost");
	remote_addr->servname = apr_pstrdup(pool, "80");
	remote_addr->port = 80;
	remote_addr->family = AF_INET;
	remote_addr->salen = sizeof(struct sockaddr_in); // size of sockaddr?
	remote_addr->ipaddr_len = sizeof(struct in_addr); // size of ip address structure
	remote_addr->addr_str_len = 16; // 16 for ipv4 and 46 for ipv6
	remote_addr->ipaddr_ptr = &remote_addr->sa.sin.sin_addr;
	remote_addr->next = 0;
	remote_addr->sa.sin.sin_family = AF_INET;
	remote_addr->sa.sin.sin_port = htons(80);
	remote_addr->sa.sin.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	r.useragent_addr = remote_addr;
	r.headers_in = apr_table_make(pool, 32);
	r.subprocess_env = apr_table_make(pool, 32);
	GP::Apache::ReqCtx ctx(r.subprocess_env);

	RequestedData args("canal=hpc&targets=BB[options]", r.headers_in, 3);
	OfferDynConfigs pns_confs{{"BB", &preds}};

	PnSUserData ud1;
	CredentialSet cs1{
		Credential(AdvAuth::CREDENTIAL_MSISDN, "33601234567"),
		Credential(AdvAuth::CREDENTIAL_IACCOUNT, "701234567")
	};
	Identity id1("alias1", Contract(IdentityType(AdvAuth::CI_TYPE), std::move(cs1)));

	PnSUserData ud2;
	CredentialSet cs2{
		Credential(AdvAuth::CREDENTIAL_MSISDN, "33601234567"),
		Credential(AdvAuth::CREDENTIAL_IACCOUNT, "701234567")
	};
	Identity id2("alias2", Contract(IdentityType(AdvAuth::CI_TYPE), std::move(cs2)));

	PnSUserData ud3;
	CredentialSet cs3{
		Credential(AdvAuth::CREDENTIAL_MSISDN, "33601234567"),
		Credential(AdvAuth::CREDENTIAL_IACCOUNT, "701234567")
	};
	Identity id3("alias3", Contract(IdentityType(AdvAuth::CM_TYPE), std::move(cs3)));

	args.parse_targets();

	do_fetch(mgr_pool, p, bck_cnf, pns_confs, r, ctx, ud1, ud2, ud3, id1,
		 id2, id3, args);


	PnS3FieldVisitor v;

	PnSUserData::const_iterator a1 = ud1.find("A");
	CPPUNIT_ASSERT(ud1.end() != a1);
	CPPUNIT_ASSERT(static_cast<bool>(NdxData(a1->second)));
	CPPUNIT_ASSERT_EQUAL(std::string("AdataM"),
			     NdxData(a1->second)->accept(v));

	PnSUserData::const_iterator b1 = ud1.find("B");
	CPPUNIT_ASSERT(ud1.end() != b1);
	CPPUNIT_ASSERT(static_cast<bool>(NdxData(b1->second)));
	CPPUNIT_ASSERT_EQUAL(std::string("BdataM"),
			     NdxData(b1->second)->accept(v));

	PnSUserData::const_iterator c1 = ud1.find("C");
	CPPUNIT_ASSERT(ud1.end() != c1);
	CPPUNIT_ASSERT(static_cast<bool>(NdxData(c1->second)));
	CPPUNIT_ASSERT_EQUAL(std::string("CdataI"),
			     NdxData(c1->second)->accept(v));

	PnSUserData::const_iterator d1 = ud1.find("D");
	CPPUNIT_ASSERT(ud1.end() != d1);
	CPPUNIT_ASSERT(static_cast<bool>(NdxData(d1->second)));
	CPPUNIT_ASSERT_EQUAL(std::string("DoptdataM"),
			     NdxData(d1->second)->accept(v));

	PnSUserData::const_iterator e1 = ud1.find("E");
	CPPUNIT_ASSERT(ud1.end() != e1);
	CPPUNIT_ASSERT(static_cast<bool>(NdxData(e1->second)));
	CPPUNIT_ASSERT_EQUAL(std::string("EoptdataM"),
			     NdxData(e1->second)->accept(v));

	PnSUserData::const_iterator f1 = ud1.find("F");
	CPPUNIT_ASSERT(ud1.end() != f1);
	CPPUNIT_ASSERT(static_cast<bool>(NdxData(f1->second)));
	CPPUNIT_ASSERT_EQUAL(std::string("FoptdataI"),
			     NdxData(f1->second)->accept(v));

	PnSUserData::const_iterator g1 = ud1.find("G");
	CPPUNIT_ASSERT(ud1.end() != g1);
	CPPUNIT_ASSERT(static_cast<bool>(NdxData(g1->second)));
	CPPUNIT_ASSERT_EQUAL(std::string("GoptdataM"),
			     NdxData(g1->second)->accept(v));

	PnSUserData::const_iterator h1 = ud1.find("H");
	CPPUNIT_ASSERT(ud1.end() != h1);
	CPPUNIT_ASSERT(static_cast<bool>(NdxData(h1->second)));
	CPPUNIT_ASSERT_EQUAL(std::string("HoptdataM"),
			     NdxData(h1->second)->accept(v));

	PnSUserData::const_iterator i1 = ud1.find("I");
	CPPUNIT_ASSERT(ud1.end() != i1);
	CPPUNIT_ASSERT(static_cast<bool>(NdxData(i1->second)));
	CPPUNIT_ASSERT_EQUAL(std::string("IoptdataI"),
			     NdxData(i1->second)->accept(v));

	///////////////////// second id

	PnSUserData::const_iterator a2 = ud2.find("A");
	CPPUNIT_ASSERT(ud2.end() != a2);
	CPPUNIT_ASSERT(static_cast<bool>(NdxData(a2->second)));
	CPPUNIT_ASSERT_EQUAL(std::string("AdataM"),
			     NdxData(a2->second)->accept(v));

	PnSUserData::const_iterator b2 = ud2.find("B");
	CPPUNIT_ASSERT(ud2.end() != b2);
	CPPUNIT_ASSERT(static_cast<bool>(NdxData(b2->second)));
	CPPUNIT_ASSERT_EQUAL(std::string("BdataM"),
			     NdxData(b2->second)->accept(v));

	PnSUserData::const_iterator c2 = ud2.find("C");
	CPPUNIT_ASSERT(ud2.end() != c2);
	CPPUNIT_ASSERT(static_cast<bool>(NdxData(c2->second)));
	CPPUNIT_ASSERT_EQUAL(std::string("CdataI"),
			     NdxData(c2->second)->accept(v));

	PnSUserData::const_iterator d2 = ud2.find("D");
	CPPUNIT_ASSERT(ud2.end() != d2);
	CPPUNIT_ASSERT(static_cast<bool>(NdxData(d2->second)));
	CPPUNIT_ASSERT_EQUAL(std::string("DoptdataM"),
			     NdxData(d2->second)->accept(v));

	PnSUserData::const_iterator e2 = ud2.find("E");
	CPPUNIT_ASSERT(ud2.end() != e2);
	CPPUNIT_ASSERT(static_cast<bool>(NdxData(e2->second)));
	CPPUNIT_ASSERT_EQUAL(std::string("EoptdataM"),
			     NdxData(e2->second)->accept(v));

	PnSUserData::const_iterator f2 = ud2.find("F");
	CPPUNIT_ASSERT(ud2.end() != f2);
	CPPUNIT_ASSERT(static_cast<bool>(NdxData(f2->second)));
	CPPUNIT_ASSERT_EQUAL(std::string("FoptdataI"),
			     NdxData(f2->second)->accept(v));

	PnSUserData::const_iterator g2 = ud2.find("G");
	CPPUNIT_ASSERT(ud2.end() != g2);
	CPPUNIT_ASSERT(static_cast<bool>(NdxData(g2->second)));
	CPPUNIT_ASSERT_EQUAL(std::string("GoptdataM"),
			     NdxData(g2->second)->accept(v));

	PnSUserData::const_iterator h2 = ud2.find("H");
	CPPUNIT_ASSERT(ud2.end() != h2);
	CPPUNIT_ASSERT(static_cast<bool>(NdxData(h2->second)));
	CPPUNIT_ASSERT_EQUAL(std::string("HoptdataM"),
			     NdxData(h2->second)->accept(v));

	PnSUserData::const_iterator i2 = ud2.find("I");
	CPPUNIT_ASSERT(ud2.end() != i2);
	CPPUNIT_ASSERT(static_cast<bool>(NdxData(i2->second)));
	CPPUNIT_ASSERT_EQUAL(std::string("IoptdataI"),
			     NdxData(i2->second)->accept(v));

	////////////// third id

	PnSUserData::const_iterator a3 = ud3.find("A");
	CPPUNIT_ASSERT(ud3.end() != a3);
	CPPUNIT_ASSERT(static_cast<bool>(NdxData(a3->second)));
	CPPUNIT_ASSERT_EQUAL(std::string("AdataM"),
			     NdxData(a3->second)->accept(v));

	PnSUserData::const_iterator b3 = ud3.find("B");
	CPPUNIT_ASSERT(ud3.end() != b3);
	CPPUNIT_ASSERT(static_cast<bool>(NdxData(b3->second)));
	CPPUNIT_ASSERT_EQUAL(std::string("BdataM"),
			     NdxData(b3->second)->accept(v));

	PnSUserData::const_iterator c3 = ud3.find("C");
	CPPUNIT_ASSERT(ud3.end() != c3);
	CPPUNIT_ASSERT(static_cast<bool>(NdxData(c3->second)));
	CPPUNIT_ASSERT_EQUAL(std::string("CdataI"),
			     NdxData(c3->second)->accept(v));

	PnSUserData::const_iterator d3 = ud3.find("D");
	CPPUNIT_ASSERT(ud3.end() != d3);
	CPPUNIT_ASSERT(static_cast<bool>(NdxData(d3->second)));
	CPPUNIT_ASSERT_EQUAL(std::string("DoptdataM"),
			     NdxData(d3->second)->accept(v));

	PnSUserData::const_iterator e3 = ud3.find("E");
	CPPUNIT_ASSERT(ud3.end() != e3);
	CPPUNIT_ASSERT(static_cast<bool>(NdxData(e3->second)));
	CPPUNIT_ASSERT_EQUAL(std::string("EoptdataM"),
			     NdxData(e3->second)->accept(v));

	PnSUserData::const_iterator f3 = ud3.find("F");
	CPPUNIT_ASSERT(ud3.end() != f3);
	CPPUNIT_ASSERT(static_cast<bool>(NdxData(f3->second)));
	CPPUNIT_ASSERT_EQUAL(std::string("FoptdataI"),
			     NdxData(f3->second)->accept(v));

	PnSUserData::const_iterator g3 = ud3.find("G");
	CPPUNIT_ASSERT(ud3.end() != g3);
	CPPUNIT_ASSERT(static_cast<bool>(NdxData(g3->second)));
	CPPUNIT_ASSERT_EQUAL(std::string("GoptdataM"),
			     NdxData(g3->second)->accept(v));

	PnSUserData::const_iterator h3 = ud3.find("H");
	CPPUNIT_ASSERT(ud3.end() != h3);
	CPPUNIT_ASSERT(static_cast<bool>(NdxData(h3->second)));
	CPPUNIT_ASSERT_EQUAL(std::string("HoptdataM"),
			     NdxData(h3->second)->accept(v));

	PnSUserData::const_iterator i3 = ud3.find("I");
	CPPUNIT_ASSERT(ud3.end() != i3);
	CPPUNIT_ASSERT(static_cast<bool>(NdxData(i3->second)));
	CPPUNIT_ASSERT_EQUAL(std::string("IoptdataI"),
			     NdxData(i3->second)->accept(v));

}

static void
memory_consumption(double& vsz, double& rss) {
	static const long pagesize = sysconf(_SC_PAGESIZE);
	std::ifstream f;
	f.exceptions(std::ifstream::badbit);
	f.open("/proc/self/stat");
	static const unsigned int MAX_BUF = 4096;
	char buf[MAX_BUF];
	f.getline(buf, MAX_BUF);
	if (f.fail() && !f.eof())
		throw std::runtime_error("/proc/self/stat: line too long");
	char *p = strtok(buf, " ");
	for (unsigned int k = 1; nullptr != p && k < 23; ++k)
		p = strtok(nullptr, " ");
	if (nullptr == p)
		throw std::runtime_error("/proc/self/stat: format");
	errno = 0;
	char *endp;
	long v = strtol(p, &endp, 10);
	if (0 != errno || '\0' != *endp || v < 0)
		throw std::runtime_error("/proc/self/stat: vsz format");
	vsz = v / 1048576.0;
	p = strtok(nullptr, " ");
	errno = 0;
	v = strtol(p, &endp, 10);
	if (0 != errno || '\0' != *endp || v < 0)
		throw std::runtime_error("/proc/self/stat: rss format");
	rss = v * pagesize / 1048576.0;
}

static void
print_mem_consumption() {
	std::cerr << std::endl;
	do {
		double vsz, rss;
		memory_consumption(vsz, rss);
		std::cerr << std::fixed << std::setprecision(2)
			  << std::setfill(' ') << std::setw(14)
			  << vsz << '\t'
			  << std::setfill(' ') << std::setw(14)
			  << rss << '\r';
		{
			std::unique_lock<std::mutex> l(mtx);
			if (running) {
				cnd.wait_for(l, seconds(1));
				if (!running) {
					std::cerr << std::endl;
					return;
				}
			}
		}
	} while (true);
}

void
TestFetch::leak() {
	std::shared_ptr<fixed_managed_shared_memory> mem(
		new fixed_managed_shared_memory(
			boost::interprocess::create_only,
			map_shm::map_shm_name,
			0x04000000), shm_deleter());
	map_shm::shm_ = mem;
	HTTPMgrFct fct;
	SyncObjPool<HTTPMgr> mgr_pool(&fct, 90, 90);
	AbstractPool p(64);
	BackendConf bck_cnf;
	bck_cnf.set_url("http://pns.com/");
	bck_cnf.set_timeout(3600000lu);
	bck_cnf.set_connect_timeout(3600000lu);
	PnSFieldsConfig preds;
	PnSFieldsConfigParser parser(preds.fields_data());
	static const char pred_conf[] =
		"["
		"  ["
		"    {"
		"      \"field\": \"A\","
		"      \"preds\": {"
		"        \"2\": \"\\\"msisdn\\\" == CRED_TYPE\""
		"      }"
		"    },"
		"    {"
		"      \"field\": \"B\","
		"      \"preds\": {"
		"        \"2\": \"\\\"msisdn\\\" == CRED_TYPE || \\\"iacct\\\" == CRED_TYPE\""
		"      }"
		"    },"
		"    {"
		"      \"field\": \"C\","
		"      \"preds\": {"
		"        \"2\": \"\\\"iacct\\\" == CRED_TYPE\""
		"      }"
		"    },"
		"    {"
		"      \"field\": \"D\","
		"      \"preds\": {"
		"        \"2\": \"\\\"msisdn\\\" == CRED_TYPE\""
		"      }"
		"    },"
		"    {"
		"      \"field\": \"Dopt\","
		"      \"options\": {\"limit\": 2},"
		"      \"preds\": {"
		"        \"2\": \"\\\"msisdn\\\" == CRED_TYPE\""
		"      }"
		"    },"
		"    {"
		"      \"field\": \"E\","
		"      \"preds\": {"
		"        \"2\": \"\\\"msisdn\\\" == CRED_TYPE || \\\"iacct\\\" == CRED_TYPE\""
		"      }"
		"    },"
		"    {"
		"      \"field\": \"Eopt\","
		"      \"options\": {\"limit\": 2},"
		"      \"preds\": {"
		"        \"2\": \"\\\"msisdn\\\" == CRED_TYPE || \\\"iacct\\\" == CRED_TYPE\""
		"      }"
		"    },"
		"    {"
		"      \"field\": \"F\","
		"      \"preds\": {"
		"        \"2\": \"\\\"iacct\\\" == CRED_TYPE\""
		"      }"
		"    },"
		"    {"
		"      \"field\": \"Fopt\","
		"      \"options\": {\"limit\": 2},"
		"      \"preds\": {"
		"        \"2\": \"\\\"iacct\\\" == CRED_TYPE\""
		"      }"
		"    },"
		"    {"
		"      \"field\": \"Gopt\","
		"      \"options\": {\"limit\": 2},"
		"      \"preds\": {"
		"        \"2\": \"\\\"msisdn\\\" == CRED_TYPE\""
		"      }"
		"    },"
		"    {"
		"      \"field\": \"Hopt\","
		"      \"options\": {\"limit\": 2},"
		"      \"preds\": {"
		"        \"2\": \"\\\"msisdn\\\" == CRED_TYPE || \\\"iacct\\\" == CRED_TYPE\""
		"      }"
		"    },"
		"    {"
		"      \"field\": \"Iopt\","
		"      \"options\": {\"limit\": 2},"
		"      \"preds\": {"
		"        \"2\": \"\\\"iacct\\\" == CRED_TYPE\""
		"      }"
		"    }"
		"  ]"
		"]";
	parser.parse(pred_conf, sizeof(pred_conf) - 1);
	parser.parse(nullptr, 0);

	OfferDynConfigs pns_confs{{"BB", &preds}};

	CredentialSet cs1{
		Credential(AdvAuth::CREDENTIAL_MSISDN, "33601234567"),
		Credential(AdvAuth::CREDENTIAL_IACCOUNT, "701234567")
	};
	Identity id1("alias1", Contract(IdentityType(AdvAuth::CI_TYPE), std::move(cs1)));

	CredentialSet cs2{
		Credential(AdvAuth::CREDENTIAL_MSISDN, "33601234567"),
		Credential(AdvAuth::CREDENTIAL_IACCOUNT, "701234567")
	};
	Identity id2("alias2", Contract(IdentityType(AdvAuth::CI_TYPE), std::move(cs2)));

	CredentialSet cs3{
		Credential(AdvAuth::CREDENTIAL_MSISDN, "33601234567"),
		Credential(AdvAuth::CREDENTIAL_IACCOUNT, "701234567")
	};
	Identity id3("alias3", Contract(IdentityType(AdvAuth::CM_TYPE), std::move(cs3)));

	running = true;
	std::thread t(&print_mem_consumption);

	static const unsigned int THREADS = 1;
	std::thread th[THREADS];
	for (unsigned int i = 0; i < THREADS; ++i)
		th[i] = std::thread(&thr_main, std::ref(mgr_pool), std::ref(p),
				    std::cref(bck_cnf), std::cref(pns_confs),
				    std::cref(id1), std::cref(id2),
				    std::cref(id3));
	for (unsigned int i = 0; i < THREADS; ++i)
		th[i].join();
	{
		std::unique_lock<std::mutex> l(mtx);
		running = false;
		cnd.notify_one();
	}
	t.join();
}

static void
thr_main(ObjPool<HTTPMgr>& mgr_pool, AbstractPool& p,
	 const BackendConf& bck_cnf, const OfferDynConfigs& pns_confs,
	 const Identity& id1, const Identity& id2, const Identity& id3) {

	request_rec r;
	r.pool = pool;
	apr_sockaddr_t *remote_addr = reinterpret_cast<apr_sockaddr_t *>(
		apr_pcalloc(pool, sizeof(apr_sockaddr_t)));
	remote_addr->pool = pool;
	remote_addr->hostname = apr_pstrdup(pool, "localhost");
	remote_addr->servname = apr_pstrdup(pool, "80");
	remote_addr->port = 80;
	remote_addr->family = AF_INET;
	remote_addr->salen = sizeof(struct sockaddr_in); // size of sockaddr?
	remote_addr->ipaddr_len = sizeof(struct in_addr); // size of ip address structure
	remote_addr->addr_str_len = 16; // 16 for ipv4 and 46 for ipv6
	remote_addr->ipaddr_ptr = &remote_addr->sa.sin.sin_addr;
	remote_addr->next = 0;
	remote_addr->sa.sin.sin_family = AF_INET;
	remote_addr->sa.sin.sin_port = htons(80);
	remote_addr->sa.sin.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	r.useragent_addr = remote_addr;
	r.headers_in = apr_table_make(pool, 32);
	r.subprocess_env = apr_table_make(pool, 32);
	GP::Apache::ReqCtx ctx(r.subprocess_env);

	RequestedData args("canal=hpc&targets=BB[options]", r.headers_in, 3);
	args.parse_targets();

	static const unsigned int ITERS = 10000;
	for (unsigned int i = 0; i < ITERS; ++i) {
		PnSUserData ud1, ud2, ud3;
		iteration(mgr_pool, p, bck_cnf, pns_confs, r, ctx, ud1, ud2,
			  ud3, id1, id2, id3, args);
	}
}

static void
iteration(ObjPool<HTTPMgr>& mgr_pool, AbstractPool& p,
	  const BackendConf& bck_cnf, const OfferDynConfigs& pns_confs,
	  request_rec& r, GP::ReqCtx& ctx, PnSUserData& ud1, PnSUserData& ud2,
	  PnSUserData& ud3, const Identity& id1, const Identity& id2,
	  const Identity& id3, const RequestedData& args) {

	do_fetch(mgr_pool, p, bck_cnf, pns_confs, r, ctx, ud1, ud2, ud3, id1,
		 id2, id3, args);
	/*
	  we should have
	  A: AdataM
	  B: BdataM or BdataI
	  C: CdataI
	  D: DoptdataM
	  E: EoptdataM or EoptdataI
	  F: FoptdataI
	  G: GoptdataM
	  H: HoptdataM or HoptdataI
	  I: IoptdataI
	 */

	PnS3FieldVisitor v;

	PnSUserData::const_iterator a1 = ud1.find("A");
	CPPUNIT_ASSERT(ud1.end() != a1);
	CPPUNIT_ASSERT(static_cast<bool>(NdxData(a1->second)));
	CPPUNIT_ASSERT_EQUAL(std::string("AdataM"),
			     NdxData(a1->second)->accept(v));

	PnSUserData::const_iterator b1 = ud1.find("B");
	CPPUNIT_ASSERT(ud1.end() != b1);
	CPPUNIT_ASSERT(static_cast<bool>(NdxData(b1->second)));
	const std::string& bdata1 = NdxData(b1->second)->accept(v);
	CPPUNIT_ASSERT(0 == bdata1.compare("BdataM") ||
		       0 == bdata1.compare("BdataI"));

	PnSUserData::const_iterator c1 = ud1.find("C");
	CPPUNIT_ASSERT(ud1.end() != c1);
	CPPUNIT_ASSERT(static_cast<bool>(NdxData(c1->second)));
	CPPUNIT_ASSERT_EQUAL(std::string("CdataI"),
			     NdxData(c1->second)->accept(v));

	PnSUserData::const_iterator d1 = ud1.find("D");
	CPPUNIT_ASSERT(ud1.end() != d1);
	CPPUNIT_ASSERT(static_cast<bool>(NdxData(d1->second)));
	CPPUNIT_ASSERT_EQUAL(std::string("DoptdataM"),
			     NdxData(d1->second)->accept(v));

	PnSUserData::const_iterator e1 = ud1.find("E");
	CPPUNIT_ASSERT(ud1.end() != e1);
	CPPUNIT_ASSERT(static_cast<bool>(NdxData(e1->second)));
	const std::string& edata1 = NdxData(e1->second)->accept(v);
	CPPUNIT_ASSERT(0 == edata1.compare("EoptdataM") ||
		       0 == edata1.compare("EoptdataI"));

	PnSUserData::const_iterator f1 = ud1.find("F");
	CPPUNIT_ASSERT(ud1.end() != f1);
	CPPUNIT_ASSERT(static_cast<bool>(NdxData(f1->second)));
	CPPUNIT_ASSERT_EQUAL(std::string("FoptdataI"),
			     NdxData(f1->second)->accept(v));

	PnSUserData::const_iterator g1 = ud1.find("G");
	CPPUNIT_ASSERT(ud1.end() != g1);
	CPPUNIT_ASSERT(static_cast<bool>(NdxData(g1->second)));
	CPPUNIT_ASSERT_EQUAL(std::string("GoptdataM"),
			     NdxData(g1->second)->accept(v));

	PnSUserData::const_iterator h1 = ud1.find("H");
	CPPUNIT_ASSERT(ud1.end() != h1);
	CPPUNIT_ASSERT(static_cast<bool>(NdxData(h1->second)));
	const std::string& hdata1 = NdxData(h1->second)->accept(v);
	CPPUNIT_ASSERT(0 == hdata1.compare("HoptdataM") ||
		       0 == hdata1.compare("HoptdataI"));

	PnSUserData::const_iterator i1 = ud1.find("I");
	CPPUNIT_ASSERT(ud1.end() != i1);
	CPPUNIT_ASSERT(static_cast<bool>(NdxData(i1->second)));
	CPPUNIT_ASSERT_EQUAL(std::string("IoptdataI"),
			     NdxData(i1->second)->accept(v));

	///////////////////// second id

	PnSUserData::const_iterator a2 = ud2.find("A");
	CPPUNIT_ASSERT(ud2.end() != a2);
	CPPUNIT_ASSERT(static_cast<bool>(NdxData(a2->second)));
	CPPUNIT_ASSERT_EQUAL(std::string("AdataM"),
			     NdxData(a2->second)->accept(v));

	PnSUserData::const_iterator b2 = ud2.find("B");
	CPPUNIT_ASSERT(ud2.end() != b2);
	CPPUNIT_ASSERT(static_cast<bool>(NdxData(b2->second)));
	const std::string& bdata2 = NdxData(b2->second)->accept(v);
	CPPUNIT_ASSERT(0 == bdata2.compare("BdataM") ||
		       0 == bdata2.compare("BdataI"));

	PnSUserData::const_iterator c2 = ud2.find("C");
	CPPUNIT_ASSERT(ud2.end() != c2);
	CPPUNIT_ASSERT(static_cast<bool>(NdxData(c2->second)));
	CPPUNIT_ASSERT_EQUAL(std::string("CdataI"),
			     NdxData(c2->second)->accept(v));

	PnSUserData::const_iterator d2 = ud2.find("D");
	CPPUNIT_ASSERT(ud2.end() != d2);
	CPPUNIT_ASSERT(static_cast<bool>(NdxData(d2->second)));
	CPPUNIT_ASSERT_EQUAL(std::string("DoptdataM"),
			     NdxData(d2->second)->accept(v));

	PnSUserData::const_iterator e2 = ud2.find("E");
	CPPUNIT_ASSERT(ud2.end() != e2);
	CPPUNIT_ASSERT(static_cast<bool>(NdxData(e2->second)));
	const std::string& edata2 = NdxData(e2->second)->accept(v);
	CPPUNIT_ASSERT(0 == edata2.compare("EoptdataM") ||
		       0 == edata2.compare("EoptdataI"));

	PnSUserData::const_iterator f2 = ud2.find("F");
	CPPUNIT_ASSERT(ud2.end() != f2);
	CPPUNIT_ASSERT(static_cast<bool>(NdxData(f2->second)));
	CPPUNIT_ASSERT_EQUAL(std::string("FoptdataI"),
			     NdxData(f2->second)->accept(v));

	PnSUserData::const_iterator g2 = ud2.find("G");
	CPPUNIT_ASSERT(ud2.end() != g2);
	CPPUNIT_ASSERT(static_cast<bool>(NdxData(g2->second)));
	CPPUNIT_ASSERT_EQUAL(std::string("GoptdataM"),
			     NdxData(g2->second)->accept(v));

	PnSUserData::const_iterator h2 = ud2.find("H");
	CPPUNIT_ASSERT(ud2.end() != h2);
	CPPUNIT_ASSERT(static_cast<bool>(NdxData(h2->second)));
	const std::string& hdata2 = NdxData(h2->second)->accept(v);
	CPPUNIT_ASSERT(0 == hdata2.compare("HoptdataM") ||
		       0 == hdata2.compare("HoptdataI"));

	PnSUserData::const_iterator i2 = ud2.find("I");
	CPPUNIT_ASSERT(ud2.end() != i2);
	CPPUNIT_ASSERT(static_cast<bool>(NdxData(i2->second)));
	CPPUNIT_ASSERT_EQUAL(std::string("IoptdataI"),
			     NdxData(i2->second)->accept(v));

	////////////// third id

	PnSUserData::const_iterator a3 = ud3.find("A");
	CPPUNIT_ASSERT(ud3.end() != a3);
	CPPUNIT_ASSERT(static_cast<bool>(NdxData(a3->second)));
	CPPUNIT_ASSERT_EQUAL(std::string("AdataM"),
			     NdxData(a3->second)->accept(v));

	PnSUserData::const_iterator b3 = ud3.find("B");
	CPPUNIT_ASSERT(ud3.end() != b3);
	CPPUNIT_ASSERT(static_cast<bool>(NdxData(b3->second)));
	const std::string& bdata3 = NdxData(b3->second)->accept(v);
	CPPUNIT_ASSERT(0 == bdata3.compare("BdataM") ||
		       0 == bdata3.compare("BdataI"));

	PnSUserData::const_iterator c3 = ud3.find("C");
	CPPUNIT_ASSERT(ud3.end() != c3);
	CPPUNIT_ASSERT(static_cast<bool>(NdxData(c3->second)));
	CPPUNIT_ASSERT_EQUAL(std::string("CdataI"),
			     NdxData(c3->second)->accept(v));

	PnSUserData::const_iterator d3 = ud3.find("D");
	CPPUNIT_ASSERT(ud3.end() != d3);
	CPPUNIT_ASSERT(static_cast<bool>(NdxData(d3->second)));
	CPPUNIT_ASSERT_EQUAL(std::string("DoptdataM"),
			     NdxData(d3->second)->accept(v));

	PnSUserData::const_iterator e3 = ud3.find("E");
	CPPUNIT_ASSERT(ud3.end() != e3);
	CPPUNIT_ASSERT(static_cast<bool>(NdxData(e3->second)));
	const std::string& edata3 = NdxData(e3->second)->accept(v);
	CPPUNIT_ASSERT(0 == edata3.compare("EoptdataM") ||
		       0 == edata3.compare("EoptdataI"));

	PnSUserData::const_iterator f3 = ud3.find("F");
	CPPUNIT_ASSERT(ud3.end() != f3);
	CPPUNIT_ASSERT(static_cast<bool>(NdxData(f3->second)));
	CPPUNIT_ASSERT_EQUAL(std::string("FoptdataI"),
			     NdxData(f3->second)->accept(v));

	PnSUserData::const_iterator g3 = ud3.find("G");
	CPPUNIT_ASSERT(ud3.end() != g3);
	CPPUNIT_ASSERT(static_cast<bool>(NdxData(g3->second)));
	CPPUNIT_ASSERT_EQUAL(std::string("GoptdataM"),
			     NdxData(g3->second)->accept(v));

	PnSUserData::const_iterator h3 = ud3.find("H");
	CPPUNIT_ASSERT(ud3.end() != h3);
	CPPUNIT_ASSERT(static_cast<bool>(NdxData(h3->second)));
	const std::string& hdata3 = NdxData(h3->second)->accept(v);
	CPPUNIT_ASSERT(0 == hdata3.compare("HoptdataM") ||
		       0 == hdata3.compare("HoptdataI"));

	PnSUserData::const_iterator i3 = ud3.find("I");
	CPPUNIT_ASSERT(ud3.end() != i3);
	CPPUNIT_ASSERT(static_cast<bool>(NdxData(i3->second)));
	CPPUNIT_ASSERT_EQUAL(std::string("IoptdataI"),
			     NdxData(i3->second)->accept(v));
}

static void
do_fetch(ObjPool<HTTPMgr>& mgr_pool, AbstractPool& p,
	 const BackendConf& bck_cnf, const OfferDynConfigs& pns_confs,
	 request_rec& r, GP::ReqCtx& ctx, PnSUserData& ud1, PnSUserData& ud2,
	 PnSUserData& ud3, const Identity& id1, const Identity& id2,
	 const Identity& id3, const RequestedData& args) {

	CredentialSet::const_iterator ci = id1.credentials().begin();
	const Credential *c11 = &*ci;
	++ci;
	const Credential *c12 = &*ci;
	ci = id2.credentials().begin();
	const Credential *c21 = &*ci;
	++ci;
	const Credential *c22 = &*ci;
	ci = id3.credentials().begin();
	const Credential *c31 = &*ci;
	++ci;
	const Credential *c32 = &*ci;

	PnS3Credential cred1(
		PnS3CredentialType(PnS3CredTypesConstants::PNS3_MSISDN),
		"33601234567");
	PnS3Credential cred2(
		PnS3CredentialType(PnS3CredTypesConstants::PNS3_IACCT),
		"701234567");
	PnS3Credential cred3(
		PnS3CredentialType(PnS3CredTypesConstants::PNS3_MSISDN),
		"33601234567");
	PnS3Credential cred4(
		PnS3CredentialType(PnS3CredTypesConstants::PNS3_IACCT),
		"701234567");
	PnS3Credential cred5(
		PnS3CredentialType(PnS3CredTypesConstants::PNS3_MSISDN),
		"33601234567");
	PnS3Credential cred6(
		PnS3CredentialType(PnS3CredTypesConstants::PNS3_IACCT),
		"701234567");
	QoSMgr *qos = nullptr;
	unsigned int clazz = 1;
	Transaction *tx = nullptr;

	PnSFieldNames pns_fields1;
	PnSFieldNames pns_fields2;
	PnSFieldNames pns_fields3;

	PnSReqs pns_reqs;
	GraphExecutor::ExecGraph graph;
	GraphExecutor executor(&graph);
	Mgr mgr(&mgr_pool);

	ExecGraphNode egn1(&executor);
	const std::string tag("0");
	Node *n1 = new Node(
		&egn1, &mgr, p, bck_cnf, "2", pns_confs, &r, ctx,
		std::move(cred1), 0, ud1, &id1, tag,
		id1.contracts().find(tag)->second, c11, args, pns_fields1, qos,
		clazz, pns_reqs, tx);
	egn1.set_node(n1);
	graph.add(std::move(egn1));

	ExecGraphNode egn2(&executor);
	Node *n2 = new Node(
		&egn2, &mgr, p, bck_cnf, "2", pns_confs, &r, ctx,
		std::move(cred2), 0, ud1, &id1, tag,
		id1.contracts().find(tag)->second, c12, args, pns_fields1, qos,
		clazz, pns_reqs, tx);
	egn2.set_node(n2);
	graph.add(std::move(egn2));

	ExecGraphNode egn3(&executor);
	Node *n3 = new Node(
		&egn3, &mgr, p, bck_cnf, "2", pns_confs, &r, ctx,
		std::move(cred3), 1, ud2, &id2, tag,
		id2.contracts().find(tag)->second, c21, args, pns_fields2, qos,
		clazz, pns_reqs, tx);
	egn3.set_node(n3);
	graph.add(std::move(egn3));

	ExecGraphNode egn4(&executor);
	Node *n4 = new Node(
		&egn4, &mgr, p, bck_cnf, "2", pns_confs, &r, ctx,
		std::move(cred4), 1, ud2, &id2, tag,
		id2.contracts().find(tag)->second, c22, args, pns_fields2, qos,
		clazz, pns_reqs, tx);
	egn4.set_node(n4);
	graph.add(std::move(egn4));

	ExecGraphNode egn5(&executor);
	Node *n5 = new Node(
		&egn5, &mgr, p, bck_cnf, "2", pns_confs, &r, ctx,
		std::move(cred5), 2, ud3, &id3, tag,
		id3.contracts().find(tag)->second, c31, args, pns_fields3, qos,
		clazz, pns_reqs, tx);
	egn5.set_node(n5);
	graph.add(std::move(egn5));

	ExecGraphNode egn6(&executor);
	Node *n6 = new Node(
		&egn6, &mgr, p, bck_cnf, "2", pns_confs, &r, ctx,
		std::move(cred6), 2, ud3, &id3, tag,
		id3.contracts().find(tag)->second, c32, args, pns_fields3, qos,
		clazz, pns_reqs, tx);
	egn6.set_node(n6);
	graph.add(std::move(egn6));

	executor.execute();

	Node *mowner, *mpassive1, *mpassive2;
	if (nullptr == n1->get_owner()) {
		CPPUNIT_ASSERT(nullptr != n3->get_owner());
		CPPUNIT_ASSERT(nullptr != n5->get_owner());
		mowner = n1;
		mpassive1 = n3;
		mpassive2 = n5;
	} else if (nullptr == n3->get_owner()) {
		CPPUNIT_ASSERT(nullptr != n5->get_owner());
		mowner = n3;
		mpassive1 = n1;
		mpassive2 = n5;
	} else {
		CPPUNIT_ASSERT(nullptr == n5->get_owner());
		mowner = n5;
		mpassive1 = n1;
		mpassive2 = n3;
	}

	Node *iowner, *ipassive1, *ipassive2;
	if (nullptr == n2->get_owner()) {
		CPPUNIT_ASSERT(nullptr != n4->get_owner());
		CPPUNIT_ASSERT(nullptr != n6->get_owner());
		iowner = n2;
		ipassive1 = n4;
		ipassive2 = n6;
	} else if (nullptr == n4->get_owner()) {
		CPPUNIT_ASSERT(nullptr != n6->get_owner());
		iowner = n4;
		ipassive1 = n2;
		ipassive2 = n6;
	} else {
		CPPUNIT_ASSERT(nullptr == n6->get_owner());
		iowner = n6;
		ipassive1 = n2;
		ipassive2 = n4;
	}

	static const char rm[] =
		"{"
		"  \"users\": ["
		"    {"
		"      \"credentials\": {\"msisdn\": 33601234567},"
		"      \"fields\": {"
		"        \"A\": \"AdataM\","
		"        \"B\": \"BdataM\","
		"        \"D\": \"DdataM\","
		"        \"E\": \"EdataM\""
		"      }"
		"    },"
		"    {"
		"      \"credentials\": {\"msisdn\": 33601234567},"
		"      \"fields\": {"
		"        \"D\": \"DoptdataM\","
		"        \"E\": \"EoptdataM\","
		"        \"G\": \"GoptdataM\","
		"        \"H\": \"HoptdataM\""
		"      }"
		"    }"
		"  ]"
		"}";
	static const char *hdrs[] = {
		"HTTP/1.1 200 OK\r\n",
		"Date: Tue, 30 Oct 2018 09:52:49 GMT\r\n",
		"Server: Apache\r\n"
	};
	char buf[128];
	AuthdBckCallbacks<HTTPAuthdBckData<BackendData<PnS3Data>>, PnSParser,
			  PnSBck> bm(mowner->get_http_data(), *mowner, nullptr);
	for (unsigned int i = 0; i < sizeof(hdrs) / sizeof(hdrs[0]); ++i)
		bm.header_handler_functor()(hdrs[i], strlen(hdrs[i]));
	int w = snprintf(buf, sizeof(buf), "Content-Length: %zu\r\n",
			 sizeof(rm) - 1);
	bm.header_handler_functor()(buf, w);
	bm.handler_functor()(rm, sizeof(rm) - 1);
	bm.eos_functor()(HTTPClient::Status::Ok, "");

	static const char ri[] =
		"{"
		"  \"users\": ["
		"    {"
		"      \"credentials\": {\"infranetaccount\": \"701234567\"},"
		"      \"fields\": {"
		"        \"B\": \"BdataI\","
		"        \"C\": \"CdataI\","
		"        \"E\": \"EdataI\","
		"        \"F\": \"FdataI\""
		"      }"
		"    },"
		"    {"
		"      \"credentials\": {\"infranetaccount\": \"701234567\"},"
		"      \"fields\": {"
		"        \"E\": \"EoptdataI\","
		"        \"F\": \"FoptdataI\","
		"        \"H\": \"HoptdataI\","
		"        \"I\": \"IoptdataI\""
		"      }"
		"    }"
		"  ]"
		"}";
	static const char ric[] = {
		'\x1f', '\x8b', '\x08', '\x00', '\x0e', '\x60', '\xd8', '\x5b',
		'\x02', '\x03', '\xab', '\xe6', '\x52', '\x50', '\x50', '\x2a',
		'\x2d', '\x4e', '\x2d', '\x2a', '\x56', '\xb2', '\x52', '\x88',
		'\x06', '\x72', '\x14', '\x14', '\xaa', '\xc1', '\x24', '\x50',
		'\x38', '\xb9', '\x28', '\x35', '\x25', '\x35', '\xaf', '\x24',
		'\x33', '\x31', '\x07', '\x24', '\x59', '\xad', '\x94', '\x99',
		'\x97', '\x56', '\x94', '\x98', '\x97', '\x5a', '\x92', '\x98',
		'\x9c', '\x9c', '\x5f', '\x9a', '\x57', '\x02', '\x14', '\x53',
		'\x32', '\x37', '\x30', '\x34', '\x32', '\x36', '\x31', '\x35',
		'\x33', '\x57', '\xaa', '\xd5', '\x81', '\x69', '\x4b', '\xcb',
		'\x4c', '\xcd', '\x49', '\x01', '\xeb', '\x80', '\x8a', '\x00',
		'\xc5', '\x9c', '\x40', '\x8a', '\x9d', '\x52', '\x12', '\x4b',
		'\x12', '\x3d', '\x95', '\x74', '\x10', '\xc2', '\xce', '\x20',
		'\x61', '\x67', '\x0c', '\x61', '\x57', '\x90', '\xb0', '\x2b',
		'\x86', '\xb0', '\x1b', '\x48', '\xd8', '\x0d', '\x22', '\x0c',
		'\x15', '\xad', '\x05', '\xd3', '\x50', '\xbb', '\x69', '\xe1',
		'\x70', '\x88', '\x53', '\xf2', '\x0b', '\x4a', '\x70', '\xb8',
		'\x06', '\x9b', '\x8c', '\x07', '\x48', '\xc6', '\x03', '\x9b',
		'\x8c', '\x27', '\x48', '\xc6', '\x13', '\x2e', '\x83', '\xea',
		'\x09', '\x20', '\x19', '\xcb', '\x55', '\x0b', '\x00', '\x29',
		'\x58', '\x7b', '\x2e', '\x91', '\x01', '\x00', '\x00'};
	AuthdBckCallbacks<HTTPAuthdBckData<BackendData<PnS3Data>>, PnSParser,
			  PnSBck> bi(iowner->get_http_data(), *iowner, nullptr);
	for (unsigned int i = 0; i < sizeof(hdrs) / sizeof(hdrs[0]); ++i)
		bi.header_handler_functor()(hdrs[i], strlen(hdrs[i]));

	static const char ce[] = "Content-Encoding: gzip\r\n";
	bi.header_handler_functor()(ce, sizeof(ce) - 1);
	w = snprintf(buf, sizeof(buf), "Content-Length: %zu\r\n", sizeof(ric));
	bi.header_handler_functor()(buf, w);
	bi.handler_functor()(ric, sizeof(ric));
	// w = snprintf(buf, sizeof(buf), "Content-Length: %zu\r\n", sizeof(ri) - 1);
	// bi.header_handler_functor()(buf, w);
	// bi.handler_functor()(ri, sizeof(ri) - 1);

	bi.eos_functor()(HTTPClient::Status::Ok, "");

	mgr.eos();

	executor.process_data();
	/*
	  we should have
	  A: AdataM
	  B: BdataM or BdataI
	  C: CdataI
	  D: DoptdataM
	  E: EoptdataM or EoptdataI
	  F: FoptdataI
	  G: GoptdataM
	  H: HoptdataM or HoptdataI
	  I: IoptdataI
	 */
	try {
		executor.errors();
	} catch (const std::exception&) {
	}
}

#include <cppunit/TestAssert.h>
#include "testLeak.hh"
#include <fstream>
#include <stdexcept>
#include <cstring>
#include <cerrno>
#include <cstdlib>
#include <unistd.h>
#include <iostream>
#include <iomanip>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <thread>
#include "pv/Config.hh"
#include "pv/intf.hh"

using std::chrono::seconds;

static void memory_consumption(double& vsz, double& rss);
static void print_mem_consumption();

static bool running = false;
static std::mutex mtx;
static std::condition_variable cnd;

CPPUNIT_TEST_SUITE_REGISTRATION(TestLeak);

TestLeak::TestLeak() {
}

TestLeak::~TestLeak() {
}

void
TestLeak::setUp() {
}

void
TestLeak::tearDown() {
}

void
TestLeak::leak() {
	running = true;
	std::thread t(&print_mem_consumption);
	work();
	{
		std::unique_lock<std::mutex> l(mtx);
		running = false;
		cnd.notify_one();
	}
	t.join();
}

void
TestLeak::work() const {
	static const unsigned int ITER = 100000;
	// static const unsigned int ITER = 1;

	pv::Config cfg;
	cfg.set_url("https://dgc-verification-prod.incert.lu:9443/api/get-certificates");
	cfg.set_key("/home/srn/mobile_certificate.key");
	cfg.set_cert("/home/srn/mobile_certificate.pem");

	pv::App app(cfg);

	app.update();

	for (unsigned int i = 0; i < ITER; ++i)
		do_work(app);
}

void
TestLeak::do_work(pv::App& app) const {
	static const std::string inp("NCFOXN%TSMAHN-HBUKN8N2A709SZ%K0IIXLH 437PG/EB2QINOUA4DML9J5NN+MNO4*J8OX4W$C2VLWLI2P53O8J.V J8$XJK*L5R17PGD.LXVH6VB8J4 K4: BI$NOV9:/P%RO$ZP9Z5*XOCTO2I0WXFE$B:PI:IG7Y47%S7Y48YIZ73423ZQTW63LD3J%4UZ2 NVV5TN%2UP20J5/5LEBFD-48YI+T4D-4HRVUMNMD3323R1370RC-4A+2XEN QT QTHC31M3+E3CP456L X4CZKHKB-43.E3KD3OAJ5%IKTCMD3QHBZQJLIFHJP7NVDEB8:IE+4SY05QNT+4IGF5JNBPI$RUY F+4WX FN*JTVB3UQUE1+W3:Q7-:V+*E1ZMNCI ZJ$2BG8A:ZJ::AHOAYK0N DNMDQJAZGA+1V2:U2E4Z6V5Z5BUME88IB4QYEOIFKTO3HH 4FFG3P8FNRM%A5K6M5OCX:7T%F58DR$I81L46LIQQ%3E8J0%08-*TPX5R-FL:B+GD:%R$2WXOFBK0Z3G");
	try {
		app.check(inp);
	} catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
	}
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
			if (!running) {
				std::cerr << std::endl;
				return;
			}
			cnd.wait_for(l, seconds(1));
		}
	} while (true);
}

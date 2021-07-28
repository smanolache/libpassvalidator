#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <unistd.h>
#include <fstream>
#include "pv/intf.hh"

static const char *fn;

static void parse_cmd_line(int argc, char *argv[]);
static void usage(const char *p);
static int do_main(int argc, char *argv[]);
static std::string process(std::istream&);

int
main(int argc, char *argv[]) {
	try {
		return do_main(argc, argv);
	} catch (const std::exception& e) {
		std::cerr << "Caught: " << e.what() << std::endl;
		return 1;
	}
}

static int
do_main(int argc, char *argv[]) {
	parse_cmd_line(argc, argv);
	std::string r;
	if (fn) {
		std::ifstream f;
		f.exceptions(std::ifstream::failbit);
		f.open(fn);
		r = process(f);
	} else
		r = process(std::cin);

	pv::Config cfg;
	cfg.set_url("https://dgc-verification-prod.incert.lu:9443/api/get-certificates");
	cfg.set_key("/home/srn/mobile_certificate.key");
	cfg.set_cert("/home/srn/mobile_certificate.pem");

	pv::App app(cfg);

	app.update();

	pv::Result rlt = app.check(r);
	std::cout << std::get<0>(rlt) << std::endl;

	pv::App::shutdown();

	switch (std::get<2>(rlt)) {
	case pv::Status::Ok:
		return 0;
	case pv::Status::InvalidSignature:
		std::cerr << "Invalid signature" << std::endl;
		return 1;
	case pv::Status::UnknownKey:
		std::cerr << "Unknown key" << std::endl;
		return 2;
	case pv::Status::ExpiredClaim:
		std::cerr << "Expired claim" << std::endl;
		return 3;
	default:
		std::cerr << "Unexpected return code" << std::endl;
		return 4;
	}
}

static std::string
process(std::istream& f) {
	f.exceptions(std::istream::badbit);
	static constexpr const std::size_t MAX_BUF = 8192;
	char buf[MAX_BUF];
	std::string r;
	do {
		f.read(buf, sizeof(buf));
		if (f.fail()) {
			if (!f.eof())
				throw std::runtime_error("I/O error");
			if (f.gcount() > 0)
				r.append(buf, buf + f.gcount());
			return r;
		}
		if (f.gcount() > 0)
			r.append(buf, buf + f.gcount());
	} while (true);
}

static void
parse_cmd_line(int argc, char *argv[]) {
	int c;

	while ((c = getopt(argc, argv, "h")) != EOF)
		switch (c) {
		case 'h':
			usage(argv[0]);
			exit(0);
		case '?':
			usage(argv[0]);
			exit(1);
		}
	if (optind + 1 < argc) {
		std::cerr << "Too many arguments." << std::endl;
		usage(argv[0]);
		exit(1);
	}
	if (optind < argc)
		fn = argv[optind];
}

static void
usage(const char *p) {
	std::cerr << "Usage: " << p << " [-d]" << std::endl;
}

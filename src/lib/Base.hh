#pragma once

#include <ostream>

namespace pv {

class Base {
public:
	virtual ~Base() = default;
	virtual void print(std::ostream&) const = 0;
};

}

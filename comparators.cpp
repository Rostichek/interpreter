#include "comparators.h"
#include "object.h"
#include "object_holder.h"

#include <functional>
#include <algorithm>
#include <optional>
#include <sstream>

using namespace std;

namespace Runtime {

	bool Equal(ObjectHolder lhs, ObjectHolder rhs) {
		if (lhs.TryAs<Number>() && rhs.TryAs<Number>())
			return (lhs.TryAs<Number>()->GetValue() == rhs.TryAs<Number>()->GetValue());
		if (lhs.TryAs<String>() && rhs.TryAs<String>())
			return (lhs.TryAs<String>()->GetValue() == rhs.TryAs<String>()->GetValue());
		if (lhs.TryAs<Bool>() && rhs.TryAs<Bool>())
			return (lhs.TryAs<Bool>()->GetValue() == rhs.TryAs<Bool>()->GetValue());
		throw runtime_error("Bad comparsion by equal;");
	}

	bool Less(ObjectHolder lhs, ObjectHolder rhs) {
		if (lhs.TryAs<Number>() && rhs.TryAs<Number>())
			return (lhs.TryAs<Number>()->GetValue() < rhs.TryAs<Number>()->GetValue());
		if (lhs.TryAs<String>() && rhs.TryAs<String>()) {
			return (lhs.TryAs<String>()->GetValue().compare(rhs.TryAs<String>()->GetValue()) < 0);
		}
		if (lhs.TryAs<Bool>() && rhs.TryAs<Bool>())
			return (lhs.TryAs<Bool>()->GetValue() < rhs.TryAs<Bool>()->GetValue());
		throw runtime_error("Bad comparsion by less;");
	}

} /* namespace Runtime */

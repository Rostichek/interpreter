#include "object.h"
#include "statement.h"

#include <sstream>
#include <string_view>

using namespace std;

namespace Runtime {

	void ClassInstance::Print(std::ostream& os) {
		if (cls.GetMethod("__str__"))
			Call("__str__", {})->Print(os);
		else {
			stringstream ss;
			ss << this;
			Runtime::String(ss.str()).Print(os);
		}
	}

	bool ClassInstance::HasMethod(const std::string& method, size_t argument_count) const {
		const Method* method_ptr = cls.GetMethod(method);
		if (method_ptr && method_ptr->formal_params.size() == argument_count)
			return true;
		return false;
	}

	const Closure& ClassInstance::Fields() const {
		return fields;
	}

	Closure& ClassInstance::Fields() {
		return fields;
	}

	ClassInstance::ClassInstance(const Class& cls) : cls(cls) {
	}

	ObjectHolder ClassInstance::Call(const std::string& method, const std::vector<ObjectHolder>& actual_args) {
		const auto* method_ptr = cls.GetMethod(method);
		Closure local;
		local["self"] = ObjectHolder::Share(*this);
		for (size_t i = 0; i < actual_args.size(); ++i)
			local[method_ptr->formal_params.at(i)] = actual_args[i];
		try {
			return method_ptr->body->Execute(local);
		}
		catch (ObjectHolder& holder) {
			return ObjectHolder(holder);
		}
		return {};
	}

	Class::Class(std::string name, std::vector<Method> methods, const Class* parent) : parent(parent), name(name) {
		for (auto& method : methods)
			this->methods[method.name] = move(method);
	}

	const Method* Class::GetMethod(const std::string& name) const {
		if (methods.count(name))
			return &methods.at(name);
		else if (parent)
			return parent->GetMethod(name);
		return nullptr;
	}

	void Class::Print(ostream& os) {
	}

	const std::string& Class::GetName() const {
		return name;
	}

	void Bool::Print(std::ostream& os) {
		if (GetValue())
			os << "True";
		else 
			os << "False";
	}

} /* namespace Runtime */

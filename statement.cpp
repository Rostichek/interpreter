#include "statement.h"
#include "object.h"

#include <iostream>
#include <sstream>
#include <sstream>

using namespace std;

namespace Ast {

	using Runtime::Closure;

	ObjectHolder Assignment::Execute(Closure& closure) {
		return closure[var_name] = right_value->Execute(closure);
	}

	Assignment::Assignment(std::string var, std::unique_ptr<Statement> rv) : var_name(var), right_value(move(rv)) {
	}

	VariableValue::VariableValue(std::string var_name) {
		dotted_ids.push_back(var_name);
	}

	VariableValue::VariableValue(std::vector<std::string> dotted_ids) : dotted_ids(dotted_ids){
	}

	std::string VariableValue::GetPath() const {
		std::string path;
		size_t counter = 0;
		for (const auto& id : dotted_ids) {
			path += id;
			if (++counter != dotted_ids.size())
				path += '.';
		}
		return path;
	}

	ObjectHolder VariableValue::Execute(Closure& closure) {
		if (dotted_ids.size() == 1) {
			if(closure.count(dotted_ids.front()))
				return closure[dotted_ids.front()];
			else throw std::runtime_error("Unknown variable");
		}
		auto& holder = closure[dotted_ids.front()];
		for (size_t i = 1; i < dotted_ids.size()-1; ++i) {
			if(static_cast<Runtime::ClassInstance*>(holder.Get())->Fields().count(dotted_ids[i]))
				holder = static_cast<Runtime::ClassInstance*>(holder.Get())->Fields()[dotted_ids[i]];
			else throw std::runtime_error("Unknown variable");
		}
		Runtime::ClassInstance* instance = holder.TryAs<Runtime::ClassInstance>();
		if (instance->Fields().count(dotted_ids.back()))
			return instance->Fields().at(dotted_ids.back());
		throw std::runtime_error("Unknown variable");

	}

	unique_ptr<Print> Print::Variable(std::string var) {
		return move(make_unique<Print>(move(make_unique<VariableValue>(var))));
	}

	Print::Print(unique_ptr<Statement> argument) {
		args.push_back(move(argument));
	}

	Print::Print(vector<unique_ptr<Statement>> args) : args(move(args)) {
	}

	ObjectHolder Print::Execute(Closure& closure) {
		size_t counter = 0;
		ObjectHolder holder;
		for (const auto& argument : args) {
			holder = argument->Execute(closure);
			if (!holder)
				*output << "None";
			else holder->Print(*output);
			if (++counter != args.size())
				*output << " ";
		}
		*output << endl;
		return {};
	}

	ostream* Print::output = &cout;

	void Print::SetOutputStream(ostream& output_stream) {
		output = &output_stream;
	}

	MethodCall::MethodCall(
		std::unique_ptr<Statement> object
		, std::string method
		, std::vector<std::unique_ptr<Statement>> args
	) : method(method), object(move(object)), args(move(args))
	{
	}

	ObjectHolder MethodCall::Execute(Closure& closure) {
		vector<ObjectHolder> holders;
		for (const auto& argument : args) {
			auto holder = argument->Execute(closure);
			holders.push_back(holder);
		}
		return static_cast<Runtime::ClassInstance*>(closure.at(static_cast<VariableValue*>(object.get())->GetPath()).Get())->Call(method, holders);
	}

	ObjectHolder Stringify::Execute(Closure& closure) {
		ObjectHolder executed = argument->Execute(closure);
		if (executed.TryAs<Runtime::ClassInstance>()) {
			if (executed.TryAs<Runtime::ClassInstance>()->HasMethod("__str__", 0))
				executed = executed.TryAs<Runtime::ClassInstance>()->Call("__str__", {});
			else {
				stringstream ss;
				ss << executed.Get();
				return ObjectHolder::Own(Runtime::String(ss.str()));
			}
		}
		if (executed.TryAs<Runtime::Number>()) {
			return ObjectHolder::Own(Runtime::String(to_string(executed.TryAs<Runtime::Number>()->GetValue())));
		}
		if (executed.TryAs<Runtime::String>()) {
			return ObjectHolder::Own(Runtime::String(executed.TryAs<Runtime::String>()->GetValue()));
		}
		if (executed.TryAs<Runtime::Bool>()) {
			if(executed.TryAs<Runtime::Bool>()->GetValue())
				return ObjectHolder::Own(Runtime::String("True"));
			return ObjectHolder::Own(Runtime::String("False"));
		}
		return {};
	}

	ObjectHolder Add::Execute(Closure& closure) {
		auto lhs_executed = lhs->Execute(closure);
		auto rhs_executed = rhs->Execute(closure);
		if (lhs_executed.TryAs<Runtime::ClassInstance>()) {
			if (lhs_executed.TryAs<Runtime::ClassInstance>()->HasMethod("__add__", 1)) {
				return lhs_executed.TryAs<Runtime::ClassInstance>()->Call("__add__", { rhs_executed });
			}
			throw std::runtime_error("Bad addition;");
		}
		if (lhs_executed.TryAs<Runtime::Number>() && rhs_executed.TryAs<Runtime::Number>()) {
			return ObjectHolder::Own<Runtime::Number>
				(Runtime::Number(lhs_executed.TryAs<Runtime::Number>()->GetValue()
					+ rhs_executed.TryAs<Runtime::Number>()->GetValue()));
		}
		if (lhs_executed.TryAs<Runtime::String>() && rhs_executed.TryAs<Runtime::String>()) {
			return ObjectHolder::Own<Runtime::String>
				(Runtime::String(lhs_executed.TryAs<Runtime::String>()->GetValue()
					+ rhs_executed.TryAs<Runtime::String>()->GetValue()));
		}
		throw std::runtime_error("Bad addition;");
	}

	ObjectHolder Sub::Execute(Closure& closure) {
		if (lhs->Execute(closure).TryAs<Runtime::Number>() && rhs->Execute(closure).TryAs<Runtime::Number>()) {
			return ObjectHolder::Own<Runtime::Number>
				(Runtime::Number(lhs->Execute(closure).TryAs<Runtime::Number>()->GetValue()
					- rhs->Execute(closure).TryAs<Runtime::Number>()->GetValue()));
		}
		throw std::runtime_error("Bad subtraction;");
	}

	ObjectHolder Mult::Execute(Runtime::Closure& closure) {
		if (lhs->Execute(closure).TryAs<Runtime::Number>() && rhs->Execute(closure).TryAs<Runtime::Number>()) {
			return ObjectHolder::Own<Runtime::Number>
				(Runtime::Number(lhs->Execute(closure).TryAs<Runtime::Number>()->GetValue()
					* rhs->Execute(closure).TryAs<Runtime::Number>()->GetValue()));
		}
		throw std::runtime_error("Bad multiplication;");
	}

	ObjectHolder Div::Execute(Runtime::Closure& closure) {
		if (lhs->Execute(closure).TryAs<Runtime::Number>() && rhs->Execute(closure).TryAs<Runtime::Number>()) {
			return ObjectHolder::Own<Runtime::Number>
				(Runtime::Number(lhs->Execute(closure).TryAs<Runtime::Number>()->GetValue()
					/ rhs->Execute(closure).TryAs<Runtime::Number>()->GetValue()));
		}
		throw std::runtime_error("Bad division;");
	}

	ObjectHolder Compound::Execute(Closure& closure) {
		for (auto& statement : statements)
			statement->Execute(closure);
		return {};
 }

	ObjectHolder Return::Execute(Closure& closure) {
		throw statement->Execute(closure);
	}

	ClassDefinition::ClassDefinition(ObjectHolder class_) : cls(class_), class_name(cls.TryAs<Runtime::Class>()->GetName()) {}

	ObjectHolder ClassDefinition::Execute(Runtime::Closure& closure) {
		closure[class_name] = cls;
		return {};
	}

	FieldAssignment::FieldAssignment(
		VariableValue object, std::string field_name, std::unique_ptr<Statement> rv
	)
		: object(std::move(object))
		, field_name(std::move(field_name))
		, right_value(std::move(rv))
	{
	}

	ObjectHolder FieldAssignment::Execute(Runtime::Closure& closure) {
		auto& holder = closure[object.dotted_ids.front()];
		for (size_t i = 1; i < object.dotted_ids.size(); ++i) {
			holder = static_cast<Runtime::ClassInstance*>(holder.Get())->Fields()[object.dotted_ids[i]];
		}
		auto* instance = static_cast<Runtime::ClassInstance*>(holder.Get());
		auto executed_rv = right_value->Execute(closure);
		return instance->Fields()[field_name] = move(executed_rv);
	}

	bool ToBool(ObjectHolder executed, Runtime::Closure& closure) {
		if (executed.TryAs<Runtime::ClassInstance>()) {
			if (executed.TryAs<Runtime::ClassInstance>()->HasMethod("__str__", 0))
				executed = executed.TryAs<Runtime::ClassInstance>()->Call("__str__", {});
			else {
				stringstream ss;
				ss << executed.Get();
				executed = ObjectHolder::Own(Runtime::String(ss.str()));
			}
		}
		if (executed.TryAs<Runtime::Number>())
			return executed.TryAs<Runtime::Number>()->GetValue();
		if (executed.TryAs<Runtime::String>())
			return executed.TryAs<Runtime::String>()->GetValue().size();
		if (executed.TryAs<Runtime::Bool>())
			return executed.TryAs<Runtime::Bool>()->GetValue();
		if (executed.TryAs<Runtime::ClassInstance>())
			return true;
		if (!executed)
			return false;
		throw runtime_error("Bad condition");
	}

	IfElse::IfElse(
		std::unique_ptr<Statement> condition,
		std::unique_ptr<Statement> if_body,
		std::unique_ptr<Statement> else_body
	) : condition(move(condition)), if_body(move(if_body)), else_body(move(else_body)) {}


	ObjectHolder IfElse::Execute(Runtime::Closure& closure) {
		if (ToBool(condition->Execute(closure), closure))
			return if_body->Execute(closure);
		else {
			if(else_body)
				return else_body->Execute(closure);
			return {};
		}
	}

	ObjectHolder Or::Execute(Runtime::Closure& closure) {		
		return ObjectHolder::Own(Runtime::Bool(ToBool(lhs->Execute(closure), closure) || ToBool(rhs->Execute(closure), closure)));
	}

	ObjectHolder And::Execute(Runtime::Closure& closure) {
		return ObjectHolder::Own(Runtime::Bool(ToBool(lhs->Execute(closure), closure) && ToBool(rhs->Execute(closure), closure)));
	}

	ObjectHolder Not::Execute(Runtime::Closure& closure) {
		return ObjectHolder::Own(Runtime::Bool(!ToBool(argument->Execute(closure), closure)));
	}

	Comparison::Comparison(
		Comparator cmp, unique_ptr<Statement> lhs, unique_ptr<Statement> rhs
	) : left(move(lhs)), right(move(rhs)), comparator(move(cmp)) {
	}

	ObjectHolder Comparison::Execute(Runtime::Closure& closure) {
		return ObjectHolder::Own(Runtime::Bool(comparator(left->Execute(closure), right->Execute(closure))));
	}

	NewInstance::NewInstance(
		const Runtime::Class& class_, std::vector<std::unique_ptr<Statement>> args
	)
		: class_(class_)
		, args(std::move(args))
	{
	}

	NewInstance::NewInstance(const Runtime::Class& class_) : NewInstance(class_, {}) {
	}

	ObjectHolder NewInstance::Execute(Runtime::Closure& closure) {
		Runtime::ClassInstance object(class_);
		vector<ObjectHolder> executed_args;
		for (const auto& argument : args) {
			executed_args.push_back(argument->Execute(closure));
		}
		if(object.HasMethod("__init__", executed_args.size()))
			object.Call("__init__", executed_args);
		return ObjectHolder::Own<Runtime::ClassInstance>(move(object));
	}


} /* namespace Ast */

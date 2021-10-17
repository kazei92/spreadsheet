#include "statement.h"

namespace Ast {

	std::ostream& operator<<(std::ostream& stream, const OperationType& operation_type) {
		switch (operation_type) {
		case OperationType::Add:
			stream << '+';
			break;
		case OperationType::Sub:
			stream << '-';
			break;
		case OperationType::Mul:
			stream << '*';
			break;
		case OperationType::Div:
			stream << '/';
		}
		return stream;
	}

	std::string UnaryOperation::ToString() const {
		std::stringstream ss;
		ss << operation_type;
		ss << rhs.get()->ToString();
		return ss.str();
	}

	IFormula::Value UnaryOperation::Evaluate(const ISheet& sheet) const {
		auto eval_res = rhs.get()->Evaluate(sheet);
		
		if (std::holds_alternative<FormulaError>(eval_res)) {
			return eval_res;
		}

		double value = std::get<double>(eval_res);
		if (operation_type == OperationType::Sub) {
			return -value;
		}

		return value;
	}

	StatementType UnaryOperation::Type() const {
		return statement_type;
	}

	std::string ValueOperation::ToString() const {
		std::stringstream ss;
		if (std::holds_alternative<double>(value)) {
			ss << std::get<double>(value);
		}
		else {
			ss << std::get<FormulaError>(value);
		}
		return ss.str();
	}

	IFormula::Value ValueOperation::Evaluate(const ISheet& sheet) const {
		return value;
	}

	StatementType ValueOperation::Type() const {
		return statement_type;
	}

	std::string BinaryOperation::ToString() const {
		std::stringstream ss;
		
		BinaryOperation* lhs_binary = dynamic_cast<BinaryOperation*>(lhs.get());
		ParentesisOperation* lhs_parentesis = dynamic_cast<ParentesisOperation*>(lhs.get());
		BinaryOperation* rhs_binary = dynamic_cast<BinaryOperation*>(rhs.get());
		ParentesisOperation* rhs_parentesis = dynamic_cast<ParentesisOperation*>(rhs.get());

		if (operation_type == OperationType::Mul) {
			if (lhs_parentesis != nullptr) {
				BinaryOperation* left_binary_op = dynamic_cast<BinaryOperation*>(lhs_parentesis->body.get());
				if (left_binary_op != nullptr && left_binary_op->operation_type == OperationType::Mul) {
					std::string left = lhs.get()->ToString();
					ss << left.substr(1, left.size() - 2);
				}
			}
			else {
				ss << lhs.get()->ToString();
			}
			
		}

		if (operation_type == OperationType::Add || operation_type == OperationType::Sub) {
			if (lhs_parentesis != nullptr) {
				BinaryOperation* left_binary_op = dynamic_cast<BinaryOperation*>(lhs_parentesis->body.get());
				if (left_binary_op != nullptr && (left_binary_op->operation_type == OperationType::Mul || left_binary_op->operation_type == OperationType::Div)) {
					std::string left = lhs.get()->ToString();
					ss << left.substr(1, left.size() - 2);
				}
			}
			else {
				ss << lhs.get()->ToString();
			}
			
		}

		if (operation_type == OperationType::Div) {
			if (lhs_parentesis != nullptr) {
				BinaryOperation* left_binary_op = dynamic_cast<BinaryOperation*>(lhs_parentesis->body.get());
				if (left_binary_op != nullptr && (left_binary_op->operation_type == OperationType::Mul || left_binary_op->operation_type == OperationType::Div)) {
					std::string left = lhs.get()->ToString();
					ss << left.substr(1, left.size() - 2);
				}
			}
			else {
				ss << lhs.get()->ToString();
			}
		}

		ss << operation_type;

		if (operation_type == OperationType::Mul) {
			if (rhs_parentesis != nullptr) {
				BinaryOperation* right_binary_op = dynamic_cast<BinaryOperation*>(rhs_parentesis->body.get());
				if (right_binary_op != nullptr && (right_binary_op->operation_type == OperationType::Mul || right_binary_op->operation_type == OperationType::Div)) {
					std::string right = rhs.get()->ToString();
					ss << right.substr(1, right.size() - 2);
					return ss.str();
				}
			}
			ss << rhs.get()->ToString();
			return ss.str();
		}

		if (operation_type == OperationType::Add || operation_type == OperationType::Sub) {
			if (rhs_parentesis != nullptr) {
				BinaryOperation* right_binary_op = dynamic_cast<BinaryOperation*>(rhs_parentesis->body.get());
				if (right_binary_op != nullptr) {
					std::string right = rhs.get()->ToString();
					ss << right.substr(1, right.size() - 2);
					return ss.str();
				}
			}
			ss << rhs.get()->ToString();
			return ss.str();
		}

		if (operation_type == OperationType::Div) {
			ss << rhs.get()->ToString();
			return ss.str();
		}

		return ss.str();
	}

	IFormula::Value BinaryOperation::Evaluate(const ISheet& sheet) const {
		auto lhs_res = lhs.get()->Evaluate(sheet);
		if (std::holds_alternative<FormulaError>(lhs_res)) {
			return lhs_res;
		}
		double l_val = std::get<double>(lhs_res);

		auto rhs_res = rhs.get()->Evaluate(sheet);
		if (std::holds_alternative<FormulaError>(rhs_res)) {
			return rhs_res;
		}
		double r_val = std::get<double>(rhs_res);
		double result;
		
		switch (operation_type) {
		case OperationType::Add: 
			result = l_val + r_val;
			break;
		case OperationType::Sub: 
			result = l_val - r_val;
			break;
		case OperationType::Mul: 
			result = l_val * r_val;
			break;
		case OperationType::Div:
			if (r_val != 0) {
				result = l_val / r_val;
			}
			else {
				return FormulaError::Category::Div0;
			}
		}
		
		if (!std::isfinite(result)) {
			return FormulaError::Category::Div0;
		}

		return result;
	}

	StatementType BinaryOperation::Type() const {
		return statement_type;
	}

	std::string CellOperation::ToString() const {
		std::stringstream ss;
		if (pos.IsValid()) {
			ss << pos.ToString();
		}
		else {
			ss << FormulaError::Category::Ref;
		}
		
		return ss.str();
	}

	IFormula::Value CellOperation::Evaluate(const ISheet& sheet) const {

		if (!pos.IsValid()) {
			return FormulaError::Category::Ref;
		}
		
		auto cell = sheet.GetCell(pos);
		if (cell == nullptr) {
			return IFormula::Value(0.0);
		}

		auto cell_value = cell->GetValue();
		if (std::holds_alternative<double>(cell_value)) {
			return IFormula::Value(std::get<double>(cell_value));
		}
		else if (std::holds_alternative<std::string>(cell_value)) {
			if (std::get<std::string>(cell_value).empty()) {
				return IFormula::Value(0.0);
			}
		}
		else if (std::holds_alternative<FormulaError>(cell_value)) {
			return std::get<FormulaError>(cell_value);
		}


		return FormulaError::Category::Value;
	}

	StatementType CellOperation::Type() const {
		return statement_type;
	}


	std::string ParentesisOperation::ToString() const {
		std::stringstream ss;
		ValueOperation* value_ptr = dynamic_cast<ValueOperation*>(body.get());
		ParentesisOperation* parentesis_ptr = dynamic_cast<ParentesisOperation*>(body.get());
		if (value_ptr != nullptr || parentesis_ptr != nullptr) {
			ss << body.get()->ToString();
		}
		else {
			ss << '(' << body.get()->ToString() << ')';
		}
		return ss.str();
	}

	IFormula::Value ParentesisOperation::Evaluate(const ISheet& sheet) const {
		return body.get()->Evaluate(sheet);
	}

	StatementType ParentesisOperation::Type() const {
		return statement_type;
	}

}
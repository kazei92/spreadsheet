#include "cell.h"

Cell::Cell(std::string expression, const ISheet& sheet_) : raw_expression(expression), sheet(sheet_) {
	if (raw_expression[0] == kFormulaSign) {
		is_formula = true;
		formula = ParseFormula(raw_expression.substr(1));
	}
	else {
		is_formula = false;
		if (raw_expression.size() > 0 && raw_expression[0] == kEscapeSign) {
			value = raw_expression.substr(1);
		}
		else {
			try {
				value = std::stod(raw_expression);
			}
			catch (const std::invalid_argument& err) {
				value = raw_expression;
			}
		}
	}
}

void Cell::UpdateValueIfFormula() {
	if (is_formula) {
		auto formula_eval_res = formula.get()->Evaluate(sheet);
		if (std::holds_alternative<double>(formula_eval_res)) {
			value = std::get<double>(formula_eval_res);
		}
		else if (std::holds_alternative<FormulaError>(formula_eval_res)) {
			value = std::get<FormulaError>(formula_eval_res);
		}
		raw_expression = "=" + formula.get()->GetExpression();
	}
}

Cell::Value Cell::GetValue() const {
	return value; 
}

std::string Cell::GetText() const { return raw_expression; }
std::vector<Position> Cell::GetReferencedCells() const {
	if (IsFormula()) {
		return formula.get()->GetReferencedCells();
	}
	else {
		return {};
	}
}

bool Cell::IsFormula() const { return is_formula; }
IFormula* Cell::GetFormula() { return formula.get(); }

void Cell::SetValue(Value new_value) {
	value = new_value;
}

void Cell::SetText(std::string new_text) {
	raw_expression = new_text;
}

void Cell::AddDependentCell(const Position& pos) {
	dependent.push_back(pos);
}

std::vector<Position>& Cell::GetDependentCells() {
	return dependent;
}

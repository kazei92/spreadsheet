#pragma once
#include "formula.h"

class Cell : public ICell {
public:
    using Value = std::variant<std::string, double, FormulaError>;
    explicit Cell(std::string expression, const ISheet& sheet_);
    Cell() = default;
    Value GetValue() const;
    void SetValue(Value new_value);
    void SetText(std::string new_text);
    std::string GetText() const;
    std::vector<Position> GetReferencedCells() const;
    bool IsFormula() const;
    IFormula* GetFormula();
    void AddDependentCell(const Position& pos);
    std::vector<Position>& GetDependentCells();
    void UpdateValueIfFormula();
private:
    bool is_formula;
    const ISheet& sheet;
    Value value;
    std::string raw_expression;
    std::unique_ptr<IFormula> formula;
    std::vector<Position> dependent;
};

std::ostream& operator<<(std::ostream& stream, const Cell::Value& value);
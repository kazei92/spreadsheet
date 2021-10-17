#pragma once
#include <memory>
#include <vector>
#include "antlr4-runtime.h"
#include "FormulaLexer.h"
#include "FormulaListener.h"
#include "FormulaParser.h"
#include "common.h"
#include "statement.h"

class Formula : public IFormula {
public:
    using Value = std::variant<double, FormulaError>;
    virtual Value Evaluate(const ISheet& sheet) const;
    virtual std::string GetExpression() const;
    virtual std::vector<Position> GetReferencedCells() const;
    virtual HandlingResult HandleInsertedRows(int before, int count = 1);
    virtual HandlingResult HandleInsertedCols(int before, int count = 1);
    virtual HandlingResult HandleDeletedRows(int first, int count = 1);
    virtual HandlingResult HandleDeletedCols(int first, int count = 1);
    virtual void ModifyStatementRowPositions(Ast::Statement* root, int before, int count);
    virtual void ModifyStatementColumnPositions(Ast::Statement* root, int before, int count);
    virtual void DeleteStatementRowPositions(Ast::Statement* root, int first, int count);
    virtual void DeleteStatementColumnPositions(Ast::Statement* root, int first, int count);
    std::unique_ptr<Ast::Statement> statement;
    std::vector<Position> references;
};
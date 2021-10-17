#pragma once
#include "common.h"

namespace Ast {
	enum class OperationType { Add, Sub, Mul, Div };
	enum class StatementType { Value, UnaryOperation, BinaryOperation, Cell, Parentesis };


    struct Statement {
        virtual ~Statement() = default;
        virtual IFormula::Value Evaluate(const ISheet& sheet) const = 0;
        virtual std::string ToString() const = 0;
        virtual StatementType Type() const = 0;
    };

    struct UnaryOperation : public Statement {
        IFormula::Value Evaluate(const ISheet& sheet) const;
        std::string ToString() const;
        StatementType Type() const;

        OperationType operation_type;
        std::unique_ptr<Statement> rhs;
        StatementType statement_type;
    };

    struct ValueOperation : public Statement {
        IFormula::Value Evaluate(const ISheet& sheet) const;
        std::string ToString() const;
        StatementType Type() const;

        IFormula::Value value;
        StatementType statement_type;
    };

    struct BinaryOperation : public Statement {
        IFormula::Value Evaluate(const ISheet& sheet) const;
        std::string ToString() const;
        StatementType Type() const;

        OperationType operation_type;
        StatementType statement_type;
        std::unique_ptr<Statement> lhs, rhs;
    };

    struct CellOperation : public Statement {
        IFormula::Value Evaluate(const ISheet& sheet) const;
        std::string ToString() const;
        StatementType Type() const;

        StatementType statement_type;
        Position pos;
    };

    struct ParentesisOperation : public Statement {
        IFormula::Value Evaluate(const ISheet& sheet) const;
        std::string ToString() const;
        StatementType Type() const;

        StatementType statement_type;
        std::unique_ptr<Statement> body;
    };

}

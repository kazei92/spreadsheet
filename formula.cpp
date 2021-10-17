#include "formula.h"
#include "statement.h"

Formula::Value Formula::Evaluate(const ISheet& sheet) const {
    return statement.get()->Evaluate(sheet);
}

std::string Formula::GetExpression() const {
    return statement.get()->ToString();
}
std::vector<Position> Formula::GetReferencedCells() const {
    return references;
}

void Formula::ModifyStatementRowPositions(Ast::Statement* root, int before, int count) {
    auto cell_op = dynamic_cast<Ast::CellOperation*>(root);
    if (cell_op != nullptr) {
        if (cell_op->pos.row >= before) {
            cell_op->pos.row += count;
        }
        return;
    }
    auto binary_op = dynamic_cast<Ast::BinaryOperation*>(root);
    if (binary_op != nullptr) {
        ModifyStatementRowPositions(binary_op->lhs.get(), before, count);
        ModifyStatementRowPositions(binary_op->rhs.get(), before, count);
        return;
    }
    auto unary_op = dynamic_cast<Ast::UnaryOperation*>(root);
    if (unary_op != nullptr) {
        ModifyStatementRowPositions(unary_op->rhs.get(), before, count);
        return;
    }
    auto parentesis_op = dynamic_cast<Ast::ParentesisOperation*>(root);
    if (parentesis_op != nullptr) {
        ModifyStatementRowPositions(parentesis_op->body.get(), before, count);
        return;
    }
}

Formula::HandlingResult Formula::HandleInsertedRows(int before, int count) {
    int n_of_changed = 0;
    for (auto& pos : references) {
        if (pos.row >= before) {
            ++n_of_changed;
            pos.row += count;
        }
    }
    ModifyStatementRowPositions(statement.get(), before, count);

    if (!n_of_changed) {
        return HandlingResult::NothingChanged;
    }
    return HandlingResult::ReferencesRenamedOnly;  
}

void Formula::ModifyStatementColumnPositions(Ast::Statement* root, int before, int count) {
    auto cell_op = dynamic_cast<Ast::CellOperation*>(root);
    if (cell_op != nullptr) {
        if (cell_op->pos.col >= before) {
            cell_op->pos.col += count;
        }
        return;
    }
    auto binary_op = dynamic_cast<Ast::BinaryOperation*>(root);
    if (binary_op != nullptr) {
        ModifyStatementColumnPositions(binary_op->lhs.get(), before, count);
        ModifyStatementColumnPositions(binary_op->rhs.get(), before, count);
        return;
    }
    auto unary_op = dynamic_cast<Ast::UnaryOperation*>(root);
    if (unary_op != nullptr) {
        ModifyStatementColumnPositions(unary_op->rhs.get(), before, count);
        return;
    }
    auto parentesis_op = dynamic_cast<Ast::ParentesisOperation*>(root);
    if (parentesis_op != nullptr) {
        ModifyStatementColumnPositions(parentesis_op->body.get(), before, count);
        return;
    }
}

Formula::HandlingResult Formula::HandleInsertedCols(int before, int count) {
    int n_of_changed = 0;
    for (auto& pos : references) {
        if (pos.col >= before) {
            ++n_of_changed;
            pos.col += count;
        }
    }
    ModifyStatementColumnPositions(statement.get(), before, count);
    if (!n_of_changed) {
        return HandlingResult::NothingChanged;
    }

    return HandlingResult::ReferencesRenamedOnly;
}

void Formula::DeleteStatementRowPositions(Ast::Statement* root, int first, int count) {
    auto cell_op = dynamic_cast<Ast::CellOperation*>(root);
    if (cell_op != nullptr) {
        if (cell_op->pos.row >= first + count) {
            cell_op->pos.row -= count;
            return;
        }
        if (cell_op->pos.row >= first) {
            cell_op->pos.row = -1;
            return;
        }

    }
    auto binary_op = dynamic_cast<Ast::BinaryOperation*>(root);
    if (binary_op != nullptr) {
        DeleteStatementRowPositions(binary_op->lhs.get(), first, count);
        DeleteStatementRowPositions(binary_op->rhs.get(), first, count);
        return;
    }
    auto unary_op = dynamic_cast<Ast::UnaryOperation*>(root);
    if (unary_op != nullptr) {
        DeleteStatementRowPositions(unary_op->rhs.get(), first, count);
        return;
    }
    auto parentesis_op = dynamic_cast<Ast::ParentesisOperation*>(root);
    if (parentesis_op != nullptr) {
        DeleteStatementRowPositions(parentesis_op->body.get(), first, count);
        return;
    }
}
void Formula::DeleteStatementColumnPositions(Ast::Statement* root, int first, int count) {
    auto cell_op = dynamic_cast<Ast::CellOperation*>(root);
    if (cell_op != nullptr) {
        if (cell_op->pos.col >= first + count) {
            cell_op->pos.col -= count;
            return;
        }
        if (cell_op->pos.col >= first) {
            cell_op->pos.col = -1;
            return;
        }
        
    }
    auto binary_op = dynamic_cast<Ast::BinaryOperation*>(root);
    if (binary_op != nullptr) {
        DeleteStatementColumnPositions(binary_op->lhs.get(), first, count);
        DeleteStatementColumnPositions(binary_op->rhs.get(), first, count);
        return;
    }
    auto unary_op = dynamic_cast<Ast::UnaryOperation*>(root);
    if (unary_op != nullptr) {
        DeleteStatementColumnPositions(unary_op->rhs.get(), first, count);
        return;
    }
    auto parentesis_op = dynamic_cast<Ast::ParentesisOperation*>(root);
    if (parentesis_op != nullptr) {
        DeleteStatementColumnPositions(parentesis_op->body.get(), first, count);
        return;
    }

}
Formula::HandlingResult Formula::HandleDeletedRows(int first, int count) {
    int n_of_changed = 0;
    int n_of_deleted = 0;
    for (auto it = begin(references); it != end(references);) {
        if (it->row >= first + count) {
            ++n_of_changed;
            it->row -= count;
            ++it;
            continue;
        }
        if (it->row >= first) {
            ++n_of_deleted;
            it = references.erase(it);
            continue;
        }
        ++it;
    }
    DeleteStatementRowPositions(statement.get(), first, count);
    if (n_of_deleted) {
        return HandlingResult::ReferencesChanged;
    }
    else if (n_of_changed) {
        return HandlingResult::ReferencesRenamedOnly;
    }
    else {
        return HandlingResult::NothingChanged;
    }
}

Formula::HandlingResult Formula::HandleDeletedCols(int first, int count) {
    int n_of_changed = 0;
    int n_of_deleted = 0;
    for (auto it = begin(references); it != end(references);) {
        if (it->col >= first + count) {
            ++n_of_changed;
            it->col -= count;
            ++it;
            continue;
        }
        if (it->col >= first) {
            ++n_of_deleted;
            it = references.erase(it);
            continue;
        }
        ++it;
    }
    DeleteStatementColumnPositions(statement.get(), first, count);
    if (n_of_deleted) {
        return HandlingResult::ReferencesChanged;
    }
    else if (n_of_changed) {
        return HandlingResult::ReferencesRenamedOnly;
    }
    else {
        return HandlingResult::NothingChanged;
    }
}

class BailErrorListener : public antlr4::BaseErrorListener {
public:
    void syntaxError(antlr4::Recognizer* /* recognizer */, antlr4::Token* /* offendingSymbol */, size_t /* line */,
        size_t /* charPositionInLine */, const std::string& msg, std::exception_ptr /* e */
    ) override {
        throw FormulaException("Error when lexing: " + msg);
    }
};


class StatementListener : public FormulaListener {
    std::stack<std::unique_ptr<Ast::Statement>> statement_stack;
    std::vector<Position> references;

    virtual void enterMain(FormulaParser::MainContext* /*ctx*/) override {}
    virtual void exitMain(FormulaParser::MainContext* /*ctx*/) override {}

    virtual void enterUnaryOp(FormulaParser::UnaryOpContext* /*ctx*/) override {}
    virtual void exitUnaryOp(FormulaParser::UnaryOpContext* ctx) override {
        auto unary_op = std::make_unique<Ast::UnaryOperation>();
        unary_op->operation_type = ctx->ADD() ? Ast::OperationType::Add : Ast::OperationType::Sub;
        unary_op->rhs = std::move(statement_stack.top());
        statement_stack.pop();
        statement_stack.push(move(unary_op));
    }

    virtual void enterParens(FormulaParser::ParensContext* ctx) override {}
    virtual void exitParens(FormulaParser::ParensContext* ctx) override  {
        auto parentesis_op = std::make_unique<Ast::ParentesisOperation>();
        parentesis_op->body = std::move(statement_stack.top());
        statement_stack.pop();
        statement_stack.push(std::move(parentesis_op));
    }

    virtual void enterLiteral(FormulaParser::LiteralContext* ctx) override  {}
    virtual void exitLiteral(FormulaParser::LiteralContext* ctx) override  {
        auto value_op = std::make_unique<Ast::ValueOperation>();
        value_op->value = std::stod(ctx->NUMBER()->getText());
        statement_stack.push(move(value_op));
    }

    virtual void enterCell(FormulaParser::CellContext* ctx) override {}
    virtual void exitCell(FormulaParser::CellContext* ctx) override {
        auto cell_op = std::make_unique<Ast::CellOperation>();
        Position pos = Position::FromString(ctx->CELL()->getText());
        cell_op.get()->pos = pos;
        references.push_back(pos);
        statement_stack.push(std::move(cell_op));
    }

    virtual void enterBinaryOp(FormulaParser::BinaryOpContext* ctx) override {}
    virtual void exitBinaryOp(FormulaParser::BinaryOpContext* ctx) override {
        auto binary_op = std::make_unique<Ast::BinaryOperation>();

        auto rhs = std::move(statement_stack.top());
        statement_stack.pop();
        auto lhs = std::move(statement_stack.top());
        statement_stack.pop();


        if (ctx->ADD()) {
            binary_op->operation_type = Ast::OperationType::Add;
        }
        else if (ctx->SUB()) {
            binary_op->operation_type = Ast::OperationType::Sub;
        }
        else if (ctx->DIV()) {
            binary_op->operation_type = Ast::OperationType::Div;
        }
        else {
            binary_op->operation_type = Ast::OperationType::Mul;
        }

        binary_op->rhs = std::move(rhs);
        binary_op->lhs = std::move(lhs);
        statement_stack.push(std::move(binary_op));
    }

    virtual void enterEveryRule(antlr4::ParserRuleContext* /*ctx*/) override {}
    virtual void exitEveryRule(antlr4::ParserRuleContext* /*ctx*/) override {}
    virtual void visitTerminal(antlr4::tree::TerminalNode* /*node*/) override {}
    virtual void visitErrorNode(antlr4::tree::ErrorNode* /*node*/) override {}
public:
    std::unique_ptr<Ast::Statement> GetResult() {
        return std::move(statement_stack.top());
    }
    std::vector<Position> GetReferences() {
        std::sort(begin(references), end(references));
        references.erase(std::unique(begin(references), end(references)), end(references));
        return std::move(references);
    }
};


std::unique_ptr<IFormula> ParseFormula(std::string expression) {
	antlr4::ANTLRInputStream input(expression);
	FormulaLexer lexer(&input);
    BailErrorListener error_listener;
    lexer.removeErrorListeners();
    lexer.addErrorListener(&error_listener);
	antlr4::CommonTokenStream tokens(&lexer);
	FormulaParser parser(&tokens);
    auto error_handler = std::make_shared<antlr4::BailErrorStrategy>();
    parser.setErrorHandler(error_handler);
    parser.removeErrorListeners();
    antlr4::tree::ParseTree* tree;
    try {
        tree = parser.main();
    }
    catch (const std::exception& ex) {
        throw FormulaException(ex.what());
    }
    
    StatementListener statement_listener;
    antlr4::tree::ParseTreeWalker::DEFAULT.walk(&statement_listener, tree);
    
    auto formula = std::make_unique<Formula>();
    formula.get()->statement = statement_listener.GetResult();
    formula.get()->references = statement_listener.GetReferences();

    for (const auto& ref : formula.get()->references) {
        if (!ref.IsValid()) {
            throw FormulaException("invalid ref in formula");
        }
    }
    
    return formula;
}

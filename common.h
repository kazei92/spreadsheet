#pragma once
#include <iosfwd>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <variant>
#include <vector>
#include <iostream>
#include <algorithm>
#include <sstream>


struct Position {
    int row = 0;
    int col = 0;
    bool operator==(const Position& rhs) const;
    bool operator<(const Position& rhs) const;
    bool IsValid() const;
    std::string ToString() const;
    static Position FromString(std::string_view str);
    static const int kMaxRows = 16384;
    static const int kMaxCols = 16384;
};


struct Size {
    int rows = 0;
    int cols = 0;
    bool operator==(const Size& rhs) const;
};


class FormulaError {
public:
    enum class Category { Ref, Value, Div0,};
    FormulaError(Category category);
    Category GetCategory() const;
    bool operator==(FormulaError rhs) const;
    std::string_view ToString() const;
private:
    Category category_;
};

std::ostream& operator<<(std::ostream& output, FormulaError fe);


class InvalidPositionException : public std::out_of_range {
public:
    using std::out_of_range::out_of_range;
};


class FormulaException : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};


class CircularDependencyException : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};


class TableTooBigException : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};


class ICell {
public:
    using Value = std::variant<std::string, double, FormulaError>;
    virtual ~ICell() = default;
    virtual Value GetValue() const = 0;
    virtual std::string GetText() const = 0;
    virtual std::vector<Position> GetReferencedCells() const = 0;
};

inline constexpr char kFormulaSign = '=';
inline constexpr char kEscapeSign = '\'';


class ISheet {
public:
    virtual ~ISheet() = default;
    virtual void SetCell(Position pos, std::string text) = 0;
    virtual const ICell* GetCell(Position pos) const = 0;
    virtual ICell* GetCell(Position pos) = 0;
    virtual void ClearCell(Position pos) = 0;
    virtual void InsertRows(int before, int count = 1) = 0;
    virtual void InsertCols(int before, int count = 1) = 0;
    virtual void DeleteRows(int first, int count = 1) = 0;
    virtual void DeleteCols(int first, int count = 1) = 0;
    virtual Size GetPrintableSize() const = 0;
    virtual void PrintValues(std::ostream& output) const = 0;
    virtual void PrintTexts(std::ostream& output) const = 0;
};


std::unique_ptr<ISheet> CreateSheet();


class IFormula {
public:
    using Value = std::variant<double, FormulaError>;
    enum class HandlingResult { NothingChanged, ReferencesRenamedOnly, ReferencesChanged };
    virtual ~IFormula() = default;
    virtual Value Evaluate(const ISheet& sheet) const = 0;
    virtual std::string GetExpression() const = 0;
    virtual std::vector<Position> GetReferencedCells() const = 0;
    virtual HandlingResult HandleInsertedRows(int before, int count = 1) = 0;
    virtual HandlingResult HandleInsertedCols(int before, int count = 1) = 0;
    virtual HandlingResult HandleDeletedRows(int first, int count = 1) = 0;
    virtual HandlingResult HandleDeletedCols(int first, int count = 1) = 0;
};

std::unique_ptr<IFormula> ParseFormula(std::string expression);

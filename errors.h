#pragma once
#include <string>
#include <stdexcept>
#include <iostream>
#include <string_view>

// Описывает ошибки, которые могут возникнуть при вычислении формулы.
class FormulaError {
public:
    enum class Category {
        Ref,  // ссылка на несуществующую ячейку (например, она была удалена)
        Value,  // ячейка не может быть трактована как число
        Div0,  // в результате вычисления возникло деление на ноль
    };

    FormulaError(Category category);
    Category GetCategory() const;
    bool operator==(FormulaError rhs) const;
    std::string_view ToString() const;
private:
    Category category_;
};

std::ostream& operator<<(std::ostream& output, FormulaError fe);


// Исключение, выбрасываемое при попытке передать в метод некорректную позицию
class InvalidPositionException : public std::out_of_range {
public:
    using std::out_of_range::out_of_range;
};

// Исключение, выбрасываемое при попытке задать синтаксически некорректную
// формулу
class FormulaException : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

// Исключение, выбрасываемое при попытке задать формулу, которая приводит к
// циклической зависимости между ячейками
class CircularDependencyException : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

// Исключение, выбрасываемое если вставка строк/столбцов в таблицу приведёт к
// ячейке с позицией больше максимально допустимой
class TableTooBigException : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};


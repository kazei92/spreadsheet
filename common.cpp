#include "common.h"
#include "sheet.h"

using namespace std;

bool Position::operator==(const Position& rhs) const { return row == rhs.row && col == rhs.col; }
bool Position::operator<(const Position& rhs) const { return make_pair(row, col) < make_pair(rhs.row, rhs.col); }
bool Position::IsValid() const { return 0 <= row && row < kMaxRows && 0 <= col && col < kMaxCols; }

string Position::ToString() const {
    string str;
    if (!IsValid()) {
        return str;
    }

    int tmp_col = col;

    while (tmp_col > 25) {
        str.push_back('A' + tmp_col % 26);
        tmp_col = tmp_col / 26 - 1;
    }

    str.push_back('A' + tmp_col);
    reverse(str.begin(), str.end());

    str += to_string(row + 1);

    return str;
}

static string_view::iterator cell_name_partition_point(string_view sv) {
    if (sv.size() < 2 || !isalpha(sv[0])) {
        return sv.end();
    }

    bool valid = is_partitioned(sv.begin(), sv.end(), [](const char c) { return isupper(c); });
    auto it = partition_point(sv.begin(), sv.end(), [](const char c) { return isupper(c); });
    valid &= all_of(it, sv.end(), [](const char c) { return isdigit(c); });
    if (valid) {
        return it;
    }

    return sv.end();
}

Position Position::FromString(string_view sv) {
    Position pos{ 0, 0 };

    auto point_it = cell_name_partition_point(sv);

    if (point_it == sv.end()) {
        pos.row = -1;
        pos.col = -1;
        return pos;
    }

    
    pos.row = atoi(string(point_it, sv.end()).c_str()) - 1;  // possible UB

    string_view columns_view = sv.substr(0, distance(sv.begin(), point_it));
    int power = 1;
    for (int i = columns_view.size() - 1; i >= 0; i--) {
        if (power == 1) {
            pos.col = columns_view[i] - 'A';
        }
        else {
            pos.col += (1 + columns_view[i] - 'A') * power;
        }
        power *= 26;
    };

    return pos;
}

bool Size::operator==(const Size& rhs) const {
    return rows == rhs.rows && cols == rhs.cols;
}

FormulaError::FormulaError(Category category) : category_(category) {}
FormulaError::Category FormulaError::GetCategory() const { return category_; }
bool FormulaError::operator==(FormulaError rhs) const { return category_ == rhs.category_; }
std::string_view FormulaError::ToString() const {
    switch (category_) {
    case Category::Ref:
        return "#REF!";
    case Category::Div0:
        return "#DIV/0!";
    case Category::Value:
        return "#VALUE!";
    default:
        return "";
    }
}

std::ostream& operator<<(std::ostream& output, FormulaError fe) {
    output << fe.ToString();
    return output;
}

std::unique_ptr<ISheet> CreateSheet() {
    return std::make_unique<Sheet>();
}


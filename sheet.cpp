#include "sheet.h"

void Sheet::SetToSize(int row_size, int col_size) {
	size.rows = row_size;
	size.cols = col_size;
}

void Sheet::UpdateSizeAfterCellInsertion(Cell* cell, const Position& pos) {
	if (pos.row + 1 > size.rows) { SetToSize(pos.row + 1, size.cols); }
	if (pos.col + 1 > size.cols) { SetToSize(size.rows, pos.col + 1); }

	if (cell->IsFormula()) {
		for (const auto& ref : cell->GetReferencedCells()) {
			if (ref.row > size.rows) { SetToSize(ref.row, size.cols); }
			if (ref.col > size.cols) { SetToSize(size.rows, ref.col); }
		}
	}
}

void Sheet::UpdateDependenedCells(const Position& pos, Cell* cell) {
	auto cell_formula = cell->GetFormula();

	for (const auto& dep_pos : cell_formula->GetReferencedCells()) {
		if (GetCell(dep_pos) == nullptr) {
			SetCell(dep_pos, "");
		}
		else {
			Cell* ref_cel = GetCell(dep_pos);
			ref_cel->AddDependentCell(pos);
		}
	}
}

void Sheet::UpdateCellText(Cell* cell) {
	auto cell_formula = cell->GetFormula();
	cell->SetText("=" + cell_formula->GetExpression());
}

void Sheet::UpdateCellValue(Cell* cell) {
	auto cell_formula = cell->GetFormula();
	auto formula_eval_res = cell_formula->Evaluate(*this);
	if (std::holds_alternative<FormulaError>(formula_eval_res)) {
		cell->SetValue(Cell::Value(std::get<FormulaError>(formula_eval_res)));
	}
	else if (std::holds_alternative<double>(formula_eval_res)) {
		cell->SetValue(Cell::Value(std::get<double>(formula_eval_res)));
	}
}

void Sheet::EvaluateFormula(const Position& pos, Cell* cell) {
	if (cell->IsFormula()) {
		UpdateDependenedCells(pos, cell);
		UpdateCellText(cell);
		UpdateCellValue(cell);
	}
}

void Sheet::ThrowErrorIfInvalidPosition(const Position& pos) const {
	if (!pos.IsValid()) {
		std::stringstream message;
		message << "invalid position: " << pos.ToString();
		throw InvalidPositionException(message.str());
	}
}

void Sheet::CheckForCircularDependency(const Position& pos, Cell* cell) {
	if (cell != nullptr) {
		auto referenced_cells = cell->GetReferencedCells();
		for (const auto& ref_pos : referenced_cells) {
			if (ref_pos == pos) {
				throw CircularDependencyException("");
			}
			else {
				CheckForCircularDependency(pos, GetCell(ref_pos));
			}
		}
	}
}

void Sheet::SetCell(Position pos, std::string text) {
	ThrowErrorIfInvalidPosition(pos);
	auto cell_ptr = std::make_unique<Cell>(text, *this);
	Cell* cell = cell_ptr.get();
	CheckForCircularDependency(pos, cell);
	
	data[pos.row][pos.col] = std::move(cell_ptr);
	UpdateSizeAfterCellInsertion(cell, pos);
	EvaluateFormula(pos, cell);
}

const Cell* Sheet::GetCell(Position pos) const {
	ThrowErrorIfInvalidPosition(pos);
	if (data.find(pos.row) != end(data)) {
		if (data.at(pos.row).find(pos.col) != end(data.at(pos.row))) {
			return data.at(pos.row).at(pos.col).get();
		}
	}
	return nullptr;
}

Cell* Sheet::GetCell(Position pos) {
	ThrowErrorIfInvalidPosition(pos);
	if (data.find(pos.row) != end(data)) {
		if (data.at(pos.row).find(pos.col) != end(data.at(pos.row))) {
			return data.at(pos.row).at(pos.col).get();
		}
	}
	return nullptr;
}

void Sheet::ClearCell(Position pos) {
	ThrowErrorIfInvalidPosition(pos);
	if (data.find(pos.row) != end(data)) {
		if (data.at(pos.row).find(pos.col) != end(data.at(pos.row))) {
			data.at(pos.row).at(pos.col) = nullptr;
		}
	}
}

void Sheet::ThrowIfTooBigAfterInsertion(int row_count, int col_count) const {
	Position pos;
	pos.row = size.rows + row_count;
	if (!pos.IsValid()) {
		throw TableTooBigException("table will be too big after insertion of number of rows: " + std::to_string(row_count));
	}
	pos.col = size.cols + col_count;
	if (!pos.IsValid()) {
		throw TableTooBigException("table will be too big after insertion of number of cols: " + std::to_string(col_count));
	}
}

void Sheet::InsertRows(int before, int count) {
	ThrowIfTooBigAfterInsertion(count, 0);
	for (auto it_r = begin(data); it_r != end(data); ++it_r) {
		for (auto it_c = begin(data.at(it_r->first)); it_c != end(data.at(it_r->first)); ++it_c) {
			it_c->second.get()->GetFormula()->HandleInsertedRows(before, count);
		}
	}
}
void Sheet::InsertCols(int before, int count) {
	ThrowIfTooBigAfterInsertion(0, count);
	for (auto it_r = begin(data); it_r != end(data); ++it_r) {
		for (auto it_c = begin(data.at(it_r->first)); it_c != end(data.at(it_r->first)); ++it_c) {
			it_c->second.get()->GetFormula()->HandleInsertedCols(before, count);
		}
	}
}

void Sheet::UpdateDependenedCellPositions(Cell* cell, int row_count, int col_count) {
	auto& dependent_cells_pos = cell->GetDependentCells();
	for (auto& pos : dependent_cells_pos) {
		pos.row -= row_count;
		pos.col -= col_count;
	}
}

void Sheet::UpdateDeletedCellDependencies(Cell* cell, int first_row, int row_count, int first_col, int col_count) {
	auto& dependent_cells = cell->GetDependentCells();
	for (const auto& pos : dependent_cells) {
		Cell* dependent_cell = GetCell(pos);
		dependent_cell->UpdateValueIfFormula();
		if (!dependent_cell->GetDependentCells().empty()) {
			UpdateDependenedCellPositions(dependent_cell, row_count, col_count);
			UpdateDeletedCellDependencies(dependent_cell, first_row, row_count, first_col, col_count);
		}
	}
}

std::vector<Sheet::CellPtr> Sheet::ExtractDeletedRows(int first, int count) {
	std::vector<CellPtr> deleted_cells;
	for (int row_num = first; row_num < first + count; ++row_num) {
		if (data.find(row_num) != end(data)) {
			auto node = data.extract(row_num);
			for (auto& row : node.mapped()) {
				deleted_cells.push_back(std::move(row.second));
			}
		}
	}
	size.rows -= count;
	return deleted_cells;
}

void Sheet::ChangeRowIndexes(int first, int count) {
	for (int i = first + count; i < size.rows + 1; ++i) {
		if (data.find(i) != end(data)) {
			data[i - count] = std::move(data.at(i));
			data.erase(i);
		}
	}
}

void Sheet::UpdateNonDeletedRows(int first, int count) {
	for (int row_num = first + count; row_num < size.rows; ++row_num) {
		if (data.find(row_num) != end(data)) {
			auto& row = data.at(row_num);
			for (auto& item : row) {
				Cell* cell = item.second.get();
				if (cell->IsFormula()) {
					cell->GetFormula()->HandleDeletedRows(first, count);
				}
			}
		}
	}
}

void Sheet::UpdateDependendCells(std::vector<CellPtr>& deleted_cells, int first_row, int row_count, int first_col, int col_count) {
	for (auto& cell_ptr : deleted_cells) {
		Cell* deleted_cell = cell_ptr.get();
		UpdateDependenedCellPositions(deleted_cell, row_count, col_count);
		UpdateDeletedCellDependencies(deleted_cell, first_row, row_count, first_col, col_count);
	}
}

void Sheet::DeleteRows(int first, int count) {
	UpdateNonDeletedRows(first, count);
	std::vector<CellPtr>& deleted_cells = ExtractDeletedRows(first, count);
	ChangeRowIndexes(first, count);
	UpdateDependendCells(deleted_cells, first, count, 0, 0);
}

std::vector<Sheet::CellPtr> Sheet::ExtractDeletedCols(int first, int count) {
	std::vector<CellPtr> deleted_cells;
	for (auto& row : data) {
		for (int col_num = first; col_num < first + count; ++col_num) {
			if (row.second.find(col_num) != end(row.second)) {
				auto node = row.second.extract(col_num);
				deleted_cells.push_back(std::move(node.mapped()));
			}
		}
	}
	return deleted_cells;
}

void Sheet::UpdateNonDeletedCols(int first, int count) {
	for (auto& row : data) {
		for (int col_num = first + count; col_num < size.cols; ++col_num) {
			if (row.second.find(col_num) != end(row.second)) {
				Cell* cell = row.second.at(col_num).get();
				if (cell->IsFormula()) {
					cell->GetFormula()->HandleDeletedCols(first, count);
				}
			}
		}
	}
}

void Sheet::ChangeColIndexes(int first, int count) {
	for (auto& row : data) {
		for (int i = first + count; i < size.cols + 1; ++i) {
			if (row.second.find(i) != end(row.second)) {
				row.second[i - count] = std::move(row.second[i]);
				row.second.erase(i);
			}
		}
	}
}

void Sheet::DeleteCols(int first, int count) {
	UpdateNonDeletedCols(first, count);
	std::vector<CellPtr> deleted_cells = ExtractDeletedCols(first, count);
	ChangeColIndexes(first, count);
	UpdateDependendCells(deleted_cells, 0, 0, first, count);
}

Size Sheet::GetPrintableSize() const { return size; }
void Sheet::PrintValues(std::ostream& output) const {
	for (int r = 0; r < size.rows; ++r) {
		int blank_cells = 0;
		for (int c = 0; c < size.cols; ++c) {
			auto cell = GetCell({ r, c });
			if (cell == nullptr) {
				++blank_cells;
				continue; 
			}
			if (cell->IsFormula()) {
				output << cell->GetValue();
			}
			else {
				output << cell->GetText();
			}
			if (c + 1 != size.cols) {
				output << '\t';
			}
		}
		if (blank_cells == size.cols) {
			output << '\t';
		}
		output << '\n';
	}
}

void Sheet::PrintTexts(std::ostream& output) const {
	for (int r = 0; r < size.rows; ++r) {
		int blank_cells = 0;
		for (int c = 0; c < size.cols; ++c) {
			auto cell = GetCell({ r, c });
			if (cell == nullptr) {
				++blank_cells;
				continue; 
			}
			output << cell->GetText();
			if (c+1 != size.cols) {
				output << '\t';
			}
		}
		if (blank_cells == size.cols) {
			output << '\t';
		}
		output << '\n';
	}
}
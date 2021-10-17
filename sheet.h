#pragma once
#include <sstream>
#include <unordered_map>
#include "common.h"
#include "cell.h"

class Sheet : public ISheet {
public:
	using CellPtr = std::unique_ptr<Cell>;
	using Row = std::unordered_map<int, CellPtr>;
public:
	Sheet() = default;
	void SetToSize(int row_size, int col_size);
	void UpdateSizeAfterCellInsertion(Cell* cell, const Position& pos);
	
	void EvaluateFormula(const Position& pos, Cell* cell);
	void UpdateDependenedCells(const Position& pos, Cell* cell);
	void UpdateCellText(Cell* cell);
	void UpdateCellValue(Cell* cell);
	
	void SetCell(Position pos, std::string text);
	const Cell* GetCell(Position pos) const;
	Cell* GetCell(Position pos);
	void ClearCell(Position pos);
	
	void InsertRows(int before, int count = 1);
	void InsertCols(int before, int count = 1);
	
	Size GetPrintableSize() const;
	void PrintValues(std::ostream& output) const;
	void PrintTexts(std::ostream& output) const;

	void ThrowErrorIfInvalidPosition(const Position& pos) const;
	void ThrowIfTooBigAfterInsertion(int row_count = 0, int col_count = 0) const;
	
	void DeleteRows(int first, int count = 1);
	void UpdateDependendCells(std::vector<CellPtr>& deleted_cells, int first_row = 0, int row_count = 0, int first_col = 0, int col_count = 0);
	void UpdateDependenedCellPositions(Cell* cell, int row_count = 0, int col_count = 0);
	void UpdateDeletedCellDependencies(Cell* cell, int first_row = 0, int row_count = 0, int first_col = 0, int col_count = 0);
	std::vector<CellPtr> ExtractDeletedRows(int first, int count);
	void ChangeRowIndexes(int first, int count);
	void UpdateNonDeletedRows(int first, int count);

	void DeleteCols(int first, int count = 1);
	void UpdateNonDeletedCols(int first, int count);
	void ChangeColIndexes(int first, int count);
	std::vector<CellPtr> ExtractDeletedCols(int first, int count);

	void CheckForCircularDependency(const Position& pos, Cell* cell);
private:
	std::unordered_map<int, Row> data;
	Size size;
};
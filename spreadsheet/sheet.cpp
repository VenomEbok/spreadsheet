#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>

using namespace std::literals;

Sheet::~Sheet() {}

void Sheet::SetCell(Position pos, std::string text) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("wrong position");
    }
    if (pos.row>= size_.rows) {
        size_.rows = pos.row+1;
        cells_.resize(size_.rows);
    }
    if (pos.col >= size_.cols) {
        size_.cols = pos.col+1;
    }
    if (cells_[pos.row].size() < static_cast<size_t>(size_.cols)) {
        cells_[pos.row].resize(size_.cols);
    }
    if (GetCell(pos) == nullptr) {
        Cell cell(*this);
        cell.Set(text);
        cell.CheckForLooping(pos, text);
        cells_[pos.row][pos.col] = std::make_unique<Cell>(std::move(cell));
    }
    else {
        cells_[pos.row][pos.col]->CheckForLooping(pos, text);
        cells_[pos.row][pos.col]->Set(text);
    }
    AddReferenceForCell(cells_[pos.row][pos.col].get(), text);
}

const CellInterface* Sheet::GetCell(Position pos) const {
    if (!pos.IsValid()) {
        throw InvalidPositionException("wrong position");
    }
    if (cells_.size() > static_cast<size_t>(pos.row)) {
        if (cells_[pos.row].size() <= static_cast<size_t>(pos.col)) {
            return nullptr;
        }
    }
    else {
        return nullptr;
    }
    if (cells_[pos.row][pos.col].get() == nullptr) {
        return nullptr;
    }
    if (!cells_[pos.row][pos.col]->IsExist()) {
        return nullptr;
    }
    return cells_[pos.row][pos.col].get();
}
CellInterface* Sheet::GetCell(Position pos) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("wrong position");
    }
    if (cells_.size() > static_cast<size_t>(pos.row)) {
        if (cells_[pos.row].size() <= static_cast<size_t>(pos.col)) {
            return nullptr;
        }
    }
    else {
        return nullptr;
    }
    if (cells_[pos.row][pos.col].get() == nullptr) {
        return nullptr;
    }
    if (!cells_[pos.row][pos.col]->IsExist()) {
        return nullptr;
    }
    return cells_[pos.row][pos.col].get();
}

Cell* Sheet::GetConcreteCell(Position pos) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("wrong position");
    }
    if (cells_.size() > static_cast<size_t>(pos.row)) {
        if (cells_[pos.row].size() <= static_cast<size_t>(pos.col)) {
            return nullptr;
        }
    }
    else {
        return nullptr;
    }
    if (cells_[pos.row][pos.col].get() == nullptr) {
        return nullptr;
    }
    if (!cells_[pos.row][pos.col]->IsExist()) {
        return nullptr;
    }
    return cells_[pos.row][pos.col].get();
}

const Cell* Sheet::GetConcreteCell(Position pos) const {
    if (!pos.IsValid()) {
        throw InvalidPositionException("wrong position");
    }
    if (cells_.size() > static_cast<size_t>(pos.row)) {
        if (cells_[pos.row].size() <= static_cast<size_t>(pos.col)) {
            return nullptr;
        }
    }
    else {
        return nullptr;
    }
    if (cells_[pos.row][pos.col].get() == nullptr) {
        return nullptr;
    }
    if (!cells_[pos.row][pos.col]->IsExist()) {
        return nullptr;
    }
    return cells_[pos.row][pos.col].get();
}

void Sheet::UpdateSize(Position& pos) {
    Size result = { 0,0 };
    Size tmp = GetPrintableSize();
    for (int i = tmp.rows-1; i >= 0; --i) {
        for (int j = tmp.cols-1; j >= 0; --j) {
            if (static_cast<int>(cells_[i].size()) > j) {
                if (cells_[i][j] != nullptr && i != pos.row && j != pos.col) {
                    result.rows = i + 1;
                    result.cols = j + 1;
                    size_ = result;
                    return;
                }
            }
        }
    }
    size_ = { 0,0 };
}

void Sheet::ClearCell(Position pos) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("wrong position");
    }
    if (cells_.size() > static_cast<size_t>(pos.row)&& GetCell(pos)!=nullptr) {
        if (cells_[pos.row].size() > static_cast<size_t>(pos.col)) {
            cells_[pos.row][pos.col]->Clear();
            cells_[pos.row][pos.col].release();
            if (pos.row == size_.rows-1 || pos.col == size_.cols-1) {
                UpdateSize(pos);
            }
        }
    }


}

void Sheet::AddReferenceForCell(Cell* actual_cell, std::string text){
    try {
        if (!text.empty()) {
            auto formula = ParseFormula(std::string(text.begin() + 1, text.end()));
            for (auto& cell : formula->GetReferencedCells()) {
                if (GetConcreteCell(cell) == nullptr) {
                    SetCell(cell, "");
                }
                Cell* referenced_cell = GetConcreteCell(cell);
                actual_cell->AddReferencesFor(referenced_cell);
                referenced_cell->AddReferencesFrom(actual_cell);
            }
        }
    }
    catch(...){
        return;
    }
}

Size Sheet::GetPrintableSize() const {
    return size_;
}

void Sheet::PrintValues(std::ostream& output) const {
    for (int i = 0; i < size_.rows; ++i) {
        for (int j = 0; j < size_.cols; ++j) {
            if (cells_[i].size() > static_cast<size_t>(j)) {
                if (cells_[i][j] != nullptr) {
                    const auto cell = cells_[i][j]->GetValue();

                    if (std::holds_alternative<double>(cell)) {
                        output << std::get<double>(cell);
                    }
                    else if (std::holds_alternative<std::string>(cell)) {
                        output << std::get<std::string>(cell);
                    }
                    else if (std::holds_alternative<FormulaError>(cell)) {
                        output << "#ARITHM!";
                    }

                }

            }
            if (j != size_.cols - 1) {
                output << "\t";
            }
        }
        output << "\n";
    }
}
void Sheet::PrintTexts(std::ostream& output) const {
    for (int i = 0; i < size_.rows;++i) {
        for (int j = 0; j < size_.cols; ++j) {
            if (cells_[i].size() > static_cast<size_t>(j)) {
                if (cells_[i][j] != nullptr) {
                    output << cells_[i][j]->GetText();

                }

            }
            if (j != size_.cols - 1) {
                output << "\t";
            }

        }
            output << "\n";
    }
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}
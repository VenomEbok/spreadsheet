#pragma once

#include "cell.h"
#include "common.h"

#include <functional>

class Sheet : public SheetInterface {
public:
    ~Sheet();

    void SetCell(Position pos, std::string text) override;

    const CellInterface* GetCell(Position pos) const override;
    CellInterface* GetCell(Position pos) override;

    void UpdateSize(Position& pos);

    void ClearCell(Position pos) override;

    Size GetPrintableSize() const override;

    void PrintValues(std::ostream& output) const override;
    void PrintTexts(std::ostream& output) const override;

    const Cell* GetConcreteCell(Position pos) const;
    Cell* GetConcreteCell(Position pos);

    void AddReferenceForCell(Cell* cell, std::string text);

private:
    void MaybeIncreaseSizeToIncludePosition(Position pos);
    void PrintCells(std::ostream& output, const std::function<void(const CellInterface&)>& printCell) const;
    Size GetActualSize() const;

    Size size_;

    std::vector<std::vector<std::unique_ptr<Cell>>> cells_;

};
#include "cell.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>

// Реализуйте следующие методы

Cell::~Cell() {}

void Cell::Set(std::string text) {
	references_for.clear();
	if (!text.empty()) {
		if (text[0] == '=') {
			if (text.size() == 1) {
				impl_ = std::make_unique<TextImpl>(std::move(TextImpl(text)));
				return;
			}
			try{
                ClearCache();
				impl_ = std::make_unique<FormulaImpl>(std::move(FormulaImpl(std::string(text.begin() + 1, text.end()), sheet_)));
				
				return;
			}
			catch (CircularDependencyException& error) {
				throw error;
			}
			catch (...) {
				throw FormulaException("wrong formula");
			}

		}
		impl_ = std::make_unique<TextImpl>(std::move(TextImpl(text)));
		return;
	}
	impl_ = std::make_unique<EmptyImpl>();
}

void Cell::Clear() {
	impl_ = std::make_unique<EmptyImpl>(std::move(EmptyImpl()));
}

Cell::Value Cell::GetValue() const {
	if (cache_.has_value()) {
		return std::get<double>(cache_.value());
	}
	try {
		cache_ = std::get<double>(impl_->GetValue());
	}
	catch (...) {

	}
	return impl_->GetValue();
}
std::string Cell::GetText() const {
	return impl_->GetText();
}


std::vector<Position> Cell::GetReferencedCells() const
{
	std::vector<Position> result;
	if (references_for.empty()) {
		return result;
	}
	for (auto& cell : ParseFormula(impl_->GetText().substr(1))->GetReferencedCells()) {
		
		result.push_back(cell);
	}
	return result;

}

bool Cell::IsExist()
{
	return impl_ != nullptr;
}

void Cell::AddReferencesFor(Cell* cell){
	references_for.insert(cell);
}

void Cell::AddReferencesFrom(Cell* cell){
	references_from.insert(cell);
}

void Cell::CheckForLooping(Position& pos,std::string text){
	try {
		if (!text.empty()) {
			auto formula = ParseFormula(std::string(text.begin() + 1, text.end()));
			for (auto& cell : formula->GetReferencedCells()) {
				CellInterface* check_cell = sheet_.GetCell(cell);
				if (pos == cell) {
					throw CircularDependencyException("Circular dependency");
				}
				if (check_cell != nullptr) {
					CheckForLooping(pos, check_cell->GetText());
				}
			}
		}
	}
	catch (FormulaException&) {

	}
}

void Cell::ClearCache() {

	cache_.reset();
	for (Cell* cell : references_from) {
		cell->ClearCache();
	}
}

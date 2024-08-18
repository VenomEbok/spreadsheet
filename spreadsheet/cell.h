#pragma once

#include "common.h"
#include "formula.h"
#include <optional>

class Cell : public CellInterface {
public:
    Cell(SheetInterface& sheet) : sheet_(sheet) {
    }
    ~Cell();
    Cell(const Cell& ex):sheet_(ex.sheet_) {
        Set(ex.GetText());
    }
    void Set(std::string text);
    void Clear();

    Value GetValue() const override;
    std::string GetText() const override;

      std::vector<Position>  GetReferencedCells() const override;

      bool IsExist();

      void AddReferencesFor(Cell* cell);
      void AddReferencesFrom(Cell* cell);


      void ClearCache();

    void CheckForLooping(Position& pos, std::string text);
private:
    class Impl
    {
    public:
        using Value = std::variant<std::string, double, FormulaError>;
        Impl() = default;
        ~Impl() = default;
        virtual std::string GetText() const = 0;
        virtual Value GetValue() const = 0;

    };

    class EmptyImpl :public Impl
    {
    public:
        EmptyImpl() = default;
        ~EmptyImpl() = default;
        Value GetValue() const override {
            return 0.0;
        }
        std::string GetText() const override {
            return value_;
        }
    private:
        std::string value_ = "";
    };

    class TextImpl :public Impl {
    public:
        TextImpl(std::string expression) :value_(expression) {
        }
        Value GetValue() const override {
            if (value_[0] == '\'') {
                return std::string(value_.begin() + 1, value_.end());
            }
            return value_;
        }
        std::string GetText() const override {
            return value_;
        }
    private:
        std::string value_;
    };

    class FormulaImpl : public Impl {
    public:
        FormulaImpl(std::string expression, SheetInterface& sheet) :value_(ParseFormula(expression)), sheet_(sheet) {
        }
        Value GetValue() const override {

            FormulaInterface::Value result;
            try {
                result = value_->Evaluate(sheet_);
                return std::get<double>(result);
            }
            catch (...) {
                return std::get<FormulaError>(result);
            }
        }
        std::string GetText() const override {
            std::string result = "=" + value_->GetExpression();
            return result;
        }



    private:
        std::unique_ptr<FormulaInterface> value_;
        SheetInterface& sheet_;
        
    };
    std::unique_ptr<Impl> impl_= nullptr;
    SheetInterface& sheet_;
    std::unordered_set<Cell*> references_for;
    std::unordered_set<Cell*> references_from;
    mutable std::optional<FormulaInterface::Value> cache_;

};


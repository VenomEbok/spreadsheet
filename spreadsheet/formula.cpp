#include "formula.h"

#include "FormulaAST.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>

using namespace std::literals;

std::ostream& operator<<(std::ostream& output, FormulaError fe) {
    return output << "#ARITHM!";
}

namespace {
    class Formula : public FormulaInterface {
    public:
        // ���������� ��������� ������:
        explicit Formula(std::string expression)try :ast_(ParseFormulaAST(expression)) {
        }
        catch (const std::exception& exc) {

        }
        Value Evaluate(const SheetInterface& sheet) const override {
            Value result;
            try {
                result = ast_.Execute(sheet);
            }
            catch (FormulaError::Category& error) {
                result = error;
            }
            return result;
        }
        std::string GetExpression() const override {
            std::ostringstream out;
            ast_.PrintFormula(out);
            return out.str();
        }

        std::vector<Position> GetReferencedCells() const override {
            std::vector<Position> result;
            auto cells = ast_.GetCells();
            for (auto& cell : cells) {
                if (result.empty()) {
                    result.push_back(cell);
                    continue;
                }
                if (!(*(result.end()-1)==cell)) {
                    result.push_back(cell);
                }

            }
            return result;
        }

    private:
        FormulaAST ast_;
    };
}  // namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
    try {
        ParseFormulaAST(expression);
        return std::make_unique<Formula>(std::move(expression));
    }
    catch (...) {
        throw FormulaException("wrong formula");
    }
}

FormulaError::FormulaError(Category category) : category_(category) {}

FormulaError::Category FormulaError::GetCategory() const {
    return category_;
}

bool FormulaError::operator==(FormulaError rhs) const {
    return ToString() == rhs.ToString();
}

std::string_view FormulaError::ToString() const {
    switch (category_) {
    case Category::Ref:
        return "#REF!"sv;
    case Category::Value:
        return "#VALUE!"sv;
    case Category::Arithmetic:
        return "#DIV/0!"sv;
    default:
        assert(false);
        return "";
    }
}

/**
 * MIT License
 *
 * Copyright (c) 2018 Maksim Piskunov

 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "Expression.hpp"

#include <algorithm>
#include <unordered_map>

namespace SetReplace {
    class AtomsIndex::Implementation {
     private:
        const std::function<AtomsVector(ExpressionID)> getAtomsVector_;
        std::unordered_map<Atom, std::unordered_set<ExpressionID>> index_;

     public:
        Implementation(const std::function<AtomsVector(ExpressionID)>& getAtomsVector)
            : getAtomsVector_(getAtomsVector) {}

        void removeExpressions(const std::vector<ExpressionID>& expressionIDs) {
            const std::unordered_set<ExpressionID> expressionsToDelete(expressionIDs.begin(), expressionIDs.end());

            std::unordered_set<Atom> involvedAtoms;
            for (const auto& expression : expressionIDs) {
                const auto atomsVector = getAtomsVector_(expression);
                // Increase set capacity to reduce number of memory allocations
                involvedAtoms.reserve(involvedAtoms.size() + atomsVector.size());
                for (const auto& atom : atomsVector) {
                    involvedAtoms.insert(atom);
                }
            }

            for (const auto& atom : involvedAtoms) {
                auto expressionIterator = index_[atom].begin();
                while (expressionIterator != index_[atom].end()) {
                    if (expressionsToDelete.count(*expressionIterator)) {
                        expressionIterator = index_[atom].erase(expressionIterator);
                    } else {
                        ++expressionIterator;
                    }
                }
                if (index_[atom].empty()) {
                    index_.erase(atom);
                }
            }
        }

        void addExpressions(const std::vector<ExpressionID>& expressionIDs) {
            for (const auto expressionID : expressionIDs) {
                for (const auto atom : getAtomsVector_(expressionID)) {
                    index_[atom].insert(expressionID);
                }
            }
        }

        const std::unordered_set<ExpressionID> expressionsContainingAtom(const Atom atom) const {
            const auto resultIterator = index_.find(atom);
            return resultIterator != index_.end() ? resultIterator->second : std::unordered_set<ExpressionID>();
        }
    };

    AtomsIndex::AtomsIndex(const std::function<AtomsVector(ExpressionID)>& getAtomsVector) {
        implementation_ = std::make_shared<Implementation>(getAtomsVector);
    }

    void AtomsIndex::removeExpressions(const std::vector<ExpressionID>& expressionIDs) {
        implementation_->removeExpressions(expressionIDs);
    }

    void AtomsIndex::addExpressions(const std::vector<ExpressionID>& expressionIDs) {
        implementation_->addExpressions(expressionIDs);
    }

    const std::unordered_set<ExpressionID> AtomsIndex::expressionsContainingAtom(const Atom atom) const {
        return implementation_->expressionsContainingAtom(atom);
    }
}

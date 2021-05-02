#pragma once

#include <unordered_set>
#include <set>

namespace wfc {

// interface for using an unordered set for storing tile state
// (left here for benchmark comparison, dynamic_flagset is many times faster)
class unorderedStateSet : public std::unordered_set<State> {
	public:
		bool constrain(const unorderedStateSet& b) {
			bool altered = false;

			for (auto it = begin(); it != end();) {
				if (!b.hasState(*it)) {
					it = erase(it);
					altered = true;

				} else {
					it = std::next(it);
					//it++;
				}
			}

			return altered;
		}

		void unify(const unorderedStateSet& b) {
			for (auto& em : b) {
				insert(em);
			}
		}

		size_t countStates() const { return size(); }
		bool hasState(const State& s) const { return count(s); }

		bool   anySet() const { return size() > 0; };
		void   setState(const State& s) { insert(s); };
		void   clearStates() { clear(); };
		void   unsetState(const State& s) { erase(s); };
		State  chooseState() {
			// TODO: for choosing random state
			size_t k = rand() % size();
			auto em = *std::next(begin(), k);

			return em;
		}
};

class orderedStateSet : public std::set<State> {
	public:
		bool constrain(const orderedStateSet& b) {
			bool altered = false;

			if (size() <= 1) {
				return false;
			}

			for (auto it = begin(); it != end();) {
				if (b.hasState(*it) == 0) {
					it = erase(it);
					altered = true;

				} else {
					it = std::next(it);
				}
			}

			return altered;
		}

		void unify(const orderedStateSet& b) {
			for (auto& em : b) {
				insert(em);
			}
		}

		size_t countStates() const { return size(); }
		bool hasState(const State& s) const { return count(s); }

		bool   anySet() const { return size() != 0; };
		void   setState(const State& s) { insert(s); };
		void   clearStates() { clear(); };
		void   unsetState(const State& s) { erase(s); };
		State  chooseState() {
			// TODO: for choosing random state
			size_t k = rand() % size();
			auto em = *std::next(begin(), k);

			return em;
		}
};

// namespace wfc
}

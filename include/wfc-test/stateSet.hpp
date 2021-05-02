#pragma once
#include <wfc-test/dynamicFlagset.hpp>
#include <wfc-test/staticFlagset.hpp>
#include <wfc-test/state.hpp>

namespace wfc {

// interface for using one of the flagset classes for storing tile state
template <class flagSet>
class stateSet : public flagSet {
	public:
		size_t countStates() const { return flagSet::size(); }
		bool hasState(const State& s) const { return flagSet::count(s); }

		bool   anySet() const { return flagSet::size() > 0; };
		void   setState(const State& s) { flagSet::insert(s); };
		void   clearStates() { flagSet::clear(); };
		void   unsetState(const State& s) { flagSet::erase(s); };
		State  chooseState() {
			// TODO: for choosing random state
			size_t k = rand() % flagSet::size();
			//auto em = *std::next(begin(), k);
			auto em = *(flagSet::begin() + k);

			return em;
		}
};

template <size_t maxStates>
using staticStateSet = stateSet<staticFlagset<maxStates>>;

using dynamicStateSet = stateSet<dynamicFlagset>;

// namespace wfc
}

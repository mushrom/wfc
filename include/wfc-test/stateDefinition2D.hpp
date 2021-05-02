#pragma once

#include <wfc-test/state.hpp>
#include <unordered_map>
namespace wfc {

struct neighbor {
	unsigned dir;
	State a, b;
};

// common logic for all dimensions and initialization constraints
template <class T, class stateSet, size_t sockets>
struct stateDefinitionBase {
	// XXX: makes accessing state set parameter from wfc stuff easier
	using StateSet = stateSet;
	static constexpr unsigned Sockets = sockets;

	std::unordered_map<State, StateSet> socketmap[sockets];
	stateSet states;

	void addNeighbor(struct neighbor n) {
		socketmap[n.dir][n.a].setState(n.b);
		// TODO: need to document this somewhere, negative direction
		//       should always be sockets/2 away
		//   eg. in 2d, 0..3 -> {-1, 0}, {0, -1}, {1, 0}, {0, 1}
		socketmap[(n.dir + sockets/2) % sockets][n.b].setState(n.a);
	}

	void initializeTile(StateSet& s, uint64_t socketMask) {
		static_cast<T*>(this)->initializeTile(s, socketMask);
	}

	const StateSet& connects(const State& s, unsigned socket) const {
		static StateSet empty = {};
		auto it = socketmap[socket].find(s);

		if (it == socketmap[socket].end()) {
			return empty;

		} else {
			return it->second;
		}
	}
};

// plain old 2D state, initializes every tile to same state
template <class stateSet>
struct stateDefinition2D
	: stateDefinitionBase<stateDefinition2D<stateSet>, stateSet, 4>
{
	void initializeTile(stateSet& s, uint64_t socketMask) {
		for (auto& em : this->states) {
			s.setState(em);
		}
	}
};

// state definition with grid boundary contraints
template <class stateSet>
struct stateDefinition2DBounded 
	: stateDefinitionBase<stateDefinition2DBounded<stateSet>, stateSet, 4>
{
	void initializeTile(stateSet& s, uint64_t socketMask) {
		for (auto& em : this->states) {
			bool satisfied = true;

			for (unsigned bit = 0; bit < 4; bit++) {
				auto& smap = this->socketmap[bit];
				bool hasDir = smap.find(em) != smap.end() && smap[em].anySet();

				satisfied &= ((socketMask & (1 << bit)) && hasDir) || !((socketMask & (1 << bit)));
			}

			if (satisfied) {
				s.setState(em);
			}
		}
	}
};

// namespace wfc
}

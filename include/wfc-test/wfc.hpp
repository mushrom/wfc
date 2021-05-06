#pragma once
#include <stddef.h>
#include <limits.h>
#include <unordered_set>
#include <set>
#include <vector>
#include <concepts>
#include <random>
#include <iostream>
#include <string.h>
#include <list>

/*
template <typename S>
concept Stateclass = requires(S a) {
	{ a.maxStates } -> std::convertible_to<size_t>;
	{ S::State foo } 
}
*/

namespace wfc {

template <typename S, size_t X, size_t Y>
class grid {
	public:
		static constexpr size_t gridSize = X * Y;
		typename S::StateSet tiles[gridSize];

		grid(S& stateclass) {
			reset(stateclass);
		}

		std::pair<size_t, size_t> curMin(void) {
			size_t mincount = UINT_MAX;
			std::vector<std::pair<size_t, size_t>> candidates;
			// TODO: more benchmarking to see if it's worth using a static array
			//       for storing candidates, or if a hash map of tile counts would
			//       work better
			//std::pair<size_t, size_t> candidates[X * Y];
			//size_t candidate_end = 0;
			//candidate_end = 0;

			for (size_t x = 0; x < X; x++) {
				for (size_t y = 0; y < Y; y++) {
					size_t n = y*X + x;
					size_t count = tiles[n].countStates();

					if (count > 1) {
						if (count < mincount) {
							mincount = count;
							candidates.clear();
							//candidate_end = 0;
						}

						candidates.push_back({x, y});
						//candidates[candidate_end++] = {x, y};
					}
				}
			}

			if (candidates.size()) {
			//if (candidate_end) {
				size_t k = rand() % candidates.size();
				//size_t k = rand() % candidate_end;
				return candidates[k];

			} else {
				return {0, 0};
			}
		}

		static inline bool validCoord(size_t x, size_t y) {
			return x < X && y < Y;
		}

		void reset(S& stateclass) {
			for (size_t y = 0; y < Y; y++) {
				for (size_t x = 0; x < X; x++) {
					size_t idx = y*X + x;
					uint64_t sockets = ~0; // default to all bits set

					sockets &= ~((x == 0)     << 0); // no left
					sockets &= ~((y == 0)     << 1); // no up
					sockets &= ~((x + 1 == X) << 2); // no right
					sockets &= ~((y + 1 == Y) << 3); // no down

					stateclass.initializeTile(tiles[idx], sockets);
				}
			}
		}
};

template <typename S, size_t X, size_t Y>
class WFCSolver {
	public:
		WFCSolver(S& _stateclass) 
			: gridState(_stateclass),
			  stateclass(_stateclass) { }

		bool toroid = false;
		size_t backtrackDepth = 20;

		grid<S, X, Y> gridState;
		S stateclass;
		std::list<std::pair<size_t, size_t>> propCoords;

		struct collapseChoice {
			typename S::StateSet choices;
			grid<S, X, Y> state;
			std::pair<size_t, size_t> coord;
		};

		std::list<collapseChoice> collapseStack;

		void reset(void) {
			gridState.reset(stateclass);
			propCoords.clear();
			collapseStack.clear();
		}

		void recalculateConstraints(size_t x, size_t y) {
			if (!gridState.validCoord(x, y)) {
				return;
			}

			size_t n = y*X + x;
			auto& tile = gridState.tiles[n];

			for (unsigned i = 0; i < S::sockets; i++) {
				auto& constraints = gridState.constraintCache[n][i];
				constraints.clear();

				for (auto& s : tile) {
					auto& tc = stateclass.connects(s, i);
					constraints.unify(tc);
				}
			}
		}

		bool collapse(size_t x, size_t y) {
			if (!gridState.validCoord(x, y)) {
				return false;
			}

			size_t n = y*X + x;
			auto& tile = gridState.tiles[n];

			if (tile.countStates() <= 1) {
				// tile is either already collapsed or in an error state,
				// nothing to do
				return false;
			}

			struct collapseChoice foo = {
				.choices = tile,
				.state   = gridState,
				.coord   = {x, y},
			};

			auto em = tile.chooseState();
			foo.choices.unsetState(em);
			collapseStack.push_back(foo);

			if (collapseStack.size() > backtrackDepth) {
				collapseStack.pop_front();
			}

			tile.clearStates();
			tile.setState(em);

			return true;
		}

		bool propagate(size_t x, size_t y) {
			static std::pair<int, int> directions[] = {
				{-1,  0},
				{ 0, -1},
				{ 1,  0},
				{ 0,  1},
			};

			if (toroid) {
				x = x%X;
				y = y%Y;

			} else if (!gridState.validCoord(x, y)) {
				return false;
			}

			auto& tile = gridState.tiles[y*X + x];

			for (unsigned i = 0; i < S::Sockets; i++) {
				typename S::StateSet constraints;

				// TODO: some sort of caching here, to avoid recalculating
				//       constraints all the time, although with the new
				//       bitset thing this is pretty fast anyway...
				// TODO: small optimization: unify() in the bitset will
				//       recount bits after each call, but this could be
				//       deferred until after unifying, saving about half
				//       the work
				for (auto& s : tile) {
					auto& tc = stateclass.connects(s, i);
					constraints.unify(tc);
				}

				auto nx = x + directions[i].first;
				auto ny = y + directions[i].second;

				if (toroid) {
					nx %= X;
					ny %= Y;

				} else if (!gridState.validCoord(nx, ny)) {
					continue;
				}

				auto& adjTile = gridState.tiles[ny*X + nx];
				// only continue propagating if this tile was changed
				if (adjTile.constrain(constraints)) {
					propCoords.push_back({nx, ny});
				}

				// tile is in an invalid state, propagation resulted in
				// contradiction
				if (adjTile.countStates() == 0) {
					return false;
				}
			}

			return true;
		}

		bool backtrack(void) {
			if (collapseStack.empty()) {
				return false;
			}

			while (!collapseStack.back().choices.anySet()) {
				collapseStack.pop_back();

				if (collapseStack.empty()) {
					return false;
				}
			}

			auto& choice = collapseStack.back();
			//size_t k = rand() % choice.choices.size();
			//auto em = *std::next(choice.choices.begin(), k);
			auto em = choice.choices.chooseState();

			choice.choices.unsetState(em);
			gridState = choice.state;

			auto& [x, y] = choice.coord;
			size_t n = y*X + x;
			auto& tile = gridState.tiles[n];

			tile.clearStates();
			tile.setState(em);
			//recalculateConstraints(x, y);

			propCoords.push_back({x, y});

			return true;
		}

		// returns {running, valid}
		std::pair<bool, bool> iterate(void) {
			if (propCoords.empty()) {
				auto [x, y] = gridState.curMin();

				if (collapse(x, y)) {
					propCoords.push_back({x, y});

				} else {
					return {false, true};
				}

			} else {
				while (!propCoords.empty()) {
					auto [x, y] = propCoords.front();
					//std::cerr << "propagating at " << std::dec << x << ", " << y << std::endl;
					propCoords.pop_front();
					if (!propagate(x, y)) {
						if (!backtrackDepth || !backtrack()) {
							fprintf(stderr, "Failed! (%lu, %lu)\n", x, y);
							return {false, false};
						}
					}
				}
			}

			return {true, true};
		}
};

// namespace wfc
}

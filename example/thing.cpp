#include <wfc-test/wfc.hpp>
#include <wfc-test/stb_image.h>
#include <wfc-test/stb_image_write.h>
#include <wfc-test/tileset.hpp>

#include <wfc-test/dynamic_flagset.hpp>
#include <wfc-test/static_flagset.hpp>

#include <string.h>
#include <unistd.h>
#include <bit>
#include <bitset>

#include <unordered_set>
#include <unordered_map>
#include <map>

#include <getopt.h>

class statedef {
	public:
		using State = unsigned;
		static constexpr size_t maxStates = 256;

		// different implementations of StateSet here for benchmarking
		// TODO: template or something
#if 1
		class StateSet : public static_flagset<256> {
			public:
				size_t countStates() const { return size(); }
				bool hasState(const State& s) const { return count(s); }

				bool   anySet() const { return size() > 0; };
				void   setState(const State& s) { insert(s); };
				void   clearStates() { clear(); };
				void   unsetState(const State& s) { erase(s); };
				State  chooseState() {
					// TODO: for choosing random state
					size_t k = rand() % size();
					//auto em = *std::next(begin(), k);
					auto em = *(begin() + k);

					return em;
				}
		};
#endif

#if 0
		class StateSet : public dynamic_flagset {
			public:
				size_t countStates() const { return size(); }
				bool hasState(const State& s) const { return count(s); }

				bool   anySet() const { return size() > 0; };
				void   setState(const State& s) { insert(s); };
				void   clearStates() { clear(); };
				void   unsetState(const State& s) { erase(s); };
				State  chooseState() {
					// TODO: for choosing random state
					size_t k = rand() % size();
					//auto em = *std::next(begin(), k);
					auto em = *(begin() + k);

					return em;
				}
		};
#endif
	
#if 0
		class StateSet : public std::unordered_set<State> {
			public:
				bool constrain(const StateSet& b) {
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

				void unify(const StateSet& b) {
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
#endif

#if 0
		class StateSet : public std::set<State> {
			public:
				bool constrain(const StateSet& b) {
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

				void unify(const StateSet& b) {
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
#endif

#if 0
		class StateSet : public std::bitset<maxStates> {
			public:
				bool constrain(const StateSet& b) {
					bool altered = ((*this) ^ b).any();
					operator&=(b);
					return altered;
				}

				void unify(const StateSet& b) {
					operator|=(b);
				}

				bool   anySet() const { return any(); };
				size_t countStates() const { return count(); };
				size_t hasState(const State& s) const { return test(s); };
				void   setState(const State& s) { set(s); };
				void   clearStates() { reset(); };
				void   unsetState(const State& s) { set(s, false); };
				State  chooseState() {
					for (unsigned i = 0; i < maxStates; i++) {
						if (test(i)) {
							return i;
						}
					}

					// invalid
					return 0;
				}
		};
#endif

		static constexpr unsigned sockets = 4;
		std::unordered_map<State, StateSet> socketmap[sockets];
		//StateSet socketmap[sockets];
		StateSet states;

		void initializeTile(StateSet& s, uint64_t socketMask) {
			//for (State em = 0; em < maxStates; em++) {
			for (auto& em : states) {
				bool satisfied = true;

				for (unsigned bit = 0; bit < 4; bit++) {
					auto& smap = socketmap[bit];
					bool hasDir =
						smap.find(em) != smap.end() && smap[em].anySet();

					satisfied &= ((socketMask & (1 << bit)) && hasDir) || !((socketMask & (1 << bit)));
				}

				if (satisfied) {
					s.setState(em);
				}
			}
		}

		const StateSet& connects(const State& s, unsigned socket) const {
			static StateSet empty = {};
			auto it = socketmap[socket].find(s);

			if (it == socketmap[socket].end()) {
				//std::cerr << "NO BAD NO" << std::endl;
				return empty;

			} else {
				return it->second;
			}

			//return socketmap
		}
};

void dumpPPM(const char *filename, int width, int height, uint8_t *px, int channels, int stride)
{
	FILE *fp = fopen(filename, "w");

	if (!fp) {
		return;
	}

	fprintf(fp, "P6\n%d %d\n255\n", width, height);

	for (unsigned y = 0; y < height; y++) {
		for (unsigned x = 0; x < width; x++) {
			unsigned idx = channels * ((width * y) + x);
			fwrite(px + idx, 1, 3, fp);
		}
	}

	fflush(fp);
	fclose(fp);
}

//#define WIDTH  181
//#define HEIGHT 181
//#define WIDTH  64
//#define WIDTH  16
//#define HEIGHT 16
//#define HEIGHT 64
//#define WIDTH  90
//#define WIDTH  90
//#define HEIGHT 160
//#define WIDTH  32
//#define HEIGHT 32
//#define WIDTH 320
//#define HEIGHT 180

#define WIDTH  18
#define HEIGHT 18

int main(int argc, char *argv[]) {
	srand(time(NULL));

	const char *output = "output.ppm";
	const char *input = "bridge.png";
	bool toroid = false;
	size_t backtrackDepth = 20;
	int tilesize = 3;
	bool debug = false;

	int opt;

	while ((opt = getopt(argc, argv, "Tdt:b:i:o:")) != -1) {
		switch (opt) {
			case 'T': toroid = true; break;
			case 'd': debug = true; break;
			case 't': tilesize = atoi(optarg); break;
			case 'b': backtrackDepth = atoi(optarg); break;
			case 'i': input = optarg; break;
			case 'o': output = optarg; break;
			default: 
				fprintf(stderr, "TODO: asdf");
				return 1;
		}
	}

	statedef foo;
	tileset tiles(input, tilesize);

	std::map<std::pair<int, int>, unsigned> dirmap = {
		{{-1,  0}, 0},
		{{ 0, -1}, 1},
		{{ 1,  0}, 2},
		{{ 0,  1}, 3},
	};

	for (auto& [s, _] : tiles.indexToHash) {
		foo.states.insert(s);
	}

	std::cerr << "Have " << foo.states.size() << " states per tile" << std::endl;

	for (auto& [idx, neighbors] : tiles.neighbors) {
		for (auto& [x, y, neighbor] : neighbors) {
			std::pair<int, int> dir(x, y);
			std::pair<int, int> negdir(-x, -y);

			foo.socketmap[dirmap[dir]][idx].setState(neighbor);
			//foo.socketmap[dirmap[negdir]][neighbor].insert(idx);
		}
	}

	wfc<statedef, WIDTH, HEIGHT> wfcgrid(foo);
	wfcgrid.toroid = toroid;
	wfcgrid.backtrackDepth = backtrackDepth;

	size_t imgsize = 4 * WIDTH * HEIGHT * tilesize * tilesize;
	uint8_t *data = new uint8_t[imgsize];
	unsigned count = 0;

	for (bool running = true, valid = true; running && valid;) {
		auto [r, v] = wfcgrid.iterate();
		running = r, valid = v;

		if (!valid) {
			return 1;
		}

		if (debug || (!running && valid)) {
			std::cerr << "Dumping output..." << std::endl;
			memset(data, 0, imgsize);
			for (unsigned n = 3; n < imgsize; n += 4) { data[n] = 0xff; };

			for (unsigned x = 0; x < WIDTH; x++) {
				for (unsigned y = 0; y < HEIGHT; y++) {
					auto& tile = wfcgrid.gridState.tiles[y*WIDTH + x];
					auto  count = tile.countStates();

					for (auto& em : tile) {
						tiles.copyTile(em, data, x*tilesize, y*tilesize,
								WIDTH*tilesize, HEIGHT*tilesize,
								count);
					}
				}
			}

			/*
			   stbi_write_png("test.png",
			   WIDTH * tilesize,
			   HEIGHT * tilesize, 4,
			   data, WIDTH * tilesize * 4);
			   */

			dumpPPM(output,
					WIDTH * tilesize,
					HEIGHT * tilesize,
					data,
					4,
					WIDTH * tilesize * 4);
			usleep(25000);

			// TODO: sequential output flag
			/*
			   char buf[64];
			   snprintf(buf, sizeof(buf), "test%03u.png", count++);

			   printf("output: %s\n", buf);
			   stbi_write_png(buf,
			   WIDTH * tilesize,
			   HEIGHT * tilesize, 4,
			   data, WIDTH * tilesize * 4);
			   */
		}
	}

	/*
		stbi_write_png("test.png",
				WIDTH * tilesize,
				HEIGHT * tilesize, 4,
				data, WIDTH * tilesize * 4);
				*/

	return 0;
}

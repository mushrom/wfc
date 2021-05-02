#include <wfc-test/wfc.hpp>
#include <wfc-test/stb_image.h>
#include <wfc-test/stb_image_write.h>
#include <wfc-test/tileset.hpp>

#include <wfc-test/stateSet.hpp>
#include <wfc-test/stateDefinition2D.hpp>

#include <string.h>
#include <unistd.h>

#include <getopt.h>

using namespace wfc;

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

//#define WIDTH  12
//#define HEIGHT 12
//#define WIDTH  15
//#define HEIGHT 23

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

	//using stateDef = stateDefinition2DBounded<stateSet<staticFlagset<256>>>;
	using stateDef = stateDefinition2D<staticStateSet<256>>;
	//using stateDef = stateDefinition2DBounded<dynamicStateSet>;
	//using stateDef = stateDefinition2D<unorderedStateSet>;
	//using stateDef = stateDefinition2D<orderedStateSet>;
	stateDef foo;
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
			foo.addNeighbor({
				.dir = dirmap[{x, y}],
				.a   = idx,
				.b   = neighbor,
			});
		}
	}

	WFCSolver<stateDef, WIDTH, HEIGHT> wfcgrid(foo);
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
			//std::cerr << "Dumping output..." << std::endl;
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

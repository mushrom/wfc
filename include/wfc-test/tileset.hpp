#pragma once

#include <wfc-test/stb_image.h>
#include <memory>
#include <stdint.h>
#include <map>
#include <set>
#include <stdexcept>
#include <iostream>

namespace wfc {

class tileset {
	public:
		uint8_t *pixels;
		int width, height, channels;
		int tilewidth, tileheight;
		int tilesize;

		std::vector<unsigned> tilehashes;
		std::map<unsigned, std::pair<int, int>> tilecoords;
		std::map<unsigned, std::vector<std::tuple<int, int, unsigned>>> neighbors;

		std::map<unsigned, unsigned> hashToIndex;
		std::map<unsigned, unsigned> indexToHash;

		tileset(std::string path, size_t _tilesize = 3) {
			int iwidth, iheight, ichannels;
			uint8_t *datas = stbi_load(path.c_str(), &iwidth, &iheight, &ichannels, 0);

			if (!datas) {
				throw std::logic_error("asdf");
			}

			tilesize   = _tilesize;
			tilewidth  = iwidth  / tilesize + !!(iwidth  % tilesize);
			tileheight = iheight / tilesize + !!(iheight % tilesize);

			width = tilesize * tilewidth;
			height = tilesize * tileheight;
			channels = 3;

			tilehashes.reserve(tilewidth * tileheight);
			tilehashes.resize(tilehashes.capacity());

			int osize = 3 * width * height;
			pixels = (uint8_t*)calloc(1, osize);

			for (int x = 0; x < iwidth; x++) {
				for (int y = 0; y < iheight; y++) {
					int idx = ichannels * ((y * iwidth) + x);
					int odx = channels * ((y * width) + x);

					for (int ch = 0; ch < channels && ch < 3; ch++) {
						pixels[odx + ch] = datas[idx + ch];
					}
				}
			}

			initializeTilemap();
			std::cerr << "Initialized tilemap! " 
				<< tilehashes.size() << " tiles, "
				<< neighbors.size() << " hash indexes "
				<< std::endl;
		}

		void initializeTilemap(void) {
			for (int x = 0, tx = 0; x < width; x += tilesize, tx++) {
				for (int y = 0, ty = 0; y < height; y += tilesize, ty++) {
					unsigned hash = hashTile(x, y);

					tilehashes[ty * tilewidth + tx] = hash;
					tilecoords[hash] = {x, y};
				}
			}

			static std::pair<int, int> directions[] = {
				{-1,  0},
				{ 0, -1},
				{ 1,  0},
				{ 0,  1},
			};

			unsigned i = 0;
			for (auto& [hash, _] : tilecoords) {
				i++;
				indexToHash[i]    = hash;
				hashToIndex[hash] = i;
			}

			for (int tx = 0; tx < tilewidth; tx++) {
				for (int ty = 0; ty < tileheight; ty++) {
					unsigned curhash = getTile(tx, ty);
					unsigned cidx = hashToIndex[curhash];

					for (unsigned i = 0; i < 4; i++) {
						int x = directions[i].first;
						int y = directions[i].second;

						if (unsigned neighbor = getTile(tx + x, ty + y)) {
							unsigned nidx = hashToIndex[neighbor];
							neighbors[cidx].push_back({x, y, nidx});
							//neighbors[nidx].push_back({-x, -y, cidx});
						}
					}
				}
			}
		}

		unsigned getTile(int tx, int ty) {
			if (tx >= 0 && tx < tilewidth && ty >= 0 && ty < tileheight) {
				return tilehashes[ty * tilewidth + tx];

			} else {
				return 0;
			}
		}

		void copyTile(unsigned index, uint8_t *opx, int ox, int oy,
					  int owidth, int oheight, int div)
		{
			if (indexToHash.count(index) == 0) {
				throw std::logic_error("Invalid index! " + std::to_string(index));
				return;
			}

			unsigned hash = indexToHash[index];
			auto& [tx, ty] = tilecoords[hash];

			for (int ix = 0; ix < tilesize; ix++) {
				for (int iy = 0; iy < tilesize; iy++) {
					int tidx = channels * (((ty + iy) * width) + tx + ix);
					int pidx = 4 * (((oy + iy) * owidth) + ox + ix);

					for (int ch = 0; ch < channels; ch++) {
						opx[pidx + ch] += pixels[tidx + ch] / float(div);
					}
					//opx[pidx] += pixels[pidx] / float(div);
				}
			}
		}

		unsigned hashTile(int x, int y) {
			unsigned sum = 793;

			for (int ix = 0; ix < tilesize; ix++) {
				for (int iy = 0; iy < tilesize; iy++) {
					int idx = channels * (((y + iy) * width) + x + ix);

					for (int ch = 0; ch < channels; ch++) {
						sum = (sum << 7) + sum + pixels[idx + ch];
					}
				}
			}

			return sum;
		}
};

// namespace wfc
}

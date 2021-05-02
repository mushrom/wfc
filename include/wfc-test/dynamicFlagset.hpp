#pragma once

namespace wfc {

// dynamic bitset for storing boolean state flags (TODO: better description)
class dynamicFlagset {
	public:
		dynamicFlagset() {
			bits.reserve(4);
			bits.resize(bits.capacity(), 0);
		};

		/*
		dynamicFlagset(const dynamicFlagset& other) {
			members = other.members;
			bits.insert(bits.end(), other.bits.begin(), other.bits.end());
		}
		*/

		size_t members = 0;
		std::vector<size_t> bits;
		static constexpr const size_t mod = 8*sizeof(size_t);

		struct Iterator {
			Iterator(const dynamicFlagset *_ptr, size_t _pos)
				: ptr(_ptr), pos(_pos) { }

			const dynamicFlagset *ptr;
			size_t pos;

			size_t& operator*() { return pos; };
			size_t const& operator*() const { return pos; };

			Iterator& operator++() {
				pos = ptr->next(pos);
				return *this;
			};

			Iterator operator++(int) {
				size_t temp = pos;
				pos = ptr->next(pos);
				return Iterator(ptr, temp);
			};

			Iterator operator+(int n) {
				size_t ret = pos;

				for (unsigned i = 0; i < n; i++) {
					ret = ptr->next(ret);
				}

				return Iterator(ptr, ret);
			};

			bool operator==(dynamicFlagset::Iterator const& rhs) { return pos == rhs.pos; };
			bool operator!=(dynamicFlagset::Iterator const& rhs) { return pos != rhs.pos; };
		};

		size_t size() const {
			return members;
		}

		size_t next(size_t pos) const {
			size_t curpos  = (pos / mod);
			size_t wordpos = (pos + 1) / mod;

			for (; wordpos < bits.size(); wordpos++) {
				if (bits[wordpos]) {
					size_t block = bits[wordpos];
					size_t startpos = (curpos == wordpos)? (pos + 1) % mod : 0;

					for (size_t k = startpos; k < 8*sizeof(size_t); k++) {
						if (block & ((size_t)1 << k)) {
							return wordpos*mod + k;
						}
					}
				}
			}

			// end position
			return bits.size() * mod;
		}

		bool constrains(dynamicFlagset const& rhs) {
			size_t sum = 0;

			for (size_t i = 0; i < bits.size() && i < rhs.bits.size(); i++) {
				sum |= (bits[i] & rhs.bits[i]) != bits[i];
			}

			return !!sum;
		}

		void recount(void) {
			size_t sum = 0;

			for (size_t i = 0; i < bits.size(); i++) {
				sum += std::popcount(bits[i]);
			}

			members = sum;
		}

		bool constrain(dynamicFlagset const& rhs) {
			size_t sum = 0;

			for (size_t i = 0; i < bits.size(); i++) {
				size_t k = (i < rhs.bits.size())? rhs.bits[i] : 0;

				sum |= (bits[i] & k) != bits[i];
				bits[i] &= k;
			}

			recount();
			return !!sum;
		}

		void unify(dynamicFlagset const& rhs) {
			if (rhs.bits.size() > bits.size()) {
				//bits.reserve(rhs.bits.size());
				bits.resize(rhs.bits.size(), 0);
			}

			for (size_t i = 0; i < bits.size() && i < rhs.bits.size(); i++) {
				bits[i] |= rhs.bits[i];
			}

			recount();
		}

		Iterator insert(size_t n) {
			auto sz = n/mod + !!(n % mod) + 1;
			if (sz >= bits.size()) {
				bits.reserve(sz);
				bits.resize(bits.capacity(), 0);
				//fprintf(stderr, "resizing to %lu\n", sz);
			}

			//fprintf(stderr, "(%lu), setting %lu:%lu: %lx\n", sizeof(size_t), n/mod, n%mod, bits[n/mod]);

			size_t idx = ((size_t)1 << (n % mod));
			members += !!(bits[n / mod] ^ idx);
			bits[n / mod] |= idx;

			return Iterator(this, n);
		}

		Iterator erase(size_t n) {
			if (n/mod >= bits.size()) {
				return end();
			}

			size_t idx = ((size_t)1 << (n % mod));
			//members -= members && !!(bits[n / mod] & idx);
			members -= !!(bits[n / mod] & idx);
			bits[n / mod] &= ~idx;

			return Iterator(this, next(n));
		}

		Iterator erase(const Iterator& it) {
			return erase(it.pos);
		}

		Iterator begin() const {
			return Iterator(this, count(0)? 0 : next(0));
		}

		Iterator end() const {
			return Iterator(this, bits.size() * mod);
		}

		size_t count(size_t n) const {
			size_t idx = ((size_t)1 << (n % mod));

			return (n/mod < bits.size())? !!(bits[n/mod] & idx) : 0;
		}

		void clear(void) {
			for (unsigned i = 0; i < bits.size(); i++) {
				bits[i] = 0;
			}
			members = 0;
			//bits.clear();
		}
};

// namespace wfc
}

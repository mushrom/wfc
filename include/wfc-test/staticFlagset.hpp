#pragma once

namespace wfc {

// dynamic bitset for storing boolean state flags (TODO: better description)
template <size_t maxStates>
class staticFlagset {
	public:
		staticFlagset() {
			clear();
		};

		/*
		staticFlagset(const staticFlagset& other) {
			members = other.members;
			bits.insert(bits.end(), other.bits.begin(), other.bits.end());
		}
		*/

		static constexpr const size_t mod = 8*sizeof(size_t);
		static constexpr const size_t bitsSize = maxStates/mod + !!(maxStates % mod);
		size_t bits[bitsSize];
		size_t members = 0;

		struct Iterator {
			Iterator(const staticFlagset *_ptr, size_t _pos)
				: ptr(_ptr), pos(_pos) { }

			const staticFlagset *ptr;
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

				for (int i = 0; i < n; i++) {
					ret = ptr->next(ret);
				}

				return Iterator(ptr, ret);
			};

			bool operator==(staticFlagset::Iterator const& rhs) { return pos == rhs.pos; };
			bool operator!=(staticFlagset::Iterator const& rhs) { return pos != rhs.pos; };
		};

		size_t size() const {
			return members;
		}

		size_t next(size_t pos) const {
			size_t curpos  = (pos / mod);
			size_t wordpos = (pos + 1) / mod;

			for (; wordpos < bitsSize; wordpos++) {
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
			return maxStates;
		}

		bool constrains(staticFlagset const& rhs) {
			size_t sum = 0;

			for (size_t i = 0; i < bitsSize; i++) {
				sum |= (bits[i] & rhs.bits[i]) != bits[i];
			}

			return !!sum;
		}

		void recount(void) {
			size_t sum = 0;

			for (size_t i = 0; i < bitsSize; i++) {
				sum += std::popcount(bits[i]);
			}

			members = sum;
		}

		bool constrain(staticFlagset const& rhs) {
			size_t sum = 0;

			for (size_t i = 0; i < bitsSize; i++) {
				sum |= (bits[i] & rhs.bits[i]) != bits[i];
				bits[i] &= rhs.bits[i];
			}

			recount();
			return !!sum;
		}

		void unify(staticFlagset const& rhs) {
			for (size_t i = 0; i < bitsSize; i++) {
				bits[i] |= rhs.bits[i];
			}

			recount();
		}

		Iterator insert(size_t n) {
			if (n >= maxStates) {
				return end();
			}

			size_t idx = ((size_t)1 << (n % mod));
			members += !!(bits[n / mod] ^ idx);
			bits[n / mod] |= idx;

			return Iterator(this, n);
		}

		Iterator erase(size_t n) {
			if (n >= maxStates) {
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
			return Iterator(this, maxStates);
		}

		size_t count(size_t n) const {
			size_t idx = ((size_t)1 << (n % mod));

			return (n < maxStates)? !!(bits[n/mod] & idx) : 0;
		}

		void clear(void) {
			for (unsigned i = 0; i < bitsSize; i++) {
				bits[i] = 0;
			}

			members = 0;
		}
};

// namespace wfc
}

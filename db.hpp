#ifndef __DB_HPP__
#define __DB_HPP__

#include <string>
#include <set>
#include <map>
#include <vector>
#include <sstream>
#include <utility>
#include <ctime>

namespace sl {
		
	template <typename T, typename = void>
	struct is_string {
		static const bool value = false;
	};

	template <class T, class Traits, class Alloc>
	struct is_string<std::basic_string<T, Traits, Alloc>, void> {
		static const bool value = true;
	};

	template <class T, template <typename, typename, typename> class STRING>
	struct is_string<T, STRING<T, std::char_traits<T>, std::allocator<T>>> {
		static const bool value = true;
	};

	using strmap = typename std::map<std::string, std::string>;

	namespace code {
		std::string decode(std::string SRC) {
			std::string ret;
			ret.reserve(SRC.length() / 2);
			bool p = false;
			for (auto& i : SRC) {
				if (i == '|') {
					p = true;
				}
				else if (i == ';') {
					p = false;
				}
				else if (p) {
					switch (i) {
					case 'a':
						ret.push_back('&');
						break;
					case 'c':
						ret.push_back('\r');
						break;
					case 'l':
						ret.push_back('\n');
						break;
					case 'o':
						ret.push_back('|');
						break;
					case 's':
						ret.push_back(';');
						break;
					case 'p':
						ret.push_back(' ');
						break;
					default:
						break;
					}
				}
				else {
					ret.push_back(i);
				}
			}
			return ret;
		}

		static std::string encode(std::string s) {
			const char* str = s.c_str();
			std::string ret;
			ret.reserve(s.length());

			for (auto& e : s) {
				if (e == '&') {
					ret.push_back('|');
					ret.push_back('a');
					ret.push_back(';');
				}
				else if (e == '\r') {
					ret.push_back('|');
					ret.push_back('c');
					ret.push_back(';');
				}
				else if (e == '\n') {
					ret.push_back('|');
					ret.push_back('l');
					ret.push_back(';');
				}
				else if (e == '|') {
					ret.push_back('|');
					ret.push_back('o');
					ret.push_back(';');
				}
				else if (e == ';') {
					ret.push_back('|');
					ret.push_back('s');
					ret.push_back(';');
				}
				else if (e == ' ') {
					ret.push_back('|');
					ret.push_back('p');
					ret.push_back(';');
				}
				else {
					ret.push_back(e);
				}
			}
			return ret;
		}
	}

	template<class MODEL>
	class Table {
	private:
		struct PtrCmp {
			bool operator()(MODEL*a, MODEL*b) const {
				return *a<*b;
			}
		};
	public:
		std::multiset<MODEL> tb;

		using iterator = typename decltype(tb)::iterator;

		auto begin() {
			return tb.begin();
		}
		auto end() {
			return tb.end();
		}

		template<typename T>
		Table<T> OrderBy() {
			Table<T> ret;
			for (auto& i : tb) {
				ret.insert(T(i));
			}
			return ret;
		}

		template<typename T>
		Table<MODEL*> Where(T chk) {
			Table<MODEL*> ret;
			for (auto& i : tb) {
				if (chk(i)) {
					ret.insert(&i);
				}
			}
			return ret;
		}

		Table<MODEL*> Select() {
			Table<MODEL*> ret;
			for (auto& i : tb) {
				ret.insert(&i);
			}
			return ret;
		}

		std::vector<MODEL*> vec() {
			std::vector<MODEL*> ret;
			int i = 0;
			tb.begin();
			for (MODEL i : tb) {
				ret.push_back(&i);
			}
			std::sort(ret.begin(), ret.end(), PtrCmp());
			return ret;
		}

		decltype(*tb.begin()) first() {
			return *tb.begin();
		}

		template<class...R>
		void insert(R...c) {
			tb.insert(MODEL(c...));
		}

		void insertRaw(MODEL c) {
			tb.insert(c);
		}

		template<class F>
		void Delete(F cond) {
			for (auto i = tb.begin(); i != tb.end(); i++) {
				if (cond(*i)) {
					tb.erase(i);
				}
			}
		}

		template<class _M, class F>
		auto map(F func)->Table<_M> {
			Table<_M> ret;
			for (auto& i : tb) {
				ret.insert(func(i));
			}
			return ret;
		}
	};

	template<class MODEL>
	class pTable {
	private:
		struct PtrCmp {
			bool operator()(MODEL* a, MODEL* b) const {
				return *a < *b;
			}
		};
	public:
		std::multiset<MODEL*, PtrCmp> tb;

		using iterator = typename decltype(tb)::iterator;

		auto begin() {
			return tb.begin();
		}
		auto end() {
			return tb.end();
		}

		template<typename T>
		Table<T> OrderBy() {
			Table<T> ret;
			for (auto& i : tb) {
				ret.insert(T(*i));
			}
			return ret;
		}

		template<typename T>
		Table<MODEL*> Where(T chk) {
			Table<MODEL*> ret;
			for (auto& i : tb) {
				if (chk(*i)) {
					ret.insert(i);
				}
			}
			return ret;
		}

		Table<MODEL*> Select() {
			Table<MODEL*> ret;
			for (auto& i : tb) {
				ret.insert(i);
			}
			return ret;
		}

		std::vector<MODEL*> vec() {
			std::vector<MODEL*> ret;
			int i = 0;
			tb.begin();
			for (MODEL* i : tb) {
				ret.push_back(i);
			}
			std::sort(ret.begin(), ret.end(), PtrCmp());
			return ret;
		}

		decltype(*tb.begin()) first() {
			return *tb.begin();
		}

		template<class...R>
		void insert(R...c) {
			MODEL* rec = new MODEL(c...);
			tb.insert(rec);
		}

		void insertRaw(MODEL c) {
			MODEL* rec;
			*rec = c;
			tb.insert(rec);
		}

		template<class F>
		void Delete(F cond) {
			for (auto i = tb.begin(); i != tb.end(); i++) {
				if (cond(**i)) {
					tb.erase(i);
				}
			}
		}

		template<class _M, class F>
		auto map(F func)->Table<_M> {
			Table<_M> ret;
			for (auto& i : tb) {
				ret.insert(func(*i));
			}
			return ret;
		}
	};
	
	class Model {};

	class Database {
	public:
		template<class O, class...T>
		static void write(O& out, const T&...arg) {
			(([&]() {
				std::ostringstream oss;
				oss << arg;
				out << code::encode(oss.str());;
				out << "&";
			})(), ...);
			
			out.seekp(-1, std::ios_base::end);
			out << '\n';
		}

		template<class...T>
		static void read(std::string from, T&...arg) {
			std::istringstream record(from);
			std::string buff2;

			(([&]() {
				std::getline(record, buff2, '&');
				if (is_string<decltype(arg)>::value) {
					arg = code::decode(buff2);
				}
				else {
					std::istringstream(code::decode(buff2)) >> arg;
				}
			})(), ...);
		}
	};

	class Controller {};
}

#endif
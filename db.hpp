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

	namespace http {
		void _urldecode(char* dst, const char* src) {
			char a, b;
			while (*src) {
				if ((*src == '%') &&
					((a = src[1]) && (b = src[2])) &&
					(isxdigit(a) && isxdigit(b))) {
					if (a >= 'a')
						a -= 'a' - 'A';
					if (a >= 'A')
						a -= ('A' - 10);
					else
						a -= '0';
					if (b >= 'a')
						b -= 'a' - 'A';
					if (b >= 'A')
						b -= ('A' - 10);
					else
						b -= '0';
					*dst++ = 16 * a + b;
					src += 3;
				}
				else if (*src == '+') {
					*dst++ = ' ';
					src++;
				}
				else {
					*dst++ = *src++;
				}
			}
			*dst++ = '\0';
		}

		std::string urlDecode(std::string SRC) {
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

		static void hexchar(unsigned char c, unsigned char& hex1, unsigned char& hex2) {
			hex1 = c / 16;
			hex2 = c % 16;
			hex1 += hex1 <= 9 ? '0' : 'a' - 10;
			hex2 += hex2 <= 9 ? '0' : 'a' - 10;
		}

		static std::string urlEncode(std::string s) {
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
		/*template<class O, class T>
		static void write(O& out, const T&arg) {
			std::ostringstream oss;
			oss << arg;
			out << http::urlEncode(oss.str());;
			out << '\n';
		}*/
		template<class O, class...T>
		static void write(O& out, const T&...arg) {
			(([&]() {
				std::ostringstream oss;
				oss << arg;
				out << http::urlEncode(oss.str());;
				out << "&";
			})(), ...);
			// write(out, arg...);
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
					arg = http::urlDecode(buff2);
				}
				else {
					std::istringstream(http::urlDecode(buff2)) >> arg;
				}
			})(), ...);
		}
	};

	class Controller {};
}

#endif
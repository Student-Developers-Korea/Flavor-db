#ifndef __DB_HPP__
#define __DB_HPP__

#include <string>
#include <set>
#include <map>
#include <vector>
#include <sstream>
#include <ctime>

namespace sl {

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
			char* st = (char*)(malloc(SRC.length() + 1));
			strcpy(st, SRC.c_str());
			_urldecode(st, SRC.c_str());
			ret = st;
			free(st);
			return (ret);
		}

		strmap form(std::string encoded) {
			strmap post;
			std::istringstream iss(encoded);
			std::string buff;
			while (std::getline(iss, buff, '&')) {
				std::string key;
				std::string value;
				int t = 0;
				for (auto i : buff) {
					if (i == '=') {
						break;
					}
					t++;
				}
				key = buff.substr(0, t);
				value = buff.substr(t + 1);
				post.insert(std::pair(key, value));
			}
			return post;
		}

		static void hexchar(unsigned char c, unsigned char& hex1, unsigned char& hex2) {
			hex1 = c / 16;
			hex2 = c % 16;
			hex1 += hex1 <= 9 ? '0' : 'a' - 10;
			hex2 += hex2 <= 9 ? '0' : 'a' - 10;
		}

		static std::string urlEncode(std::string s) {
			const char* str = s.c_str();
			std::vector<char> v(s.size());
			v.clear();
			for (size_t i = 0, l = s.size(); i < l; i++)
			{
				char c = str[i];
				if ((c >= '0' && c <= '9') ||
					(c >= 'a' && c <= 'z') ||
					(c >= 'A' && c <= 'Z') ||
					c == '-' || c == '_' || c == '.' || c == '!' || c == '~' ||
					c == '*' || c == '\'' || c == '(' || c == ')')
				{
					v.push_back(c);
				}
				else if (c == ' ')
				{
					v.push_back('+');
				}
				else
				{
					v.push_back('%');
					unsigned char d1, d2;
					hexchar(c, d1, d2);
					v.push_back(d1);
					v.push_back(d2);
				}
			}

			return std::string(v.cbegin(), v.cend());
		}
	}

	template<class MODEL>
	class Table {
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

	class Model {};

	class Database {
	private:
		template<class O>
		static auto encode(O a) {
			return http::urlEncode((std::ostringstream() << a).str());
		}
	public:
		template<class O, class...T>
		static void write(O& out, T&...arg) {
			(([&]() {
				out << encode(arg) << "&";
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
				std::istringstream(http::urlDecode(buff2)) >> arg;
				})(), ...);
		}
	};

	class Controller {};
}

#endif
#include <format>
#include <string>

int main() {
	std::string a = std::format("{}.{}.{}", "Hello", "World", "C++");
	return 0;
}
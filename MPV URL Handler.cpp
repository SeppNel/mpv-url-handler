#include "Registry.hpp" // https://github.com/m4x1m1l14n/Registry/tree/master
#include <iostream>
#include <libloaderapi.h>
#include <yaml-cpp/yaml.h>
#include <filesystem>
#include <vector>
#include <regex>

using namespace m4x1m1l14n;
using std::cout;
using std::string;
using std::wstring;
using std::vector;

const vector<string> protocolWhitelist = { "http", "https" };

void wait4input() {
	getchar();
}

template<typename T>
bool inVector(T needle, vector<T> haystack) {
	return std::find(haystack.begin(), haystack.end(), needle) != haystack.end();
}

void createRegistryKey(const wstring exePath) {
	try {
		wstring value = L"\"" + exePath + L"\" \"%1\"";
		auto access = Registry::DesiredAccess::Read | Registry::DesiredAccess::Write;

		auto key = Registry::ClassesRoot->Create(L"mpv", access);
		key->SetString(L"URL Protocol", L"");
		key = Registry::ClassesRoot->Create(L"mpv\\shell\\open\\command", access);
		wstring defaultValue;
		if (key->HasValue(L"(Default)")) {
			defaultValue = L"(Default)";
		}
		else if (key->HasValue(L"(Predeterminado)")) {
			defaultValue = L"(Predeterminado)";
		}
		key->SetString(defaultValue, value);
	}
	catch (const std::exception&) {
		cout << "Exception during registry key creation\n";
		cout << "Run as admin if you didn't\n";
		wait4input();
		exit(EXIT_FAILURE);
	}

}

bool keyExists() {
	try {
		bool returnVal;
		auto key = Registry::ClassesRoot;
		if (key->HasKey(L"mpv")) {
			returnVal = true;
		}
		else {
			returnVal = false;
		}

		return returnVal;
	}
	catch (const std::exception&) {
		cout << "Exception during registry key checking\n";
		exit(EXIT_FAILURE);
	}
}

string getFilenameFromPath(string path) {
	//Remove everything before the last "\"
	auto it = std::find(path.rbegin(), path.rend(), '\\'); //escape the escape character
	if (it != path.rend()) {
		path.erase(path.begin(), it.base());
	}

	return path;
}

YAML::Node loadConfig() {
	try {
		return YAML::LoadFile("config.yml");
	}
	catch (std::exception& e) {
		cout << "Exception loading config file:  " << e.what() << "\n";
		wait4input();
		exit(EXIT_FAILURE);
	}
}

void createHandler() {
	wchar_t buffer[MAX_PATH];
	wstring exePath(buffer, GetModuleFileNameW(NULL, buffer, MAX_PATH));
	createRegistryKey(exePath);
}

void removeHandler() {
	try {
		Registry::ClassesRoot->Delete(L"mpv");
	}
	catch (const std::exception& e) {
		cout << "Exception removing handler:  " << e.what() << "\n";
		cout << "Run as admin if you didn't\n";
		exit(EXIT_FAILURE);
	}
}

void updateHandler() {
	if (keyExists()) {
		removeHandler();
	}
	createHandler();
}

void argumentHandler(int argc, char* argv[]) {
	string option(argv[1]);
	if (option == "-r") {
		if (!keyExists()) {
			cout << "Handler is not installed\n";
			exit(EXIT_FAILURE);
		}

		removeHandler();
		cout << "Handler removed\n";
		exit(EXIT_SUCCESS);
	}
	else if (option == "-u") {
		updateHandler();
		cout << "Handler updated\n";
		exit(EXIT_SUCCESS);
	}
}

std::string decodeURIComponent(std::string encoded) { //https://gist.github.com/arthurafarias/56fec2cd49a32f374c02d1df2b6c350f

	std::string decoded = encoded;
	std::smatch sm;
	std::string haystack;

	int dynamicLength = decoded.size() - 2;

	if (decoded.size() < 3) return decoded;

	for (int i = 0; i < dynamicLength; i++) {

		haystack = decoded.substr(i, 3);

		if (std::regex_match(haystack, sm, std::regex("%[0-9A-F]{2}"))) {
			haystack = haystack.replace(0, 1, "0x");
			std::string rc = { (char)std::stoi(haystack, nullptr, 16) };
			decoded = decoded.replace(decoded.begin() + i, decoded.begin() + i + 3, rc);
		}

		dynamicLength = decoded.size() - 2;

	}

	return decoded;
}

int main(int argc, char* argv[]) {
	if (!keyExists()) {
		createHandler();
		cout << "Handler Registered succesfully\n";
		wait4input();
		exit(EXIT_SUCCESS);
	}

	if (argc != 2) {
		cout << "Invalid arguments\n";
		exit(EXIT_FAILURE);
	}

	argumentHandler(argc, argv);

	char buffer[MAX_PATH];
	string exePath(buffer, GetModuleFileNameA(NULL, buffer, MAX_PATH));
	std::string exeName = getFilenameFromPath(exePath);

	//Change working path
	string workingPath = exePath.substr(0, exePath.size() - exeName.size() - 1);
	std::filesystem::current_path(workingPath);

	YAML::Node config = loadConfig();

	string mpvPath = config["mpvPath"].as<string>();
	string fileToPlay(argv[1]);
	fileToPlay = decodeURIComponent(fileToPlay);
	fileToPlay = fileToPlay.substr(6, fileToPlay.npos);

	string protocol = fileToPlay.substr(0, fileToPlay.find(':'));

	if (!inVector(protocol, protocolWhitelist)) {
		cout << "Protocol not allowed\n";
		wait4input();
		exit(EXIT_FAILURE);
	}

	string cmdArgs = "mpv.exe " + fileToPlay;

	STARTUPINFOA info = { sizeof(info) };
	PROCESS_INFORMATION processInfo;
	CreateProcessA(mpvPath.c_str(), cmdArgs.data(), NULL, NULL, TRUE, 0, NULL, NULL, &info, &processInfo);
}
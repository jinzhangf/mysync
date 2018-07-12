#include <iostream>
#include <sstream>
#include <map>
#include <set>
#include <string>
#include <cstdlib>
#include <cctype>
#include <windows.h>
#include <sys/stat.h>

using namespace std;

string trim(const string &str)
{
	char *tmp = new char[str.size() + 1];
	int i = 0;
	for (int j = 0; j < str.size(); ++j) {
		if (isprint(str[j])) {
			tmp[i++] = (str[j] == '\\' ? '/': str[j]);
		}
	}
	tmp[i] = '\0';
	string ret(tmp);
	delete []tmp;
	return ret;
}

string exec(const char* cmd) 
{
	FILE* pipe = popen(cmd, "r");
	if (!pipe) return cmd + string (" ERROR");
	static char buffer[4096];
	string result = "";
	
	while(!feof(pipe)) {
		if(fgets(buffer, 4096, pipe) != NULL) {
			for (size_t i = 0; i < 4096; ++i)
			{
				if (buffer[i] == '\0')
					break;
				
				if (buffer[i] == '\\')
					buffer[i] = '/';
			}
			result += buffer;
		}
	}
	
	pclose(pipe);
	return result;
}

int main(int argc, char* argv[])
{
	if (argc < 2) {
		string line;
		while (getline(cin, line)) {
			for (size_t i = 0; i < line.size(); ++i) {
				if (line[i] == '\\')
					line[i] = '/';			
				if (line[i] == '\r')
					line[i] = '\n';
			}
			cout << line << endl;
		}
	}
	
	string cmd;
	for (size_t i = 1; i < argc; ++i)
	{
		cmd.append(trim(argv[i])).append(" ");
	}
	cout << exec(cmd.c_str()) << flush;
	
	return 0;
}

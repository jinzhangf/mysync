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

string exec(const char* cmd) 
{
	FILE* pipe = popen(cmd, "r");
	if (!pipe) return "ERROR";
	static char buffer[4096];
	string result = "";
	
	while(!feof(pipe)) {
		if(fgets(buffer, 4096, pipe) != NULL)
			result += buffer;
	}
	
	pclose(pipe);
	return result;
}

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

int scp_file(const string &src_path, const string &dst_path, const string &file)
{

	string cmd = "scp -P 36000 " + src_path + "/" + file +" dylanfang@10.12.142.125:" + dst_path + "/" + file + " > /dev/null 2>&1";
	int ret = system(cmd.c_str());
	if (ret != 0) {
		cout << "[Error]" << cmd << endl;
	}
	return ret;
}

int main(int argc, char* argv[])
{
	int ret = system("svn info > /dev/null 2>&1");
	if (ret != 0) {//svn path error
		cerr << "current path is not a svn path" << endl;
		return -1;
	}
	
	string src_path = trim(exec("pwd"));
	string cmd = "echo " + src_path + " | sed 's/cygdrive\\/e/mnt\\/data/'";
	string dst_path = trim(exec(cmd.c_str()));

	cout << "src_path=" << src_path << endl;
	cout << "dst_path=" << dst_path << endl << endl;
	map<string, string> table;
	set<string> visited;
	set<string> deleted;

	string last_st = exec("svn st");
	string curr_st;
	cout << endl << "-------------------------------------svn status-------------------------------------" << endl; 
	cout << last_st << "------------------------------------------------------------------------------------" << endl;

sync:
	curr_st = exec("svn st");
	if (curr_st != last_st) {
		cout << endl << endl << "-------------------------------------svn status-------------------------------------" << endl; 
		cout << curr_st << "------------------------------------------------------------------------------------" << endl;
		last_st = curr_st;
	}

	visited.clear();
	deleted.clear();
	cmd = "svn st | awk '$1 == \"M\" || $1 == \"?\" {print $NF}'";
	istringstream files(exec(cmd.c_str()));
	string file;
	struct stat buf;
	while (files >> file) {
		file = trim(file);

		stat(file.c_str(), &buf);
		if (S_ISDIR(buf.st_mode)) continue; //忽略文件夹

		visited.insert(file);
		cmd = "md5sum " + file + " | awk '{print $1}'";
		string md5sum = trim(exec(cmd.c_str())); 
		if (table.count(file) != 0 && table[file] == md5sum)
			continue;

		if (scp_file(src_path, dst_path, file))
			return -1;

		table[file] = md5sum;
		cout << "sync\t--->>>\t" << file << endl;
	}
	
	for (auto t : table) {
		if (visited.count(t.first) == 0) {
			if(scp_file(src_path, dst_path, t.first))
			       return -2;
			
			cout << "sync\t--->>>\t" << t.first << endl;
			deleted.insert(t.first);
		}
	}
	for (auto t : deleted) {
		table.erase(t);
	}	

	
	Sleep(200);
	goto sync;

	return 0;
}


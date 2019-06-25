#include <iostream>
#include <sstream>
#include <map>
#include <set>
#include <string>
#include <cstdlib>
#include <cctype>
#include <sys/stat.h>

#ifdef __APPLE__
#include <unistd.h>
#else
#include <windows.h>
#endif

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

string get_target()
{
	static string target = "";
	if (!target.empty()) return target;

	char *p = std::getenv("TARGET");
	if (p == NULL || strlen(p) == 0) {
		target = "linfu@10.224.9.170";
	}
	else {
		target = string (p);
	}
	return target;
}

int scp_file(const string &src_path, const string &dst_path, const string &file)
{
	if (file == ".git")
		return 0;
	// tx : -P 36000
	//string cmd = "scp -P 36000 -r " + src_path + "/" + file +" dylanfang@9.77.4.129:" + dst_path + "/" + file + " > /dev/null 2>&1";
	string cmd = "scp -r " + src_path + "/" + file + " " + get_target() + ":" + dst_path + "/" + file + " > /dev/null 2>&1";
	int ret = system(cmd.c_str());
	if (ret != 0) {
		cout << "[Error]" << cmd << endl;
	}
	string time = trim(exec("date +'%Y-%m-%d %T'"));
	cout << "[" << time << "] " << "sync\t--->>>\t" << file << endl;
	
	return ret;
}

int svn_main(int argc, char* argv[])
{	
	string src_path = trim(exec("pwd"));
	#ifdef __APPLE__
	string cmd = "echo " + src_path + " | sed 's/Users/home/'";
	#else
	string cmd = "echo " + src_path + " | sed 's/cygdrive\\/./mnt\\/data/'";
	#endif
	string dst_path = trim(exec(cmd.c_str()));
	
	if (argc > 1)
	{
		for (size_t i = 1; i < argc; ++i) {
			if (0 != scp_file(src_path, dst_path, trim(argv[i])))
				break;
		}
		return 0;
	}

	cout << "target=" << get_target() << endl;
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
		//cout << "sync\t--->>>\t" << file << endl;
	}
	
	for (auto t : table) {
		if (visited.count(t.first) == 0) {
			if(scp_file(src_path, dst_path, t.first))
			       return -2;
			
			//cout << "sync\t--->>>\t" << t.first << endl;
			deleted.insert(t.first);
		}
	}
	for (auto t : deleted) {
		table.erase(t);
	}	

	#ifdef __APPLE__
	usleep(200000);
	#else
	Sleep(200);
	#endif
	goto sync;

	return 0;
}


int git_main(int argc, char* argv[])
{
	string src_path = trim(exec("pwd"));
	#ifdef __APPLE__
	string cmd = "echo " + src_path + " | sed 's/\\/Users\\/linfu/~/'";
	#else
	string cmd = "echo " + src_path + " | sed 's/cygdrive\\/./mnt\\/data/'";
	#endif
	string dst_path = trim(exec(cmd.c_str()));
	
	if (argc > 1)
	{
		for (size_t i = 1; i < argc; ++i) {
			if (0 != scp_file(src_path, dst_path, trim(argv[i])))
				break;
		}
		return 0;
	}

	cout << "target=" << get_target() << endl;
	cout << "src_path=" << src_path << endl;
	cout << "dst_path=" << dst_path << endl << endl;
	map<string, string> table;
	set<string> visited;
	set<string> deleted;

	string last_st = exec("git status -s .");
	string curr_st;
	cout << endl << "-------------------------------------git status -s----------------------------------" << endl; 
	cout << last_st << "------------------------------------------------------------------------------------" << endl;

sync2:
	curr_st = exec("git status -s .");
	if (curr_st != last_st) {
		cout << endl << endl << "-------------------------------------git status -s----------------------------------" << endl; 
		cout << curr_st << "------------------------------------------------------------------------------------" << endl;
		last_st = curr_st;
	}

	visited.clear();
	deleted.clear();
	cmd = "git status -s . | awk '$1 == \"MM\" || $1 == \"AM\" || $1 == \"M\" || $1 == \"A\" || $1 == \"??\" {print $2}'";
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
		//cout << "sync\t--->>>\t" << file << endl;
	}

	for (auto t : table) {
		if (visited.count(t.first) == 0) {
			if(scp_file(src_path, dst_path, t.first))
			       return -2;

			//cout << "sync\t--->>>\t" << t.first << endl;
			deleted.insert(t.first);
		}
	}
	for (auto t : deleted) {
		table.erase(t);
	}

	#ifdef __APPLE__
	usleep(200000);
	#else
	Sleep(200);
	#endif
	goto sync2;

	return 0;
}

int main(int argc, char* argv[])
{
	int ret = system("svn info > /dev/null 2>&1");
	if (ret == 0) { //svn proj
		return svn_main(argc, argv);
	}

	ret = system("git status -s . > /dev/null 2>&1");
	if (ret == 0) { //git proj
		return git_main(argc, argv);
	}

	cerr << "[Error] unknow path, not svn or git proj." << endl;
	return -1;
}


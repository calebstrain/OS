#include <iostream>
#include <queue>
#include <string>
#include <vector>
#include <cstring>
#include <fstream>
#include <regex>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

using namespace std;

struct Character
{
	int freq;
	char c;
	int ascii;
	string message;
	string code = "";
	Character(int freq, char c, int ascii )
	{
		this->freq = freq;
		this->ascii = ascii;
		this->c = c;
	}
	Character(string message, char c )
	{
		this->message = message;
		this->c = c;
		freq = ascii = 0;
	}
};

struct Compare
{
	bool operator()(const Character& a, const Character& b) const
	{
		if (a.freq == b.freq)
		{
			return a.ascii > b.ascii;
		}
		else
			return a.freq < b.freq;
	}
};

void createList(int freq[], priority_queue<Character, vector<Character>, Compare>& q)
{
	for (int i = 0; i < 113; i++)
	{
		if (freq[i] != 0) 
		{
			Character c(freq[i], i, i );
			q.push(c);
		}
	}
}

void createCode(const char character, string& message, int processNumber)
{
	string code;
	for (char c : message)
	{
		if (c == character)
			code += "1";
		else
			code += "0";
	}

	ofstream ofs(to_string(processNumber) + ".txt");
	ofs << code; ofs.close();
	exit(0);
}

ostream& operator<<(ostream& os, const Character& c) 
{
	string s(1, c.c);
	if (c.code == "") 
	{
		os << ((c.c == '\n') ? "<EOL>" : s) + " frequency = " + to_string(c.freq);
	}
	else
		os << "Symbol " + ((c.c == '\n') ? "<EOL>" : s) + " code: " + c.code;
	
	return os;
}

bool replaceAll(std::string& str, const std::string& from, const std::string& to) {
	if (from.empty())
		return false;
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length(); 
	}
	return true;
}

int main()
{
	char input;
	string message;
	priority_queue<Character, vector<Character>, Compare> q;
	queue<Character> messageQueue;
	int freq[113];

	memset(freq, 0, sizeof(freq));

	while (cin.get(input))
	{
		message += input;
		freq[input]++;
	}

	createList(freq, q);
	int size = q.size();
	// Version 1

	pid_t pid;
	int x = 0;
	while (!q.empty())
	{ 
		Character c( message, q.top().c );
		cout << q.top() << endl;
		if ((pid = fork()) == 0) 
		{
			createCode(c.c, message, x); 
		}
		else // else is superfluous because we called exit(0) in our child process, but we will include it anyways 
		{
			message.erase(remove(message.begin(), message.end(), q.top().c), message.end());
			messageQueue.push( c );
			x++;
			q.pop();
		}
	}

	// We must wait here in order to get rid of our zombie-state child processes 
	for( int i = 0; i < messageQueue.size(); i++)
	{
		wait(0);
	}

	int count = 0;
	while(!messageQueue.empty())
	{
		if (replaceAll(messageQueue.front().message, "\n", "<EOL>"))
			cout << ((messageQueue.size() == size) ? "Original" : "Remaining") << " Message: " << messageQueue.front().message << endl;
		ifstream ifs(to_string(count++) + ".txt");
		string contents((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
		messageQueue.front().code = contents;
		cout << messageQueue.front() << endl;
		messageQueue.pop();
	}

}
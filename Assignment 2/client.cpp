#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> //defines the structure hostent
#include <string>
#include <iostream>
#include <queue>
#include <pthread.h>
#include <algorithm>
#include <vector>

using namespace std;

void error(string msg)
{
	perror(msg.c_str());
	exit(0);
}

struct Character // this struct is re-used with different values
{
	int freq;
	char c;
	int ascii;
	char* hostname;
	int portno;
	string message;
	string code = "";
	Character() {}
	Character(int freq, char c, int ascii)
	{
		this->freq = freq;
		this->ascii = ascii;
		this->c = c;
	}
	void setValues(string message, char c, char* hostname, int portno)
	{
		this->message = message;
		this->c = c;
		freq = ascii = 0;
		this->hostname = hostname;
		this->portno = portno;
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

void createQueue(int freq[], priority_queue<Character, vector<Character>, Compare>& q)
{
	for (int i = 0; i < 113; i++)
	{
		if (freq[i] != 0)
		{
			Character c(freq[i], i, i);
			q.push(c);
		}
	}
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

bool replaceAll(string& str, const string& from, const string& to) {
	if (from.empty())
		return false;
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != string::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length();
	}
	return true;
}

void* pthread_function(void* character_pointer)
{

	int sockfd, portno, n;
	struct Character* character_ptr = (struct Character*)character_pointer;

	struct sockaddr_in serv_addr;
	struct hostent* server;

	char buffer[256];

	portno = character_ptr->portno;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		error("ERROR opening socket");

	server = gethostbyname(character_ptr->hostname);
	if (server == NULL) {
		fprintf(stderr, "ERROR, no such host\n");
		exit(0);
	}

	bzero((char*)& serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char*)server->h_addr, (char*)& serv_addr.sin_addr.s_addr, server->h_length);
	serv_addr.sin_port = htons(portno);
	if (connect(sockfd, (struct sockaddr*) & serv_addr, sizeof(serv_addr)) < 0)
		error("ERROR connecting");

	n = write(sockfd, (character_ptr->message + (character_ptr->c)).c_str(), strlen((character_ptr->message + (character_ptr->c)).c_str()));
	if (n < 0) error("ERROR writing to socket");

	n = read(sockfd, buffer, 255);
	if (n < 0) error("ERROR reading from socket");

	character_ptr->code = buffer;
	return NULL;

}

int main(int argc, char* argv[])
{
	char input;
	string message;
	priority_queue<Character, vector<Character>, Compare> q;
	int freq[113];

	memset(freq, 0, sizeof(freq));

	if (argc < 3) {
		fprintf(stderr, "usage %s hostname port\n", argv[0]);
		exit(0);
	}

	while (cin.get(input))
	{
		message += input;
		freq[input]++;
	}

	// Create frequency queue 

	createQueue(freq, q);
	int size = q.size();
	static struct Character* c = new struct Character[size];

	pthread_t threads[size];

	for (int i = 0; i < size; i++)
	{
		//cout << "Creating thread " << i << endl;
		c[i].setValues(message, q.top().c, argv[1], atoi(argv[2]));
		cout << q.top() << endl;
		message.erase(remove(message.begin(), message.end(), q.top().c), message.end());
		q.pop();
		if (pthread_create(&threads[i], NULL, pthread_function, &c[i]))
		{
			fprintf(stderr, "Error creating thread\n");
			return 1;
		}
	}

	for (int i = 0; i < size; i++)
	{
		pthread_join(threads[i], NULL);
	}

	for (int i = 0; i < size; i++)
	{
		if (replaceAll(c[i].message, "\n", "<EOL>"))
			cout << ((i == 0) ? "Original" : "Remaining") << " Message: " << c[i].message << endl;
		cout << c[i] << endl;
	}

	delete[] c;

	return 0;
}
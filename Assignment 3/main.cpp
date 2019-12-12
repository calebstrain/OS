#include <iostream>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h> 
#include <stdexcept>
#include <string>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t parent_cond = PTHREAD_COND_INITIALIZER;

using namespace std;

struct Args
{
	string message;
	char character;
	string code;
	int id;
	int turn;
	// Initialize turn to -1 so that no thread will start the decompression until all threads have been created and are in memory 
	Args() { message = " "; turn = -1; }
};

void* pthread_function(void* args)
{
	pthread_mutex_lock(&mutex);

	Args* arg = (Args*)args;
	char character = arg->character;
	string code = arg->code;
	int id = arg->id;
	// This threads local variables have been set, main thread can now create another thread
	pthread_cond_signal(&parent_cond);

	// Use id and turn to synchronize threads
	while (id != arg->turn) {
		pthread_cond_wait(&cond, &mutex);
	}
	pthread_mutex_unlock(&mutex);

	for (int i = 0; i < code.length(); i++)
	{
		if (code[i] == '1')
		{
			arg->message.insert(i, string(1, character));
		}
	}
	cout << ((character == '\n') ? "<EOL>" : string(1, character)) << " Binary code = " << code << endl;

	pthread_mutex_lock(&mutex);
	arg->turn--;
	pthread_cond_broadcast(&cond);
	pthread_mutex_unlock(&mutex);

	pthread_exit(NULL);
}


int main()
{
	char character;
	string line;
	int id = 0;
	static Args args;
	pthread_t threads[100];

	// In this while loop we get input, and set up each thread with different local variables. Threads CANNOT finish execution in this loop. 
	while (getline(cin, line))
	{
		character = (line[0] == '<') ? '\n' : line[0];
		string code = (character == '\n') ? line.substr(6, line.size()) : line.substr(2, line.size());

		pthread_mutex_lock(&mutex); // Aquire the mutex to ensure exclusion 
		args.code = code, args.character = character; args.id = id;
		pthread_create(&threads[id++], NULL, pthread_function, &args);
		// Do not create another thread until the local variables have been set for the thread that was just created. We CANNOT use a busy wait here. 
		pthread_cond_wait(&parent_cond, &mutex);
		pthread_mutex_unlock(&mutex);
	}

	// Threads will began decompression. Synchronized in the correct order (starting with the smallest symbol).  
	args.turn = id - 1;
	pthread_cond_broadcast(&cond);

	// We must call pthread_join outside of the create thread loop to guarantee that there is more than one thread in memory at a time. 
	// We must wait until all pthreads are done execution before outputting the final decompressed message. 
	for (int i = 0; i < id; i++)
	{
		pthread_join(threads[i], NULL);
	}

	cout << "Decompressed file contents:\n" << args.message << endl;

	pthread_mutex_destroy(&mutex);
	pthread_cond_destroy(&cond);
	pthread_cond_destroy(&parent_cond);

	return 0;
}
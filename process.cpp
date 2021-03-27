#include <iostream>
#include <string>
#include <unistd.h>
#include <csignal>
#include <sstream>
#include <fstream>
#include <sys/stat.h>
#include <fcntl.h>

using namespace std;
void signal_handler(int sig);
/**
* process number
*/
int processNum;
/**
* path of the process output file
*/
string process_output_path;
/**
* pointer to the file that process will write
*/
ofstream process_output;
/**
*
* @author Hamza AKYILDIZ
*
* changes the default signal handling functions to the sig_handler() function for the relavent signals
* 
* initalize the processNum, process_output_path and process_out
*
* stops itself until watchdog sends SIGCONT to arrange printing order
* 
* waits until signal comes 
*/
int main(int argc, char const *argv[])
{
	signal(SIGTERM,signal_handler);
	signal(SIGHUP,signal_handler);
	signal(SIGINT,signal_handler);
	signal(SIGILL,signal_handler);
	signal(SIGTRAP,signal_handler);
	signal(SIGFPE,signal_handler);
	signal(SIGBUS,signal_handler);
	signal(SIGSEGV,signal_handler);
	signal(SIGXCPU,signal_handler);

	processNum=stoi(argv[1]);
	process_output_path=argv[2];
	process_output.open(process_output_path,ios_base::app);
	raise(SIGSTOP);
	process_output<<"P"<<processNum<<" is waiting for a signal"<<endl;
	while(1){
		sleep(1);
	}
	return 0;
}
/**
*
* @param sig integer value of the in-coming signal
* 
* handles the signal by checking if it is equal to 15 which is SIGTERM signal
*
* If it is 15, prints the message and exits
*
* else prints the message
*/
void signal_handler(int sig){
	if(sig==15){
		process_output<<"P"<<processNum<<" received signal 15, terminating gracefully"<<endl;
		exit(0);
	}
	process_output<<"P"<<processNum<<" received signal " <<sig <<endl;


}
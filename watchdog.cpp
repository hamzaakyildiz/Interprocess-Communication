#include <iostream>
#include <string>
#include <unistd.h>
#include <csignal>
#include <sstream>
#include <fstream>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <cstring>

using namespace std;

void start();
void childProcess(int processNum);
void send();
void killAll();
void sig_handler(int sig);
void murder(int deadPid);

/**
* number of processes
*/
int num_of_process;
/**
* path of the watchdog putput file
*/
string watchdog_output_path;
/**
* path of the process output file
*/
string process_output_path;
/**
* list of the child process pid
*/
int* pidList;
/**
* pointer to the file that watchdog will write
*/
ofstream watchdog_output;
/**
* pointer to the file that process will write
*/
ofstream process_output;
/**
* the pipe that is used to communicate with executor.cpp
*/
int unnamedPipe;
/**
* char pointer to keep the path of pipe
*/
char * myfifo = (char*) "/tmp/myfifo";
/**
*
* @author Hamza AKYILDIZ
*
* changes SIGTERM handling function to the sig_handler() function

* initializes the command line parameters which are num_of_process, watchdog_output_path, process_output_path.

* opens the files that will be written the outputs on

* writes its(watchdog) pid into the pipe.

* calls start() function

* waits for child processes to be dead and call murder() function with the passsing parameter of the return value of wait(NULL)
* 
*/
int main(int argc, char const *argv[])
{
	signal(SIGTERM,sig_handler);

	mkfifo(myfifo, 0666);
	unnamedPipe = open(myfifo,O_WRONLY);

	num_of_process = stoi(argv[1]);
	pidList = (int*) malloc((num_of_process+1)*sizeof(int));
	pidList[0]=getpid();
	char write_msg[30];
	sprintf(write_msg,"P%d %d",0,pidList[0]);
	write(unnamedPipe,write_msg,30);
	process_output_path=argv[2];
    watchdog_output_path=argv[3];
    watchdog_output.open(watchdog_output_path);
    process_output.open(process_output_path);
    start();
    int deadPid;
    while(1){
    	if((deadPid=wait(NULL))>0){//there is a murder
    		murder(deadPid);
    	}else{
    		sleep(1);
    	}
    }
    watchdog_output.close();
	return 0;
}
/**
*
* starts the processes by calling fork() function for num_of_process times

* child process with fork == 0 calls childProcess() function with parameter of i which is the process number

* parent process waits for each child process to stop , and lets it run to arrange the printing order of the child processes

* calls the send() function
*
*/
void start(){
	for (int i = 1; i <= num_of_process; i++)
	{
		if((pidList[i]=fork())==0){//child process
			childProcess(i);
		}else{					   //parent process
			watchdog_output<<"P"<<i<<" is started and it has a pid of "<< pidList[i]<<endl;
			waitpid(pidList[i],NULL,WUNTRACED);
			kill(pidList[i],SIGCONT);
		}
		
	}
	send();
}
/**
* @param processNumber number of the child process that will be restarted
*
* restarts the process with given process number. 

* updates the pidList[processNumber] with the return value of fork()

* parent process waits until child stops, and lets it run to print to arrange the printing order of the child processes

* writes the new pid of the process into the pipe
*
*/
void restart(int processNumber){
	if((pidList[processNumber]=fork())==0){//childprocess
		childProcess(processNumber);
	}else{
		waitpid(pidList[processNumber],NULL,WUNTRACED);
        watchdog_output<<"Restarting "<<"P"<<processNumber<<endl;
        watchdog_output<<"P"<<processNumber<<" is started and it has a pid of "<< pidList[processNumber]<<endl;
		kill(pidList[processNumber],SIGCONT);
    	char write_msg[30];
    	sprintf(write_msg,"P%d %d",processNumber,pidList[processNumber]);
		write(unnamedPipe,write_msg,30);
	}	
}
/**
*
* kills the child processes and waits until they are killed
*
*/
void killAll(){
	for (int i = 1; i <= num_of_process; ++i)
	{
		kill(pidList[i],SIGTERM);
		waitpid(pidList[i],NULL,WUNTRACED);
	}
}
/**
*
* @param processNum number of the child process that was created
*
* only child processes execute this function

* arranges passing arguments to the child process which are "./process", process number and process output path
*
*/
void childProcess(int processNum){
	char processNumber[10];
	sprintf(processNumber,"%d",processNum);

		string s = process_output_path;
		int n = process_output_path.length();
		char char_array1[n + 1];
		strcpy(char_array1, s.c_str());
    	s = "./process";
	    n = s.length();
	    char char_array2[n + 1];
	    strcpy(char_array2, s.c_str());

	char *const passedArr[] = {char_array2,processNumber,char_array1,NULL};
	execvp("./process",passedArr);
}
/**
*
* writes the pids of the child processes into the pipe
*
*/
void send(){
    char write_msg[30];
	for (int i = 1; i <= num_of_process; i++)
	{
		sprintf(write_msg,"P%d %d",i,pidList[i]);
		write(unnamedPipe,write_msg,30);
	}
}
/**
*
* @param sig integer value of the in-coming signal
*
* signal handling function for the SIGTERM signal which firstly calls killAll() function and exits
*
*/
void sig_handler(int sig){
    sleep(2);
	watchdog_output<<"Watchdog is terminating gracefully"<<endl;
	killAll();
	exit(0);
}
/**
*
* @param deadPid pid of the killed child process
* 
* checks if the deadPid is the pid of the head process(P1)

* if it is head process it calls killAll() and after that start()

* else calls the restart(int processNumber) with passing parameter i which is the process number that was killed
*
*/
void murder(int deadPid){
	for (int i = 1; i <= num_of_process; i++)
	{
		if(pidList[1]==deadPid){
			watchdog_output<<"P1 is killed, all processes must be killed"<<endl;
			killAll();
			watchdog_output<<"Restarting all processes"<<endl;
			start();
			break;
		}else if(pidList[i]==deadPid){
			watchdog_output<<"P"<<i<<" is killed"<<endl;
			restart(i);
			break;
		}
	}
}




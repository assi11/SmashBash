#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"

using namespace std;

void ctrlCHandler(int sig_num) {
    cout<<"smash: got ctrl-C"<<endl;
   // pid_t currentpid = getpid();
    pid_t pid = SmallShell::getInstance().GetFGPID();
    //SmallShell::getInstance().SetFGPID(currentpid);
    if(pid != -1){
    
    cout<<"smash: process "<<pid<<" was killed"<<endl;
    
    if(kill(pid,SIGINT)==-1){
        perror("smash error: kill failed");
        return;
    }
    SmallShell::getInstance().SetFGPID(-1);
    }else{
//    std::cout << SmallShell::getInstance().GetPromptName();
//    std::cout<<"> "<<std::flush;
    }
    // TODO: Add your implementation
}

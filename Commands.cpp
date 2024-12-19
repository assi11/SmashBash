#include <unistd.h>       // For syscall and system calls
#include <string.h>       // For string manipulations (std::memcpy, std::strcmp)
#include <iostream>       // For input/output operations (std::cin, std::cout, std::cerr)
#include <vector>         // For using the std::vector container
#include <sstream>        // For splitting strings (std::istringstream)
#include <sys/wait.h>     // For process management (waitpid)
#include <iomanip>        // For formatting output (std::setw)
#include "Commands.h"     // For command class definitions (assumed custom header file)
#include <algorithm>      // For sorting containers (std::sort)
#include <regex>          // For regular expressions (if needed in your code)
#include <sys/types.h>    // For types like pid_t, off_t, etc.
#include <dirent.h>       // For traditional directory handling (not needed with getdents64)
#include <iostream>       // Duplicated include - already included above
#include <iomanip>        // Duplicated include - already included above
#include <vector>         // Duplicated include - already included above
#include <algorithm>      // Duplicated include - already included above
#include <cstring>        // Duplicated include - already included above
#include <sys/syscall.h>  // For syscall numbers, like SYS_getdents64
#include <fcntl.h>        // For open flags (O_RDONLY, O_DIRECTORY)
#include <sys/stat.h>     // For stat and related structures (if needed)
#include <cerrno>         // For accessing error codes (errno)
#include <arpa/inet.h>    // For network address conversion (inet_ntop, inet_ntoa)
#include <netinet/in.h>   // For sockaddr_in structures
#include <sys/ioctl.h>    // For network interface control (ioctl)
#include <net/if.h>       // For interface request structures (ifreq)
#define BUF_SIZE 1024

using namespace std;
string _trim(const std::string &s) ;
void removeAmpersandvector(std::vector<std::string>& Argv);

const std::string WHITESPACE = " \n\r\t\f\v";

struct linux_dirent64 {
    uint64_t d_ino;      // inode number
    int64_t d_off;       // offset to next dirent
    unsigned short d_reclen; // length of this record
    unsigned char d_type;   // type of file (e.g., DT_DIR, DT_REG, etc.)
    char d_name[];        // filename (variable length)
};
#if 0
#define FUNC_ENTRY()  \
  cout << _PRETTY_FUNCTION_ << " --> " << endl;

#define FUNC_EXIT()  \
  cout << _PRETTY_FUNCTION_ << " <-- " << endl;
#else
#define FUNC_ENTRY()
#define FUNC_EXIT()
#endif




//////////////////// OUR FUNCTIONS ///////////////////////////////
//Helper Functions:-

    // Convert to char* for execvp
void removeTrailingAmpersand(std::string &str) {
    if (!str.empty() && str.back() == '&') {
        str.pop_back(); // Remove the last character
    }
}
int redirectOutput(const std::string& filename, int append) {
    int fileDescriptor;

    // Open the file for writing or appending
    if (!append) {

        fileDescriptor = open(filename.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0644);
    } else {
        fileDescriptor = open(filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    }
    if (fileDescriptor == -1) {
        perror("smash error: open failed");
        return -1; // Or handle the error as needed
    }


    // Redirect stdout to the file
    if (dup2(fileDescriptor, STDOUT_FILENO) < 0) {
        perror("smash error: redirecting failed");
        close(fileDescriptor);
        return -1;
    }

    // Close the file descriptor as it's now duplicated
    close(fileDescriptor);
    return 0;
}
const char* vectorToCmdLine(const std::vector<std::string>& input) {
    // Concatenate all strings with a space in between
    static std::string concatenated; // Needs to be static to persist outside this function
    concatenated.clear();            // Clear previous data if the function is reused
    std::ostringstream oss;

    for (size_t i = 0; i < input.size(); ++i) {
        oss << input[i];
        if (i < input.size() - 1) {
            oss << " "; // Add a space between words
        }
    }

    concatenated = oss.str();
    return concatenated.c_str();
}



std::vector<std::string> appendVectors(const std::vector<std::string>& vec1, const std::vector<std::string>& vec2) {
    std::vector<std::string> result = vec1; // Copy the first vector

    // Append vec2 elements starting from the second element (vec2[1] onwards)
    if (vec2.size() > 1) {
        result.insert(result.end(), vec2.begin() + 1, vec2.end());
    }

    return result;
}


std::string removeSingleQuotes(const std::string& input) {
    // Check if the string starts and ends with single quotes
    if (input.front() == '\'' && input.back() == '\'') {
        return input.substr(1, input.size() - 2); // Remove the first and last characters
    }
    return input; // Return the original string if no surrounding quotes
}
int isNumber(const std::string& str) {
    if (str.empty()) return 0; // An empty string is not a number
    for (char c : str) {
        if (!std::isdigit(c)) return 0; // If any character is not a digit, it's not a number
    }
    return 1; // All characters are digits
}
void removeAmpersandvector(std::vector<std::string>& Argv) {
    if (!Argv.empty()) { // Ensure the vector is not empty
        std::string& lastElement = Argv.back(); // Access the last element

        if (lastElement == "&") {
            // If the last element is exactly "&", erase it
            Argv.pop_back();
        } else if (!lastElement.empty() && lastElement.back() == '&') {
            // If the last element ends with '&', remove it
            lastElement.pop_back();
        }
    }
}
std::string removeSpacesBeforeAmpersand(const std::string& str) {
    // Find the position of the last '&'
    size_t ampPos = str.find_last_of('&');

    if (ampPos != std::string::npos) {
        // Trim spaces before the '&'
        size_t lastNonSpaceBeforeAmp = str.find_last_not_of(' ', ampPos - 1);

        // Reconstruct the string: everything before `lastNonSpaceBeforeAmp + 1`,
        // and everything from `ampPos` onward.
        return str.substr(0, lastNonSpaceBeforeAmp + 1) + str.substr(ampPos);
    }

    // If no '&' is found, return the original string
    return str;
}
void removeAmpersand(std::string& str) {
    // Check if the string is non-empty and ends with '&'
    _trim(str);
    removeSpacesBeforeAmpersand(str);
    if (!str.empty() && str.back() == '&') {
        str.pop_back(); // Remove the last character
    }

}

string preprocess(const string &str) {
    string result;
    for (size_t i = 0; i < str.size(); ++i) {
        if (str[i] == '>' || str[i] == '<' || str[i] == '|') {
            // Add spaces around the operator
            if (i > 0 && !isspace(str[i - 1])) {
                result += ' ';
            }
            result += str[i];
            if (i + 1 < str.size()) {
                if (str[i] == '>' && str[i + 1] == '>') {
                    // Handle '>>'
                    result += str[++i];
                } else if (str[i] == '|' && str[i + 1] == '&') {
                    // Handle '|&'
                    result += str[++i];
                }
                if (!isspace(str[i + 1])) {
                    result += ' ';
                }
            }
        } else {
            result += str[i];
        }
    }
    return result;
}

std::vector<std::string> split(const std::string& str) {
    std::vector<std::string> tokens;
    std::string token;
    bool inQuotes = false;
    char quoteChar = '\0';

    for (size_t i = 0; i < str.size(); ++i) {
        char ch = str[i];

        if (inQuotes) {
            // If inside quotes, add characters until the closing quote
            if (ch == quoteChar) {
                inQuotes = false; // End of quoted section
                tokens.push_back(token);
                token.clear();
            } else {
                token += ch;
            }
        } else {
            // If not in quotes
            if (ch == '\'' || ch == '"') {
                inQuotes = true; // Start a quoted section
                quoteChar = ch;
            } else if (std::isspace(ch)) {
                if (!token.empty()) {
                    tokens.push_back(token);
                    token.clear();
                }
            } else {
                token += ch;
            }
        }
    }

    // Add the last token if it exists
    if (!token.empty()) {
        tokens.push_back(token);
    }

    return tokens;
}

void executeBuiltInCommand(const char* cmd_line,string command) {
    // Split the input command line into words


    // Execute the corresponding built-in command based on the first word (command)
    if (command == "showpid") {
        ShowPidCommand spc(cmd_line);
        spc.execute();
    }
    else if (command == "chprompt") {
        ChangePromptCommand spc(cmd_line);
        spc.execute();
    }
    else if (command == "pwd") {
        GetCurrDirCommand gcdc(cmd_line);
        gcdc.execute();
    }
    else if (command == "cd") {
        ChangeDirCommand cdc(cmd_line);
        cdc.execute();
    }
    else if (command == "alias") {
        aliasCommand ac(cmd_line);  // Ensure that aliasCommand is declared
        ac.execute();  // Correct execution
    }
    else if (command == "unalias") {
        unaliasCommand uac(cmd_line);  // Ensure that unaliasCommand is declared
        uac.execute();  // Correct execution
    }
    return;
    // Add more commands here as needed
}

//////////////////////END OF OUR FUNCTIONS /////////////////////////////////////
//Staff Functions:-
std::string trimTrailingSpaces(const std::string& str) {
    // Find the position of the last non-space character
    size_t end = str.find_last_not_of(WHITESPACE);

    if (end != std::string::npos) {
        // Return the substring without trailing spaces
        return str.substr(0, end + 1);
    } else {
        // If the string is all spaces, return an empty string
        return "";
    }
}
string _ltrim(const std::string &s) {
    size_t start = s.find_first_not_of(WHITESPACE);
    return (start == std::string::npos) ? "" : s.substr(start);
}

string _rtrim(const std::string &s) {
    size_t end = s.find_last_not_of(WHITESPACE);
    return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

string _trim(const std::string &s) {
    trimTrailingSpaces(s);
    return _rtrim(_ltrim(s));
}

int _parseCommandLine(const char *cmd_line, char **args) {
    FUNC_ENTRY()
    int i = 0;
    std::istringstream iss(_trim(string(cmd_line)).c_str());
    for (std::string s; iss >> s;) {
        args[i] = (char *) malloc(s.length() + 1);
        memset(args[i], 0, s.length() + 1);
        strcpy(args[i], s.c_str());
        args[++i] = NULL;
    }
    return i;

    FUNC_EXIT()
}

bool _isBackgroundComamnd(const char *cmd_line) {
    const string str(cmd_line);
    return str[str.find_last_not_of(WHITESPACE)] == '&';
}

void _removeBackgroundSign(char *cmd_line) {
    const string str(cmd_line);
    // find last character other than spaces
    unsigned int idx = str.find_last_not_of(WHITESPACE);
    // if all characters are spaces then return
    if (idx == string::npos) {
        return;
    }
    // if the command line does not end with & then return
    if (cmd_line[idx] != '&') {
        return;
    }
    // replace the & (background sign) with space and then remove all tailing spaces.
    cmd_line[idx] = ' ';
    // truncate the command line string up to the last non-space character
    cmd_line[str.find_last_not_of(WHITESPACE, idx) + 1] = 0;
}
//////////////////////////////////////////////////////////////////////////////////////
// TODO: Add your implementation for classes in Commands.h
// SmallShell Funcs
SmallShell::SmallShell() {
    this->PromptName = "smash";
    this->LastDir = nullptr;
    this->BuiltInCommandSet.insert("chprompt");
    this->BuiltInCommandSet.insert("showpid");
    this->BuiltInCommandSet.insert("pwd");
    this->BuiltInCommandSet.insert("cd");
    this->BuiltInCommandSet.insert("jobs");
    this->BuiltInCommandSet.insert("fg");
    this->BuiltInCommandSet.insert("quit");
    this->BuiltInCommandSet.insert("kill");
    this->BuiltInCommandSet.insert("alias");
    this->BuiltInCommandSet.insert("unalias");
    this->BuiltInCommandSet.insert("listdir");
    this->BuiltInCommandSet.insert(">");
    this->BuiltInCommandSet.insert(">>");
    this->BuiltInCommandSet.insert("|");
    this->BuiltInCommandSet.insert("&");
    this->BuiltInCommandSet.insert("whoami");
    this->BuiltInCommandSet.insert("netinfo");

    ForeGroundPid = -1;
}

SmallShell::~SmallShell() {
// TODO: add your implementation
    for (Command* cmd : commandtodelete) {
        delete cmd;
    }
    commandtodelete.clear();
}

/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/
Command *SmallShell::CreateCommand(const char *cmd_line) {
    // For example:
    string temp2;
    string commandforfunc = _trim(cmd_line);
    current_command = cmd_line;
    SmallShell &smash = SmallShell::getInstance();

    vector<string> Argv = split(commandforfunc);
    if (aliasTable.find(Argv[0]) != aliasTable.end()) {
        string temp = aliasTable[Argv[0]];
        temp = removeSingleQuotes(temp);
        vector<string> Argv_temp = split(temp);
        Argv = appendVectors(Argv_temp, Argv);
        cmd_line = vectorToCmdLine(Argv);
        commandforfunc = _trim(cmd_line);
        Argv = split(commandforfunc);
        if (Argv[0] == "alias") {
            // Reconstruct the alias string
            if (Argv.size() > 1) {
                string aliasName = Argv[1]; // e.g., "something=something"
                size_t equalPos = aliasName.find('=');

                if (equalPos != string::npos) {
                    string key = aliasName.substr(0, equalPos);
                    string value = aliasName.substr(equalPos + 1);

                    // Enclose the value in single quotes if not already
                    if (!value.empty() && value.front() != '\'' && value.back() != '\'') {
                        value = "'" + value + "'";
                    }

                    // Reconstruct the alias
                    commandforfunc = "alias " + key + "=" + value;
                }
            }
        }
    }
    if(std::find(Argv.begin(), Argv.end(),"|") != Argv.end() ||std::find(Argv.begin(), Argv.end(),"|&") != Argv.end()){
        smash.getJobs().removeFinishedJobs();
        return new PipeCommand(commandforfunc.c_str());
    }
    if(Argv[0] == "chprompt" || Argv[0] == "chprompt&"){
        smash.getJobs().removeFinishedJobs();
        return new ChangePromptCommand(commandforfunc.c_str());
    }

    if(Argv[0] == "showpid" ||Argv[0] == "showpid&"){
        smash.getJobs().removeFinishedJobs();
        return new ShowPidCommand(commandforfunc.c_str());
    }
    if(Argv[0] == "listdir" ||Argv[0] == "listdir&"){
        smash.getJobs().removeFinishedJobs();
        return new ListDirCommand(commandforfunc.c_str());
    }

    if(Argv[0] == "pwd"||Argv[0] == "pwd&"){
        smash.getJobs().removeFinishedJobs();
        return new GetCurrDirCommand(commandforfunc.c_str());
    }

    if(Argv[0] == "cd"||Argv[0] == "cd&"){
        smash.getJobs().removeFinishedJobs();
        return new ChangeDirCommand(commandforfunc.c_str());
    }

    if(Argv[0] == "jobs"|| Argv[0] == "jobs&"){
        smash.getJobs().removeFinishedJobs();
        return new JobsCommand(commandforfunc.c_str(),&smash.getJobs());
    }

    if(Argv[0] == "fg"||Argv[0] == "fg&"){
        smash.getJobs().removeFinishedJobs();
        return new ForegroundCommand(commandforfunc.c_str(),&smash.getJobs());
    }

    if(Argv[0] == "quit"||Argv[0] == "quit&"){
        smash.getJobs().removeFinishedJobs();
        return new QuitCommand(commandforfunc.c_str(),&smash.getJobs());
    }
// TO DO:-
    if(Argv[0] == "kill" || Argv[0] == "kill&"){
        smash.getJobs().removeFinishedJobs();
        return new KillCommand(commandforfunc.c_str(),&smash.getJobs());
    }

    if(Argv[0] == "alias" || Argv[0] == "alias&"){
        smash.getJobs().removeFinishedJobs();
        return new aliasCommand(commandforfunc.c_str());
    }
    if(Argv[0] == "whoami" || Argv[0] == "whoami&" ){
        smash.getJobs().removeFinishedJobs();
        return new WhoAmICommand(commandforfunc.c_str());
    }
    if(Argv[0] == "unalias" || Argv[0] == "unalias&"){
        smash.getJobs().removeFinishedJobs();
        return new unaliasCommand(commandforfunc.c_str());
    }
    if(Argv[0] == "netinfo" || Argv[0] == "netinfo&"){
        smash.getJobs().removeFinishedJobs();
        if(Argv.size()< 2){
            return new NetInfo("nullbtr");
        }
        char* iface = const_cast<char*>(Argv[1].c_str());
        return new NetInfo(Argv[1].c_str());
    }
    else{
        smash.getJobs().removeFinishedJobs();
        Command *cmd = new ExternalCommand(commandforfunc.c_str());
        commandtodelete.push_back(cmd);
        return cmd;
    }
}

void SmallShell::executeCommand(const char *cmd_line) {
    int type = -1;

    string processed = preprocess(cmd_line);

    cmd_line = processed.c_str();
    vector<string> args = split(cmd_line);
    if (args.size() == 0) return;

    int index = -1;

// Find redirection operator and type

if(args[0] != "alias"){
    for (size_t i = 0; i < args.size(); ++i) {
        if (args[i] == ">") {
            type = 1; // Overwrite
            index = i;
            break;
        } else if (args[i] == ">>") {
            type = 0; // Append
            index = i;
            break;
        }
    }
     if((index + 1) == args.size()){
        cerr<<"smash error: open failed: No such file or directory"<<endl;
        return;
     }
     removeTrailingAmpersand(args[index +1]);
}
   
   // args[index +1 ] = temp;

    int consoleFD = dup(STDOUT_FILENO);
    if (type != -1 && index != -1) {
        // Save current stdout
        // consoleFD = dup(STDOUT_FILENO);
        if (consoleFD < 0) {
            perror("dup failed");
            return;
        }

        // Redirect output
        if (index + 1 < args.size()) {
            if(redirectOutput(args[index + 1], type) == -1){
                return;
            }
        } else {
            cerr << "Error: No file specified for redirection\n";
            close(consoleFD);
            return;
        }

        // Remove redirection arguments from command
        args.erase(args.begin() + index, args.begin() + index + 2);
        cmd_line = vectorToCmdLine(args);
    }

// Create and execute command

    Command* cmd = CreateCommand(cmd_line);
    if (cmd) {

        cmd->execute();
    } else {
        cerr << "Error: Command creation failed\n";
    }

// Restore stdout if redirected
    if (type != -1 && consoleFD >= 0) {
        if (dup2(consoleFD, STDOUT_FILENO) < 0) {
            perror("dup2 failed");
        }
        close(consoleFD);
    }



}
//////// SETTER GETTER FOR SMASH
void SmallShell::setPromptName(string Name){ this->PromptName = Name;}
string SmallShell::GetPromptName(){return this->PromptName;}
void SmallShell::setLastDir(const string &dir) {
    if (LastDir == nullptr) {
        // Initialize the pointer if it's null
        LastDir = new string(dir);
    } else {
        *LastDir = dir; // Update the value
    }
}
string* SmallShell::getLastDir() const {
    return LastDir; // Return the pointer directly (nullptr if not initialized)
}

void SmallShell::insertAlias(const string &alias,const string &command){
    if (aliasTable.find(alias) != aliasTable.end())
    {
        cerr<<"smash error: alias: " << alias << " already exists or is a reserved command"<<endl;
        return;
    }
   this->commandtoprint.push_back({alias,command});
    this->aliasTable[alias] = command;
}
void SmallShell::removeAlias(const string &alias){
    auto command = this->aliasTable.find(alias);
std::pair<string,string> target = {command->first,command->second};
    this->aliasTable.erase(alias);
    this->commandtoprint.erase(std::remove(commandtoprint.begin(), commandtoprint.end(), target), commandtoprint.end());
}
string SmallShell::getCommand(const string &alias){
    return aliasTable[alias];
}

JobsList& SmallShell::getJobs() {
    return jobsList;  // Return reference to the JobsList
}
void SmallShell::SetFGPID(pid_t pid){
    this->ForeGroundPid = pid;
}
pid_t SmallShell::GetFGPID(){
    return this->ForeGroundPid ;
}

// End Of SmallShell Funcs

///Change Prompt 1)
void ChangePromptCommand::execute(){
    vector<string> Argv = split(this->argv);
    removeAmpersandvector(Argv);
    if (Argv.size() == 1){
        SmallShell::getInstance().setPromptName("smash");
    }else
    {
        SmallShell::getInstance().setPromptName(Argv[1]);
    }
}

////ShowPid 2)
ShowPidCommand::ShowPidCommand(const char *cmd_line):BuiltInCommand(cmd_line){}
void ShowPidCommand::execute() {
    pid_t pid = getpid();
    cout<<"smash pid is "<<pid<<endl;
}

//3) print current diroctory (PWD)
GetCurrDirCommand::GetCurrDirCommand(const char *cmd_line):BuiltInCommand(cmd_line){}
void GetCurrDirCommand::execute() {
    char *path = getcwd(nullptr, 0);
    if (path != nullptr) {
        printf("%s\n", path);
        free(path);
    } else {
        perror("getcwd error");
    }
}


/////cd 4)
ChangeDirCommand::ChangeDirCommand(const char *cmd_line)
        : BuiltInCommand(cmd_line){}

void ChangeDirCommand::execute() {
    vector<string> Argv = split(this->argv);
    if(Argv.size()==1){
        return;
    }
    removeAmpersandvector(Argv);

    if(Argv.size() > 2) {
        cerr<<"smash error: cd: too many arguments"<<endl;
        return;
    }
    string NewDir;

    // Save the current directory to LastDir before any change

    string currentDir = getcwd(nullptr, 0);
    if (Argv[1] == "-") {
        // If the user entered "cd -", change to the last directory
        if(SmallShell::getInstance().getLastDir() == nullptr){
            cerr<<"smash error: cd: OLDPWD not set"<<endl;
            return;
        }
        string lastDir = *SmallShell::getInstance().getLastDir();
        char* Cur = getcwd(nullptr, 0);
       
        if (lastDir.empty()) {
            std::cerr << "No previous directory to go back to!" << std::endl;
            return;
        }
        if(chdir(lastDir.c_str()) != 0){
             perror("smash error: chdir failed");
                return;
        }
         SmallShell::getInstance().setLastDir(Cur);
    } else if (Argv[1] == "..") {
        // If the user entered "cd ..", move to the parent directory
        char* currentDir = getcwd(nullptr, 0); // Get current working directory
        
        if (currentDir != nullptr) {
            // Construct the path to the parent directory
            string parentDir = string(currentDir) + "/..";
            if(chdir(parentDir.c_str()) != 0){
                perror("smash error: chdir failed");
                return;
            }
            SmallShell::getInstance().setLastDir(currentDir);
            free(currentDir); // Free the memory allocated by getcwd
        }
    } else {
        // Otherwise, use the directory provided in argv[1]
        NewDir = Argv[1];
        char* Cur = getcwd(nullptr, 0);
       
        if(chdir(NewDir.c_str()) != 0){
             perror("smash error: chdir failed");
                return;
        }
        SmallShell::getInstance().setLastDir(Cur);
    }
}

//////// 5) jobs command assumed we have a vector of pairs and is already cleaned of finished jobs we need to clean it of finished jobs before printing
JobsCommand::JobsCommand(const char *cmd_line, JobsList *jobs):BuiltInCommand(cmd_line),jobs(jobs){}
void JobsCommand::execute() {
    //we clean the finished data if we havent cleaneed it already ///
    jobs->printJobsList();
}


//6) fg
ForegroundCommand::ForegroundCommand(const char *cmd_line, JobsList *jobs):BuiltInCommand(cmd_line),jobs(jobs){}
void ForegroundCommand::execute() {
    vector<string> Argv = split(this->argv);
    if(Argv.size() >2){
        cerr <<"smash error: fg: invalid arguments"<<endl;
        return;
    }
    if(Argv.size() == 2){
        if(isNumber(Argv[1]) == 0){
            cerr <<"smash error: fg: invalid arguments"<<endl;
            return;
        }
    }
    removeAmpersandvector(Argv);
    string command;
    int status;
    std::pair<std::pair<int,int> , std::pair<std::string,std::string>> job ;
    if(Argv.size() >1){
        job = jobs->getJobById((stoi(Argv[1])));
    }
    else{
        if(jobs->AllJobs.empty()){
            cerr <<"smash error: fg: jobs list is empty"<<endl;
            return;
        }
        else{
            job = jobs->AllJobs.back();
        }
    }
    if(job.first.first == -1 ){
        cerr << "smash error: fg: job-id "
                ""<<Argv[1] <<" does not exist"<<endl;
        return;
    }
    cout << job.second.second <<" " <<job.first.second<<endl;
    jobs->removeJobById(job.first.first);
    SmallShell::getInstance().SetFGPID(job.first.second);
    pid_t result;  
    do {
        result = waitpid(job.first.second, &status, 0);
    } while (result == -1 && errno == EINTR);
    SmallShell::getInstance().SetFGPID(-1);
}


////////////////////quit command 7)
QuitCommand::QuitCommand(const char *cmd_line, JobsList *jobs):BuiltInCommand(cmd_line),jobs(jobs){}

void QuitCommand::execute() {
    // I assume that i will print every thing in killalljobs

    std::vector<std::string> obg = split(this->argv);
    removeAmpersandvector(obg);

    for(auto & obji : obg){
        if(obji == "kill"){
            jobs->killAllJobs(1);
            break;
        }
    }
    jobs->killAllJobs(0);

}

// 8) this is missing
KillCommand::KillCommand(const char *cmd_line, JobsList *jobs):BuiltInCommand(cmd_line),KillJob(jobs){}
void KillCommand::execute() {
    vector<string> Argv = split(argv); // Assuming split is a valid function

    // Error: Invalid arguments
    if (Argv.size() != 3 || Argv[1][0] != '-') {
        cerr << "smash error: kill: invalid arguments" << endl;
        return;
    }

    // Parse signal and job ID
    int Signal;
    try {
        Signal = std::stoi(Argv[1]); // Extract signal number (after '-')
    } catch (...) {
        cerr << "smash error: kill: invalid arguments" << endl;
        return;
    }
    Signal = Signal * (-1);
    int JobId;
    try {
        JobId = std::stoi(Argv[2]); // Convert job ID to integer
    } catch (...) {
        cerr << "smash error: kill: invalid arguments" << endl;
        return;
    }
    if(JobId <= 0){
        cerr << "smash error: kill: invalid arguments" << endl;
        return;
    }
    // Retrieve job's process ID
    bool jobFound = false;
    auto job = KillJob->getJobById(JobId);
    //   JobId = job.first.first;
    pid_t Process = job.first.second;
    if (Process == -1) { // Error: Job ID does not exist
        cerr << "smash error: kill: job-id " << JobId << " does not exist" << endl;
        return;
    }


    // Send the signal to the process
    cout << "signal number " << Signal << " was sent to pid " << Process << endl;
    if (kill(Process, Signal) == -1) { // Error: Kill system call failed
        perror("smash error: kill failed");
        return;
    }

    KillJob->removeFinishedJobs();

}

//9)
////////////////////alias command
aliasCommand::aliasCommand(const char *cmd_line): BuiltInCommand(cmd_line) {}
void aliasCommand::execute() {
    // Check the format of the alias input using a regex
    const std::regex alias_regex("^alias [a-zA-Z0-9_]+='[^']*'$");
    std::string input_str = this->argv; // Capture the input string for processing
    // Check if it's a dispflay request for all aliases (no additional argument)
    std::vector<std::string> Argv = split(input_str);
    removeAmpersandvector(Argv);
    removeAmpersand(input_str);
    if (Argv.size() == 1 && Argv[0] == "alias") {
        const auto& aliasTable = SmallShell::getInstance().commandtoprint;
        for (const auto& pair : aliasTable) {
            std::cout << pair.first << "='" << pair.second << "'" << std::endl;
        }
        return;
    }

    // Ensure the input matches the alias format
    input_str = _trim(input_str);
    if (!std::regex_match(input_str, alias_regex)) {
        std::cerr << "smash error: alias: invalid alias format" << std::endl;
        return;
    }

    // Extract the alias name and command
    size_t alias_start = input_str.find("alias ") + 6; // Position after "alias "
    size_t equals_pos = input_str.find('=', alias_start);
    if (equals_pos == std::string::npos) {
        std::cerr << "smash error: alias: invalid alias format" << std::endl;
        return;
    }

    std::string name = input_str.substr(alias_start, equals_pos - alias_start);
    std::string command = input_str.substr(equals_pos + 1);

    // Ensure name is valid (alphanumeric and underscore only)
    const std::regex name_regex("^[a-zA-Z0-9_]+$");
    if (!std::regex_match(name, name_regex)) {
        std::cerr << "smash error: alias: invalid alias name" << std::endl;
        return;
    }

    // Remove surrounding single quotes from the command
    if (command.front() == '\'' && command.back() == '\'') {
        command = command.substr(1, command.size() - 2);
    } else {
        std::cerr << "smash error: alias: command must be enclosed in single quotes" << std::endl;
        return;
    }

    // Check if the alias name conflicts with built-in commands
    if (SmallShell::getInstance().isBuildinCommand(name)) {
     
        cerr<<"smash error: alias: " << name << " already exists or is a reserved command"<<endl;
        return;
    }

    // Insert the alias into the table
    if(name != "alias")
        SmallShell::getInstance().insertAlias(name,command);
}

// 10)
//unalais
unaliasCommand::unaliasCommand(const char *cmd_line):BuiltInCommand(cmd_line){}
void unaliasCommand::execute() {
    vector<string> Argv = split(this->argv);
    removeAmpersandvector(Argv);

    int size = Argv.size();
    if (size == 1) {
        cerr << "smash error: unalias: not enough arguments" << endl;
        return;
    }
    string command = Argv[1];
    for (int i = 1; i < size; i++) {
        if (SmallShell::getInstance().getAliasTable().find(Argv[i]) ==
            SmallShell::getInstance().getAliasTable().end()) {
            // Alias does not exist, print error
            cerr << "smash error: unalias: " << Argv[i] << " alias does not exist" << endl;
            return;
        }
        SmallShell::getInstance().removeAlias(Argv[i]);


    }
}
//////////////////// here is implementation of part of JobList
JobsList::JobsList(){}
void JobsList::printJobsList(){
    SmallShell &smash = SmallShell::getInstance();
    this->removeFinishedJobs();
    for (auto & obj: this->AllJobs) {
        std::cout << "["<<obj.first.first<<"] "<<obj.second.second<<endl;
    }
}

void JobsList::killAllJobs(bool flag) {
    if (flag) {
        //we need to clean finished jobs
        SmallShell &smash = SmallShell::getInstance();
        std::string prompt = smash.getInstance().GetPromptName();

        smash.getJobs().removeFinishedJobs();
        cout << prompt << ": sending SIGKILL signal to " << AllJobs.size() << " jobs:" << endl;
        for (auto &job: AllJobs) {
            cout << job.first.second << ": " << job.second.second<<endl;
            kill(job.first.second, SIGKILL);
        }
        exit(0);
    }
    else{
        exit(0);
    }
}

std::pair<std::pair<int,int> , std::pair<std::string,std::string>> JobsList::getJobById(int jobId){
    for (auto & obj : AllJobs) {
        if(obj.first.first == jobId){
            //cout<<"Job Id is "<<jobId<<endl;
            return obj;
        }
    }
    return {{-1, -1}, {"",""}};
}

void JobsList::removeJobById(int jobId){
    for(int i=0 ; i< AllJobs.size();i++){
        if(AllJobs[i].first.first == jobId){
            AllJobs.erase(AllJobs.begin()+i);
            return;
        }
    }
}

void JobsList::removeFinishedJobs() {
    pid_t result;
    for (auto it = AllJobs.begin(); it != AllJobs.end();) { // Do not increment it in the loop header
        result = waitpid(it->first.second, nullptr, WNOHANG); // Check if job is finished
        if (result == 0) {
            ++it; // Job is still running, move to the next
        } else if (result > 0) {
            // Process has finished, remove it from the vector
            it = AllJobs.erase(it); // erase() returns the next valid iterator
        } else {
            if (errno == ECHILD) {
                // No such child process, likely already finished and cleaned up
                it = AllJobs.erase(it); // Remove from the list, as it's no longer valid
            } else {
                perror("waitpid failed");
                ++it; // Move to the next to prevent infinite loop
            }
        }
    }
}



void JobsList::addJob(std::pair<int,std::pair<string,string >> cmd, bool isStopped){
    JobsList::removeFinishedJobs();
    if(AllJobs.empty()){
        AllJobs.push_back({{1, cmd.first}, {cmd.second.first,cmd.second.second}});
    }
    else{
        int max = AllJobs.back().first.first +1;
        AllJobs.push_back({{max, cmd.first}, {cmd.second.first,cmd.second.second}});
    }
}












//////////////////////////////

//////////////////////////////////////////external

ExternalCommand::ExternalCommand(const char *cmd_line):Command(cmd_line){}
void ExternalCommand::execute() {
    SmallShell &smash = SmallShell::getInstance();

// Split the command string into individual arguments
    std::vector<std::string> args = split(argv);
    std::string command = args[0];  // The command, e.g., "sleep"

// Flags for special characters (e.g., wildcard * or background &)
    int flag = -1;
    int flag2 = -1;

    for (auto &a : args) {
        for (auto &b : a) {
            if (b == '*' || b == '?') {
                flag = 1;
            }
        }
    }

// Check if `&` is in the last position of the last argument or connected
flag2 = _isBackgroundComamnd(argv.c_str());
    if (!args.empty()) {
        std::string &lastArg = args.back();  // Reference to the last argument
        if (lastArg.back() == '&') {
            flag2 = 1;  // Set the flag for background processing

            if (lastArg.size() == 1) {
                // If the last argument is just '&', remove it entirely
                args.pop_back();
            } else {
                // If '&' is attached to a word, remove it
                lastArg.pop_back();
            }
        }
    }


    // Fork a child process
    pid_t p = fork();

    if (p == 0) {  // Child process
        setpgrp();  // Create a new process group

        if (flag == 1) {  // If the command contains wildcards, use bash
            const char* bashPath = "/bin/bash";
            const char* bashOption = "-c";

            // Join all arguments into a single string to pass to bash
            std::string fullCommand;
            for (const auto& arg : args) {
                fullCommand += arg + " ";  // Join with space
            }

            // Trim trailing space
            if (!fullCommand.empty()) {
                fullCommand.pop_back();
            }

            char* const execArgs[] = {
                    const_cast<char*>(bashPath),
                    const_cast<char*>(bashOption),
                    const_cast<char*>(fullCommand.c_str()),  // Pass full command including args
                    nullptr
            };
            execv(bashPath, execArgs);  // Execute bash command
        } else {  // Normal command execution without wildcards
            std::vector<char*> execArgs;
            for (auto& arg : args) {
                // Remove "&" from arguments

                execArgs.push_back(const_cast<char*>(arg.c_str()));  // Convert std::string to char*

            }
            execArgs.push_back(nullptr);  // Null-terminate the argument list

            // Execute the command using execvp
            if (execvp(execArgs[0], execArgs.data()) == -1) {
                perror("smash error: execvp failed");

                _exit(1);  // Exit the child process if execvp fails
            }
        }
    } else if (p > 0) {
// Parent process
        if (flag2 == 1) {  // If the command should run in the background
            smash.getJobs().addJob({p, {argv,smash.get_current_command()}},0);
        } else {  // Foreground job
            smash.SetFGPID(p);

            // Wait for the specific child process (foreground job)
            if (waitpid(p, nullptr, 0) == -1) {
                perror("smash error: waitpid failed");
            }

            smash.SetFGPID(-1);
        }
    } else if (p == -1) {  // Fork failed
        perror("smash error: fork failed");
        smash.SetFGPID(-1);
    }
}

JobsList::~JobsList(){

}

/////////////////////////////////////////////////
ListDirCommand::ListDirCommand(const char *cmd_line) : Command(cmd_line) {}

void ListDirCommand::execute() {
    // Parse the command line into arguments
    std::vector<std::string> args = split(this->argv);
    removeAmpersandvector(args);

    if (args.size() > 2) {
        std::cerr << "smash error: listdir: too many arguments" << std::endl;
        return;
    }

    std::string directoryPath = ".";
    if (args.size() == 2) {
        directoryPath = args[1];
    }

    // Check if the specified directory exists
    int fd = open(directoryPath.c_str(), O_RDONLY | O_DIRECTORY);
    if (fd == -1) {
        perror("smash error: listdir");
        return;
    }

    close(fd);

    listDirectory(directoryPath, 0);
}

void ListDirCommand::listDirectory(const std::string& directoryPath, int depth) {
    int fd = open(directoryPath.c_str(), O_RDONLY | O_DIRECTORY);
    if (fd == -1) {
        perror("smash error: listdir");
        return;
    }

    char buf[1024];  // buffer for reading directory entries
    std::vector<std::string> directories;
    std::vector<std::string> files;

    while (true) {
        ssize_t len = syscall(SYS_getdents64, fd, buf, sizeof(buf));
        if (len == -1) {
            perror("smash error: listdir");
            close(fd);
            return;
        }

        if (len == 0) {
            break; // No more entries
        }

        struct linux_dirent64* entry;
        for (int bpos = 0; bpos < len; bpos += entry->d_reclen) {
            entry = (struct linux_dirent64*)(buf + bpos);
            std::string name = entry->d_name;

            // Skip "." and ".." entries
            if (name == "." || name == "..") {
                continue;
            }

            // Check if entry is a directory or file based on d_type
            if (entry->d_type == DT_DIR) {
                directories.push_back(name);
            } else {
                files.push_back(name);
            }
        }
    }

    close(fd);

    // Sort directories and files alphabetically
    std::sort(directories.begin(), directories.end());
    std::sort(files.begin(), files.end());

    // Print directories first, using tabs for indentation
    for (const auto& subdir : directories) {
        std::cout << std::string(depth, '\t') << subdir << std::endl;
        listDirectory(directoryPath + "/" + subdir, depth + 1);
    }

    // Print files, using tabs for indentation
    for (const auto& file : files) {
        std::cout << std::string(depth, '\t') << file << std::endl;
    }
}





WhoAmICommand::WhoAmICommand(const char *cmd_line): Command(cmd_line) {}

void WhoAmICommand::execute() {
    // Get the current user's UID
    uid_t uid = getuid();

    // Open the /etc/passwd file
    int fd = open("/etc/passwd", O_RDONLY);
    if (fd == -1) {
        perror("smash error");
        return;
    }

    char buffer[1024];
    ssize_t bytesRead;
    std::string passwd_content;

    // Read the entire /etc/passwd into a string
    while ((bytesRead = read(fd, buffer, 1024)) > 0) {
        passwd_content.append(buffer, bytesRead);
    }
    close(fd);

    // Parse the content of /etc/passwd for the UID
    size_t start = 0, end;
    while ((end = passwd_content.find('\n', start)) != std::string::npos) {
        std::string line = passwd_content.substr(start, end - start);
        start = end + 1;

        // Parse fields separated by colon ':'
        size_t colon1 = line.find(':');
        size_t colon2 = line.find(':', colon1 + 1);
        size_t colon3 = line.find(':', colon2 + 1);
        size_t colon4 = line.find(':', colon3 + 1);
        size_t colon5 = line.find(':', colon4 + 1);
        size_t colon6 = line.find(':', colon5 + 1);

        if (colon1 == std::string::npos || colon2 == std::string::npos || colon3 == std::string::npos ||
            colon4 == std::string::npos || colon5 == std::string::npos || colon6 == std::string::npos) {
            continue; // Skip malformed lines
        }

        // Extract the UID (3rd field)
        int file_uid = std::stoi(line.substr(colon2 + 1, colon3 - colon2 - 1));
        if (file_uid == uid) {
            // Extract home directory (5th field)
            std::string home_dir = line.substr(colon5 + 1, colon6 - colon5 - 1);

            // Print the username and home directory
            std::string username = line.substr(0, colon1); // Extract the username (1st field)
            std::cout << username << " " << home_dir << std::endl;
            return;
        }
    }

    // If no matching UID was found
    std::cerr << "smash error: whoami failed to find user information" << std::endl;
}
string SmallShell::get_current_command(){
    return current_command;
}
PipeCommand::PipeCommand(const char *cmd_line): Command(cmd_line){}
void PipeCommand::execute() {
    SmallShell &smash = SmallShell::getInstance();

    // Detect pipe type and split into command1 and command2
    std::string delimiter = "|";
    bool isPipeError = false;

    size_t pos = argv.find("|&");
    if (pos != std::string::npos) {
        delimiter = "|&";
        isPipeError = true;
    } else {
        pos = argv.find("|");
    }

    if (pos == std::string::npos) {
        std::cerr << "smash error: invalid pipe command" << std::endl;
        return;
    }

    std::string command1 = argv.substr(0, pos);
    std::string command2 = argv.substr(pos + delimiter.size());

    // Create a pipe
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("smash error: pipe failed");
        return;
    }

    pid_t pid1 = fork();
    if (pid1 == -1) {
        perror("smash error: fork failed");
        close(pipefd[0]);
        close(pipefd[1]);
        return;
    }

    if (pid1 == 0) {
        // Child process 1: Executes command1
        setpgrp();
        if (isPipeError) {
            dup2(pipefd[1], STDERR_FILENO); // Redirect stderr to pipe's write end
        } else {
            dup2(pipefd[1], STDOUT_FILENO); // Redirect stdout to pipe's write end
        }
        close(pipefd[0]); // Close unused read end of the pipe

        try {
            smash.executeCommand(command1.c_str());
            close(pipefd[1]); // Close original write end
        } catch (...) {
            close(pipefd[1]); // Close original write end
            _exit(1); // Exit if an exception occurs
        }
        _exit(0); // Explicitly exit to avoid falling through
    }

    pid_t pid2 = fork();
    if (pid2 == -1) {
        perror("smash error: fork failed");
        close(pipefd[0]);
        close(pipefd[1]);
        return;
    }

    if (pid2 == 0) {
        // Child process 2: Executes command2
        setpgrp();
        dup2(pipefd[0], STDIN_FILENO); // Redirect stdin to pipe's read end
        close(pipefd[1]); // Close unused write end of the pipe

        try {
            smash.executeCommand(command2.c_str());
            close(pipefd[0]); // Close original read end
        } catch (...) {
            close(pipefd[0]); // Close original read end
            _exit(1); // Exit if an exception occurs
        }
        _exit(0); // Explicitly exit to avoid falling through
    }

    // Parent process: Close unused file descriptors
    close(pipefd[0]);
    close(pipefd[1]);

    // Wait for both child processes to finish
    if (waitpid(pid1, nullptr, 0) == -1) {
        perror("smash error: waitpid failed for command1");
    }
    if (waitpid(pid2, nullptr, 0) == -1) {
        perror("smash error: waitpid failed for command2");
    }
}

NetInfo::NetInfo(const char *cmd_line): Command(cmd_line) {}

bool interfaceExists(const char* iface) {
    struct ifreq ifr;
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1) {
        perror("smash error: netinfo");
        return false;
    }

    strncpy(ifr.ifr_name, iface, IFNAMSIZ - 1);
    if (ioctl(sockfd, SIOCGIFCONF, &ifr) == -1) {
        close(sockfd);
        return false;
    }

    close(sockfd);
    return true;
}

// Function to get the IP address using ioctl
std::string getIPAddress(const char* iface) {
    struct ifreq ifr;
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1) {
        perror("smash error: netinfo");
        return "";
    }

    strncpy(ifr.ifr_name, iface, IFNAMSIZ - 1);
    if (ioctl(sockfd, SIOCGIFADDR, &ifr) == -1) {
        close(sockfd);
        return "";
    }

    struct sockaddr_in* ipaddr = (struct sockaddr_in*)&ifr.ifr_addr;
    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &ipaddr->sin_addr, ip, INET_ADDRSTRLEN);
    close(sockfd);
    return std::string(ip);
}

// Function to get the subnet mask using ioctl
std::string getSubnetMask(const char* iface) {
    struct ifreq ifr;
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1) {
        perror("smash error: netinfo");
        return "";
    }

    strncpy(ifr.ifr_name, iface, IFNAMSIZ - 1);
    if (ioctl(sockfd, SIOCGIFNETMASK, &ifr) == -1) {
        close(sockfd);
        return "";
    }

    struct sockaddr_in* netmask = (struct sockaddr_in*)&ifr.ifr_addr;
    char mask[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &netmask->sin_addr, mask, INET_ADDRSTRLEN);
    close(sockfd);
    return std::string(mask);
}

// Function to get the default gateway by reading /proc/net/route
std::string getDefaultGateway() {
    int fd = open("/proc/net/route", O_RDONLY);
    if (fd == -1) {
        perror("smash error: netinfo");
        return "";
    }

    char buf[4096];
    ssize_t len = read(fd, buf, sizeof(buf));
    if (len == -1) {
        close(fd);
        return "";
    }

    close(fd);

    // Manually parse the /proc/net/route
    char* line = strtok(buf, "\n");
    while (line != nullptr) {
        char iface[32], destination[32], gateway[32];
        unsigned int flags, refcnt, use, metric, mtu, window, irtt;
        if (sscanf(line, "%31s %31s %31s %x %x %x %x %x %x %x", iface, destination, gateway, &flags, &refcnt, &use, &metric, &mtu, &window, &irtt) == 10) {
            if (strcmp(destination, "00000000") == 0) { // Default route (0.0.0.0)
                struct in_addr gw_addr;
                gw_addr.s_addr = strtol(gateway, nullptr, 16);
                return inet_ntoa(gw_addr);
            }
        }
        line = strtok(nullptr, "\n");
    }

    return "";
}

// Function to get DNS servers from /etc/resolv.conf
std::vector<std::string> getDNSServers() {
    int fd = open("/etc/resolv.conf", O_RDONLY);
    if (fd == -1) {
        perror("smash error: netinfo");
        return {};
    }

    char buf[4096];
    ssize_t len = read(fd, buf, sizeof(buf));
    if (len == -1) {
        close(fd);
        return {};
    }

    close(fd);

    std::vector<std::string> dnsServers;
    char* line = strtok(buf, "\n");
    while (line != nullptr) {
        if (strncmp(line, "nameserver", 10) == 0) {
            dnsServers.push_back(line + 11);  // Skip "nameserver " (length 11)
        }
        line = strtok(nullptr, "\n");
    }

    return dnsServers;
}

// The netinfo command function
void NetInfo::execute() {
    const char* iface = argv.c_str();
    if (argv == "nullbtr" || strlen(iface) == 0) {
        std::cerr << "smash error: netinfo: interface not specified" << std::endl;
        return;
    }

    if (!interfaceExists(iface)) {
        std::cerr << "smash error: netinfo: interface " << iface << " does not exist" << std::endl;
        return;
    }

    // Get the IP address
    std::string ip = getIPAddress(iface);
    if (ip.empty()) {
        std::cerr << "smash error: netinfo: could not retrieve IP address for " << iface << std::endl;
        return;
    }

    // Get the subnet mask
    std::string subnetMask = getSubnetMask(iface);
    if (subnetMask.empty()) {
        std::cerr << "smash error: netinfo: could not retrieve subnet mask for " << iface << std::endl;
        return;
    }

    // Get the default gateway
    std::string gateway = getDefaultGateway();
    if (gateway.empty()) {
        std::cerr << "smash error: netinfo: could not retrieve default gateway" << std::endl;
        return;
    }

    // Get the DNS servers
    std::vector<std::string> dnsServers = getDNSServers();
    if (dnsServers.empty()) {
        std::cerr << "smash error: netinfo: could not retrieve DNS servers" << std::endl;
        return;
    }

    // Print the network information in the requested format
    std::cout << "IP Address: " << ip << std::endl;
    std::cout << "Subnet Mask: " << subnetMask << std::endl;
    std::cout << "Default Gateway: " << gateway << std::endl;

    std::cout << "DNS Servers: ";
    for (size_t i = 0; i < dnsServers.size(); ++i) {
        std::cout << dnsServers[i];
        if (i < dnsServers.size() - 1) {
            std::cout << ", ";
        }
    }
    std::cout << std::endl;
}

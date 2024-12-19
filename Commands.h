#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <vector>
#include <utility>
#include <string>
#include <csignal>
#include <unordered_map>
#include <fcntl.h>
#include <set>
using std::string; 
using std::unordered_map;
using std::vector;
using std::set;
#define COMMAND_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)

class Command {

    // TODO: Add your data members
protected:
    string argv;
   
public:
    Command(const char *cmd_line) : argv(cmd_line) {}

    virtual ~Command(){}

    virtual void execute() = 0;

    //virtual void prepare();
    //virtual void cleanup();
    // TODO: Add your extra methods if needed
};

class BuiltInCommand : public Command {
public:
    BuiltInCommand(const char *cmd_line):Command(cmd_line){}

    virtual ~BuiltInCommand() {
    }
    
};

class ExternalCommand : public Command {
public:
    ExternalCommand(const char *cmd_line);

    virtual ~ExternalCommand() {
    }

    void execute() override;
};

class PipeCommand : public Command {
    // TODO: Add your data members
public:
    PipeCommand(const char *cmd_line);

    virtual ~PipeCommand() {
    }

    void execute() override;
};

class RedirectionCommand : public Command {
    // TODO: Add your data members
public:
    explicit RedirectionCommand(const char *cmd_line);

    virtual ~RedirectionCommand() {
    }

    void execute() override;
};

class ChangeDirCommand : public BuiltInCommand {

    // TODO: Add your data members public:
public:

    ChangeDirCommand(const char *cmd_line);

    virtual ~ChangeDirCommand() {
    }

    void execute() override;
};

class GetCurrDirCommand : public BuiltInCommand {
public:
    GetCurrDirCommand(const char *cmd_line);

    virtual ~GetCurrDirCommand() {
    }

    void execute() override;
};


class ShowPidCommand : public BuiltInCommand {
public:
    ShowPidCommand(const char *cmd_line);

    virtual ~ShowPidCommand() {
    }

    void execute() override;
};

class JobsList;

class QuitCommand : public BuiltInCommand {
public:

    // TODO: Add your data members public:
    JobsList *jobs;
    QuitCommand(const char *cmd_line, JobsList *jobs);

    virtual ~QuitCommand() {
    }

    void execute() override;
};


class JobsList {
public:
    class JobEntry {
        // TODO: Add your data members
    };
    // TODO: Add your data members
    std::vector<std::pair<std::pair<int,int> , std::pair<std::string,std::string>>> AllJobs;
public:

    // i will assume that we put here a victor of pairs that we put inside it [0]jobId and [1] the command
    JobsList();

    ~JobsList();

    void addJob(std::pair<int,std::pair<std::string,std::string>> cmd, bool isStopped = false);

    void printJobsList();

    void killAllJobs(bool flag);

    void removeFinishedJobs();

    std::pair<std::pair<int,int> , std::pair<std::string,std::string>> getJobById(int jobId);

    void removeJobById(int jobId);

    JobEntry *getLastJob(int *lastJobId);

    JobEntry *getLastStoppedJob(int *jobId);

    // TODO: Add extra methods or modify exisitng ones as needed
};

class JobsCommand : public BuiltInCommand {
    // TODO: Add your data members
    JobsList *jobs;
public:
    // i will assume that we put here a victor of pairs that we put inside it [0]jobId and [1] the command

    JobsCommand(const char *cmd_line, JobsList *jobs);

    virtual ~JobsCommand() {
    }

    void execute() override;
};

class KillCommand : public BuiltInCommand {
    // TODO: Add your data members
    JobsList * KillJob;
public:
    KillCommand(const char *cmd_line, JobsList *jobs);

    virtual ~KillCommand() {
    }

    void execute() override;
};

class ForegroundCommand : public BuiltInCommand {
    // TODO: Add your data members
    JobsList *jobs;
public:
    ForegroundCommand(const char *cmd_line, JobsList *jobs);

    virtual ~ForegroundCommand() {
    }

    void execute() override;
};

class ListDirCommand : public Command {
public:
    ListDirCommand(const char *cmd_line);
    void execute() override;
private:
    void listDirectory(const std::string& directoryPath, int depth);
};

class WhoAmICommand : public Command {
public:
    WhoAmICommand(const char *cmd_line);

    virtual ~WhoAmICommand() {
    }

    void execute() override;
};

class NetInfo : public Command {
    // TODO: Add your data members
public:
    NetInfo(const char *cmd_line);

    virtual ~NetInfo() {
    }

    void execute() override;
};

class aliasCommand : public BuiltInCommand {
public:
    aliasCommand(const char *cmd_line);

    virtual ~aliasCommand() {
    }

    void execute() override;
};

class unaliasCommand : public BuiltInCommand {
public:
    unaliasCommand(const char *cmd_line);

    virtual ~unaliasCommand() {
    }

    void execute() override;
};



class SmallShell {
private:
    // TODO: Add your data members
    string current_command;
    string PromptName;
    string* LastDir;
    pid_t ForeGroundPid;
    std::vector<Command *> commandtodelete;
    unordered_map<string,string> aliasTable;
    set<std::string> BuiltInCommandSet;
   // set<std::string> reservedWord;
    SmallShell();
public:
        std::vector<std::pair<string,string>> commandtoprint;
    JobsList jobsList;

    Command *CreateCommand(const char *cmd_line);

    SmallShell(SmallShell const &) = delete; // disable copy ctor
    void operator=(SmallShell const &) = delete; // disable = operator
    static SmallShell &getInstance() // make SmallShell singleton
    {
        static SmallShell instance; // Guaranteed to be destroyed.
        // Instantiated on first use.
        return instance;
    }

    ~SmallShell();

    void executeCommand(const char *cmd_line);
    void setPromptName(string Name);
    string GetPromptName();
    void setLastDir(const string &dir);
    string* getLastDir() const;
    JobsList & getJobs();
    void insertAlias(const string &alias,const string &command);
    void removeAlias(const string &alias);
    void SetFGPID(pid_t pid);
    pid_t GetFGPID();
    std::string get_current_command();
    string getCommand(const string &alias);

      std::unordered_map<std::string, std::string>& getAliasTable() {
        return aliasTable;
    }
    bool isBuildinCommand(const string& value) const{
        if (BuiltInCommandSet.find(value) != BuiltInCommandSet.end()) return true;
        return false;
        }
    // TODO: add extra methods as needed
};
////////////////////////////////////////////////////////////////
class ChangePromptCommand : public BuiltInCommand {
private:
public:
    ChangePromptCommand(const char *cmd_line):BuiltInCommand(cmd_line){}
    void execute() override;
    virtual ~ChangePromptCommand(){}
};
#endif //SMASH_COMMAND_H_

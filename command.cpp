// === class Command Implementation =======================================
//
// ========================================================================

#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <wait.h>
#include <fcntl.h>
#include <map>
#include <sstream>

#include "deps.hpp"
#include "command.hpp"
#include "fdstream.hpp"

// --- public constructor(s) ----------------------------------------------
Command::Command(void)
{
    // TODO: ensure cross-platform compatibility
    if (getenv("SHELL"))
    {
        m_shell = getenv("SHELL");
    }
    else
    {
        m_shell = "/bin/sh";
    }
}// end Command::Command(void)

Command::Command(
    const std::vector<string>& args,
    const bool pipeStdin,
    const bool pipeStdout,
    const bool pipeStderr
) : Command()
{
    m_args              = args;
    m_isShellCommand    = false;
    m_pipeStdin         = pipeStdin;
    m_pipeStdout        = pipeStdout;
    m_pipeStderr        = pipeStderr;
}// end Command::Command

Command::Command(
    const string& shellCommand,
    const bool pipeStdin,
    const bool pipeStdout,
    const bool pipeStderr
) : Command(
    std::vector<string> { shellCommand },
    pipeStdin, pipeStdout, pipeStderr
)
{
    m_isShellCommand = true;
}// end Command::Command

// --- public accessor(s) -------------------------------------------------
auto Command::args(void) const              -> const std::vector<string>&
{
    return m_args;
}// end Command::args

auto Command::shell_command(void) const     -> bool
{
    return m_isShellCommand;
}// end Command::shell_command

auto Command::stdin_piped(void) const       -> bool
{
    return m_pipeStdin;
}// end Command::stdin_piped

auto Command::stdout_piped(void) const      -> bool
{
    return m_pipeStdout;
}// end Command::stdout_piped

auto Command::stderr_piped(void) const      -> bool
{
    return m_pipeStderr;
}// end Command::stderr_piped

auto Command::shell(void) const          -> const string&
{
    return m_shell;
}// end Command::shell

auto Command::spawn(void) const             -> Subprocess
{
    return Subprocess(
        m_args,
        env,
        m_isShellCommand,
        m_shell,
        m_pipeStdin,
        m_pipeStdout,
        m_pipeStderr
    );
}// end Command::spawn

// --- public mutator(s) --------------------------------------------------
auto Command::append_arg(const string& arg)                     -> Command&
{
    m_args.push_back(arg);
    return *this;
}// end Command::append_arg

auto Command::set_env(const string& key, const string& val)     -> Command&
{
    env[key] = val;
    return *this;
}// end Command::set_env

auto Command::set_is_shell_command(const bool state)            -> Command&
{
    m_isShellCommand = state;
    return *this;
}// end Command::set_is_shell_command

auto Command::set_shell(const string& shell)                 -> Command&
{
    m_shell = shell;
    return *this;
}// end Command::set_shell

auto Command::set_stdin_piped(const bool state)                 -> Command&
{
    m_pipeStdin = state;
    return *this;
}// end Command::set_stdin_piped

auto Command::set_stdout_piped(const bool state)                -> Command&
{
    m_pipeStdout = state;
    return *this;
}// end Command::set_stdout_piped

auto Command::set_stderr_piped(const bool state)                -> Command&
{
    m_pipeStderr = state;
    return *this;
}// end Command::set_stderr_piped

// === Command::Subprocess Implementation =================================
//
// ========================================================================

// --- public accessor(s) -------------------------------------------------

Command::Subprocess::operator   bool(void) const
{
    return not terminated();
}// end Command::Subprocess::operator bool

auto Command::Subprocess::terminated(void) const     -> bool
{
    return m_hasTerminated;
}// end Command::Subprocess::terminated

auto Command::Subprocess::stdin_piped(void) const    -> bool
{
    return m_stdin.get();
}// end Command::Subprocess::stdin_piped

auto Command::Subprocess::stdout_piped(void) const   -> bool
{
    return m_stdout.get();
}// end Command::Subprocess::stdout_piped

auto Command::Subprocess::stderr_piped(void) const   -> bool
{
    return m_stderr.get();
}// end Command::Subprocess::stderr_piped

// --- public mutator(s) --------------------------------------------------
auto Command::Subprocess::stdin(void)    -> ofdstream&
{
    if (not m_stdin)
    {
        throw except_file_unpiped("stdin");
    }

    return *m_stdin.get();
}// end Command::Subprocess::stdin

auto Command::Subprocess::stdout(void)   -> ifdstream&
{
    if (not m_stdout)
    {
        throw except_file_unpiped("stdout");
    }

    return *m_stdout.get();
}// end Command::Subprocess::stdout

auto Command::Subprocess::stderr(void)   -> ifdstream&
{
    if (not m_stderr)
    {
        throw except_file_unpiped("stderr");
    }

    return *m_stderr.get();
}// end Command::Subprocess::stderr

auto Command::Subprocess::wait(void)                 -> int
{
    int     woptions    = 0;

    waitpid(m_pid, &m_exitStatus, woptions);

    return m_exitStatus;
}// end Command::Subprocess::wait

auto Command::Subprocess::kill(int sig) -> int
{
    return ::kill(m_pid, sig);
}// end Command::Subprocess::kill

// --- protected member constructor(s) ------------------------------------
Command::Subprocess::Subprocess(
    std::vector<string> args,
    const std::map<string,string>& env,
    const bool isShellCommand,
    string shell,
    const bool pipeStdin,
    const bool pipeStdout,
    const bool pipeStderr
)
{
    using namespace std;

    static char             shellCommandFlag[]      = "-c";
    vector<char*>           argsRaw                 = {};
    vector<string>          envArray                = {};
    vector<char*>           envArrayRaw             = {};
    int                     inPipe[2]               = {};
    int                     outPipe[2]              = {};
    int                     errPipe[2]              = {};

    // NOTE: need to allocate at least one more than number of args.
    // That way, final const char* ptr is null terminator.
    argsRaw.reserve(args.size() + 3);
    envArray.reserve(env.size());
    envArrayRaw.reserve(env.size() + 1);

    if (isShellCommand)
    {
        argsRaw.push_back(&shell[0]);
        argsRaw.push_back(shellCommandFlag);
    }

    for (auto& arg : args)
    {
        argsRaw.push_back(&arg[0]);
    }// end for arg

    argsRaw.push_back(NULL);

    // build env array
    for (auto& kv : env)
    {
        envArray.push_back(kv.first + "=" + kv.second);
        envArrayRaw.push_back(&envArray.back()[0]);
    }// end for kv

    envArrayRaw.push_back(NULL);

    if (pipeStdin)
    {
        switch (pipe(inPipe))
        {
            // success
            case 0:
                m_stdin = make_unique<ofdstream>(inPipe[1]);
                break;
            // failure
            case -1:
                throw logic_error("call to pipe() failed");
                break;
            default:
                throw logic_error("unrecognized pipe() return value");
                break;
        }// end switch (pipe(inPipe))
    }

    if (pipeStdout)
    {
        switch (pipe(outPipe))
        {
            // success
            case 0:
                m_stdout = make_unique<ifdstream>(outPipe[0]);
                break;
            // failure
            case -1:
                throw logic_error("call to pipe() failed");
                break;
            default:
                throw logic_error("unrecognized pipe() return value");
                break;
        }// end switch (pipe(outPipe))
    }

    if (pipeStderr)
    {
        switch (pipe(errPipe))
        {
            // success
            case 0:
                m_stderr = make_unique<ifdstream>(errPipe[0]);
                break;
            // failure
            case -1:
                throw logic_error("call to pipe() failed");
                break;
            default:
                throw logic_error("unrecognized pipe() return value");
                break;
        }// end switch (pipe(errPipe))
    }

    // perform fork
    m_pid = fork();
    switch (m_pid)
    {
        // in child process
        case 0:
            if (pipeStdin)
            {
                close(inPipe[1]);
                dup2(inPipe[0], 0);
            }
            if (pipeStdout)
            {
                close(outPipe[0]);
                dup2(outPipe[1], 1);
            }
            if (pipeStderr)
            {
                close(errPipe[0]);
                dup2(errPipe[1], 2);
            }
            if (env.empty())
                execvp(argsRaw[0], &argsRaw[0]);
            else
                execvpe(argsRaw[0], &argsRaw[0], &envArrayRaw[0]);
            break;
        // error
        case -1:
            throw logic_error("call to fork() failed");
            break;
        // in parent process
        default:
            if (pipeStdin)
                close(inPipe[0]);
            if (pipeStdout)
                close(outPipe[1]);
            if (pipeStderr)
                close(errPipe[1]);
            break;
    }// switch (m_pid)
}// end Command::Subprocess::Subprocess

// === class Command::Subprocess::except_file_unpiped Implementation ======
//
// ========================================================================

// ------ protected constructor(s) ----------------------------------------
Command::Subprocess::except_file_unpiped::except_file_unpiped(
    const string& fileName
)
{
    set_text("file not piped: " + fileName);
}// end Command::Subprocess::except_file_unpiped::except_file_unpiped

#ifndef __COMMAND_HPP__
#define __COMMAND_HPP__

#include <sys/types.h>
#include <map>

#include "deps.hpp"
#include "fdstream.hpp"

// === class Command ======================================================
//
// Command builder, for preparing to run a subprocess. Allows you to build
// a command.
//
// ========================================================================
class   Command
{
    public:
        // === public member type(s) ======================================
        class   Subprocess;

        // === public member variable(s) ==================================
        std::map<string,string>     env;

        // === public constructor(s) ======================================
        Command(void);// default
        Command(
            const std::vector<string>& args,
            const bool pipeStdin = false,
            const bool pipeStdout = false,
            const bool pipeStderr = false
        );// type 1
        Command(
            const string& shellCommand,
            const bool pipeStdin = false,
            const bool pipeStdout = false,
            const bool pipeStderr = false
        );// type 2

        // === public accessor(s) =========================================
        auto args(void) const           -> const std::vector<string>&;
        auto shell_command(void) const  -> bool;
        auto stdin_piped(void) const    -> bool;
        auto stdout_piped(void) const   -> bool;
        auto stderr_piped(void) const   -> bool;
        auto shell(void) const          -> const string&;
        // TODO: make sure move constructor/operator is implemented
        auto spawn(void) const          -> Subprocess;

        // === public mutator(s) ==========================================
        auto append_arg(const string& arg)                  -> Command&;
        auto set_env(const string& key, const string& val)  -> Command&;
        auto set_is_shell_command(const bool state)         -> Command&;
        auto set_shell(const string& shell)                 -> Command&;
        auto set_stdin_piped(const bool state)              -> Command&;
        auto set_stdout_piped(const bool state)             -> Command&;
        auto set_stderr_piped(const bool state)             -> Command&;
    protected:
        // === protected member variable(s) ===============================
        std::vector<string>     m_args              = {};
        bool                    m_isShellCommand    = false;
        string                  m_shell             = "";
        bool                    m_pipeStdin         = false;
        bool                    m_pipeStdout        = false;
        bool                    m_pipeStderr        = false;
};// end class Command

class   Command::Subprocess
{
    friend class Command;

    public:
        // === public member type(s) ======================================
        class   except_file_unpiped;

        // === public accessor(s) =========================================

        // ------ operator bool -------------------------------------------
        //
        // Returns false if the subprocess has terminated, true otherwise.
        //
        // ----------------------------------------------------------------
        operator    bool(void) const;

        auto terminated(void) const     -> bool;
        auto stdin_piped(void) const    -> bool;
        auto stdout_piped(void) const   -> bool;
        auto stderr_piped(void) const   -> bool;

        // === public mutator(s) ==========================================
        auto stdin(void)    -> ofdstream&;
        auto stdout(void)   -> ifdstream&;
        auto stderr(void)   -> ifdstream&;
        auto wait(void)     -> int;
        auto kill(int sig)  -> int;
    protected:
        // === protected member variable(s) ===============================
        pid_t                   m_pid               = 0;
        int                     m_exitStatus        = -1;
        bool                    m_hasTerminated     = false;
        u_ptr<ofdstream>        m_stdin             = nullptr;
        u_ptr<ifdstream>        m_stdout            = nullptr;
        u_ptr<ifdstream>        m_stderr            = nullptr;

        // === protected member constructor(s) ============================
        Subprocess(
            std::vector<string> args,
            const std::map<string,string>& env,
            const bool isShellCommand,
            string shell,
            const bool pipeStdin,
            const bool pipeStdout,
            const bool pipeStderr
        );// type
};// end class Command::Subprocess

class   Command::Subprocess::except_file_unpiped : StringException
{
    friend class Command::Subprocess;

    protected:
        // === protected constructor(s) ===================================
        except_file_unpiped(const string& fileName);
};// end class Command::Subprocess::except_file_unpiped

#endif

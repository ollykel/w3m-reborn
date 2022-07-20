#include "../deps.hpp"
#include "../mailcap.hpp"

class MailcapTester : public Mailcap
{
    public:
        // --- public member type(s) --------------------------------------
        typedef     MailcapTester   Self;

        // --- friend function(s) -----------------------------------------
        friend auto operator<<(std::ostream& outs, const Self& obj)
            -> std::ostream&;
};// end class MailcapTester

auto operator<<(std::ostream& outs, const MailcapTester& obj)
    -> std::ostream&
{
    using namespace std;

    for (const auto& superKeyVal : obj.m_typeMap)
    {
        const auto&     superType       = superKeyVal.first;
        const auto&     subTypeMap      = superKeyVal.second;

        outs << superType << ":" << endl;

        for (const auto& subKeyVal : subTypeMap)
        {
            const auto&     subType         = subKeyVal.first;
            const auto&     entryCont       = subKeyVal.second;

            outs << '\t' << subType << ": [" << endl;
            for (const auto& entry : entryCont)
            {
                outs << "\t\t\""
                    << utils::join_str(entry.command_template(), "|")
                    << "\"";
                if (entry.has_test())
                {
                    outs << "; test: \"";
                    outs << utils::join_str(entry.test().args(), " ");
                    outs << '"';
                }
                outs << ',' << endl;
            }
            outs << "\t]," << endl;
        }// end for subKeyVal
    }// end for superKeyVal

    return outs;
}// end operator<<

// === main ===============================================================
//
// ========================================================================
int main(const int argc, const char **argv)
{
    using namespace std;

    MailcapTester       tester;

    tester.append_entry("*", "*", { "cat" });
    tester.append_entry("image", "jpeg", { "mpv %s" });
    tester.append_entry("image", "png", { "mpv %s" }).set_test("false");
    tester.append_entry("application", "pdf", { "evince %s" });
    tester.append_entry("application", "pdf",
        { "pdf-view --property=%{some_property} %s" });
    tester.append_entry("image", "jpeg", { "viewer --image --file=%s %s" })
        .set_test("false");
    tester.append_entry("image", "png", { "foo %s" }).set_test("which foo");
    tester.append_entry("image", "png", { "bar %s" }).set_test("which bar");
    tester.append_entry("video/mp4",
        { "do-mailcap --format=\"%%s\" --type=\"%t\" %s" });
    tester.append_entry("text/markdown", { "pandoc -f markdown -t html" })
        .set_output_type("text/html")
        .set_test("which pandoc");

    cout << "Mailcap:" << endl;
    cout << tester << endl;

    auto    testRunner      = [&tester](const string& mimeType)
    {
        auto    *entry      = tester.get_entry(mimeType);

        cout << mimeType << " handler: ";
        if (entry)
        {
            auto    cmd     = entry->create_command
                                ("temp0001.ext", mimeType);

            cout << utils::join_str(entry->command_template()) << endl;

            cout << "Command: " << cmd.args().at(0) << endl;
            cout << "stdin piped? " << cmd.stdin_piped() << endl;
            cout << "stdout piped? " << cmd.stdout_piped() << endl;
            cout << endl;
        }
        else
        {
            cout << "<NONE>" << endl;
        }
    };// end testRunner

    testRunner("image/jpeg");
    testRunner("image/png");
    testRunner("text/markdown");

    return EXIT_SUCCESS;
}// end main

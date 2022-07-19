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

    tester.append_entry("*", "*", { "cat %s" });
    tester.append_entry("image", "jpeg", { "mpv %s" });
    tester.append_entry("image", "png", { "mpv %s" }).set_test("false");
    tester.append_entry("application", "pdf", { "evince %s" });
    tester.append_entry("image", "jpeg", { "viewer --image --file=%s %s" })
        .set_test("false");
    tester.append_entry("image", "png", { "foo %s" }).set_test("which foo");
    tester.append_entry("image", "png", { "bar %s" }).set_test("which bar");

    cout << "Mailcap:" << endl;
    cout << tester << endl;

    cout << "image/jpeg handler: ";
    {
        auto    *entry      = tester.get_entry("image/jpeg");

        if (entry)
        {
            cout << utils::join_str(entry->command_template());
        }
        else
        {
            cout << "<NONE>";
        }
    }
    cout << endl;

    cout << "image/png handler: ";
    {
        auto    *entry      = tester.get_entry("image/png");

        if (entry)
        {
            cout << utils::join_str(entry->command_template());
        }
        else
        {
            cout << "<NONE>";
        }
    }
    cout << endl;

    return EXIT_SUCCESS;
}// end main

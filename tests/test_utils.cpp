#include <cstddef>
#include <cstdlib>
#include <iostream>

#include <string>
#include <vector>

#include "../utils.hpp"

// === forward declarations ===============================================

template <typename T>
auto operator==(const std::vector<T>& a, const std::vector<T>& b)
    -> bool;

template <typename T>
auto operator!=(const std::vector<T>& a, const std::vector<T>& b)
    -> bool;

template <typename T>
auto operator<<(std::ostream& outs, const std::vector<T>& cont)
    -> std::ostream&;

void test_splitn(
        const std::string& testStr,
        const std::vector<std::string>& result,
        const std::string& delims = " \t\r\n",
        const char escape = '\0',
        const size_t nSplits = 0
    );

template <class SRC_T, class DEST_T, typename MAP_FUNC>
void test_map(
    const std::vector<SRC_T>& src,
    const std::vector<DEST_T>& desiredResult,
    MAP_FUNC mapFunc
);

template <class CONT, typename OPERATION>
void test_for_each(
    CONT& cont,
    const CONT& desiredResult,
    OPERATION op
);

// === main ===============================================================
//
// ========================================================================
int main(const int argc, const char **argv)
{
    using namespace std;

    cout << "Testing splitn..." << endl;
    try
    {
        test_splitn("oh hai mark", { "oh", "hai", "mark" });
        test_splitn("one  two three", { "one", "two", "three" });
        test_splitn("uno|dos||tres", { "uno", "dos", "tres" }, "|");
        test_splitn("this; is| ;a;;sentence", { "this", "is", "a", "sentence" }, " ;|");
        test_splitn(
            "image/jpeg; tmux new-window \"sleep 3\\; mpv %s\"; test=which mpv; needsterminal",
            {"image/jpeg", " tmux new-window \"sleep 3; mpv %s\"",
                " test=which mpv", " needsterminal"},
            ";",
            '\\'
        );
    }
    catch (int status)
    {
        cout << "Test failed" << endl;
        return status;
    }
    cout << "All tests passed" << endl;

    cout << "Testing map..." << endl;
    try
    {
        test_map(
            std::vector<int>{ 1, 2, 3 },
            std::vector<int>{ 2, 4, 6 },
            [](const int input)
            {
                return input * 2;
            }
        );

        test_map(
            std::vector<string>{"11", "22", "55"},
            std::vector<int>{11, 22, 55},
            [](const string& str)
            {
                return std::stoi(str);
            }
        );

        test_map(
            std::vector<string>{"image/jpeg", " tmux new-window \"sleep 3; mpv %s\"",
                " test=which mpv", " needsterminal"},
            std::vector<string>{"image/jpeg", "tmux new-window \"sleep 3; mpv %s\"",
                "test=which mpv", "needsterminal"},
            [](const string& str)
            {
                return str.substr(
                    str.find_first_not_of(" \t\r\n"), 
                    str.find_last_not_of(" \t\r\n") + 1
                );
            }
        );
    }
    catch (int status)
    {
        cout << "Test failed" << endl;
        return status;
    }
    cout << "All tests passed" << endl;

    cout << "Testing for_each..." << endl;
    try
    {
        {
            std::vector<int>    tester      = { 1, 2, 3};
            test_for_each(
                tester,
                std::vector<int>{ 2, 4, 6 },
                [](int& input)
                {
                    input *= 2;
                }
            );
        }

        {
            std::vector<string>     tester      = {"image/jpeg",
                " tmux new-window \"sleep 3; mpv %s\"",
                " test=which mpv", " needsterminal"};
            test_for_each(
                tester,
                std::vector<string>{"image/jpeg", "tmux new-window \"sleep 3; mpv %s\"",
                    "test=which mpv", "needsterminal"},
                [](string& str)
                {
                    str.erase(0, str.find_first_not_of(" \t\r\n"));
                    str.erase(str.find_last_not_of(" \t\r\n") + 1);
                }
            );
        }
    }
    catch (int status)
    {
        cout << "Test failed" << endl;
        return status;
    }
    cout << "All tests passed" << endl;

    return EXIT_SUCCESS;
}// end main

// === definitions ========================================================

template <typename T>
auto operator==(const std::vector<T>& a, const std::vector<T>& b)
    -> bool
{
    if (a.size() != b.size())
    {
        return false;
    }

    auto            iterA       = a.begin();
    auto            iterB       = b.begin();
    const auto      endA        = a.end();

    for (; iterA != endA; ++iterA, ++iterB)
    {
        if (*iterA != *iterB)
        {
            return false;
        }
    }// end for

    return true;
}// end operator==

template <typename T>
auto operator!=(const std::vector<T>& a, const std::vector<T>& b)
    -> bool
{
    return not (a == b);
}// end operator!=

template <typename T>
auto operator<<(std::ostream& outs, const std::vector<T>& cont)
    -> std::ostream&
{
    outs << '[';
    if (not cont.empty())
    {
        auto        iter    = cont.begin();
        const auto  end     = cont.end();

        outs << *iter;
        ++iter;

        for (; iter != end; ++iter)
        {
            outs << ", " << *iter;
        }// end for iter
    }
    outs << ']';
    return outs;
}// end operator<<

void test_splitn(
    const std::string& testStr,
    const std::vector<std::string>& result,
    const std::string& delims,
    const char escape,
    const size_t nSplits
)
{
    using namespace std;

    std::vector<string>     testResult;

    utils::splitn(testResult, testStr, delims, escape, nSplits);

    if (testResult != result)
    {
        cout << "Split incorrect:" << endl;
        cout << "\texpected: " << result << endl;
        cout << "\tgot: " << testResult << endl;

        throw EXIT_FAILURE;
    }
}// end test_splitn

template <class SRC_T, class DEST_T, typename MAP_FUNC>
void test_map(
    const std::vector<SRC_T>& src,
    const std::vector<DEST_T>& desiredResult,
    MAP_FUNC mapFunc
)
{
    using namespace std;

    std::vector<DEST_T>     testResult;

    utils::map(testResult, src, mapFunc);

    if (testResult != desiredResult)
    {
        cout << "Test did not achieve desired result." << endl;
        cout << "\texpected: " << desiredResult << endl;
        cout << "\tgot: " << testResult << endl;

        throw EXIT_FAILURE;
    }
}// end test_map

template <class CONT, typename OPERATION>
void test_for_each(
    CONT& cont,
    const CONT& desiredResult,
    OPERATION op
)
{
    using namespace std;

    utils::for_each(cont, op);

    if (cont != desiredResult)
    {
        cout << "Test did not achieve desired result." << endl;
        cout << "\texpected: " << desiredResult << endl;
        cout << "\tgot: " << cont << endl;

        throw EXIT_FAILURE;
    }
}// end test_map

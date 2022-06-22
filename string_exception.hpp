#ifndef __STRING_EXCEPTION_HPP__
#define __STRING_EXCEPTION_HPP__

#include <string>

// === class StringException ==============================================
//
// ========================================================================
class   StringException
{
    public:
        // === public operator(s) =========================================
        // ------ accessor(s) ---------------------------------------------
        operator        const std::string&(void) const;
    protected:
        // === protected mutator(s) =======================================
        void            set_text(const std::string& text);
    private:
        // === private member variable(s) =================================
        std::string     m_text      = "";
};// end class StringException

#endif

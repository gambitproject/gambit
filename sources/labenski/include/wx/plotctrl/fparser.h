/////////////////////////////////////////////////////////////////////////////
// Name:        fparser.h
// Purpose:     wxFunctionParser - a thin wrapper around FunctionParser
// Author:      John Labenski, Warp, please see fparser.hh/cpp/txt
// Modified by:
// Created:     6/5/2004
// Copyright:   (c) John Labenski, Juha Nieminen, Joel Yliluoma
// Licence:     wxWidgets licence
/////////////////////////////////////////////////////////////////////////////

/***************************************************************************\
|* Function parser v2.7 by Warp                                            *|
|* ----------------------------                                            *|
|* Parses and evaluates the given function with the given variable values. *|
|*                                                                         *|
\***************************************************************************/

#ifndef ONCE_wxFPARSER_H_
#define ONCE_wxFPARSER_H_

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
    #pragma interface "fparser.h"
#endif

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#include "wx/plotctrl/plotdefs.h"

class WXDLLEXPORT wxString;
class FunctionParser;

//-----------------------------------------------------------------------------
// wxFunctionParser usage (see fparser.txt for details)
//
// After creating call Parse(wxT("x*sin(x)+x/2.8"), wxT("x"))
//     see available funtions are listed in fparser.txt
//     "x" is the var or a comma separated list or vars "x,y,z"
//
// ex. Parse("x*sin(x)+x/2.8+y", "x,y") double f, vars[2]; f = Eval(&vars)
//
// Parse() returns -1 if ok or the position in the function string where
//         an error occured, it can also return func.size() if vars are bad
// ErrorMsg() returns a human readable message about the error or wxEmptyString
// EvalError() returns -1 if no error or an index into the internal error message strings
// Eval() returns the f(x,y,...) value, must pass in exactly the same number
//        elements as you had characters in Parse(func, -> vars <- )
//-----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// wxFunctionParser - a thin wrapper around FunctionParser
//
// It converts wxString to std::string (for unicode) and isolates the std::
// warnings that MSVC 6 generates from it's own headers.
// Additionally, the original fparser.hh and fparser.cpp files do not require
// any modification other than adding some code.
// ----------------------------------------------------------------------------

class WXDLLIMPEXP_PLOTCTRL wxFunctionParser
{
public:
    enum ParseErrorType
    {
        SYNTAX_ERROR=0, MISM_PARENTH, MISSING_PARENTH, EMPTY_PARENTH,
        EXPECT_OPERATOR, OUT_OF_MEMORY, UNEXPECTED_ERROR, INVALID_VARS,
        ILL_PARAMS_AMOUNT, PREMATURE_EOS, EXPECT_PARENTH_FUNC,
        FP_NO_ERROR
    };

    wxFunctionParser();
    // Copy constructor and assignment operator (implemented using the
    // copy-on-write technique for efficiency):
    wxFunctionParser(const wxFunctionParser&);
    virtual ~wxFunctionParser();

    // Parse the function string and a comma delimited list of the variables
    // in the function. func = "sin(x) + 2*t*x", vars = "x,t"
    int Parse(const wxString& Function, const wxString& Vars,
              bool useDegrees = false);

    // Get a readable error message if error after a call to parse or empty string
    wxString ErrorMsg() const;
    // Get the wxFunctionParser::ParseErrorType error id
    ParseErrorType GetParseErrorType() const;

    // Evaluate the function (must have called Parse first) where vars is an
    //  array of the variable values to use.
    double Eval(const double* Vars);
    // Not really sure - some sort of error flag?
    int EvalError() const;

    // Add a constant to the function parser
    bool AddConstant(const wxString& name, double value);

    // Function type that can be added to the parser
    typedef double (*FunctionPtr)(const double*);
    // Add a function to the parser
    bool AddFunction(const wxString& name,
                     FunctionPtr, unsigned paramsAmount);
    bool AddFunction(const wxString& name, wxFunctionParser&);

    // Try to optimize the parser, use after sucessful call to Parse
    void Optimize();

    // Ok is true after a sucessful call to Parse
    bool Ok() const { return m_ok; }
    // Get the number of comma delimited variables sent to Parse
    int GetNumberVariables() const;
    // Get the function string sent to Parse
    wxString GetFunctionString() const { return m_function; }
    // Get the variable string sent to Parse
    wxString GetVariableString() const { return m_variables; }
    // Get one of the variable names from the string sent to Parse
    wxString GetVariableName(size_t n) const;
    // Was this parsed using degrees
    bool GetUseDegrees() const;

    // operators
    wxFunctionParser& operator=(const wxFunctionParser&);

    // implementation
    FunctionParser* GetFunctionParser() const { return m_functionParser; }

protected:
    FunctionParser *m_functionParser;
    wxString m_function, m_variables;
    bool m_ok;
};

#endif // ONCE_wxFPARSER_H_

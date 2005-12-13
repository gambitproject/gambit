/////////////////////////////////////////////////////////////////////////////
// Name     : optvalue.cpp
// Author   : John Labenski
// Created  : 07/01/02
// Copyright: John Labenski, 2002
// License  : wxWidgets v2
/////////////////////////////////////////////////////////////////////////////

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
    #pragma implementation "optvalue.h"
#endif

// For compilers that support precompilation, includes "wx/wx.h"
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
    #include "wx/object.h"
    #include "wx/string.h"
    #include "wx/gdicmn.h"
#endif // WX_PRECOMP

#include "wx/tokenzr.h"
#include "wx/wxthings/optvalue.h"

#include "wx/arrimpl.cpp"
WX_DEFINE_OBJARRAY(wxArrayOptionValue);

//----------------------------------------------------------------------------
// Global wxString utilities
//----------------------------------------------------------------------------
/*
wxArrayString wxStringToWords( const wxString &string )
{
    
//  wxArrayString arr;
//  wxString word, s = string.Strip(wxString::both);
//  wxMemoryInputStream memstream( s.c_str(), string.Length() );
//  wxTextInputStream textstream(memstream);
//  while (!memstream.Eof())
//  {
//      word = textstream.ReadWord();
//      if (!word.IsEmpty()) arr.Add(word.Strip(wxString::both));
//  }
//  return arr;

    wxArrayString arr;
    wxString s = string.Strip(wxString::both);
    while (!s.IsEmpty())
    {
        arr.Add(s.BeforeFirst(wxT(' ')));
        s = s.AfterFirst(wxT(' ')).Strip(wxString::both);
    }
    return arr;
}
*/

class wxOptionValueRefData: public wxObjectRefData
{
public:
    wxOptionValueRefData()
    { 
        m_children = NULL; 
    }
    
    ~wxOptionValueRefData() 
    {
        if (m_children) delete m_children;
    }
    
    wxString m_type;
    wxArrayString m_optionNames;
    wxArrayString m_optionValues;
    wxArrayOptionValue *m_children;
};

#define M_OPTVALUDATA ((wxOptionValueRefData *)m_refData)

IMPLEMENT_DYNAMIC_CLASS(wxOptionValue, wxObject);

bool wxOptionValue::Create()
{
    UnRef();
    m_refData = new wxOptionValueRefData();
    return Ok();
}

bool wxOptionValue::Create( const wxOptionValue &optvalue )
{
    wxCHECK_MSG( optvalue.Ok(), false, wxT("Invalid wxOptionValue") );
    
    UnRef();
    Ref( optvalue );
    return Ok();
}

bool wxOptionValue::Create( const wxString &string )
{
    UnRef();
    m_refData = new wxOptionValueRefData();
    
    int i, start = 0, length = string.Length();
    
    wxString buff;
    
    const wxChar *s = string.GetData();
    
//  const wxChar comma = 44; // comma
    const wxChar tab = 9;  // tab
//    const wxChar space = 32; // space
    const wxChar cr = 13; // carrage return
    const wxChar lf = 10; // line feed
    const wxChar openbracket  = wxT('[');
    const wxChar closebracket  = wxT(']');
    const wxChar equals = wxT('=');
    const wxChar ccr = wxT('\n');
    
    bool has_type = false;
    
    for (i=0; i<length; i++, s++)                    // find opening [ for type
    {
        if (*s == openbracket) 
        { 
            start = i+1; 
            s++;                               // don't include bracket in type
            has_type = true;
            break; 
        }
    }
    if (has_type)
    {
        for (i=start; i<length; i++, s++)            // find closing ] for type
        {
            if ((*s == closebracket))
            {
                M_OPTVALUDATA->m_type = buff;
                s++;
                start = i+1;
                break;
            }
            else if ((*s != tab) && (*s != cr) && (*s != lf) && (*s != ccr))
            {
                buff.Append(*s);
            }
            else 
                return false;
        }
    }
    
    buff.Clear();
    for (i=start; i<length; i++)  // add options
    {
        // add up characters until an = sign then the word before is the name
        //   the rest of the string before that is the value for the previous name
        if (*s != equals)
            buff.Append(*s);
        else
        {
            buff.Trim(false).Trim(true);
            if (!buff.IsEmpty())
            {
                const wxChar *t = buff.GetData();
                int j;
                for (j = buff.Length()-1; j>=0; j--)
                {
                    const wxChar c = t[j];
                    if ((c == cr) || (c == lf) || (c == ccr) || (c == tab))
                    {
                        j++;
                        break;
                    }
                }
                if (j<0) j = 0;
                M_OPTVALUDATA->m_optionNames.Add(buff.Mid(j));
                buff.Remove(j).Trim(true);
            }
            if (!buff.IsEmpty()) 
                M_OPTVALUDATA->m_optionValues.Add(buff);
            
            buff.Clear();
        }
        s++;
    }
    
    buff.Trim(false).Trim(true);
    if (!buff.IsEmpty())
        M_OPTVALUDATA->m_optionValues.Add(buff);
    
    if ((M_OPTVALUDATA->m_optionValues.GetCount() != M_OPTVALUDATA->m_optionNames.GetCount())) 
    {
        int i;
        wxPrintf(wxT("wxOptionValue::wxOptionValue( const wxString &string BUSTED\n"));
        
        wxPrintf(wxT("[%s]\n"), M_OPTVALUDATA->m_type.c_str());
        for (i=0; i<(int)M_OPTVALUDATA->m_optionNames.GetCount(); i++)
            wxPrintf(wxT("{%s}\n"), M_OPTVALUDATA->m_optionNames[i].c_str());
        for (i=0; i<(int)M_OPTVALUDATA->m_optionValues.GetCount(); i++)
            wxPrintf(wxT("{%s}\n"), M_OPTVALUDATA->m_optionValues[i].c_str());
        fflush(stdout);
    }

    return ((M_OPTVALUDATA->m_optionValues.GetCount() > 0) && 
            (M_OPTVALUDATA->m_optionValues.GetCount() != 
             M_OPTVALUDATA->m_optionNames.GetCount()));
}

bool wxOptionValue::Copy( const wxOptionValue &optValue )
{
    wxCHECK_MSG( optValue.Ok(), false, wxT("Invalid wxOptionValue") );
    
    M_OPTVALUDATA->m_type = optValue.GetType();
    
    M_OPTVALUDATA->m_optionNames.Clear();
    M_OPTVALUDATA->m_optionValues.Clear();
    
    WX_APPEND_ARRAY(M_OPTVALUDATA->m_optionNames, optValue.GetOptionNames());
    WX_APPEND_ARRAY(M_OPTVALUDATA->m_optionValues, optValue.GetOptionValues());
    
    return true;
}

bool wxOptionValue::Ok() const
{
    return M_OPTVALUDATA != NULL;
}

void wxOptionValue::Destroy()
{
    UnRef();
}

//-----------------------------------------------------------------------------

wxString wxOptionValue::GetType() const
{
    wxCHECK_MSG( Ok(), wxEmptyString, wxT("Invalid wxOptionValue") );
    return M_OPTVALUDATA->m_type;
}

void wxOptionValue::SetType( const wxString &type )
{
    wxCHECK_RET( Ok(), wxT("Invalid wxOptionValue") );
    M_OPTVALUDATA->m_type = type;
}

//-----------------------------------------------------------------------------

int wxOptionValue::HasChildren() const
{
    wxCHECK_MSG( Ok(), 0, wxT("Invalid wxOptionValue") );
    if (!M_OPTVALUDATA->m_children) return 0;
    return int(M_OPTVALUDATA->m_children->GetCount());
}
wxArrayOptionValue *wxOptionValue::GetChildren() const
{
    wxCHECK_MSG( Ok(), NULL, wxT("Invalid wxOptionValue") );
    return M_OPTVALUDATA->m_children;
}
    
bool wxOptionValue::AddChild( wxOptionValue *child )
{
    wxCHECK_MSG( Ok() && child, 0, wxT("Invalid wxOptionValue") );
    if (!M_OPTVALUDATA->m_children) 
        M_OPTVALUDATA->m_children = new wxArrayOptionValue();
    
    M_OPTVALUDATA->m_children->Add(child);
    return true;
}

void wxOptionValue::DeleteChildren() 
{
    wxCHECK_RET( Ok(), wxT("Invalid wxOptionValue") );
    if (M_OPTVALUDATA->m_children) 
    {
        delete M_OPTVALUDATA->m_children;
        M_OPTVALUDATA->m_children = NULL;
    }
}

//-----------------------------------------------------------------------------

int wxOptionValue::GetOptionCount() const
{
    wxCHECK_MSG( Ok(), 0, wxT("Invalid wxOptionValue") );
    return int(M_OPTVALUDATA->m_optionNames.GetCount());
}

wxArrayString wxOptionValue::GetOptionNames() const
{
    wxCHECK_MSG( Ok(), wxArrayString(), wxT("Invalid wxOptionValue") );
    return M_OPTVALUDATA->m_optionNames;
}
wxArrayString wxOptionValue::GetOptionValues() const
{
    wxCHECK_MSG( Ok(), wxArrayString(), wxT("Invalid wxOptionValue") );
    return M_OPTVALUDATA->m_optionValues;
}

wxString wxOptionValue::GetOptionName( int n ) const
{
    wxCHECK_MSG( Ok() && (n>=0) && (n<int(M_OPTVALUDATA->m_optionNames.GetCount())), 
                 wxEmptyString, wxT("Invalid wxOptionValue") );
    return M_OPTVALUDATA->m_optionNames[n];
}

wxString wxOptionValue::GetOptionValue( int n ) const
{
    wxCHECK_MSG( Ok() && (n>=0) && (n<int(M_OPTVALUDATA->m_optionValues.GetCount())), wxEmptyString, wxT("Invalid wxOptionValue") );
    return M_OPTVALUDATA->m_optionValues[n];
}

int wxOptionValue::HasOption(const wxString& name) const
{
    wxCHECK_MSG( Ok(), wxNOT_FOUND, wxT("Invalid wxOptionValue") );
    int index = M_OPTVALUDATA->m_optionNames.Index(name, false);
    return index;
}

int wxOptionValue::FindOption(const wxString &part_of_option_name) const
{
    wxCHECK_MSG( Ok(), wxNOT_FOUND, wxT("Invalid wxOptionValue") );
    int i, count = M_OPTVALUDATA->m_optionNames.GetCount();
    
    for (i=0; i<count; i++)
    {
        if (M_OPTVALUDATA->m_optionNames[i].Contains(part_of_option_name))
            return i;
    }
    return wxNOT_FOUND;
}

bool wxOptionValue::DeleteOption(const wxString &name)
{
    wxCHECK_MSG( Ok(), false, wxT("Invalid wxOptionValue"));
    int index = M_OPTVALUDATA->m_optionNames.Index(name, false);
    if (index == wxNOT_FOUND) return false;
    M_OPTVALUDATA->m_optionNames.RemoveAt(index);
    M_OPTVALUDATA->m_optionValues.RemoveAt(index);   
    return true;
}

bool wxOptionValue::DeleteOption( int n )
{
    wxCHECK_MSG( Ok(), false, wxT("Invalid wxOptionValue") );
    wxCHECK_MSG( (n>=0) && (n<int(M_OPTVALUDATA->m_optionValues.GetCount())), false, wxT("invalid index"));
    M_OPTVALUDATA->m_optionNames.RemoveAt(n);
    M_OPTVALUDATA->m_optionValues.RemoveAt(n);  
    return true;    
}

//-----------------------------------------------------------------------------
// Set Options

// Option functions (arbitrary name/value mapping)
void wxOptionValue::SetOption(const wxString& name, const wxString& value, bool force)
{
    wxCHECK_RET( Ok() && (name.Length() > 0), wxT("Invalid wxOptionValue or option") );

    int idx = M_OPTVALUDATA->m_optionNames.Index(name, false);
    if (idx == wxNOT_FOUND)
    {
        M_OPTVALUDATA->m_optionNames.Add(name);
        M_OPTVALUDATA->m_optionValues.Add(value);
    }
    else if (force)
    {
        M_OPTVALUDATA->m_optionNames[idx] = name;
        M_OPTVALUDATA->m_optionValues[idx] = value;
    }
}

void wxOptionValue::SetOption(const wxString &name, bool update, const wxChar *format, ...)
{
    va_list argptr;
    va_start(argptr, format);
    wxString s;
    s.PrintfV(format, argptr);
    va_end(argptr);
    SetOption(name, s, update);
}

//-----------------------------------------------------------------------------
// Get Options

wxString wxOptionValue::GetOption(const wxString& name) const
{
    wxCHECK_MSG( Ok(), wxEmptyString, wxT("Invalid wxOptionValue") );

    int idx = M_OPTVALUDATA->m_optionNames.Index(name, false);
    if (idx != wxNOT_FOUND)
        return M_OPTVALUDATA->m_optionValues[idx];
    
    return wxEmptyString;
}

int wxOptionValue::GetOptionInt(const wxString& name) const
{
    return wxAtoi(GetOption(name));
}

bool wxOptionValue::GetOption(const wxString& name, wxString &value ) const
{
    wxString s = GetOption(name);
    if (!s.IsEmpty()) { value = s; return true; }
    return false;
}
bool wxOptionValue::GetOption(const wxString& name, int *value ) const
{
    int n;
    if ((wxSscanf(GetOption(name).c_str(), wxT("%d"), &n) == 1))
    {
        *value = n;
        return true;
    }
    return false;
}
bool wxOptionValue::GetOption(const wxString& name, float *value ) const
{
    float n;
    if ((wxSscanf(GetOption(name).c_str(), wxT("%f"), &n) == 1))
    {
        *value = n;
        return true;
    }
    return false;
}
bool wxOptionValue::GetOption(const wxString& name, double *value ) const
{
    double n;
    if ((wxSscanf(GetOption(name).c_str(), wxT("%lf"), &n) == 1))
    {
        *value = n;
        return true;
    }
    return false;
}

int wxOptionValue::GetOption(const wxString& name, const wxChar *format, ...) const
{
    wxString n = GetOption(name);
    if (n.IsEmpty()) return 0;
    va_list argptr;
    va_start(argptr, format);
//  int i = wxVsscanf(n.c_str(), format, argptr); // VisualStudio doesn't have this 
    int i = wxSscanf(n.c_str(), format, argptr);
    va_end(argptr);
    return i;
}

int wxOptionValue::GetOption(const wxString& name, wxArrayInt &values ) const
{
    wxString value = GetOption(name);
    wxStringTokenizer tokens(value, wxT(" ,\t\r\n"),wxTOKEN_STRTOK);
    int count = 0;
    
    while (tokens.HasMoreTokens())
    {
        long num;
        if (!tokens.GetNextToken().ToLong(&num)) return count;
        values.Add(num);
        count++;
    }
    return count;
}

bool wxOptionValue::GetOption(const wxString& name, unsigned char *value, int count ) const
{
    long *nums = (long*)malloc(sizeof(long)*count);
    if (GetOption(name, nums, count))
    {
        for (int i=0; i<count; i++) value[i] = (unsigned char)nums[i];
        free(nums);
        return true;
    }
    free(nums);
    return false;
}
bool wxOptionValue::GetOption(const wxString& name, int *value, int count ) const
{
    long *nums = (long*)malloc(sizeof(long)*count);
    if (GetOption(name, nums, count))
    {
        for (int i=0; i<count; i++) value[i] = (int)nums[i];
        free(nums);
        return true;
    }
    free(nums);
    return false;
}
bool wxOptionValue::GetOption(const wxString& name, long *value, int count ) const
{
    int i;
    long *nums = (long*)malloc(sizeof(long)*count);
    wxString word, s = GetOption(name).Strip(wxString::both);

    for (i=0; i<count; i++)
    {
        word = s.BeforeFirst(wxT(' '));
        if ((!word.IsEmpty()) || !word.ToLong( &nums[i] ))
        {
            free(nums);
            return false;
        }
        s = s.AfterFirst(wxT(' ')).Strip(wxString::both);
    }
    if (i==count) for (i=0; i<count; i++) value[i] = nums[i];
    free(nums);
    return true;
}
bool wxOptionValue::GetOption(const wxString& name, float *value, int count ) const
{
    double *nums = (double*)malloc(sizeof(double)*count);
    if (GetOption(name, nums, count))
    {
        for (int i=0; i<count; i++) value[i] = (float)nums[i];
        free(nums);
        return true;
    }
    free(nums);
    return false;
}
bool wxOptionValue::GetOption(const wxString& name, double *value, int count ) const
{
    int i;
    double *nums = (double*)malloc(sizeof(double)*count);
    wxString word, s = GetOption(name).Strip(wxString::both);

    for (i=0; i<count; i++)
    {
        word = s.BeforeFirst(wxT(' '));
        if ((!word.IsEmpty()) || !word.ToDouble( &nums[i] ))
        {
            free(nums);
            return false;
        }
        s = s.AfterFirst(wxT(' ')).Strip(wxString::both);
    }
    if (i==count) for (i=0; i<count; i++) value[i] = nums[i];
    free(nums);
    return true;
}


bool wxOptionValue::GetOption(const wxString& name, int *v1, int *v2, int *v3 ) const   
{
    int n1, n2, n3;
    if (wxSscanf(GetOption(name).c_str(), wxT("%d %d %d"), &n1, &n2, &n3) == 3)
    {
        *v1 = n1; *v2 = n2; *v3 = n3;
        return true;
    }
    return false;
}
bool wxOptionValue::GetOption(const wxString& name, float *v1, float *v2, float *v3 ) const
{
    float n1, n2, n3;
    if (wxSscanf(GetOption(name).c_str(), wxT("%f %f %f"), &n1, &n2, &n3) == 3)
    {
        *v1 = n1; *v2 = n2; *v3 = n3;
        return true;
    }
    return false;
}
    
bool wxOptionValue::GetOption(const wxString& name, wxPoint &value ) const  
{
    int n1, n2;
    if (wxSscanf(GetOption(name).c_str(), wxT("%d %d"), &n1, &n2) == 2)
    {
        value = wxPoint( n1, n2 );
        return true;
    }
    return false;
}
bool wxOptionValue::GetOption(const wxString& name, wxSize &value ) const   
{
    int n1, n2;
    if (wxSscanf(GetOption(name).c_str(), wxT("%d %d"), &n1, &n2) == 2)
    {
        value = wxSize( n1, n2 );
        return true;
    }
    return false;
}
bool wxOptionValue::GetOption(const wxString& name, wxRect &value ) const   
{
    int n1, n2, n3, n4;
    if (wxSscanf(GetOption(name).c_str(), wxT("%d %d %d %d"), &n1, &n2, &n3, &n4) == 4)
    {
        value = wxRect( n1, n2, n3, n4 );
        return true;
    }
    return false;
}

/////////////////////////////////////////////////////////////////////////////
// Name:        plotdata.cpp
// Purpose:     wxPlotData container class for wxPlotCtrl
// Author:      John Labenski
// Modified by:
// Created:     12/01/2000
// Copyright:   (c) John Labenski
// Licence:     wxWindows license
/////////////////////////////////////////////////////////////////////////////

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
    #include "wx/bitmap.h"
    #include "wx/textdlg.h"
    #include "wx/msgdlg.h"
    #include "wx/dcmemory.h"
#endif // WX_PRECOMP

#include "wx/file.h"
#include "wx/wfstream.h"
#include "wx/textfile.h"
#include "wx/math.h"

#include "wx/plotctrl/plotdata.h"
#include "wx/plotctrl/fourier.h"
#include "wx/wxthings/medsort.h"
#include "wx/wxthings/range.h"
#include <cmath>

#define wxPLOT_MAX_DATA_COLUMNS 64

#define CHECK_INDEX_COUNT_MSG(index, count, max_count, ret) \
    wxCHECK_MSG((int(index) >= 0) && (int(index)+int(count) <= int(max_count)), ret, wxT("invalid index or count"))
#define CHECK_INDEX_COUNT_RET(index, count, max_count) \
    wxCHECK_RET((int(index) >= 0) && (int(index)+int(count) <= int(max_count)), wxT("invalid index or count"))

#define CHECK_START_END_INDEX_MSG(start_index, end_index, max_count, ret) \
    wxCHECK_MSG((int(start_index)>=0)&&(int(start_index)<int(max_count))&&(int(end_index)>int(start_index))&&(int(end_index)<int(max_count)), ret, wxT("Invalid data index") )
#define CHECK_START_END_INDEX_RET(start_index, end_index, max_count) \
    wxCHECK_RET((int(start_index)>=0)&&(int(start_index)<int(max_count))&&(int(end_index)>int(start_index))&&(int(end_index)<int(max_count)), wxT("Invalid data index") )

//----------------------------------------------------------------------------
// wxPlotDataRefData
//----------------------------------------------------------------------------

class wxPlotDataRefData: public wxPlotCurveRefData
{
public:
    wxPlotDataRefData();
    wxPlotDataRefData(const wxPlotDataRefData& data);
    virtual ~wxPlotDataRefData();

    void Destroy();
    void CopyData(const wxPlotDataRefData &source);
    void CopyExtra(const wxPlotDataRefData &source);

    int     m_count;

    double *m_Xdata;
    double *m_Ydata;
    double *m_Yidata; // imaginary component, not normally used (see FFT)
    bool    m_static;

    bool    m_Xordered;

    wxBitmap m_normalSymbol,
             m_activeSymbol,
             m_selectedSymbol;
};

wxPlotDataRefData::wxPlotDataRefData() : wxPlotCurveRefData()
{
    m_count  = 0;

    m_Xdata  = (double*)NULL;
    m_Ydata  = (double*)NULL;
    m_Yidata = (double*)NULL;

    m_static = false;

    m_Xordered = false;

    m_normalSymbol   = wxPlotSymbolNormal;
    m_activeSymbol   = wxPlotSymbolActive;
    m_selectedSymbol = wxPlotSymbolSelected;
}

wxPlotDataRefData::wxPlotDataRefData(const wxPlotDataRefData& data)
                  :wxPlotCurveRefData()
{
    CopyData(data);
    CopyExtra(data);
}

wxPlotDataRefData::~wxPlotDataRefData()
{
    Destroy();
}

void wxPlotDataRefData::Destroy()
{
    if (!m_static)
    {
        if ( m_Xdata  ) free( m_Xdata );
        if ( m_Ydata  ) free( m_Ydata );
        if ( m_Yidata ) free( m_Yidata );
    }

    m_count    = 0;
    m_Xdata    = NULL;
    m_Ydata    = NULL;
    m_Yidata   = NULL;
    m_Xordered = false;
}

void wxPlotDataRefData::CopyData(const wxPlotDataRefData &source)
{
    Destroy();

    m_count    = source.m_count;
    m_static   = false; // we're creating our own copy
    m_Xordered = source.m_Xordered;

    if (m_count && source.m_Xdata)
    {
        m_Xdata = (double*)malloc( m_count*sizeof(double) );
        memcpy( m_Xdata, source.m_Xdata, m_count*sizeof(double) );
    }
    if (m_count && source.m_Ydata)
    {
        m_Ydata = (double*)malloc( m_count*sizeof(double) );
        memcpy( m_Ydata, source.m_Ydata, m_count*sizeof(double) );
    }
    if (m_count && source.m_Yidata)
    {
        m_Yidata = (double*)malloc( m_count*sizeof(double) );
        memcpy( m_Ydata, source.m_Yidata, m_count*sizeof(double) );
    }
}

void wxPlotDataRefData::CopyExtra(const wxPlotDataRefData &source)
{
    wxPlotCurveRefData::Copy(source);

    m_normalSymbol   = source.m_normalSymbol;
    m_activeSymbol   = source.m_activeSymbol;
    m_selectedSymbol = source.m_selectedSymbol;
}

#define M_PLOTDATA ((wxPlotDataRefData*)m_refData)

//-----------------------------------------------------------------------------
// wxPlotData
//-----------------------------------------------------------------------------
IMPLEMENT_DYNAMIC_CLASS(wxPlotData, wxPlotCurve)

wxObjectRefData *wxPlotData::CreateRefData() const
{
    return new wxPlotDataRefData;
}
wxObjectRefData *wxPlotData::CloneRefData(const wxObjectRefData *data) const
{
    return new wxPlotDataRefData(*(const wxPlotDataRefData *)data);
}

bool wxPlotData::Ok() const
{
    return m_refData && (M_PLOTDATA->m_count > 0);
}

void wxPlotData::Destroy()
{
    UnRef();
}

int wxPlotData::GetCount() const
{
    wxCHECK_MSG( Ok(), 0, wxT("Invalid wxPlotData") );
    return M_PLOTDATA->m_count;
}

bool wxPlotData::Create( const wxPlotData& plotData )
{
    wxCHECK_MSG(plotData.Ok(), false, wxT("Invalid wxPlotData"));

    Ref(plotData);
    CalcBoundingRect(); // just to be sure we're ok
    return true;
}

bool wxPlotData::Create( const wxPlotFunction &plotFunc, double x_start, double dx, int points )
{
    wxCHECK_MSG(plotFunc.Ok(), false, wxT("Invalid wxPlotFunction"));
    wxCHECK_MSG(wxFinite(x_start) && wxFinite(dx), false, wxT("values are NaN"));

    if (!Create(points, false)) return false; // this checks points

    double x = x_start, y = 0;
    double *x_data = M_PLOTDATA->m_Xdata;
    double *y_data = M_PLOTDATA->m_Ydata;

    wxPlotFunction pf = plotFunc; // since it's passed as const

    for (int i = 0; i < points; i++, x += dx, x_data++, y_data++)
    {
        if (wxFinite(x))
        {
            y = pf.GetY(x);
            *x_data = x;
            *y_data = wxFinite(y) ? y : 0;
        }
        else
        {
            *x_data = 0;
            *y_data = 0;
        }
    }

    CalcBoundingRect();
    return true;
}

bool wxPlotData::Create( int points, bool zero )
{
    wxCHECK_MSG( points > 0, false, wxT("Can't create wxPlotData with < 1 points") );

    UnRef();
    m_refData = new wxPlotDataRefData();

    if ( !M_PLOTDATA )
    {
        wxFAIL_MSG(wxT("memory allocation error creating plot"));
        return false;
    }

    M_PLOTDATA->m_count = points;
    M_PLOTDATA->m_Xdata = (double*)malloc( points*sizeof(double) );
    M_PLOTDATA->m_Ydata = (double*)malloc( points*sizeof(double) );
    if (!M_PLOTDATA->m_Xdata || !M_PLOTDATA->m_Ydata)
    {
        UnRef();
        wxFAIL_MSG(wxT("memory allocation error creating plot"));
        return false;
    }

    if (zero)
    {
        memset(M_PLOTDATA->m_Xdata, 0, points*sizeof(double));
        memset(M_PLOTDATA->m_Ydata, 0, points*sizeof(double));
    }

    return true;
}

bool wxPlotData::Create( double *x_data, double *y_data, int points, bool static_data )
{
    wxCHECK_MSG((points > 0) && x_data && y_data, false,
                wxT("Can't create wxPlotData with < 1 points or invalid data") );

    UnRef();
    m_refData = new wxPlotDataRefData();

    if ( !M_PLOTDATA )
    {
        wxFAIL_MSG(wxT("memory allocation error creating plot"));
        return false;
    }

    M_PLOTDATA->m_Xdata  = x_data;
    M_PLOTDATA->m_Ydata  = y_data;
    M_PLOTDATA->m_count  = points;
    M_PLOTDATA->m_static = static_data;

    CalcBoundingRect();
    return true;
}

bool wxPlotData::Copy( const wxPlotData &source, bool copy_all )
{
    wxCHECK_MSG( source.Ok(), false, wxT("Invalid wxPlotData") );

    int count = source.GetCount();

    if (!Create(count, false)) return false;

    memcpy( M_PLOTDATA->m_Xdata, source.GetXData(), count*sizeof(double) );
    memcpy( M_PLOTDATA->m_Ydata, source.GetYData(), count*sizeof(double) );

    if (source.GetYiData())
    {
        M_PLOTDATA->m_Yidata = (double*)malloc(count*sizeof(double));
        if (!M_PLOTDATA->m_Yidata)
        {
            UnRef();
            return false;
        }
        memcpy( M_PLOTDATA->m_Yidata, source.GetYiData(), count*sizeof(double) );
    }

    if (copy_all)
        CopyExtra(source);

    CalcBoundingRect();
    return true;
}

bool wxPlotData::CopyExtra( const wxPlotData &source )
{
    wxCHECK_MSG( Ok() && source.Ok(), false, wxT("Invalid wxPlotData") );

    M_PLOTDATA->Copy(*((wxPlotDataRefData*)source.GetRefData()));

    return true;
}

bool wxPlotData::Resize( int new_size, bool zero )
{
    wxCHECK_MSG(Ok(), false, wxT("Invalid wxPlotData"));
    wxCHECK_MSG(new_size > 0, false, wxT("Can't create wxPlotData with < 1 points"));
    if (M_PLOTDATA->m_count == new_size)
        return true;

    M_PLOTDATA->m_Xdata = (double*)realloc( M_PLOTDATA->m_Xdata, new_size*sizeof(double) );
    M_PLOTDATA->m_Ydata = (double*)realloc( M_PLOTDATA->m_Ydata, new_size*sizeof(double) );

    bool has_yi = false;
    if (M_PLOTDATA->m_Yidata)
    {
        has_yi = true;
        M_PLOTDATA->m_Yidata = (double*)realloc( M_PLOTDATA->m_Yidata, new_size*sizeof(double) );
    }

    // realloc failed
    if (!M_PLOTDATA->m_Xdata || !M_PLOTDATA->m_Ydata || (has_yi && (!M_PLOTDATA->m_Yidata)))
    {
        UnRef();
        return false;
    }

    if (zero && (M_PLOTDATA->m_count < new_size ))
    {
        int old_count = M_PLOTDATA->m_count;
        int points    = new_size - M_PLOTDATA->m_count;
        memset(M_PLOTDATA->m_Xdata + old_count, 0, points*sizeof(double));
        memset(M_PLOTDATA->m_Ydata + old_count, 0, points*sizeof(double));
        if (M_PLOTDATA->m_Yidata)
            memset(M_PLOTDATA->m_Yidata + old_count, 0, points*sizeof(double));
    }

    M_PLOTDATA->m_count = new_size;
    CalcBoundingRect();
    return true;
}

bool wxPlotData::Resize( int new_size, double dx, double y )
{
    wxCHECK_MSG(Ok(), false, wxT("Invalid wxPlotData"));
    int orig_count = GetCount();
    if (!Resize(new_size, false)) return false;

    SetYValues(orig_count, -1, y);
    SetXStepValues(orig_count, -1, M_PLOTDATA->m_Xdata[orig_count - 1], dx);
    return true;
}

wxPlotData wxPlotData::Append( const wxPlotData &source ) const
{
    wxCHECK_MSG( Ok() && source.Ok(), wxPlotData(), wxT("Invalid wxPlotData") );

    int count     = M_PLOTDATA->m_count;
    int src_count = source.GetCount();

    wxPlotData newCurve(count + src_count, false);
    if (!newCurve.Ok()) return newCurve;

    bool has_yi = false;
    if (M_PLOTDATA->m_Yidata && source.GetYiData())
    {
        has_yi = true;
        double *yi = (double*)malloc((count+src_count)*sizeof(double));
        if (!yi)
        {
            newCurve.Destroy();
            return newCurve;
        }
        newCurve.SetYiData(yi);
    }

    memcpy(newCurve.GetXData(), M_PLOTDATA->m_Xdata, count*sizeof(double));
    memcpy(newCurve.GetYData(), M_PLOTDATA->m_Ydata, count*sizeof(double));
    if (has_yi)
        memcpy(newCurve.GetYiData(), M_PLOTDATA->m_Yidata, count*sizeof(double));

    memcpy(newCurve.GetXData()+count, source.GetXData(), src_count*sizeof(double));
    memcpy(newCurve.GetYData()+count, source.GetYData(), src_count*sizeof(double));
    if (has_yi)
        memcpy(newCurve.GetYiData(), source.GetYiData(), src_count*sizeof(double));

    newCurve.CalcBoundingRect();
    newCurve.CopyExtra( *this );
    return newCurve;
}

wxPlotData wxPlotData::Insert( const wxPlotData &source, int index ) const
{
    wxCHECK_MSG( Ok() && source.Ok(), wxPlotData(), wxT("Invalid wxPlotData") );
    wxPCHECK_MINMAX_MSG(index, 0, M_PLOTDATA->m_count, wxPlotData(), wxT("invalid index"));

    int count     = M_PLOTDATA->m_count;
    int src_count = source.GetCount();

    wxPlotData newCurve(count + src_count, false);
    if (!newCurve.Ok()) return newCurve;

    double *src_x_data  = source.GetXData();
    double *src_y_data  = source.GetYData();
    double *src_yi_data = source.GetYiData();

    double *x_data  = M_PLOTDATA->m_Xdata;
    double *y_data  = M_PLOTDATA->m_Ydata;
    double *yi_data = M_PLOTDATA->m_Yidata;

    bool has_yi = false;
    if (yi_data && src_yi_data)
    {
        has_yi = true;
        double *yi = (double*)malloc((count+src_count)*sizeof(double));
        if (!yi)
        {
            newCurve.Destroy();
            return newCurve;
        }
        newCurve.SetYiData(yi);
    }

    double *new_Xdata  = newCurve.GetXData();
    double *new_Ydata  = newCurve.GetYData();
    double *new_Yidata = newCurve.GetYiData();

    if (index > 0)
    {
        memcpy(new_Xdata, x_data, index*sizeof(double));
        memcpy(new_Ydata, y_data, index*sizeof(double));
        if (has_yi)
            memcpy(new_Yidata, yi_data, index*sizeof(double));
    }
    memcpy(new_Xdata+index, src_x_data, src_count*sizeof(double));
    memcpy(new_Ydata+index, src_y_data, src_count*sizeof(double));
    if (has_yi)
        memcpy(new_Yidata+index, src_yi_data, src_count*sizeof(double));

    memcpy(new_Xdata+index+src_count, x_data+index, (count-index)*sizeof(double));
    memcpy(new_Ydata+index+src_count, y_data+index, (count-index)*sizeof(double));
    if (has_yi)
        memcpy(new_Yidata+index+src_count, yi_data+index, (count-index)*sizeof(double));

    newCurve.CalcBoundingRect();
    newCurve.CopyExtra( *this );
    return newCurve;
}

wxPlotData wxPlotData::Remove(int index, int count) const
{
    wxPlotData newCurve;
    wxCHECK_MSG( Ok(), newCurve, wxT("Invalid wxPlotData") );
    if (count <= 0) count = M_PLOTDATA->m_count - index;
    wxCHECK_MSG((index >= 0) && (index+count <= M_PLOTDATA->m_count), newCurve, wxT("invalid index"));

    // should probably give an error here?
    if ((index == 0) && (count < 0)) return newCurve;

    if (count < 0)       // return first part
    {
        newCurve = GetSubPlotData(0, index);
    }
    else if (index == 0) // return last part
    {
        newCurve = GetSubPlotData(index+count, -1);
    }
    else                 // return first and last, removing middle
    {
        newCurve = GetSubPlotData(0, index);
        if (index+count < M_PLOTDATA->m_count - 1)
            newCurve = newCurve.Append(GetSubPlotData(index+count, -1));
    }

    newCurve.CopyExtra( *this );
    return newCurve;
}

wxPlotData wxPlotData::GetSubPlotData(int index, int count) const
{
    wxCHECK_MSG( Ok(), wxPlotData(), wxT("Invalid wxPlotData") );
    if (count <= 0) count = M_PLOTDATA->m_count - index;
    wxCHECK_MSG((index >= 0) && (index+count <= M_PLOTDATA->m_count), wxPlotData(), wxT("invalid index"));

    wxPlotData newCurve(count, false);
    if (!newCurve.Ok()) return newCurve;

    bool has_yi = false;
    if (M_PLOTDATA->m_Yidata)
    {
        has_yi = true;
        double *yi = (double*)malloc(count*sizeof(double));
        newCurve.SetYiData(yi);
    }

    memcpy(newCurve.GetXData(), M_PLOTDATA->m_Xdata+index, count*sizeof(double));
    memcpy(newCurve.GetYData(), M_PLOTDATA->m_Ydata+index, count*sizeof(double));
    if (has_yi)
        memcpy(newCurve.GetYiData(), M_PLOTDATA->m_Yidata+index, count*sizeof(double));

    newCurve.CalcBoundingRect();
    newCurve.CopyExtra( *this );
    return newCurve;
}

void wxPlotData::CalcBoundingRect()
{
    wxCHECK_RET( Ok(), wxT("Invalid wxPlotData") );

    M_PLOTDATA->m_boundingRect = wxRect2DDouble(0, 0, 0, 0);

    double *x_data = M_PLOTDATA->m_Xdata,
           *y_data = M_PLOTDATA->m_Ydata;

    double x = *x_data,
           y = *y_data,
           xmin = x,
           xmax = x,
           ymin = y,
           ymax = y,
           xlast = x;

    bool xordered = true, valid = false;

    register int i, count = M_PLOTDATA->m_count;

    for (i=0; i<count; i++)
    {
        x = *x_data++;
        y = *y_data++;

        if ((wxFinite(x) == 0) || (wxFinite(y) == 0)) continue;

        if (!valid) // initialize the bounds
        {
           valid = true;
           xmin = xmax = xlast = x;
           ymin = ymax = y;
           continue;
        }

        if      ( x < xmin ) xmin = x;
        else if ( x > xmax ) xmax = x;

        if      ( y < ymin ) ymin = y;
        else if ( y > ymax ) ymax = y;

        if ( xlast > x ) xordered = false;
        else             xlast = x;
    }

    if (valid)
        M_PLOTDATA->m_boundingRect = wxRect2DDouble(xmin, ymin, xmax-xmin, ymax-ymin);
    else
        M_PLOTDATA->m_boundingRect = wxRect2DDouble(0, 0, 0, 0);

    M_PLOTDATA->m_Xordered = xordered;
}

bool wxPlotData::GetIsXOrdered() const
{
    wxCHECK_MSG( Ok(), false, wxT("Invalid wxPlotData") );
    return M_PLOTDATA->m_Xordered;
}

//----------------------------------------------------------------------------
// Load/Save Get/Set Filename, Header
//----------------------------------------------------------------------------

int NumberParse (double *nums, const wxString &string)
{
    const wxChar d1 = wxT(',');  // 44; // comma
    const wxChar d2 = wxT('\t'); //  9; // tab
    const wxChar d3 = wxT(' ');  // 32; // space
    const wxChar d4 = wxT('\n'); // 13; // carrage return
    const wxChar d5 = wxT('\r'); // 10; // line feed

//    char *D = "D";    // for Quick Basic, uses 1D3 not 1E3
//    char *E = "E";

    const wxChar *s = string.GetData();

    int i, count = string.Length();
    double number;

    int n = 0;
    int start_word = -1;

    for (i=0; i<=count; i++)
    {
        if ((*s == d1 || *s == d2 || *s == d2 || *s == d3 || *s == d4 || *s == d5) || (i >= count))
        {
            if (start_word != -1)
            {
                if ( string.Mid(start_word, i - start_word).ToDouble(&number) )
                {
                    nums[n] = number;
                    n++;
                    if (n >= wxPLOT_MAX_DATA_COLUMNS) return n;
                    start_word = -1;
                }
                else
                    return n;
            }
        }
        else if (start_word == -1) start_word = i;

        if (*s == d4 || *s == d5) return n;

        s++;
    }
    return n;
}

static char GetNextFileStreamChar( wxFileInputStream &fs )
{
    return fs.Eof() ? '\0' : fs.GetC();

/*
    if (fs.Eof()) return wxT('\0');
    // see txtstream.cpp, ReadLine()
#if wxUSE_UNICODE
    // FIXME: this is only works for single byte encodings
    // How-to read a single char in an unkown encoding???
    char buf[10] = {0};
    memset(buf, 0, 10*sizeof(char));
    buf[0] = fs.GetC();

    wxChar wbuf[10] = {0};
    memset(wbuf, 0, 10*sizeof(wxChar));
    wxConvUTF8.MB2WC( wbuf, buf, 2 );
    wxChar c = wbuf[0];
#else
    wxChar c = fs.GetC();
#endif
    return c;
*/
}

bool wxPlotData::LoadFile( const wxString &filename, int x_col, int y_col, int options )
{
    if (filename.IsEmpty()) return false;

    wxFile loadfile;
    loadfile.Open( filename, wxFile::read );
    if (!loadfile.IsOpened()) return false;

    wxFileInputStream fileStream( loadfile );
    if (!fileStream.Ok())
    {
        loadfile.Close();
        return false;
    }

    wxEOL eol = wxEOL_NATIVE;
    if (1)
    {
        while ((eol == wxEOL_NATIVE) && !fileStream.Eof())
        {
            const char c = GetNextFileStreamChar(fileStream);

            if (c == '\n')
            {
                eol = wxEOL_UNIX;
            }
            else if (c == '\r')
            {
                const char cc = GetNextFileStreamChar(fileStream);

                if (cc == '\n')
                    eol = wxEOL_DOS;
                else
                    eol = wxEOL_MAC;
            }
        }
    }

    // Rewind file
    fileStream.SeekI(off_t(0));

    wxTextInputStream textstream( fileStream );

    int allocated_data = 400;
    double *x_data = (double*)malloc(allocated_data*sizeof(double));
    double *y_data = (double*)malloc(allocated_data*sizeof(double));

    if (!x_data || !y_data)
    {
        if (x_data) free(x_data);
        if (y_data) free(y_data);
        return false;
    }

    int points = 0, line_number = 0;
    wxString wxstr;
    wxString header;
    wxString separator;
    bool stop_load = false;

    double nums[wxPLOT_MAX_DATA_COLUMNS];
    int n;

    bool select_cols = (x_col < 0) || (y_col < 0);

    while ( !fileStream.Eof() && !stop_load )
    {
        wxstr = textstream.ReadLine().Strip(wxString::both);
        line_number++;

        if (wxstr.IsEmpty())
        {
            if ((points > 0) && (options & wxPLOTDATA_LOAD_BREAKONBLANKLINE))
                stop_load = true;
        }
        else if (wxstr.Left(1) == wxT("#"))
        {
            if ((points == 0) && (options & wxPLOTDATA_LOAD_HEADER))
                header += (wxstr + wxT("\n"));
        }
        else
        {
            n = NumberParse( nums, wxstr );

            if (select_cols)
            {
                if (n == 0) // FAIL
                {
                    x_col = y_col = 1;
                    select_cols = false;
                }
                else if (n == 1)
                {
                    x_col = y_col = 0;
                    select_cols = false;
                }
                else if (n == 2)
                {
                    x_col = 0;
                    y_col = 1;
                    select_cols = false;
                }

                while (select_cols && !stop_load )
                {
                    wxString colStr = wxGetTextFromUser(
                        wxString::Format(wxT("%d columns found.\n")
                                             wxT("Enter x and y cols separated by a space starting from 1.\n")
                                             wxT("If x = y then x values are 0,1,2... and y is that col."), n),
                                             wxT("Enter columns of data file to use"));

                    if (colStr.IsEmpty())
                    {
                        stop_load = true;
                    }
                    else
                    {
                        colStr.Trim(true).Trim(false);

                        wxString leftStr = colStr.BeforeFirst(wxT(' '));
                        wxString rightStr = colStr.AfterFirst(wxT(' '));
                        if (rightStr.IsEmpty()) rightStr = leftStr;

                        long xcol=0, ycol=0;
                        if ((!leftStr.IsEmpty()) &&
                            leftStr.ToLong(&xcol) && rightStr.ToLong(&ycol))
                        {
                            xcol--;
                            ycol--;
                            if ((xcol>=0) && (ycol>=0) && (xcol<n) && (ycol<n))
                            {
                                x_col = int(xcol);
                                y_col = int(ycol);
                                select_cols = false;
                            }
                        }
                    }
                }
            }

            if (((x_col >= n) || (y_col >= n)) && (!stop_load || select_cols))
            {
                // FIXME - wxEOL_MAC - and end of file filestream.Eof() == 1, last line is a '@'
                if (fileStream.Eof() && (eol==wxEOL_MAC) && (wxstr == wxT("@")))
                {
                    printf("MAC EOF @"); fflush(stdout);
                    break;
                }

                wxMessageBox(
                    wxString::Format(wxT("Loading cols (%d,%d) aborted after %d points\n\n")
                                         wxT("First 100 characters of offending line number: %d\n")
                                         wxT("\"%s\"\n\n")
                                         wxT("# for comments, blank lines Ok, comma, tab, space for separators\n")
                                         wxT("7   4\n33  2.5e-2\n...\n"),
                                         x_col, y_col, points, line_number, wxstr.Left(100).c_str()),
                                         wxT("Error loading ")+filename, wxOK|wxICON_ERROR);
                stop_load = true;
                break;
            }
            else if (!stop_load)
            {
                // figure out what the separator is for the first line only
                if (points == 0)
                {
                    if (wxstr.Find(wxT(',')) != wxNOT_FOUND)
                        separator = wxPLOTCURVE_DATASEPARATOR_COMMA;
                    else if (wxstr.Find(wxT('\t')) != wxNOT_FOUND)
                        separator = wxPLOTCURVE_DATASEPARATOR_TAB;
                }
                // reallocate data if necessary
                if ( points >= allocated_data - 1)
                {
                    allocated_data += 400;
                    x_data = (double*)realloc( x_data, allocated_data*sizeof(double) );
                    y_data = (double*)realloc( y_data, allocated_data*sizeof(double) );

                    if (!x_data || !y_data)
                    {
                        if (x_data) free(x_data);
                        if (y_data) free(y_data);
                        loadfile.Close();
                        return false;
                    }
                }

                if (x_col == y_col)
                    x_data[points] = points;
                else
                    x_data[points] = nums[x_col];

                y_data[points] = nums[y_col];

                points++;
            }
        }
    }
    loadfile.Close();

    if (points > 0)
    {
        x_data = (double*)realloc( x_data, points*sizeof(double) );
        y_data = (double*)realloc( y_data, points*sizeof(double) );

        bool ok = Create( x_data, y_data, points, false );
        if (ok)
        {
            SetFilename( filename );
            SetEOLMode( eol );
            if (!separator.IsEmpty())
                SetDataColumnSeparator(separator);
            if (header.Len() > 0u)
                SetHeader(header);
        }
        return ok;
    }

    if (x_data) free(x_data);
    if (y_data) free(y_data);

    return false;
}

bool wxPlotData::SaveFile( const wxString &filename, bool save_header, const wxString &format )
{
    wxCHECK_MSG(Ok(), false, wxT("Invalid wxPlotData") );

    wxCHECK_MSG((!format.IsEmpty()) && (format.Find(wxT('%')) != wxNOT_FOUND), false, wxT("invalid format"));

    if (filename.IsEmpty()) return false;

    wxFile savefile;
    savefile.Open( filename, wxFile::write );
    if (!savefile.IsOpened()) return false;

    wxFileOutputStream filestream( savefile );
    wxTextOutputStream textstream( filestream );

    textstream.SetMode( GetEOLMode() );
    int i;

    wxString header = GetOption(wxPLOTCURVE_OPTION_HEADER);
    if (save_header && (header.Len() > 0u))
    {
        textstream.WriteString(header);
        const wxChar lastChar = header[header.Len()-1];
        if ((lastChar != wxT('\r')) || (lastChar != wxT('\n')))
            textstream.WriteString(wxT("\n"));
    }

    wxString s;
    wxString sep = GetDataColumnSeparator();
    wxString f(wxT("%g"));
    if (!format.IsEmpty()) f = format;

    double x, y;

    for (i=0; i<M_PLOTDATA->m_count; i++)
    {
        x = M_PLOTDATA->m_Xdata[i];
        y = M_PLOTDATA->m_Ydata[i];

        if (wxFinite(x)) s = wxString::Format(f.c_str(), x);
        else             s = wxT("nan");

        s += sep;

        if (wxFinite(y)) s += wxString::Format(f.c_str(), y);
        else             s += wxT("nan");

        s += wxT("\n");
        textstream.WriteString( s );
    }

    savefile.Close();

    SetFilename(filename);

    return true;
}

wxString wxPlotData::GetFilename() const
{
    wxCHECK_MSG( Ok(), wxEmptyString, wxT("Invalid wxPlotData") );
    return GetOption(wxPLOTCURVE_OPTION_FILENAME);
}
void wxPlotData::SetFilename( const wxString &filename )
{
    wxCHECK_RET( Ok(), wxT("Invalid wxPlotData") );
    SetOption(wxPLOTCURVE_OPTION_FILENAME, filename, true);
}

wxString wxPlotData::GetHeader() const
{
    wxCHECK_MSG( Ok(), wxEmptyString, wxT("Invalid wxPlotData") );
    return GetOption(wxPLOTCURVE_OPTION_HEADER);
}

void wxPlotData::SetHeader( const wxString &header )
{
    wxCHECK_RET( Ok(), wxT("Invalid wxPlotData") );
    SetOption(wxPLOTCURVE_OPTION_HEADER, header);
}

wxEOL wxPlotData::GetEOLMode() const
{
    wxCHECK_MSG( Ok(), wxEOL_NATIVE, wxT("Invalid wxPlotData") );
    int eol = HasOption(wxPLOTCURVE_OPTION_EOLMODE) ? GetOptionInt(wxPLOTCURVE_OPTION_EOLMODE) : wxEOL_NATIVE;
    switch (eol)
    {
        case wxEOL_NATIVE : return wxEOL_NATIVE;
        case wxEOL_UNIX   : return wxEOL_UNIX;
        case wxEOL_MAC    : return wxEOL_MAC;
        case wxEOL_DOS    : return wxEOL_DOS;
        default : break;
    }

    return wxEOL_NATIVE;
}
void wxPlotData::SetEOLMode( wxEOL eol )
{
    wxCHECK_RET( Ok(), wxT("Invalid wxPlotData") );
    SetOption(wxPLOTCURVE_OPTION_EOLMODE, (int)eol);
}

wxString wxPlotData::GetDataColumnSeparator() const
{
    wxCHECK_MSG( Ok(), wxPLOTCURVE_DATASEPARATOR_SPACE, wxT("Invalid wxPlotData") );

    wxString s = wxPLOTCURVE_DATASEPARATOR_SPACE;
    GetOption(wxPLOTCURVE_OPTION_DATASEPARATOR, s);
    return s;
}
void wxPlotData::SetDataColumnSeparator( const wxString &separator )
{
    wxCHECK_RET( Ok(), wxT("Invalid wxPlotData") );
    SetOption(wxPLOTCURVE_OPTION_DATASEPARATOR, separator, true);
}

//----------------------------------------------------------------------------
// Get(X/Y)Data
//----------------------------------------------------------------------------

double *wxPlotData::GetXData() const
{
    wxCHECK_MSG( Ok(), (double*)NULL, wxT("Invalid wxPlotData") );
    return M_PLOTDATA->m_Xdata;
}
double *wxPlotData::GetYData() const
{
    wxCHECK_MSG( Ok(), (double*)NULL, wxT("Invalid wxPlotData") );
    return M_PLOTDATA->m_Ydata;
}
double *wxPlotData::GetYiData() const
{
    wxCHECK_MSG( Ok(), (double*)NULL, wxT("Invalid wxPlotData") );
    return M_PLOTDATA->m_Yidata;
}

void wxPlotData::SetYiData( double *yi_data )
{
    wxCHECK_RET( Ok(), wxT("Invalid wxPlotData") );

    if (M_PLOTDATA->m_Yidata) free(M_PLOTDATA->m_Yidata);
    M_PLOTDATA->m_Yidata = yi_data;
}

double wxPlotData::GetXValue( int index ) const
{
    wxCHECK_MSG( Ok() && (index < M_PLOTDATA->m_count), 0.0, wxT("Invalid wxPlotData") );
    return M_PLOTDATA->m_Xdata[index];
}
double wxPlotData::GetYValue( int index ) const
{
    wxCHECK_MSG( Ok() && (index < M_PLOTDATA->m_count), 0.0, wxT("Invalid wxPlotData") );
    return M_PLOTDATA->m_Ydata[index];
}
wxPoint2DDouble wxPlotData::GetPoint( int index ) const
{
    wxCHECK_MSG( Ok() && (index < M_PLOTDATA->m_count), wxPoint2DDouble(0,0), wxT("Invalid wxPlotData") );
    return wxPoint2DDouble(M_PLOTDATA->m_Xdata[index], M_PLOTDATA->m_Ydata[index]);
}

double wxPlotData::GetY( double x )
{
    wxCHECK_MSG(Ok(), 0, wxT("invalid wxPlotData"));

    int i = GetIndexFromX( x, index_floor );

    if (M_PLOTDATA->m_Xdata[i] == x)
        return M_PLOTDATA->m_Ydata[i];

    if (i >= M_PLOTDATA->m_count - 1)
        return M_PLOTDATA->m_Ydata[i];

    int i1 = GetIndexFromX( x, index_ceil );

    double y0 = M_PLOTDATA->m_Ydata[i];
    double y1 = M_PLOTDATA->m_Ydata[i1];

    if ( y0 == y1 )
        return y0;

    return LinearInterpolateY( M_PLOTDATA->m_Xdata[i], y0,
                               M_PLOTDATA->m_Xdata[i1], y1, x );
}

void wxPlotData::SetXValue( int index, double x )
{
    wxCHECK_RET( Ok() && (index < M_PLOTDATA->m_count), wxT("Invalid wxPlotData") );

    if (M_PLOTDATA->m_count == 1)
        M_PLOTDATA->m_boundingRect.m_x = x;
    else
    {
        if (x < M_PLOTDATA->m_boundingRect.m_x)
            M_PLOTDATA->m_boundingRect.SetLeft(x);
        else if (x > M_PLOTDATA->m_boundingRect.GetRight())
            M_PLOTDATA->m_boundingRect.SetRight(x);
        else
            CalcBoundingRect(); // don't know recalc it all
    }

    M_PLOTDATA->m_Xdata[index] = x;

}
void wxPlotData::SetYValue( int index, double y )
{
    wxCHECK_RET( Ok() && (index < M_PLOTDATA->m_count), wxT("Invalid wxPlotData") );

    if (M_PLOTDATA->m_count == 1)
        M_PLOTDATA->m_boundingRect.m_y = y;
    else
    {
        if (y < M_PLOTDATA->m_boundingRect.m_y)
            M_PLOTDATA->m_boundingRect.SetTop(y);
        else if (y > M_PLOTDATA->m_boundingRect.GetBottom())
            M_PLOTDATA->m_boundingRect.SetBottom(y);
        else
            CalcBoundingRect(); // don't know recalc it all
    }

    M_PLOTDATA->m_Ydata[index] = y;
}

void wxPlotData::SetValue(int index, double x, double y)
{
    wxCHECK_RET( Ok() && (index < M_PLOTDATA->m_count), wxT("Invalid wxPlotData") );

    double x_old = M_PLOTDATA->m_Xdata[index];
    double y_old = M_PLOTDATA->m_Ydata[index];

    M_PLOTDATA->m_Xdata[index] = x;
    M_PLOTDATA->m_Ydata[index] = y;

    if (M_PLOTDATA->m_count == 1)
    {
        M_PLOTDATA->m_boundingRect.m_x = x;
        M_PLOTDATA->m_boundingRect.m_y = y;
    }
    else
    {
        if ( (x_old <= M_PLOTDATA->m_boundingRect.m_x) ||
             (x_old >= M_PLOTDATA->m_boundingRect.GetRight()) ||
             (y_old >= M_PLOTDATA->m_boundingRect.m_y) ||
             (y_old <= M_PLOTDATA->m_boundingRect.GetBottom()) )
            CalcBoundingRect(); // don't know recalc it all
        else
        {
            if (x < M_PLOTDATA->m_boundingRect.m_x)
                M_PLOTDATA->m_boundingRect.m_x = x;
            if (x > M_PLOTDATA->m_boundingRect.GetRight())
                M_PLOTDATA->m_boundingRect.SetRight(x);

            if (y > M_PLOTDATA->m_boundingRect.m_y)
                M_PLOTDATA->m_boundingRect.m_y = y;
            if (y < M_PLOTDATA->m_boundingRect.GetBottom())
                M_PLOTDATA->m_boundingRect.SetBottom(y);
        }
    }
}

void wxPlotData::SetXValues( int start_index, int count, double x )
{
    wxCHECK_RET( Ok(), wxT("Invalid wxPlotData") );
    if (count == 0) return;
    if (count < 0) count = M_PLOTDATA->m_count-start_index;
    int end_index = start_index + count - 1;
    wxPCHECK_MINMAX_RET(start_index, 0, M_PLOTDATA->m_count-1, wxT("Invalid starting index"));
    wxPCHECK_MINMAX_RET(end_index,   0, M_PLOTDATA->m_count-1, wxT("Invalid ending index"));
    double *x_data = M_PLOTDATA->m_Xdata;
    for (int n = start_index; n <= end_index; n++)
        *x_data++ = x;
}
void wxPlotData::SetYValues( int start_index, int count, double y )
{
    wxCHECK_RET( Ok(), wxT("Invalid wxPlotData") );
    if (count == 0) return;
    if (count < 0) count = M_PLOTDATA->m_count-start_index;
    int end_index = start_index + count - 1;
    wxPCHECK_MINMAX_RET(start_index, 0, M_PLOTDATA->m_count-1, wxT("Invalid starting index"));
    wxPCHECK_MINMAX_RET(end_index,   0, M_PLOTDATA->m_count-1, wxT("Invalid ending index"));
    double *y_data = M_PLOTDATA->m_Ydata;
    for (int n = start_index; n <= end_index; n++)
        *y_data++ = y;
}

void wxPlotData::SetXStepValues( int start_index, int count, double x_start, double dx )
{
    wxCHECK_RET( Ok(), wxT("Invalid wxPlotData") );
    if (count == 0) return;
    if (count < 0) count = M_PLOTDATA->m_count-start_index;
    int end_index = start_index + count - 1;
    wxPCHECK_MINMAX_RET(start_index, 0, M_PLOTDATA->m_count-1, wxT("Invalid starting index"));
    wxPCHECK_MINMAX_RET(end_index,   0, M_PLOTDATA->m_count-1, wxT("Invalid ending index"));

    double *x_data = M_PLOTDATA->m_Xdata + start_index;
    for (int i = 0; i < count; i++, x_data++)
        *x_data = x_start + (i * dx);
}
void wxPlotData::SetYStepValues( int start_index, int count, double y_start, double dy )
{
    wxCHECK_RET( Ok(), wxT("Invalid wxPlotData") );
    if (count == 0) return;
    if (count < 0) count = M_PLOTDATA->m_count-start_index;
    int end_index = start_index + count - 1;
    wxPCHECK_MINMAX_RET(start_index, 0, M_PLOTDATA->m_count-1, wxT("Invalid starting index"));
    wxPCHECK_MINMAX_RET(end_index,   0, M_PLOTDATA->m_count-1, wxT("Invalid ending index"));

    double *y_data = M_PLOTDATA->m_Ydata + start_index;
    for (int i = 0; i < count; i++, y_data++)
        *y_data = y_start + (i * dy);
}

int wxPlotData::GetIndexFromX( double x, wxPlotData::Index_Type type ) const
{
    wxCHECK_MSG( Ok(), 0, wxT("Invalid wxPlotData") );

    int count = M_PLOTDATA->m_count;
    double *x_data = M_PLOTDATA->m_Xdata;

    if ( !M_PLOTDATA->m_Xordered )
    {
        register int i;
        int index = 0, index_lower = 0, index_higher = 0;
        double closest = fabs( x - *x_data++ );

        for (i=1; i<count; i++)
        {
            if ( fabs( x - *x_data ) < closest )
            {
                if (x == *x_data) return i;

                closest = fabs( x - *x_data );
                index = i;

                if (x > *x_data)
                    index_lower = i;
                else
                    index_higher = i;
            }
            x_data++;
        }

        // out of bounds so just return the closest
        if ((x < M_PLOTDATA->m_boundingRect.GetLeft()) ||
            (x > M_PLOTDATA->m_boundingRect.GetRight()) )
            return index;

        if (type == index_floor) return index_lower;
        if (type == index_ceil) return index_higher;
        return index;
    }

    // if the data is ordered use a faster search
    if (x < M_PLOTDATA->m_boundingRect.GetLeft())
        return 0;
    if (x > M_PLOTDATA->m_boundingRect.GetRight())
        return M_PLOTDATA->m_count - 1;

    if (x <= x_data[0]) return 0;
    if (x >= x_data[count-1]) return count-1;

    int i, lo = 0, hi = count;
    double res;

    while ( lo < hi )
    {
        i = (lo + hi)/2;
        res = x - x_data[i];
        if      ( res < 0 )  hi = i;
        else if ( res > 0 )  lo = i + 1;
        else               { lo = i; break; }
    }

    if (type == index_floor)
    {
        if ((lo > 0) && (x < x_data[lo])) lo--;
        return lo;
    }
    if (type == index_ceil)
    {
        if ((lo < count - 1) && (x > x_data[lo])) lo++;
        return lo;
    }

    if ((lo > 0) && (fabs(x - x_data[lo-1]) < fabs(x - x_data[lo]))) lo--;
    if ((lo < count-1) && (fabs(x - x_data[lo+1]) < fabs(x - x_data[lo]))) lo++;

    return lo;
}

int wxPlotData::GetIndexFromY( double y, wxPlotData::Index_Type type ) const
{
    wxCHECK_MSG( Ok(), 0, wxT("Invalid wxPlotData") );

    register int i;
    int index = 0, index_lower = 0, index_higher = 0;
    int count = M_PLOTDATA->m_count;
    double *y_data = M_PLOTDATA->m_Ydata;
    double closest = fabs( y - *y_data++ );

    for (i=1; i<count; i++)
    {
        if ( fabs( y - *y_data ) < closest )
        {
            if (y == *y_data) return i;

            closest = fabs( y - *y_data );
            index = i;

            if (y > *y_data)
                index_lower = i;
            else
                index_higher = i;
        }
        y_data++;
    }

    // out of bounds so just return the closest
    if ((y < M_PLOTDATA->m_boundingRect.GetLeft()) ||
        (y > M_PLOTDATA->m_boundingRect.GetRight()) )
        return index;

    if (type == index_floor) return index_lower;
    if (type == index_ceil) return index_higher;
    return index;
}

int wxPlotData::GetIndexFromXY( double x, double y, double x_range ) const
{
    wxCHECK_MSG( Ok() && (x_range >= 0), 0, wxT("Invalid wxPlotData") );

    int start = 1, end = M_PLOTDATA->m_count - 1;

    // find bounding indexes for range
    if (M_PLOTDATA->m_Xordered && (x_range != 0))
    {
        start = GetIndexFromX(x - x_range, wxPlotData::index_floor) + 1;
        end = GetIndexFromX(x + x_range, wxPlotData::index_ceil);
    }

    int i, index = start - 1;

    double *x_data = &M_PLOTDATA->m_Xdata[index];
    double *y_data = &M_PLOTDATA->m_Ydata[index];

    double xdiff = (*x_data++) - x;
    double ydiff = (*y_data++) - y;
    double diff = xdiff*xdiff + ydiff*ydiff;
    double min_diff = diff;

    double x_lower = x - x_range, x_higher = x + x_range;

    for (i=start; i<=end; i++)
    {
        if ((x_range != 0) && ((*x_data < x_lower) || (*x_data > x_higher)))
        {
            x_data++;
            y_data++;
            continue;
        }

        xdiff = (*x_data++) - x;
        ydiff = (*y_data++) - y;
        diff = xdiff*xdiff + ydiff*ydiff;

        if (diff < min_diff)
        {
            min_diff = diff;
            index = i;
        }
    }

    return index;
}

double wxPlotData::GetAverage( int start_index, int count ) const
{
    wxCHECK_MSG( Ok(), 0.0, wxT("Invalid wxPlotData") );
    if (count < 0) count = M_PLOTDATA->m_count-start_index;
    int end_index = start_index + count - 1;
    wxCHECK_MSG((start_index<M_PLOTDATA->m_count) && (end_index<M_PLOTDATA->m_count), 0.0, wxT("invalid input"));

    double ave = 0.0;
    double *y_data = M_PLOTDATA->m_Ydata + start_index;
    for (int i=start_index; i<=end_index; i++) ave += *y_data++;

    ave /= double(count);
    return ave;
}

int wxPlotData::GetMinMaxAve( const wxRangeIntSelection& rangeSel,
                              wxPoint2DDouble* minXY_, wxPoint2DDouble* maxXY_,
                              wxPoint2DDouble* ave_,
                              int *x_min_index_, int *x_max_index_,
                              int *y_min_index_, int *y_max_index_ ) const
{
    wxCHECK_MSG(Ok(), 0, wxT("Invalid data curve"));
    wxCHECK_MSG(rangeSel.GetCount() != 0, 0, wxT("Invalid range selection"));

    int min_index = rangeSel.GetRange(0).m_min;
    //int max_index = rangeSel.GetRange(sel_count-1).m_max;

    wxCHECK_MSG((min_index >= 0) && (min_index < (int)M_PLOTDATA->m_count), 0,
                wxT("Invalid range selection index in data curve"));

    double *x_data = M_PLOTDATA->m_Xdata;
    double *y_data = M_PLOTDATA->m_Ydata;

    double x = x_data[min_index];
    double y = y_data[min_index];

    // Find the X and Y min/max/ave values of the selection
    int x_min_index = min_index, x_max_index = min_index;
    int y_min_index = min_index, y_max_index = min_index;
    double x_min_x = x, x_max_x = x;
    double y_min_y = y, y_max_y = y;
    double ave_x = 0, ave_y = 0;
    int i, j, sel_count = rangeSel.GetCount(), sel_point_count = 0;

    for (i=0; i<sel_count; i++)
    {
        wxRangeInt r = rangeSel.GetRange(i);
        wxCHECK_MSG((r.m_min >= 0) && (r.m_min < (int)M_PLOTDATA->m_count) &&
                    (r.m_max >= 0) && (r.m_max < (int)M_PLOTDATA->m_count), 0,
                    wxT("Invalid range selection index in data curve"));

        for (j=r.m_min; j<=r.m_max; j++) // yes we duplicate first point
        {
            sel_point_count++;
            x = x_data[j];
            y = y_data[j];

            if (x < x_min_x) { x_min_x = x; x_min_index = j; }
            if (x > x_max_x) { x_max_x = x; x_max_index = j; }
            if (y < y_min_y) { y_min_y = y; y_min_index = j; }
            if (y > y_max_y) { y_max_y = y; y_max_index = j; }

            ave_x += x;
            ave_y += y;
        }
    }

    ave_x /= double(sel_point_count);
    ave_y /= double(sel_point_count);

    if (ave_)   *ave_   = wxPoint2DDouble(ave_x, ave_y);
    if (minXY_) *minXY_ = wxPoint2DDouble(x_min_x, y_min_y);
    if (maxXY_) *maxXY_ = wxPoint2DDouble(x_max_x, y_max_y);
    if (x_min_index_) *x_min_index_ = x_min_index;
    if (x_max_index_) *x_max_index_ = x_max_index;
    if (y_min_index_) *y_min_index_ = y_min_index;
    if (y_max_index_) *y_max_index_ = y_max_index;

    return sel_point_count;
}

wxArrayInt wxPlotData::GetCrossing( double y_value ) const
{
    wxArrayInt points;
    wxCHECK_MSG( Ok(), points, wxT("Invalid wxPlotData") );

    int i;
    double *y_data = M_PLOTDATA->m_Ydata;
    double y, last_y = M_PLOTDATA->m_Ydata[0];

    for (i=1; i<M_PLOTDATA->m_count; i++)
    {
        y = y_data[i];

        if (((last_y >= y_value) && (y <= y_value)) ||
            ((last_y <= y_value) && (y >= y_value)))
        {
            if (fabs(last_y - y_value) < fabs(y - y_value))
                points.Add(i-1);
            else
                points.Add(i);
        }
        last_y = y;
    }

    return points;
}

int wxPlotData::GetMinYIndex(int start_index, int end_index) const
{
    wxCHECK_MSG( Ok(), 0, wxT("Invalid wxPlotData") );
    const int count = M_PLOTDATA->m_count;
    if (end_index < 0) end_index = count - 1;
    CHECK_START_END_INDEX_MSG(start_index, end_index, count, 0);

    double *y_data = &M_PLOTDATA->m_Ydata[start_index];
    double min_y = *y_data;
    int min_y_index = start_index;

    for (int i=start_index+1; i<end_index; i++)
    {
        if (*y_data < min_y)
        {
            min_y = *y_data;
            min_y_index = i;
        }

        y_data++;
    }
    return min_y_index;
}

int wxPlotData::GetMaxYIndex(int start_index, int end_index) const
{
    wxCHECK_MSG( Ok(), 0, wxT("Invalid wxPlotData") );
    const int count = M_PLOTDATA->m_count;
    if (end_index < 0) end_index = count - 1;
    CHECK_START_END_INDEX_MSG(start_index, end_index, count, 0);

    double *y_data = &M_PLOTDATA->m_Ydata[start_index];
    double max_y = *y_data;
    int max_y_index = start_index;

    for (int i=start_index+1; i<end_index; i++)
    {
        if (*y_data > max_y)
        {
            max_y = *y_data;
            max_y_index = i;
        }

        y_data++;
    }
    return max_y_index;
}

//----------------------------------------------------------------------------
// Data processing functions
//----------------------------------------------------------------------------

void wxPlotData::OffsetX( double offset, int start_index, int end_index )
{
    wxCHECK_RET( Ok(), wxT("Invalid wxPlotData") );
    const int count = M_PLOTDATA->m_count;
    if (end_index < 0) end_index = count - 1;
    CHECK_START_END_INDEX_RET(start_index, end_index, count);

    register int i;
    double *x = M_PLOTDATA->m_Xdata + start_index;

    for (i = start_index; i <= end_index; i++)
        *x++ += offset;

    CalcBoundingRect();
}

void wxPlotData::OffsetY( double offset, int start_index, int end_index )
{
    wxCHECK_RET( Ok(), wxT("Invalid wxPlotData") );
    const int count = M_PLOTDATA->m_count;
    if (end_index < 0) end_index = count - 1;
    CHECK_START_END_INDEX_RET(start_index, end_index, count);

    register int i;
    double *y = M_PLOTDATA->m_Ydata + start_index;

    for (i = start_index; i <= end_index; i++)
        *y++ += offset;

    CalcBoundingRect();
}

void wxPlotData::OffsetXY( double offsetX, double offsetY, int start_index, int end_index )
{
    wxCHECK_RET( Ok(), wxT("Invalid wxPlotData") );
    const int count = M_PLOTDATA->m_count;
    if (end_index < 0) end_index = count - 1;
    CHECK_START_END_INDEX_RET(start_index, end_index, count);

    register int i;
    double *x = M_PLOTDATA->m_Xdata + start_index;
    double *y = M_PLOTDATA->m_Ydata + start_index;

    for (i = start_index; i <= end_index; i++)
    {
        *x++ += offsetX;
        *y++ += offsetY;
    }

    CalcBoundingRect();
}

void wxPlotData::ScaleX( double scale, double offset, int start_index, int end_index )
{
    wxCHECK_RET( Ok(), wxT("Invalid wxPlotData") );
    const int count = M_PLOTDATA->m_count;
    if (end_index < 0) end_index = count - 1;
    CHECK_START_END_INDEX_RET(start_index, end_index, count);

    register int i;
    double *x = M_PLOTDATA->m_Xdata + start_index;

    for (i = start_index; i <= end_index; i++)
    {
        *x = ((*x) - offset)*scale + offset;
        x++;
    }

    CalcBoundingRect();
}

void wxPlotData::ScaleY( double scale, double offset, int start_index, int end_index )
{
    wxCHECK_RET( Ok(), wxT("Invalid wxPlotData") );
    const int count = M_PLOTDATA->m_count;
    if (end_index < 0) end_index = count - 1;
    CHECK_START_END_INDEX_RET(start_index, end_index, count);

    register int i;
    double *y = M_PLOTDATA->m_Ydata + start_index;

    for (i = start_index; i <= end_index; i++)
    {
        *y = ((*y) - offset)*scale + offset;
        y++;
    }

    CalcBoundingRect();
}

void wxPlotData::ScaleXY(double scaleX, double scaleY, double offsetX, double offsetY,
                         int start_index, int end_index )
{
    wxCHECK_RET( Ok(), wxT("Invalid wxPlotData") );
    const int count = M_PLOTDATA->m_count;
    if (end_index < 0) end_index = count - 1;
    CHECK_START_END_INDEX_RET(start_index, end_index, count);

    double *x = M_PLOTDATA->m_Xdata + start_index;
    double *y = M_PLOTDATA->m_Ydata + start_index;

    for (int i = start_index; i < end_index; i++)
    {
        *x = ((*x) - offsetX)*scaleX + offsetX;
        x++;
        *y = ((*y) - offsetY)*scaleY + offsetY;
        y++;
    }

    CalcBoundingRect();
}

void wxPlotData::PowerX( double power, int start_index, int end_index )
{
    wxCHECK_RET( Ok(), wxT("Invalid wxPlotData") );
    const int count = M_PLOTDATA->m_count;
    if (end_index < 0) end_index = count - 1;
    CHECK_START_END_INDEX_RET(start_index, end_index, count);

    register int i;
    double *x = M_PLOTDATA->m_Xdata + start_index;
    for (i = start_index; i <= end_index; i++, x++)
        *x = pow(*x, power);

    CalcBoundingRect();
}

void wxPlotData::PowerY( double power, int start_index, int end_index )
{
    wxCHECK_RET( Ok(), wxT("Invalid wxPlotData") );
    const int count = M_PLOTDATA->m_count;
    if (end_index < 0) end_index = count - 1;
    CHECK_START_END_INDEX_RET(start_index, end_index, count);

    register int i;
    double *y = M_PLOTDATA->m_Ydata + start_index;
    for (i = start_index; i <= end_index; i++, y++)
        *y = pow(*y, power);

    CalcBoundingRect();
}

void wxPlotData::PowerXY( double powerX, double powerY, int start_index, int end_index )
{
    wxCHECK_RET( Ok(), wxT("Invalid wxPlotData") );
    const int count = M_PLOTDATA->m_count;
    if (end_index < 0) end_index = count - 1;
    CHECK_START_END_INDEX_RET(start_index, end_index, count);

    register int i;
    double *x = M_PLOTDATA->m_Xdata + start_index;
    double *y = M_PLOTDATA->m_Ydata + start_index;
    for (i = start_index; i <= end_index; i++, x++, y++)
    {
        *x = pow(*x, powerX);
        *y = pow(*y, powerY);
    }

    CalcBoundingRect();
}

wxPlotData wxPlotData::Resample( double start_x, double dx, int points ) const
{
    wxPlotData dst;
    wxCHECK_MSG(Ok(), dst, wxT("Invalid wxPlotData"));
    wxCHECK_MSG(points > 0, dst, wxT("invalid number of points"));

    if (!dst.Create(points, false)) return dst;

    double x = start_x;

    wxPlotData thisData = *this;

    for (int i=0; i< points; i++, x += dx)
    {
        dst.SetPoint(i, wxPoint2DDouble(x, thisData.GetY(x)));
    }

    dst.CalcBoundingRect();
    return dst;
}

wxPlotData wxPlotData::Resample( const wxPlotData &source ) const
{
    wxPlotData dst;
    wxCHECK_MSG( Ok(), dst, wxT("Invalid wxPlotData") );

    int src_index_min = source.GetIndexFromX(wxMax(GetBoundingRect().m_x, source.GetBoundingRect().m_x));
    int src_index_max = source.GetIndexFromX(wxMin(GetBoundingRect().GetRight(), source.GetBoundingRect().GetRight()));

    int index_min = GetIndexFromX(wxMax(GetBoundingRect().m_x, source.GetBoundingRect().m_x));
    //int index_max = GetIndexFromX(wxMin(GetBoundingRect().GetRight(), source.GetBoundingRect().GetRight()));

    int count = src_index_max - src_index_min;
    if (count <= 0)
        return dst;

    if (!dst.Create(src_index_max - src_index_min, false)) return dst;

    double *dst_x_data = dst.GetXData();
    double *dst_y_data = dst.GetYData();

    double x, s_x, y;

    for (int i=0; i<count; i++)
    {
        s_x = source.GetXValue(i+src_index_min);

        x = GetXValue(i+index_min);

        if (s_x == x)
        {
            y = GetYValue(i+index_min);
        }
        else
        {
            int index1 = i+index_min;
            int index2 = i+index_min;

            if (x > s_x)
            {
                if (i+index_min > 0)
                    index1--;
            }
            else
            {
                if (i+index_min > 0)
                    index2++;
            }

            y = LinearInterpolateY( GetXValue(index1), GetYValue(index1),
                                    GetXValue(index2), GetYValue(index2), x );

        }

        *dst_x_data++ = x;
        *dst_y_data++ = y;
    }

    dst.CalcBoundingRect();
    return dst;
}

//----------------------------------------------------------------------------
// Data manipulation
//----------------------------------------------------------------------------
wxPlotData wxPlotData::Modify( const wxPlotFunction &func_, FuncModify_Type type ) const
{
    wxPlotData dst;
    wxCHECK_MSG( Ok() && func_.Ok(), dst, wxT("Invalid wxPlotData") );

    int i, count = M_PLOTDATA->m_count;

    if (!dst.Create(count)) return dst;

    double *x_data = M_PLOTDATA->m_Xdata;
    double *y_data = M_PLOTDATA->m_Ydata;
    double *dst_x_data = dst.GetXData();
    double *dst_y_data = dst.GetYData();

    if (M_PLOTDATA->m_Yidata)
    {
        double *yi = (double*)malloc(count*sizeof(double));
        if (!yi)
        {
            dst.Destroy();
            return dst;
        }
        dst.SetYiData(yi);
    }

    wxPlotFunction func(func_);

    switch (type)
    {
        case add_x :
        {
            memcpy(dst_y_data, y_data, count*sizeof(double));
            if (M_PLOTDATA->m_Yidata)
                memcpy(dst.GetYiData(), M_PLOTDATA->m_Yidata, count*sizeof(double));

            for (i=0; i<count; i++)
            {
                *dst_x_data++ = (*x_data) + func.GetY(*x_data);
                x_data++;
            }
            break;
        }
        case add_y :
        {
            if (M_PLOTDATA->m_Yidata)
                memcpy(dst.GetYiData(), M_PLOTDATA->m_Yidata, count*sizeof(double));

            for (i=0; i<count; i++)
            {
                *dst_y_data++ = (*y_data++) + func.GetY(*x_data);
                *dst_x_data++ = *x_data++;
            }
            break;
        }
        case mult_x :
        {
            memcpy(dst_y_data, y_data, count*sizeof(double));
            if (M_PLOTDATA->m_Yidata)
                memcpy(dst.GetYiData(), M_PLOTDATA->m_Yidata, count*sizeof(double));

            for (i=0; i<count; i++)
            {
                *dst_x_data++ = (*x_data) * func.GetY(*x_data);
                x_data++;
            }
            break;
        }
        case mult_y :
        {
            if (M_PLOTDATA->m_Yidata)
                memcpy(dst.GetYiData(), M_PLOTDATA->m_Yidata, count*sizeof(double));

            for (i=0; i<count; i++)
            {
                *dst_y_data++ = (*y_data++) * func.GetY(*x_data);
                *dst_x_data++ = *x_data++;
            }
            break;
        }
        case add_yi :
        {
            memcpy(dst_y_data, y_data, count*sizeof(double));

            double *yi_data = M_PLOTDATA->m_Yidata;
            double *dst_yi_data = dst.GetYiData();

            for (i=0; i<count; i++)
            {
                *dst_yi_data++ = (*yi_data++) + func.GetY(*x_data);
                *dst_x_data++ = *x_data++;
            }
            break;
        }
        case mult_yi :
        {
            memcpy(dst_y_data, y_data, count*sizeof(double));

            double *yi_data = M_PLOTDATA->m_Yidata;
            double *dst_yi_data = dst.GetYiData();

            for (i=0; i<count; i++)
            {
                *dst_yi_data++ = (*yi_data++) * func.GetY(*x_data);
                *dst_x_data++ = *x_data++;
            }
            break;
        }
        default :
        {
            dst.Destroy();
            return dst;
        }
    }

    dst.CalcBoundingRect();
    return dst;
}

wxPlotData wxPlotData::Add( const wxPlotData &curve2, double factor1, double factor2 ) const
{
    wxPlotData dst;
    wxCHECK_MSG( Ok() && curve2.Ok(), dst, wxT("Invalid wxPlotData") );

    double curve2_x_min = curve2.GetBoundingRect().GetLeft();
    double curve2_x_max = curve2.GetBoundingRect().GetRight();
    int curve2_count = curve2.GetCount();

    int count = M_PLOTDATA->m_count;
    double *x_data = M_PLOTDATA->m_Xdata;
    double *y_data = M_PLOTDATA->m_Ydata;

    if (!dst.Create(count)) return dst;

    double *dst_x_data = dst.GetXData();
    double *dst_y_data = dst.GetYData();

    int i, index1, index2;

    for (i=0; i<count; i++)
    {
        *dst_x_data = *x_data;

        if ((*x_data > curve2_x_min) && (*x_data < curve2_x_max))
        {
            index1 = curve2.GetIndexFromX(*x_data);
            if ( curve2.GetXValue(index1) == *x_data )
            {
                *dst_y_data = factor1*(*y_data) + factor2*curve2.GetYValue(index1);
            }
            else // need to interpolate to find data's y value at this x
            {
                if (*x_data < curve2.GetXValue(index1))
                {
                    index2 = index1;
                    index1--;
                }
                else //if (*x_data < data.GetX(index1))
                {
                    index2 = index1 + 1;
                }
                if ((index1 >= 0) && (index1 < curve2_count) && (index2 >= 0) && (index2 < curve2_count) )
                {
                    *dst_y_data = factor1*(*y_data) + factor2*LinearInterpolateY( curve2.GetXValue(index1), curve2.GetYValue(index1),
                                             curve2.GetXValue(index2), curve2.GetYValue(index2),
                                             *x_data );
                }
                else
                {
                    *dst_y_data = *x_data;
                }
            }
        }
        x_data++;
        y_data++;
        dst_x_data++;
        dst_y_data++;
    }

    dst.CalcBoundingRect();
    return dst;
}

//----------------------------------------------------------------------------
// RunAverage
//----------------------------------------------------------------------------
wxPlotData wxPlotData::RunAverage( int points, int start_index, int count ) const
{
    wxPlotData dst;

    wxCHECK_MSG( Ok(), dst, wxT("Invalid wxPlotData") );
    int data_count = M_PLOTDATA->m_count;
    wxCHECK_MSG((start_index < data_count), dst, wxT("Invalid runaverage range") );

    int i;
    int half_width = points/2;
    int width = half_width * 2 + 1;
    double dpoints = width;
    int end_index = count <= 0 ? data_count : start_index + count;
    int run_start = wxMax(start_index, half_width);
    int run_end = wxMin(end_index, data_count - half_width);

    //wxCHECK_MSG((points > 2) && (start_index+count<data_count), dst, wxT("invalid data indexes"));

    if (!dst.Copy(*this)) return dst;

    if (run_start >= run_end) return dst;

    //double *src = M_PLOTDATA->m_Ydata;
    //double *dest = dst.GetYData();

    double run_sum = 0.0;

    for (i=run_start-half_width; i<=run_start+half_width; i++)
        run_sum += GetYValue(i); //src[i];

    dst.SetYValue(run_start, run_sum/dpoints); //dest[half_width] = runsum / dpoints;

    // fake the initial runaverage (ideally this should be thrown away)
    double run_sum_init = run_sum;
    for (i=run_start-1; i>=start_index; i--)
    {
        run_sum_init += GetYValue(i) - GetYValue(i+half_width);
        dst.SetYValue(i, run_sum_init/dpoints);
    }

    for (i=run_start+1; i<run_end; i++)
    {
        run_sum += GetYValue(i+half_width) - GetYValue(i-half_width-1); //src[i+half_width] - src[i-half_width-1];
        dst.SetYValue(i, run_sum/dpoints); //dest[i] = run_sum / dpoints;
    }

    // fake the final runaverage (ideally this should be thrown away)
    for (i=run_end; i<end_index; i++)
    {
        run_sum += GetYValue(i) - GetYValue(i-half_width-1);
        dst.SetYValue(i, run_sum/dpoints);
    }


/*
    int i;
    int half_width = points/2;
    int width = half_width * 2 + 1;
    double dpoints = width;
    int runend = data_count - half_width;
    double runsum = 0.0;

    // THIS METHOD DOES AVE THE END, i DON'T THINK I REALLY CARE

    // setup the runave
    for (i=0; i<width; i++) runsum += src[i];

    // these first points < half_width are "undefined" but we'll do it anyway
    double init_runsum = runsum/dpoints;
    for (i=half_width-1; i>=0; i--)
    {
        init_runsum += src[i];
        dest[i] = init_runsum / double(half_width - i + 1);
    }

    // middle "good" runaveraged points
    dest[half_width] = runsum / dpoints;

    for (i=half_width+1; i<runend; i++)
    {
        runsum += src[i+half_width] - src[i-half_width-1];
        dest[i] = runsum / dpoints;
    }

    // end points, do as in the beginning
    runsum /= dpoints;
    for (i=runend; i<M_PLOTDATA->m_count; i++)
    {
        runsum += src[i];
        dest[i] = runsum / double(half_width - (M_PLOTDATA->m_count - i) + 2);
    }

    memcpy(dst.GetXData(), M_PLOTDATA->m_Xdata, M_PLOTDATA->m_count*sizeof(double));
*/

    dst.CalcBoundingRect();

    return dst;
}

//----------------------------------------------------------------------------
// Abs
//----------------------------------------------------------------------------

wxPlotData wxPlotData::Abs() const
{
    wxPlotData dst;

    wxCHECK_MSG(Ok(), dst, wxT("invalid plotdata"));
    int points = M_PLOTDATA->m_count;

    if (!dst.Create(points, false)) return dst;

    double *y_data = M_PLOTDATA->m_Ydata;

    double *dst_y_data = dst.GetYData();

    memcpy(dst.GetXData(), GetXData(), points*sizeof(double));

    for (int i=0; i<points; i++)
    {
        *dst_y_data++ = fabs(*y_data++);
    }

    dst.CalcBoundingRect();
    return dst;
}

//----------------------------------------------------------------------------
// Linearize
//----------------------------------------------------------------------------

wxPlotData wxPlotData::LinearizeY(int start_index, int count) const
{
    wxPlotData dst;

    wxCHECK_MSG(Ok(), dst, wxT("invalid plotdata"));
    int points = M_PLOTDATA->m_count;
    if (count <= 0) count = points - start_index;
    wxCHECK_MSG((start_index + count <= points), dst, wxT("invalid index"));

    double x0 = M_PLOTDATA->m_Xdata[start_index];
    double y0 = M_PLOTDATA->m_Ydata[start_index];
    double x1 = M_PLOTDATA->m_Xdata[start_index+count-1];
    double y1 = M_PLOTDATA->m_Ydata[start_index+count-1];

    wxCHECK_MSG(wxFinite(x0)&&wxFinite(y0)&&wxFinite(x1)&&wxFinite(y1), dst, wxT("values are NaN"));

    if (!dst.Copy(*this, true)) return dst;

    double x, y, m = (y1-y0)/count;

    // equally spaced y values
    if (x1 == x0)
    {
        y = y0;
        for (int i=start_index+1; i<start_index+count-1; i++)
        {
            y += m;
            if (wxFinite(y))
                dst.SetYValue(i, y);
        }
        dst.CalcBoundingRect();
        return dst;
    }

    m = (y1 - y0) / (x1 - x0);

    for (int i=start_index+1; i<start_index+count-1; i++)
    {
        x = M_PLOTDATA->m_Xdata[i];
        if (wxFinite(x))
        {
            y = (m*x + (y0 - m*x0));
            if (wxFinite(y))
                dst.SetYValue(i, y);
        }
    }

    dst.CalcBoundingRect();
    return dst;
}

// FIXME - this is not strictly correct should be xnew_n = (x_(n+1)+x_n)/2, do I care?
wxPlotData wxPlotData::Derivitive() const
{
    wxCHECK_MSG( Ok(), wxPlotData(), wxT("Invalid wxPlotData") );

    int count = M_PLOTDATA->m_count;

    wxPlotData dst(count, false);

    double *y_data = M_PLOTDATA->m_Ydata;
    double *dst_y_data = dst.GetYData();

    memcpy(dst.GetXData(), GetXData(), count*sizeof(double));

    *dst_y_data++ = 0;

    for (int i=1; i<count-1; i++)
    {
        y_data++;

        if (wxFinite(*(y_data)) && wxFinite(*(y_data-1)))
            *dst_y_data++ = (*(y_data)) - (*(y_data-1));
        else
            *dst_y_data++ = 0;
    }

    *dst_y_data = 0;

    dst.CalcBoundingRect();
    return dst;
}

// SumOverN(sqrt(E(yn-<y>)^2))/N
double wxPlotData::Variance(int start_index, int count) const
{
    wxCHECK_MSG( Ok(), 0, wxT("Invalid wxPlotData") );
    int points = M_PLOTDATA->m_count;
    if (count <= 0) count = points - start_index;
    wxCHECK_MSG((start_index >= 0) && (start_index + count <= points), 0, wxT("invalid index"));

    double *y_data = GetYData() + start_index;
    double sum = 0;
    double ave = GetAverage(start_index, count);

    for (register int i=0; i<count; i++)
    {
        sum += ((*y_data) - ave)*((*y_data) - ave);
        y_data++;
    }

    return sqrt(sum/count);
}

wxPlotData wxPlotData::VarianceCurve(int points) const
{
    wxPlotData dst;
    wxCHECK_MSG( Ok(), dst, wxT("Invalid wxPlotData") );

    int data_count = M_PLOTDATA->m_count;

    int i;
    int half_width = points/2;
    int width = half_width * 2 + 1;
    int end_index = data_count; //count <= 0 ? data_count : start_index + count;
    int run_start = wxMax(0, half_width);
    int run_end = wxMin(end_index, data_count - half_width);

    wxCHECK_MSG((data_count > half_width), dst, wxT("invalid data indexes"));

    if (!dst.Create(data_count, false)) return dst;

    memcpy(dst.GetXData(), GetXData(), data_count*sizeof(double));
    memset(dst.GetYData(), 0, half_width*sizeof(double));
    memset(dst.GetYData()+data_count-half_width, 0, half_width*sizeof(double));

    if (run_start >= run_end) return dst;

    for (i=run_start; i<run_end; i++)
    {
        dst.SetYValue(i, Variance(i-half_width, width));
    }

    dst.CalcBoundingRect();
    return dst;
}

double wxPlotData::Deviation( const wxPlotData &data, int min, int max ) const
{
    wxCHECK_MSG( Ok() && data.Ok(), 0.0, wxT("Invalid wxPlotData") );
    const int count = GetCount();
    wxCHECK_MSG((min >= 0)&&(min<count), 0.0, wxT("invalid min index"));
    if (max < 0) max = count;
    wxCHECK_MSG(max > min, 0.0, wxT("invalid index range"));

    double data_x_min = data.GetBoundingRect().GetLeft();
    double data_x_max = data.GetBoundingRect().GetRight();
    const int data_count = data.GetCount();

    double *x_data = M_PLOTDATA->m_Xdata + min;
    double *y_data = M_PLOTDATA->m_Ydata + min;

    double x, y, dev = 0.0;

    int index1=0, index2=0;
    int points = 0;
    register int i;

    for (i=min; i<max; i++)
    {
        x = GetXValue(i);
        y = GetYValue(i);

        if ( (x >= data_x_min) && (x <= data_x_max) )
        {
            index1 = data.GetIndexFromX(x);
            if ( data.GetXValue(index1) == x )
            {
                double yd = (y - data.GetYValue(index1));
                dev += yd*yd;
                points++;
            }
            else // need to interpolate to find data's y value at this x
            {
                if (x < data.GetXValue(index1))
                {
                    index2 = index1;
                    index1--;
                }
                else //if (*x_data < data.GetX(index1))
                {
                    index2 = index1 + 1;
                }

                if ((index1 >= 0) && (index2 < data_count) )
                {
                    double yd = y - LinearInterpolateY( data.GetXValue(index1), data.GetYValue(index1),
                                                        data.GetXValue(index2), data.GetYValue(index2), x );
                    dev += yd*yd;
                    points++;
                }
            }
        }
        x_data++;
        y_data++;
    }

    dev = sqrt(dev);

    if (points > 0)
        return dev/double(points);

    return -1;
}

double wxPlotData::CrossCorrelation( const wxPlotData &other, int runave, int min, int max ) const
{
    //http://astronomy.swin.edu.au/~pbourke/analysis/correlate/ for discussion of crosscorrelation

    wxCHECK_MSG( Ok() && other.Ok(), 0.0, wxT("Invalid wxPlotData") );
    const int count = GetCount();
    wxCHECK_MSG((min >= 0)&&(min<count), 0.0, wxT("invalid min index"));
    if (max < 0) max = count;
    wxCHECK_MSG(max > min, 0.0, wxT("invalid index range"));

    double cc = 0.0;

    wxPlotData smooth(RunAverage(runave));
    wxPlotData other_smooth(other.RunAverage(runave));

    smooth.CalcBoundingRect();
    other_smooth.CalcBoundingRect();

    double other_x_min = other.GetBoundingRect().GetLeft();
    double other_x_max = other.GetBoundingRect().GetRight();
    int other_count = other.GetCount();

    register int i;

    double *x_data = M_PLOTDATA->m_Xdata;
    double *y_data = M_PLOTDATA->m_Ydata;
    double *smooth_y_data = smooth.GetYData();

    double x;

    int index1, index2;

    for (i=min; i<max; i++)
    {
        x = GetXValue(i);

        if ( (x >= other_x_min) && (x <= other_x_max) )
        {
            index1 = other.GetIndexFromX(x);
            if ( other.GetXValue(index1) == x )
            {
                //*y_data -= data.GetY(index1);
                //cc += (*y_data - data.GetY(index1))*(*y_data - data.GetY(index1));
                cc += (GetYValue(i) - smooth.GetYValue(i))*(other.GetYValue(index1)-other_smooth.GetYValue(index1));
            }
            else // need to interpolate to find data's y value at this x
            {
                if (x < other.GetXValue(index1))
                {
                    index2 = index1;
                    index1--;
                }
                else //if (*x_data < data.GetX(index1))
                {
                    index2 = index1 + 1;
                }
                if ((index1 >= 0) && (index1 < other_count) && (index2 >= 0) && (index2 < other_count) )
                {
                    double y = LinearInterpolateY( other.GetXValue(index1), other.GetYValue(index1),
                                             other.GetXValue(index2), other.GetYValue(index2),
                                             x );

                    double ys = LinearInterpolateY( other_smooth.GetXValue(index1), other_smooth.GetYValue(index1),
                                             other_smooth.GetXValue(index2), other_smooth.GetYValue(index2),
                                             x );

                    cc += (GetYValue(i) - smooth.GetYValue(i))*(y-ys);
                }
            }
        }

        x_data++;
        y_data++;
        smooth_y_data++;
    }

    //cc = sqrt(cc);

    return cc;
}

double wxPlotData::MinShiftX( const wxPlotData &other ) const
{
    wxCHECK_MSG(Ok() && other.Ok(), 0.0, wxT("invalid plotcurve"));

    wxRect2DDouble boundRect = GetBoundingRect();
    wxRect2DDouble other_boundRect = other.GetBoundingRect();

    double start_shift = (boundRect.m_x - other_boundRect.m_x) - other_boundRect.m_width/2.0;
    double end_shift = (boundRect.GetRight() - other_boundRect.m_x) - other_boundRect.m_width/2.0;
    double min_dev = 0, min_shift = start_shift;
    double step = fabs(M_PLOTDATA->m_Xdata[1] - M_PLOTDATA->m_Xdata[0]);

    wxPlotData shifted;
    shifted.Copy(other);
    shifted.OffsetX(min_shift);

    for (double x=start_shift; x<end_shift; x+=step)
    {
        double dev = Deviation(shifted);

        if (dev >= 0)
        {
            if (x == start_shift)
                min_dev = dev;
            else if (dev < min_dev)
            {
                min_dev = dev;
                min_shift = x;
            }
        }

        shifted.OffsetX(step);
    }

    return min_shift;
}


//----------------------------------------------------------------------------
// Fast Fourier Transform
//----------------------------------------------------------------------------
wxPlotData wxPlotData::FFT( bool forward )
{
    wxCHECK_MSG( Ok(), wxPlotData(), wxT("Invalid wxPlotData") );
    wxCHECK_MSG( GetCount() < 2, wxPlotData(), wxT("Invalid wxPlotData size") );

    // Find the next highest 2^n power
    int samples    = 1;
    int orig_count = M_PLOTDATA->m_count;
    while (samples < orig_count) samples *= 2;

    wxPlotData source;
    source.Copy(*this);

    // resize the curve to the 2^n sample size required by fft_double
    if (samples > orig_count)
    {
        double dx = M_PLOTDATA->m_Xdata[1] - M_PLOTDATA->m_Xdata[0];
        source.Resize(samples, dx, M_PLOTDATA->m_Ydata[orig_count - 1]);
    }

    wxPlotData trans(samples);
    trans.SetYiData( (double*)malloc(samples*sizeof(double)) );

    if (fft_double( samples, (forward ? 0 : 1),
                    source.GetYData(), source.GetYiData(),
                    trans.GetYData(),  trans.GetYiData() ) == 0)
        return wxPlotData();

    if (forward)
    {
        double *trans_xData = trans.GetXData();
        trans_xData[0] = 0.0;
        double timestep = (M_PLOTDATA->m_Xdata[1] - M_PLOTDATA->m_Xdata[0])*samples;
        for (int i=0; i<samples; i++)
        {
            trans_xData[i] = double(i)/timestep;
        }
/*      FIXME - what to do with the other half of the FFT transform ?
                if you throw it away, should you create other half automatically

        for (int i=1; i<=samples/2; i++)
        {
            trans_xData[i] = double(i)/timestep;
            trans_xData[samples-i] = double(i)/timestep;
        }
*/
        //trans = trans.Remove(samples/2, samples/2);
    }
    else
    {
        double *trans_xData  = trans.GetXData();
        double timestep = 1.0/((M_PLOTDATA->m_Xdata[1] - M_PLOTDATA->m_Xdata[0])*double(samples));
        for (int i=0; i<samples; i++)
        {
            trans_xData[i] = timestep*double(i);
        }
    }

    trans.CalcBoundingRect();
    return trans;
}

wxPlotData wxPlotData::PowerSpectrum()
{
    wxCHECK_MSG( Ok(), wxPlotData(), wxT("Invalid wxPlotData, wxPlotData::RunAverage()") );

    wxPlotData power(FFT(true));
    wxCHECK_MSG(power.Ok(), wxPlotData(), wxT("Invalid FFT data curve"));

    int i, count = power.GetCount();
    for (i=0; i<count; i++)
    {
        power.GetYData()[i] = pow(power.GetYData()[i]*power.GetYData()[i] +
                                  power.GetYiData()[i]*power.GetYiData()[i], 0.5)/double(count);
    }

    power.CalcBoundingRect();

    return power;
}

wxString wxPlotData::FFTHiPassFilterFormat( double hi, wxPlotData::FFTFilter_Type filter, double n )
{
    switch (filter)
    {
        case FilterStep        : return wxString::Format(wxT("x>%lg"), hi);
        case FilterButterworth : return wxString::Format(wxT("1 - 1/(1+(x/%lg)^(2*%lg))"), hi, n);
        case FilterGaussian    : return wxString::Format(wxT("exp(-(%lg^2)/(2*x*x))"), hi);
        case FilterFermi       : return wxString::Format(wxT("1/(1+exp((%lg-x)/%lg))"), hi, n);
        default : break;
    }
    return wxEmptyString;
}

wxString wxPlotData::FFTLoPassFilterFormat( double lo, wxPlotData::FFTFilter_Type filter, double n )
{
    switch (filter)
    {
        case FilterStep        : return wxString::Format(wxT("x<%lg"), lo);
        case FilterButterworth : return wxString::Format(wxT("1/(1+(x/%lg)^(2*%lg))"), lo, n);
        case FilterGaussian    : return wxString::Format(wxT("1 - exp(-(%lg^2)/(2*x*x))"), lo);
        case FilterFermi       : return wxString::Format(wxT("1/(1+exp((%lg-x)/(-%lg)))"), lo, n);
        default : break;
    }
    return wxEmptyString;
}

wxString wxPlotData::FFTNotchFilterFormat( double lo, double hi, wxPlotData::FFTFilter_Type filter, double n )
{
    switch (filter)
    {
        case FilterStep        : return wxString::Format(wxT("(x<%lg)|(x>%lg)"), lo, hi);
        case FilterButterworth : return wxString::Format(wxT("1/(1+(x/%lg)^(2*%lg)) + (1-1/(1+(x/%lg)^(2*%lg)))"), lo, n, hi, n);
        case FilterGaussian    : return wxString::Format(wxT("2-(exp(-(%lg^2)/(2*x*x))+(1-exp(-(%lg^2)/(2*x*x))))"), lo, hi);
        case FilterFermi       : return wxString::Format(wxT("1/(1+exp((%lg-x)/(-%lg))) + 1/(1+exp((%lg-x)/(%lg)))"), lo, n, hi, n);
        default : break;
    }
    return wxEmptyString;
}

wxString wxPlotData::FFTBandPassFilterFormat( double lo, double hi, wxPlotData::FFTFilter_Type filter, double n )
{
    switch (filter)
    {
        case FilterStep        : return wxString::Format(wxT("(x>%lg)&(x<%lg)"), lo, hi);
        case FilterButterworth : return wxString::Format(wxT("1 - (1/(1+(x/%lg)^(2*%lg))+(1-1/(1+(x/%lg)^(2*%lg))))"), lo, n, hi, n);
        case FilterGaussian    : return wxString::Format(wxT("exp(-(%lg^2)/(2*x*x)) - exp(-(%lg^2)/(2*x*x))"), lo, hi);
        case FilterFermi       : return wxString::Format(wxT("1 - 1/(1+exp((%lg-x)/(-%lg))) - 1/(1+exp((%lg-x)/(%lg)))"), lo, n, hi, n);
        default : break;
    }
    return wxEmptyString;
}

// butterworth lo=1./(1.+ (x/fc)**(2*n)), hi=1-1./(1.+ (x/fc)**(2*n))
// gaussian lo = 1.0-exp(-fc**2/(2.*x**2)), hi = exp(-fc**2/(2.*x**2))
// fermi lo= 1/(1+exp[(fc-x)/n]) , hi =  1/(1+exp((x-fc)/(-n)))

wxPlotData wxPlotData::FFTHiPassFilter( double fc, wxPlotData::FFTFilter_Type filter, double n )
{
    wxCHECK_MSG( Ok() && (n>0), wxPlotData(), wxT("Invalid wxPlotData") );

    wxPlotData xform( FFT(true) );

    int i, count = xform.GetCount();
    double f, x;
    double *ydata = xform.GetYData(),
           *yidata = xform.GetYiData();

    for (i=0; i<count; i++)
    {
        x = xform.GetXData()[i];

        if ((filter == FilterStep) && (x < fc))
        {
            *ydata = 0;
            *yidata = 0;
        }
        else
        {
            if (filter == FilterButterworth)
                f = 1.0 - 1.0/(1.0 + pow(x/fc, 2.0*n));
            else if (filter == FilterGaussian)
                f = exp(-fc*fc/(2.0*x*x));
            else // (filter == FilterFermi)
                f = 1.0/(1.0+exp((fc-x)/(n)));

            *ydata *= f;
            *yidata *= f;
        }

        ydata++;
        yidata++;
    }

    wxPlotData dest( xform.FFT(false) );
    dest.OffsetX(M_PLOTDATA->m_Xdata[0]);
    dest.CalcBoundingRect();
    return dest;
}

wxPlotData wxPlotData::FFTLoPassFilter( double fc, wxPlotData::FFTFilter_Type filter, double n )
{
    wxCHECK_MSG( Ok() && (n>0), wxPlotData(), wxT("Invalid wxPlotData") );

    wxPlotData xform( FFT(true) );

    int i, count = xform.GetCount();
    double f, x;
    double *ydata = xform.GetYData(),
           *yidata = xform.GetYiData();

    for (i=0; i<count; i++)
    {
        x = xform.GetXData()[i];

        if ((filter == FilterStep) && (x > fc))
        {
            *ydata = 0;
            *yidata = 0;
        }
        else
        {
            if (filter == FilterButterworth)
                f = 1.0/(1.0 + pow(x/fc, 2.0*n));
            else if (filter == FilterGaussian)
                f = 1.0 - exp(-fc*fc/(2.0*x*x));
            else // (filter == FilterFermi)
                f = 1.0/(1.0+exp((fc-x)/(-n)));

            *ydata *= f;
            *yidata *= f;
        }

        ydata++;
        yidata++;
    }

    wxPlotData dest( xform.FFT(false) );
    dest.OffsetX(M_PLOTDATA->m_Xdata[0]);
    dest.CalcBoundingRect();
    return dest;
}

wxPlotData wxPlotData::FFTNotchFilter( double lo, double hi, wxPlotData::FFTFilter_Type filter, double n )
{
    wxCHECK_MSG( Ok() && (lo <= hi) && (n>0), wxPlotData(), wxT("Invalid wxPlotData") );

    wxPlotData xform( FFT(true) );

    int i, count = xform.GetCount();
    double x, f;
    double *ydata = xform.GetYData(),
           *yidata = xform.GetYiData();

    for (i=0; i<count; i++)
    {
        x = xform.GetXData()[i];

        if ((filter == FilterStep) && ((x >= lo) && (x <= hi)))
        {
            *ydata = 0;
            *yidata = 0;
        }
        else
        {
            if (filter == FilterButterworth)
                f = 1.0/(1.0 + pow(x/lo, 2.0*n)) + (1.0-1.0/(1.0 + pow(x/hi, 2.0*n)));
            else if (filter == FilterGaussian)
                f = 2.0 - (exp(-lo*lo/(2.0*x*x)) + (1.0 - exp(-hi*hi/(2.0*x*x))));
            else // (filter == FilterFermi)
                f = 1.0/(1.0+exp((lo-x)/(-n))) + 1.0/(1.0+exp((hi-x)/(n)));

            *ydata *= f;
            *yidata *= f;
        }

        ydata++;
        yidata++;
    }

    wxPlotData dest( xform.FFT(false) );
    dest.OffsetX(M_PLOTDATA->m_Xdata[0]);
    dest.CalcBoundingRect();
    return dest;
}

wxPlotData wxPlotData::FFTBandPassFilter( double lo, double hi, wxPlotData::FFTFilter_Type filter, double n )
{
    wxPlotData xform;
    wxCHECK_MSG( Ok() && (lo <= hi) && (n>0), xform, wxT("Invalid wxPlotData") );

    xform = FFT(true);
    if (!xform.Ok()) return xform;

    int i, count = xform.GetCount();
    double x, f;

    double *xform_xdata = xform.GetXData(),
           *xform_ydata = xform.GetYData(),
           *xform_yidata = xform.GetYiData();

    for (i=0; i<count; i++)
    {
        x = *xform_xdata++;

        if ((filter == FilterStep) && ((x <= lo) || (x >= hi)))
        {
            *xform_ydata = 0;
            *xform_yidata = 0;
        }
        else
        {
            if (filter == FilterButterworth)
                f = 1.0-(1.0/(1.0 + pow(x/lo, 2.0*n)) + (1.0-1.0/(1.0 + pow(x/hi, 2.0*n))));
            else if (filter == FilterGaussian)
                f = exp(-lo*lo/(2.0*x*x)) - exp(-hi*hi/(2.0*x*x));
            else // (filter == FilterFermi)
                f = 1.0 - 1.0/(1.0+exp((lo-x)/(-n))) - 1.0/(1.0+exp((hi-x)/(n)));

            *xform_ydata *= f;
            *xform_yidata *= f;
        }

        xform_ydata++;
        xform_yidata++;
    }

    wxPlotData dst( xform.FFT(false) );
    if (!dst.Ok()) return dst;
    dst.OffsetX(M_PLOTDATA->m_Xdata[0]);
    dst.CalcBoundingRect();
    return dst;
}

wxPlotData wxPlotData::FFTCustomFilter( const wxPlotFunction &func )
{
    wxCHECK_MSG(Ok() && func.Ok(), wxPlotData(), wxT("invalid curve"));

    wxPlotData xform = FFT(true);
    if (!xform.Ok()) return xform;

    xform = xform.Modify(func, mult_y);
    if (!xform.Ok()) return xform;

    xform = xform.Modify(func, mult_yi);
    if (!xform.Ok()) return xform;

    wxPlotData dst( xform.FFT(false) );
    if (!dst.Ok()) return dst;

    dst.OffsetX(M_PLOTDATA->m_Xdata[0]);
    dst.CalcBoundingRect();
    return dst;
}

bool wxPlotData::SortByX()
{
    wxCHECK_MSG( Ok(), false, wxT("Invalid wxPlotData") );
    IMPLEMENT_PIXEL_QSORT2(double, M_PLOTDATA->m_Xdata, M_PLOTDATA->m_Ydata, M_PLOTDATA->m_count);
    return true;
}

bool wxPlotData::SortByY()
{
    wxCHECK_MSG( Ok(), false, wxT("Invalid wxPlotData") );
    IMPLEMENT_PIXEL_QSORT2(double, M_PLOTDATA->m_Ydata, M_PLOTDATA->m_Xdata, M_PLOTDATA->m_count);
    return true;
}

//-------------------------------------------------------------------------
// Curve fitting
//-------------------------------------------------------------------------
/*
#include "amoeba.h"

static wxPlotFunction s_amoebaPlotFunc;
static wxPlotData     s_amoebaPlotData;

double amoema_func( double *vars )
{
    double diff = 0;
    int i, count = s_amoebaPlotData.GetCount();
    double *x_data = s_amoebaPlotData.GetXData();
    double *y_data = s_amoebaPlotData.GetYData();
\
    int ndim = s_amoebaPlotFunc.GetNumberVariables();

    for (i=0; i<count; i++)
    {
        vars[ndim-1] = *x_data++;
        diff += s_amoebaPlotFunc.GetValue(vars) - (*y_data++);
    }

    printf("diff %lf %lf %lf\n", diff, vars[0], vars[1]);
    return diff;
}

int wxPlotData::AmoebaFit( const wxPlotFunction &func, double *vars )
{
    int i, j, ndim = func.GetNumberVariables()-1;

    double *init = (double*)malloc(ndim*(ndim+1)*sizeof(double));

    double tweak[6] = { .5, .1, -1, 1, -.1, 1 };

    for (j=0; j<ndim+1; j++)
    {
        for (i=0; i<ndim; i++)
        {
            if (i == j-1)
            {
                init[j*ndim+i] = vars[i] + tweak[i];
            }
            else
                init[j*ndim+i] = vars[i];

            printf("%lf ", init[j*ndim+i]);

        }
        printf("\n");
    }

    int iter = 0;
    double tolerance = 1e-6;

    s_amoebaPlotFunc = func;
    s_amoebaPlotData = *this;

    //Amoeba( amoema_func, init, vars, ndim, tolerance, &iter );

    printf("iters %d\n", iter);

    free(init);
    return iter;
}
*/
//----------------------------------------------------------------------------
// Get/Set bitmap symbol -- FIXME - this is NOT FINISHED OR WORKING
//----------------------------------------------------------------------------

wxBitmap wxPlotData::GetSymbol(wxPlotPen_Type colour_type) const
{
    wxCHECK_MSG( Ok(), M_PLOTDATA->m_normalSymbol, wxT("Invalid wxPlotData") );

    switch (colour_type)
    {
        case wxPLOTPEN_ACTIVE :
            return M_PLOTDATA->m_activeSymbol;
        case wxPLOTPEN_SELECTED :
            return M_PLOTDATA->m_selectedSymbol;
        default : break; //case wxPLOTPEN_NORMAL :
    }

    return M_PLOTDATA->m_normalSymbol;
}

void wxPlotData::SetSymbol( const wxBitmap &bitmap, wxPlotPen_Type colour_type )
{
    wxCHECK_RET( Ok(), wxT("Invalid wxPlotData") );
    wxCHECK_RET( bitmap.Ok(), wxT("Invalid bitmap") );

    switch (colour_type)
    {
        case wxPLOTPEN_ACTIVE :
            M_PLOTDATA->m_activeSymbol = bitmap;
            break;
        case wxPLOTPEN_SELECTED :
            M_PLOTDATA->m_selectedSymbol = bitmap;
            break;
        default : //case wxPLOTPEN_NORMAL :
            M_PLOTDATA->m_normalSymbol = bitmap;
            break;
    }
}
void wxPlotData::SetSymbol( wxPlotSymbol_Type type, wxPlotPen_Type colour_type, int width , int height,
                            const wxPen *pen, const wxBrush *brush)
{
    wxCHECK_RET( Ok(), wxT("Invalid wxPlotData") );

    switch (colour_type)
    {
        case wxPLOTPEN_ACTIVE :
            M_PLOTDATA->m_activeSymbol = CreateSymbol(type, colour_type, width, height, pen, brush);
            break;
        case wxPLOTPEN_SELECTED :
            M_PLOTDATA->m_selectedSymbol = CreateSymbol(type, colour_type, width, height, pen, brush);
            break;
        default : //case wxPLOTPEN_NORMAL :
            M_PLOTDATA->m_normalSymbol = CreateSymbol(type, colour_type, width, height, pen, brush);
            break;
    }
}

wxBitmap wxPlotData::CreateSymbol( wxPlotSymbol_Type type, wxPlotPen_Type colour_type, int width, int height,
                                   const wxPen *pen, const wxBrush *brush)
{
    wxBitmap b(width, height);

    wxMemoryDC mdc;
    mdc.SelectObject(b);
    mdc.SetPen(*wxWHITE_PEN);
    mdc.SetBrush(*wxWHITE_BRUSH);
    mdc.DrawRectangle(0, 0, width, height);
/*
    if (pen)
        mdc.SetPen(*pen);
    else
        mdc.SetPen(GetNormalPen());
*/
    switch (type)
    {
        case wxPLOTSYMBOL_ELLIPSE :
        {
            mdc.DrawEllipse(width/2, height/2, width/2, height/2);
            break;
        }
        case wxPLOTSYMBOL_RECTANGLE :
        {
            mdc.DrawRectangle(0, 0, width, height);
            break;
        }
        case wxPLOTSYMBOL_CROSS :
        {
            mdc.DrawLine(0, 0, width , height);
            mdc.DrawLine(0, height, width, 0);
            break;
        }
        case wxPLOTSYMBOL_PLUS :
        {
            mdc.DrawLine(0, height/2, width, height/2);
            mdc.DrawLine(width/2, 0, width/2, height);
            break;
        }

        default : break;
    }

    b.SetMask(new wxMask(b, *wxWHITE));

    return b;
}

// ----------------------------------------------------------------------------
// Functions for the wxClipboard
// ----------------------------------------------------------------------------
#include "wx/clipbrd.h"
#if wxUSE_DATAOBJ && wxUSE_CLIPBOARD

const wxChar* wxDF_wxPlotData     = wxT("wxDF_wxPlotData");

static wxPlotData s_clipboardwxPlotData;    // temp storage of clipboard data
static wxString s_clipboardwxPlotData_data; // holds wxNow() to match clipboard data

// For some strange reason you get a segfault destroying s_clipboardwxPlotData
//  using gcc 3.4 at least. Using a wxModule to destroy it fixes this.
//#0  0x40aefee1 in posix_memalign () from /lib/tls/libc.so.6
//#1  0x40aedd4c in mallopt () from /lib/tls/libc.so.6
//#2  0x40aeb8ba in free () from /lib/tls/libc.so.6
//#3  0x4060e885 in ~wxPlotDataRefData (this=0x82bc4b0) at plotcurve.cpp:203
//#4  0x40518d1a in wxObject::UnRef() (this=0x40630ef4) at src/common/object.cpp:309
//#5  0x0809d0e4 in ~wxObject (this=0x40630ef4) at object.h:431
//#6  0x4061cd42 in ~wxPlotCurve (this=0x40630ef4) at plotcurve.h:110
//#7  0x4061c43c in ~wxPlotData (this=0x40630ef4) at plotcurve.h:297
//#8  0x4061c247 in __tcf_7 () at plotcurve.cpp:3044

#include "wx/module.h"
class wxPlotDataModule: public wxModule
{
DECLARE_DYNAMIC_CLASS(wxPlotDataModule)
public:
    wxPlotDataModule() : wxModule() {}
    bool OnInit()
    {
        return true;
    }
    void OnExit()
    {
        s_clipboardwxPlotData.Destroy();
    }
};
IMPLEMENT_DYNAMIC_CLASS(wxPlotDataModule, wxModule)

wxPlotData wxClipboardGetPlotData()
{
    bool is_opened = wxTheClipboard->IsOpened();
    wxPlotData plotData;

    if (is_opened || wxTheClipboard->Open())
    {
        wxPlotDataObject plotDataObject;
        if (wxTheClipboard->IsSupported(wxDataFormat(wxDF_wxPlotData)) &&
            wxTheClipboard->GetData(plotDataObject) &&
            (plotDataObject.GetText() == s_clipboardwxPlotData_data))
        {
            plotData.Copy(plotDataObject.GetPlotData(), true);
        }

        if (!is_opened)
            wxTheClipboard->Close();
    }

    return plotData;
}
bool wxClipboardSetPlotData(const wxPlotData& plotData)
{
  wxCHECK_MSG(plotData.Ok(), false, wxT("Invalid wxPlotData to copy to clipboard"));
    bool is_opened = wxTheClipboard->IsOpened();

    if (is_opened || wxTheClipboard->Open())
    {
        wxPlotDataObject *plotDataObject = new wxPlotDataObject(plotData);
        bool ret = wxTheClipboard->SetData( plotDataObject );

        if (!is_opened)
            wxTheClipboard->Close();

        return ret;
    }

    return false;
}

// ----------------------------------------------------------------------------
// wxPlotDataObject Clipboard object
// ----------------------------------------------------------------------------

wxPlotDataObject::wxPlotDataObject() : wxTextDataObject()
{
    SetFormat(wxDataFormat(wxDF_wxPlotData));
}
wxPlotDataObject::wxPlotDataObject(const wxPlotData& plotData) : wxTextDataObject()
{
    SetFormat(wxDataFormat(wxDF_wxPlotData));
    SetPlotData(plotData);
}
wxPlotData wxPlotDataObject::GetPlotData() const
{
    return s_clipboardwxPlotData;
}
void wxPlotDataObject::SetPlotData(const wxPlotData& plotData)
{
    s_clipboardwxPlotData_data = wxNow();
    SetText(s_clipboardwxPlotData_data);

    if (plotData.Ok())
        s_clipboardwxPlotData.Copy(plotData, true);
    else
        s_clipboardwxPlotData.Destroy();
}

#endif // wxUSE_DATAOBJ && wxUSE_CLIPBOARD

//
// FILE: gambdraw.cc -- Draw settings implementation
//
// $Id$
//

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // WX_PRECOMP
#include "wxmisc.h"

#include "gambit.h"
#include "gambdraw.h"

GambitDrawSettings::GambitDrawSettings(void)
  : m_colors(-1, 0)
{
  m_colors[-1] = *wxBLACK;
  m_colors[0] = *wxLIGHT_GREY;
  LoadOptions();
}

void GambitDrawSettings::SetPlayerColor(int p_player, const wxColour &p_color)
{
  while (m_colors.Last() < p_player) {
    m_colors.Append(((m_colors.Last() + 1) % 2 == 0) ? *wxRED : *wxBLUE);
  }
  m_colors[p_player] = p_color;
}

const wxColour &GambitDrawSettings::GetPlayerColor(int p_player) const
{
  while (m_colors.Last() < p_player) {
    m_colors.Append(((m_colors.Last() + 1) % 2 == 0) ? *wxRED : *wxBLUE);
  }
  return m_colors[p_player];
}

void GambitDrawSettings::LoadOptions(void)
{
  wxConfig config("Gambit");
  for (int i = -1; ; i++) {
    gText playerString = "Colors/Player" + ToText(i);
    if (!config.Exists((char *) (playerString + "-Red"))) {
      break;
    }
    if (i > m_colors.Last()) {
      m_colors.Append(*wxRED);
    }
    
    long red, green, blue;
    config.Read((char *) (playerString + "-Red"), &red, 0l);
    config.Read((char *) (playerString + "-Green"), &green, 0l);
    config.Read((char *) (playerString + "-Blue"), &blue, 0l);
    m_colors[i].Set(red, green, blue);
  } 
}

void GambitDrawSettings::SaveOptions(void) const
{
  wxConfig config("Gambit");
  for (int i = -1; i <= m_colors.Last(); i++) {
    gText playerString = "Colors/Player" + ToText(i);
    config.Write((char *) (playerString + "-Red"), (long) m_colors[i].Red());
    config.Write((char *) (playerString + "-Green"), (long) m_colors[i].Green());
    config.Write((char *) (playerString + "-Blue"), (long) m_colors[i].Blue());
  }
}

#include "base/garray.imp"
#include "base/gblock.imp"

gOutput &operator<<(gOutput &p_stream, const wxColour &)
{ return p_stream; }

template class gArray<wxColour>;
template class gBlock<wxColour>;

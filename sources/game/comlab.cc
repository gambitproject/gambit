//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Utilities for reading and writing ComLabGames files
//
// This file is part of Gambit
// Copyright (c) 2002, The Gambit Project
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//

#include <stdlib.h>
#include <ctype.h>
#include "nfg.h"
#include "nfgciter.h"

//
// This file implements reading/writing ComLabGames files (still experimental).
// ComLabGames strategic form files are line-oriented, with pairs of
// the form Identifier = \t value.  Values with spaces appear to be
// enclosed in quotes.  Additionally, comment lines beginning with a
// hash mark also appear in the file delineating various sections.
// This implementation just ignores comment lines.
//
// Much of this implementation is ugly.
// 

//=========================================================================
//               Reading ComLabGames strategic form files
//=========================================================================

//
// Reads a line of input (i.e., until a '\n' or EOF), and returns it.
// The newline is read, but not included in the return value.
//
static gText GetLine(gInput &p_input)
{
  gText line;

  while (!p_input.eof()) {
    char c;
    p_input.get(c);
    if (c != '\n') {
      line += c;
    }
    else {
      break;
    }
  }
  return line;
}

static void ParseLine(const gText &p_line, gText &p_field, gText &p_value)
{
  unsigned int i = 0;

  while (isalpha(p_line[i])) {
    p_field += p_line[i++];
  }

  while (isspace(p_line[i]) || p_line[i] == '=') {
    i++;
  }

  while (p_line[i] != '\0') {
    p_value += p_line[i++];
  }
}

static void EatWhitespace(const gText &p_string, unsigned int &p_index)
{
  while (isspace(p_string[p_index])) {
    p_index++;
  }
}

static bool ParsePayoffs(Nfg *p_nfg, const gText &p_payoffs)
{
  unsigned int i = 0;
  while (isspace(p_payoffs[i])) {
    i++;
  }

  // Outer bracket
  if (p_payoffs[i++] != '[') {
    return false;
  }
  EatWhitespace(p_payoffs, i);
  gbtNfgSupport support(*p_nfg);
  NfgContIter iter(support);
  while (p_payoffs[i++] == '[') {
    gText pay;
    iter.SetOutcome(p_nfg->NewOutcome());
    EatWhitespace(p_payoffs, i);
    while (isdigit(p_payoffs[i]) ||
	   p_payoffs[i] == '.' || p_payoffs[i] == '-') {
      pay += p_payoffs[i++];
    }
    p_nfg->SetPayoff(iter.GetOutcome(), 1, ToNumber(pay));
    EatWhitespace(p_payoffs, i);
    if (p_payoffs[i++] != ',') {
      return false;
    }
    EatWhitespace(p_payoffs, i);
    pay = "";
    while (isdigit(p_payoffs[i]) ||
	   p_payoffs[i] == '.' || p_payoffs[i] == '-') {
      pay += p_payoffs[i++];
    }
    p_nfg->SetPayoff(iter.GetOutcome(), 2, ToNumber(pay));
    EatWhitespace(p_payoffs, i);
    if (p_payoffs[i++] != ']') {
      return false;
    }
    EatWhitespace(p_payoffs, i);
    if (p_payoffs[i] == ',') {
      i++;
    }
    EatWhitespace(p_payoffs, i);
    iter.NextContingency();
  }
  return true;
}

Nfg *ReadComLabSfg(gInput &p_input)
{
  Nfg *nfg = 0;
  gText title, description, rowPlayer, colPlayer;
  // As of this implementation, ComLabGames only has 2-player games
  gArray<int> dim(2);   
  dim[1] = dim[2] = -1;

  while (!p_input.eof()) {
    gText line = GetLine(p_input), field, value;
    if (line[0u] == '#') {
      continue;
    }
    ParseLine(line, field, value);
    
    if (field == "Title") {
      title = value;
    }
    else if (field == "Description") {
      description = value;
    }
    else if (field == "RowPlayer") {
      rowPlayer = value;
    }
    else if (field == "ColPlayer") {
      colPlayer = value;
    }
    else if (field == "Width") {
      dim[1] = atoi((char *) value);
    }
    else if (field == "Height") {
      dim[2] = atoi((char *) value);
    }
    else if (field == "Payoffs") {
      if (!nfg) {
	return 0;
      }
      ParsePayoffs(nfg, value);
    }

    // Create the game as soon as possible
    if (dim[1] > 0 && dim[2] > 0 && !nfg) {
      nfg = new Nfg(dim);
    }
  }

  return nfg;
}


//=========================================================================
//                Writing ComLabGames strategic form files
//=========================================================================

void WriteComLabSfg(gOutput &p_output, Nfg *p_nfg)
{
  p_output << "\n# This is a ComLabGames game description file.\n";
  p_output << "ModuleClass =\tclg.sfg.SfgModule\n";
  p_output << "ModuleVersion =\t0.1\n";

  p_output << "\n# Game information\n";
  p_output << "Title =\t\"" << p_nfg->GetTitle() << "\"\n";
  p_output << "Description =\t\n";
  p_output << "WindowWidth =\t600\n";
  p_output << "WindowHeight =\t519\n";

  p_output << "\n# Players\n";
  p_output << "RowPlayer =\t\"" << p_nfg->GetPlayer(1).GetLabel() << "\"\n";
  p_output << "ColPlayer =\t\"" << p_nfg->GetPlayer(2).GetLabel() << "\"\n";

  p_output << "\n# Table information\n";
  // There's an extra space after Width, apparently so file formats nicely
  p_output << "Width = \t" << p_nfg->NumStrats(1) << "\n";
  p_output << "Height =\t" << p_nfg->NumStrats(2) << "\n";
  p_output << "RowNames =\t[]\n";
  p_output << "ColNames =\t[]\n";

  p_output << "\n# Payoff information [row,column]\n";
  p_output << "Payoffs =\t\n";
}

//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/tools/gt/nfggt.cc
// Gambit frontend to Gametracer
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

#include "gambit.h"

using namespace Gambit;

List<MixedStrategyProfile<double>> ReadStrategyPerturbations(const Game &p_game,
                                                             std::istream &p_stream)
{
  List<MixedStrategyProfile<double>> profiles;
  while (!p_stream.eof() && !p_stream.bad()) {
    MixedStrategyProfile<double> p(p_game->NewMixedStrategyProfile(0.0));
    for (size_t i = 1; i <= p.MixedProfileLength(); i++) {
      if (p_stream.eof() || p_stream.bad()) {
        break;
      }
      p_stream >> p[i];
      if (i < p.MixedProfileLength()) {
        char comma;
        p_stream >> comma;
      }
    }
    // Read in the rest of the line and discard
    std::string foo;
    std::getline(p_stream, foo);
    profiles.push_back(p);
  }
  return profiles;
}

List<MixedStrategyProfile<double>> RandomStrategyPerturbations(const Game &p_game, int p_count)
{
  std::default_random_engine engine;
  List<MixedStrategyProfile<double>> profiles;
  for (int i = 1; i <= p_count; i++) {
    profiles.push_back(p_game->NewRandomStrategyProfile(engine));
  }
  return profiles;
}

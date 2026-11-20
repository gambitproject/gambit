//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/solvers/enummixed/clique.h
// Maximal bicliques and connected components of a bipartite graph
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

#ifndef GAMBIT_SOLVERS_ENUMMIXED_CLIQUE_H
#define GAMBIT_SOLVERS_ENUMMIXED_CLIQUE_H

#include <utility>
#include <vector>

namespace Gambit::Nash {

/// A maximal complete bipartite subgraph: every node in `left` is adjacent to every node in
/// `right`
struct Biclique {
  std::vector<int> left, right;
};

/// The connected components of the bipartite graph with edges `p_edges` (left node, right node),
/// each given as the list of its maximal bicliques.
///
/// Nodes are arbitrary integers, labelled separately on the two sides; a repeated edge counts
/// once. Nodes are ascending within each side of a biclique, bicliques are in lexicographic order
/// of (left, right) within a component, and components are in order of their smallest left node.
///
/// The method is von Stengel's variant (1998) of version 2 of the algorithm of
/// [BK] C. Bron and J. Kerbosch, Finding all cliques of an undirected graph, Comm. ACM 16:9
/// (1973), 575-577.  The maximal bicliques of a bipartite graph are the maximal cliques of the
/// graph in which all left nodes are also joined to each other, and all right nodes to each
/// other, except for those cliques lying on one side only.  Each step extends the current
/// biclique by one candidate, and keeps the nodes already tried (NOT) and the candidates (CAND)
/// separately for each side.  The node chosen to extend by is a fixpoint: the node in NOT or CAND,
/// on either side, with the fewest disconnections to the candidates on the other side.  It is
/// tried if it is a candidate, and then only the candidates disconnected from it are tried.
std::vector<std::vector<Biclique>>
EnumerateBicliqueComponents(const std::vector<std::pair<int, int>> &p_edges);

} // end namespace Gambit::Nash

#endif // GAMBIT_SOLVERS_ENUMMIXED_CLIQUE_H

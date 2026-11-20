//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/solvers/enummixed/clique.cc
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

#include <algorithm>
#include <array>
#include <iterator>
#include <limits>
#include <numeric>

#include "clique.h"

namespace Gambit::Nash {

namespace {

constexpr size_t LEFT = 0;
constexpr size_t RIGHT = 1;

/// One connected component. Its nodes are numbered from zero on each side, in the order of
/// their labels.
struct BipartiteGraph {
  std::array<std::vector<int>, 2> labels;
  std::vector<char> adjacent; // row-major, one row per left node

  size_t Size(size_t p_side) const { return labels[p_side].size(); }

  /// Is `p_node` on `p_side` adjacent to `p_other` on the opposite side?
  bool Joined(size_t p_side, size_t p_node, size_t p_other) const
  {
    const auto [left, right] =
        (p_side == LEFT) ? std::pair(p_node, p_other) : std::pair(p_other, p_node);
    return adjacent[left * Size(RIGHT) + right] != 0;
  }
};

/// Node numbers on each side
using NodeLists = std::array<std::vector<size_t>, 2>;

/// The nodes already tried and the candidates for extending the current biclique
struct NodeSets {
  NodeLists tried, candidates;
};

struct Search {
  const BipartiteGraph &graph;
  NodeLists current;            // the biclique under construction
  std::vector<NodeLists> found; // the maximal bicliques found so far
};

/// The node whose disconnected candidates are tried
struct Pivot {
  size_t side{LEFT};
  size_t node{0};
  bool isCandidate{false};
  /// Candidates on the opposite side that are not adjacent to `node`
  std::vector<size_t> disconnected;
};

/// Collects into `p_out` the nodes in `p_others` not adjacent to `p_node`.
/// Returns false, possibly before finishing, if there are `p_limit` or more.
bool CollectDisconnected(const BipartiteGraph &p_graph, size_t p_side, size_t p_node,
                         const std::vector<size_t> &p_others, size_t p_limit,
                         std::vector<size_t> &p_out)
{
  p_out.clear();
  for (const size_t other : p_others) {
    if (!p_graph.Joined(p_side, p_node, other)) {
      p_out.push_back(other);
      if (p_out.size() >= p_limit) {
        return false;
      }
    }
  }
  return p_out.size() < p_limit;
}

/// The node of the tried or candidate nodes on either side with the fewest disconnections to the
/// candidates on the opposite side
Pivot FindPivot(const BipartiteGraph &p_graph, const NodeSets &p_sets)
{
  Pivot pivot;
  size_t best = std::numeric_limits<size_t>::max();
  std::vector<size_t> disconnected;
  for (const size_t side : {LEFT, RIGHT}) {
    for (const auto *group : {&p_sets.tried[side], &p_sets.candidates[side]}) {
      for (const size_t node : *group) {
        if (CollectDisconnected(p_graph, side, node, p_sets.candidates[1 - side], best,
                                disconnected)) {
          best = disconnected.size();
          pivot.side = side;
          pivot.node = node;
          pivot.isCandidate = (group == &p_sets.candidates[side]);
          pivot.disconnected.swap(disconnected);
        }
      }
    }
  }
  return pivot;
}

/// One level of the search: the sets at that level, and the extensions of the current biclique
/// still to try from it
struct Frame {
  NodeSets sets;
  std::vector<std::pair<size_t, size_t>> trials; // (side, node), in the order tried
  size_t next{0};                                // the trial under way, or the next to start
  bool expanded{false};
};

/// Records the current biclique if no tried node or candidate could extend it, and otherwise
/// lists the extensions to try
void Expand(Search &p_search, Frame &p_frame)
{
  p_frame.expanded = true;
  const NodeSets &sets = p_frame.sets;
  if (sets.candidates[LEFT].empty() && sets.candidates[RIGHT].empty()) {
    const bool maximal = sets.tried[LEFT].empty() && sets.tried[RIGHT].empty();
    if (maximal && !p_search.current[LEFT].empty() && !p_search.current[RIGHT].empty()) {
      p_search.found.push_back(p_search.current);
    }
    return;
  }
  const Pivot pivot = FindPivot(p_search.graph, sets);
  if (pivot.isCandidate) {
    p_frame.trials.emplace_back(pivot.side, pivot.node);
  }
  for (const size_t node : pivot.disconnected) {
    p_frame.trials.emplace_back(1 - pivot.side, node);
  }
}

/// Extends the current biclique by `p_node`, a candidate on `p_side`, and returns the next level
Frame ExtendBy(Search &p_search, const NodeSets &p_sets, size_t p_side, size_t p_node)
{
  const size_t other = 1 - p_side;
  NodeSets next;
  next.tried[p_side] = p_sets.tried[p_side];
  std::ranges::copy_if(p_sets.candidates[p_side], std::back_inserter(next.candidates[p_side]),
                       [p_node](const size_t v) { return v != p_node; });
  const auto joined = [&](const size_t v) { return p_search.graph.Joined(p_side, p_node, v); };
  std::ranges::copy_if(p_sets.tried[other], std::back_inserter(next.tried[other]), joined);
  std::ranges::copy_if(p_sets.candidates[other], std::back_inserter(next.candidates[other]),
                       joined);
  p_search.current[p_side].push_back(p_node);
  return Frame{.sets = std::move(next), .trials = {}, .next = 0, .expanded = false};
}

/// Takes the node of the trial just finished out of the current biclique, and makes it a tried
/// node
void FinishTrial(Search &p_search, Frame &p_frame)
{
  const auto [side, node] = p_frame.trials[p_frame.next];
  p_search.current[side].pop_back();
  std::erase(p_frame.sets.candidates[side], node);
  p_frame.sets.tried[side].push_back(node);
  p_frame.next++;
}

/// Searches depth first, from the empty biclique, on a stack of levels held on the heap: the depth
/// is the size of the largest biclique, which can be the number of nodes in the graph
void Enumerate(Search &p_search, NodeSets p_root)
{
  std::vector<Frame> stack;
  stack.push_back(Frame{.sets = std::move(p_root), .trials = {}, .next = 0, .expanded = false});
  while (!stack.empty()) {
    if (!stack.back().expanded) {
      Expand(p_search, stack.back());
    }
    const Frame &frame = stack.back();
    if (frame.next < frame.trials.size()) {
      const auto [side, node] = frame.trials[frame.next];
      stack.push_back(ExtendBy(p_search, frame.sets, side, node));
    }
    else {
      stack.pop_back();
      if (!stack.empty()) {
        FinishTrial(p_search, stack.back());
      }
    }
  }
}

/// The maximal bicliques of one component, as labels, in the order the contract states
std::vector<Biclique> EnumerateBicliques(const BipartiteGraph &p_graph)
{
  Search search{.graph = p_graph, .current = {}, .found = {}};
  NodeSets sets;
  for (const size_t side : {LEFT, RIGHT}) {
    sets.candidates[side].resize(p_graph.Size(side));
    std::iota(sets.candidates[side].begin(), sets.candidates[side].end(), size_t{0});
  }
  Enumerate(search, std::move(sets));

  for (auto &biclique : search.found) {
    std::ranges::sort(biclique[LEFT]);
    std::ranges::sort(biclique[RIGHT]);
  }
  std::ranges::sort(search.found);

  std::vector<Biclique> result;
  result.reserve(search.found.size());
  for (const auto &biclique : search.found) {
    Biclique labelled;
    for (const size_t node : biclique[LEFT]) {
      labelled.left.push_back(p_graph.labels[LEFT][node]);
    }
    for (const size_t node : biclique[RIGHT]) {
      labelled.right.push_back(p_graph.labels[RIGHT][node]);
    }
    result.push_back(std::move(labelled));
  }
  return result;
}

std::vector<int> SortedLabels(const std::vector<std::pair<int, int>> &p_edges, size_t p_side)
{
  std::vector<int> labels;
  labels.reserve(p_edges.size());
  for (const auto &edge : p_edges) {
    labels.push_back(p_side == LEFT ? edge.first : edge.second);
  }
  std::ranges::sort(labels);
  labels.erase(std::ranges::unique(labels).begin(), labels.end());
  return labels;
}

size_t Rank(const std::vector<int> &p_sorted, int p_label)
{
  return static_cast<size_t>(std::ranges::lower_bound(p_sorted, p_label) - p_sorted.begin());
}

size_t FindRoot(std::vector<size_t> &p_parent, size_t p_node)
{
  while (p_parent[p_node] != p_node) {
    p_parent[p_node] = p_parent[p_parent[p_node]];
    p_node = p_parent[p_node];
  }
  return p_node;
}

/// The connected components, in order of their smallest left node
std::vector<BipartiteGraph> SplitComponents(const std::vector<std::pair<int, int>> &p_edges)
{
  const std::array<std::vector<int>, 2> sorted = {SortedLabels(p_edges, LEFT),
                                                  SortedLabels(p_edges, RIGHT)};
  const size_t numLeft = sorted[LEFT].size();

  // Nodes are numbered 0..numLeft-1 on the left, then numLeft.. on the right
  std::vector<std::pair<size_t, size_t>> edges;
  edges.reserve(p_edges.size());
  std::vector<size_t> parent(numLeft + sorted[RIGHT].size());
  std::iota(parent.begin(), parent.end(), size_t{0});
  for (const auto &[left, right] : p_edges) {
    edges.emplace_back(Rank(sorted[LEFT], left), numLeft + Rank(sorted[RIGHT], right));
    parent[FindRoot(parent, edges.back().first)] = FindRoot(parent, edges.back().second);
  }

  std::vector<size_t> componentOfRoot(parent.size(), std::numeric_limits<size_t>::max());
  std::vector<BipartiteGraph> components;
  std::vector<size_t> componentOf(parent.size()), localIndex(parent.size());
  for (size_t node = 0; node < parent.size(); node++) {
    const size_t side = (node < numLeft) ? LEFT : RIGHT;
    const size_t root = FindRoot(parent, node);
    if (componentOfRoot[root] == std::numeric_limits<size_t>::max()) {
      componentOfRoot[root] = components.size();
      components.emplace_back();
    }
    auto &component = components[componentOfRoot[root]];
    componentOf[node] = componentOfRoot[root];
    localIndex[node] = component.labels[side].size();
    component.labels[side].push_back(sorted[side][side == LEFT ? node : node - numLeft]);
  }
  for (auto &component : components) {
    component.adjacent.assign(component.Size(LEFT) * component.Size(RIGHT), 0);
  }
  for (const auto &[left, right] : edges) {
    auto &component = components[componentOf[left]];
    component.adjacent[localIndex[left] * component.Size(RIGHT) + localIndex[right]] = 1;
  }
  return components;
}

} // end anonymous namespace

std::vector<std::vector<Biclique>>
EnumerateBicliqueComponents(const std::vector<std::pair<int, int>> &p_edges)
{
  std::vector<std::vector<Biclique>> result;
  for (const auto &component : SplitComponents(p_edges)) {
    result.push_back(EnumerateBicliques(component));
  }
  return result;
}

} // end namespace Gambit::Nash

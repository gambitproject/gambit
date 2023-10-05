.. _file-formats-agg:

The action graph game (.agg) file format
========================================

Action graph games (AGGs) are a compact representation of simultaneous-move games with structured utility functions.
For more information on AGGs, the following paper gives a comprehensive discussion.

    A.X. Jiang, K. Leyton-Brown and N. Bhat, `Action-Graph Games <http://www.cs.ubc.ca/~jiang/papers/AGG.pdf>`_,
    Games and Economic Behavior, Volume 71, Issue 1, January 2011, Pages 141-173.

Each file in this format describes an action graph game.
In order for the file to be recognized as AGG by GAMBIT, the initial line of the file should be::

    #AGG

The rest of the file consists of 8 sections, separated by whitespaces. Lines with starting '#' are treated as comments and are allowed between sections.

#. The number of players, n.

#. The number of action nodes, \|S\|.

#. The number of function nodes, \|P\|.

#. Size of action set for each player. This is a row of n integers:

   \|S\ :sub:`1`\ \| \|S\ :sub:`2`\ \| .... \|S\ :sub:`n`\ \|

#. Each Player's action set. We have N rows; row i has \|S\ :sub:`i`\ \| integers in ascending order,
   which are indices of Action nodes. Action nodes are indexed from 0 to \|S\|-1.

#. The Action Graph. We have \|S\| + \|P\| nodes, indexed from 0 to \|S\| + \|P\|-1.
   The function nodes are indexed after the action nodes. The graph
   is represented as (\|S\| + \|P\|) neighbor lists, one list per row.
   Rows 0 to \|S\| - 1 are for action nodes; rows \|S\| to \|S\| + \|P\|-1 are for
   function nodes. In each row, the first number \|v\| specifies the
   number of neighbors of the node. Then follows \|v\| numbers,
   corresponding to the indices of the neighbors.

   We require that each function node has at least one neighbor, and
   the neighbors of function nodes are action nodes. The action graph
   restricted to the function nodes has to be a directed acyclic graph (DAG).

#. Signatures of functions.   This is \|P\| rows, each specifying the mapping
   f_p that maps from the configuration of the function node p's neighbors to
   an integer for p's "action count". Each function is specified by its "signature"
   consisting of an integer type, possibly followed by further parameters. Several types of mapping are
   implemented:

   * Types 0-3 require no further input.

       * Type 0: Sum. i.e. The action count of a function node p is the sum of
         the action counts  of p's neighbors.
       * Type 1: Existence: boolean for whether the sum of the counts of
         neighbors are positive.
       * Type 2: The index of the neighbor with the highest index that has
         non-zero counts, or \|S\| + \|P\| if none applies.
       * Type 3: The index of the neighbor with the lowest index that has
         non-zero counts, or \|S\| + \|P\| if none applies.

   * Types 10-13 are extended versions of type 0-3, each requiring
     further parameters of an integer default value and a list of weights,
     \|S\| integers enclosed in square brackets. Each action node is thus associated with an integer weight.

       * Type 10: Extended Sum. Each instance of an action in p's neighborhood being chosen contributes the
         weight of that action to the sum. These are added to the default value.
       * Type 11: Extended Existence: boolean for whether the extended sum is positive. The input default value
         and weights are required to be nonnegative.
       * Type 12: The weight of the neighbor with the highest index that has
         non-zero counts, or the default value if none applies.
       * Type 13: The weight of the neighbor with the lowest index that has
         non-zero counts, or the default value if none applies.

   The following is an example of the signatures for an AGG with three action nodes and two function nodes::

     2
     10 0 [2 3 4]

#. The payoff function for each action node. So we have
   \|S\| subblocks of numbers. Payoff function for action s is a mapping
   from configurations to real numbers. Configurations are
   represented as a tuple of integers; the size of the tuple is the size
   of the neighborhood of s. Each configuration specifies the action counts
   for the neighbors of s, in the same order as the neighbor list of s.

   The first number of each subblock specifies
   the type of the payoff function. There are multiple ways of representing
   payoff functions; we (or other people) can extend the file format by
   defining new types of payoff functions. We define two basic types:

   *Type 0*
    The complete representation. The set of possible
    configurations can be derived from the action graph. This set of
    configurations can also be sorted in lexicographical order. So we can
    just specify the payoffs without explicitly giving the configurations.
    So we just need to give one row of real numbers, which correspond to
    payoffs for the ordered set of configurations.

    If action s is in multiple players' action sets (say players
    i, j),
    then it is possible that the set of possible configurations
    given s\ :sub:`i`
    is different from the set of possible configurations given
    s\ :sub:`j`\ .
    In such cases, we need to specify payoffs for the union of the
    sets of configurations (sorted in lexicographical order).

   *Type 1*
    The mapping representation, in which we specify the configurations
    and the corresponding payoffs. For the payoff function of action s,
    first give Delta_s, the number of elements in the mapping.
    Then follows Delta_s rows. In each row, first specify the configuration,
    which is a tuple of integers, enclosed by a pair of brackets "[" and "]",  then the payoff.
    For example, the following specifies a payoff function of type 1, with two configurations::

      1 2
      [1 0] 2.5
      [1 1] -1.2


The Bayesian action graph game (.bagg) format
=============================================

Bayesian action graph games (BAGGs) are a compact representation of Bayesian (i.e., incomplete-information) games.
For more information on BAGGs, the following paper gives a detailed discussion.

    A.X. Jiang and K. Leyton-Brown, `Bayesian Action-Graph Games <http://www.cs.ubc.ca/~jiang/papers/BAGG.pdf>`_. NIPS, 2010.

Each file in this format describes a BAGG.
In order for the file to be recognized as BAGG by GAMBIT, the initial line of the file should be::

    #BAGG

The rest of the file consists of the following sections,
separated by whitespaces. Lines with starting '#' are treated as comments and are allowed between sections.

#. The number of Players, n.
#. The number of action nodes, \|S\|.
#. The number of function nodes, \|P\|.

#. The number of types for each player, as a row of n integers.
#. Type distribution for each player. The distributions are assumed to be independent.
   Each distribution is represented as a row of real numbers.
   The following example block gives the type distributions for a BAGG with two players and two types for each player::

     0.5 0.5
     0.2 0.8

#. Size of type-action set for each player's each type.

#. Type-action set for each player's each type.
   Each type-action set is represented as a row of integers in ascending order,
   which are indices of action nodes. Action nodes are indexed from 0 to \|S\|-1.

#. The action graph: same as in `the AGG format`_.

#. types of functions: same as in `the AGG format`_.

#. utility function for each action node: same as in `the AGG format`_.

.. _the AGG format:  _file-formats-agg

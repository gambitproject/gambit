.. _pygambit-api:

API documentation
-----------------

.. currentmodule:: pygambit

Representation of games
~~~~~~~~~~~~~~~~~~~~~~~

.. currentmodule:: pygambit.gambit

.. autosummary::
   :toctree: api/

   Game
   Player
   Outcome
   Node
   Infoset
   Action
   Strategy


Creating, reading, and writing games
....................................

.. autosummary::
   :toctree: api/

   read_gbt
   read_efg
   read_nfg
   read_agg

   Game.new_tree
   Game.new_table
   Game.from_arrays
   Game.to_arrays
   Game.from_dict
   Game.to_efg
   Game.to_nfg
   Game.to_html
   Game.to_latex


Transforming game trees
.......................

.. autosummary::
   :toctree: api/

   Game.append_move
   Game.append_infoset
   Game.insert_move
   Game.insert_infoset
   Game.copy_tree
   Game.move_tree
   Game.delete_parent
   Game.delete_tree


Transforming game information structure
.......................................

.. autosummary::
   :toctree: api/

   Game.set_player
   Game.set_infoset
   Game.leave_infoset
   Game.set_chance_probs
   Game.reveal
   Game.sort_infosets


Transforming game components
............................

.. autosummary::
   :toctree: api/

   Game.add_player
   Game.add_outcome
   Game.delete_outcome
   Game.set_outcome
   Game.add_strategy
   Game.delete_strategy


Information about the game
..........................

.. autosummary::
   :toctree: api/

   Game.title
   Game.description
   Game.is_const_sum
   Game.is_tree
   Game.is_perfect_recall
   Game.players
   Game.outcomes
   Game.min_payoff
   Game.max_payoff
   Game.strategies
   Game.root
   Game.actions
   Game.infosets
   Game.nodes
   Game.contingencies

.. autosummary::
   :toctree: api/

   Player.label
   Player.number
   Player.game
   Player.strategies
   Player.infosets
   Player.actions
   Player.is_chance
   Player.min_payoff
   Player.max_payoff
   Player.strategies

.. autosummary::
   :toctree: api/

   Outcome.label
   Outcome.number
   Outcome.game

.. autosummary::
   :toctree: api/

   Node.label
   Node.game
   Node.outcome
   Node.children
   Node.parent
   Node.is_subgame_root
   Node.is_terminal
   Node.is_strategy_reachable
   Node.prior_action
   Node.prior_sibling
   Node.next_sibling
   Node.infoset
   Node.player
   Node.is_successor_of
   Node.plays
   Node.own_prior_action

.. autosummary::

   :toctree: api/

   Infoset.label
   Infoset.game
   Infoset.is_chance
   Infoset.is_absent_minded
   Infoset.player
   Infoset.actions
   Infoset.members
   Infoset.precedes
   Infoset.plays
   Infoset.own_prior_actions

.. autosummary::

   :toctree: api/

   Action.label
   Action.infoset
   Action.precedes
   Action.prob
   Action.plays

.. autosummary::

   :toctree: api/

   Strategy.label
   Strategy.game
   Strategy.player
   Strategy.number
   Strategy.action


Player behavior
...............

.. autosummary::
   :toctree: api/

   Game.mixed_strategy_profile
   Game.random_strategy_profile
   Game.mixed_behavior_profile
   Game.random_behavior_profile
   Game.strategy_support_profile


Representation of strategic behavior
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. currentmodule:: pygambit.gambit


Probability distributions over strategies
.........................................

.. autosummary::
   :toctree: api/

   MixedStrategyProfile
   MixedStrategyProfile.game
   MixedStrategyProfile.mixed_strategies
   MixedStrategyProfile.__iter__
   MixedStrategyProfile.__getitem__
   MixedStrategyProfile.__setitem__
   MixedStrategyProfile.payoff
   MixedStrategyProfile.strategy_value
   MixedStrategyProfile.strategy_regret
   MixedStrategyProfile.player_regret
   MixedStrategyProfile.strategy_value_deriv
   MixedStrategyProfile.max_regret
   MixedStrategyProfile.liap_value
   MixedStrategyProfile.as_behavior
   MixedStrategyProfile.normalize
   MixedStrategyProfile.copy

   MixedStrategy
   MixedStrategy.__iter__
   MixedStrategy.__getitem__
   MixedStrategy.__setitem__


Probability distributions over behavior
.......................................

.. autosummary::
   :toctree: api/

   MixedBehaviorProfile
   MixedBehaviorProfile.game
   MixedBehaviorProfile.mixed_behaviors
   MixedBehaviorProfile.mixed_actions
   MixedBehaviorProfile.__iter__
   MixedBehaviorProfile.__getitem__
   MixedBehaviorProfile.__setitem__
   MixedBehaviorProfile.payoff
   MixedBehaviorProfile.action_value
   MixedBehaviorProfile.action_regret
   MixedBehaviorProfile.infoset_value
   MixedBehaviorProfile.infoset_regret
   MixedBehaviorProfile.node_value
   MixedBehaviorProfile.realiz_prob
   MixedBehaviorProfile.infoset_prob
   MixedBehaviorProfile.belief
   MixedBehaviorProfile.is_defined_at
   MixedBehaviorProfile.agent_max_regret
   MixedBehaviorProfile.agent_liap_value
   MixedBehaviorProfile.max_regret
   MixedBehaviorProfile.liap_value
   MixedBehaviorProfile.as_strategy
   MixedBehaviorProfile.normalize
   MixedBehaviorProfile.copy

   MixedBehavior
   MixedBehavior.mixed_actions
   MixedBehavior.__iter__
   MixedBehavior.__getitem__
   MixedBehavior.__setitem__

   MixedAction
   MixedAction.__iter__
   MixedAction.__getitem__
   MixedAction.__setitem__


Computation on supports
~~~~~~~~~~~~~~~~~~~~~~~

.. currentmodule:: pygambit.supports

.. autosummary::
   :toctree: api/

   undominated_strategies_solve


Computation of Nash equilibria
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. currentmodule:: pygambit.nash

.. autosummary::
   :toctree: api/

   NashComputationResult
   enumpure_solve
   enumpure_agent_solve
   enummixed_solve
   enumpoly_solve
   lp_solve
   lcp_solve
   liap_solve
   liap_agent_solve
   logit_solve
   simpdiv_solve
   ipa_solve
   gnm_solve


Computation of quantal response equilibria
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. currentmodule:: pygambit.qre

.. autosummary::
   :toctree: api/

   logit_solve_branch
   logit_solve_lambda
   logit_estimate
   LogitQREMixedStrategyFitResult
   LogitQREMixedBehaviorFitResult


Game catalog
~~~~~~~~~~~~

.. currentmodule:: pygambit.catalog

.. autosummary::
   :toctree: api/

   games
   Artist1
   Artist2
   Badgame1
   Badgame2
   Bagwell
   Bayes1a
   Bayes2a
   Bcp2
   Bcp3
   Bcp4
   Bhg1
   Bhg2
   Bhg3
   Bhg4
   Bhg5
   Caro2
   Cent2
   Cent2NFG
   Cent3
   Cent4
   Cent6
   Centcs10
   Centcs6
   Condjury
   Coord2
   Coord2NFG
   Coord2ts
   Coord3
   Coord333
   Coord3NFG
   Coord4
   Coord4NFG
   Cross
   Cs
   Csg1
   Csg2
   Csg3
   Csg4
   Deg1
   Deg2
   E01
   E01NFG
   E02
   E02NFG
   E03
   E04
   E04NFG
   E05
   E06
   E07
   E07NFG
   E08
   E09
   E10
   E10a
   E13
   E16
   E17
   E18
   G1
   G1NFG
   G2
   G2NFG
   G3
   G3NFG
   Game2s2x2x2
   Game2smp
   Game2x2
   Game2x2a
   Game2x2const
   Game2x2x2
   Game2x2x2NFG
   Game2x2x2_nau
   Game2x2x2x2
   Game2x2x2x2x2
   Game3x3x3
   Game4cards
   Game5x4x3
   Game8x2x2
   Game8x8
   Holdout
   Hs1
   Jury_mr
   Jury_un
   Km1
   Km2
   Km3
   Km6
   Loopback
   Mixdom
   Mixdom2
   Montyhal
   My_2_1
   My_2_4
   My_2_8
   My_3_3a
   My_3_3b
   My_3_3c
   My_3_3d
   My_3_3e
   My_3_4
   Myerson
   Myerson_fig_4_2
   Nim
   Nim7
   OneShotTrust
   Oneill
   Palf
   Palf2
   Palf3
   Pbride
   Perfect1
   Perfect2
   Perfect3
   Poker
   Poker2
   PokerNFG
   PrisonersDilemma
   Pvw
   Pvw2
   Sh3
   Sh3NFG
   Spence
   Stengel
   Sww1
   Sww1NFG
   Sww2
   Sww3
   Tim
   Todd1
   Todd2
   Todd3
   Ttt
   Vd
   VdNFG
   W_ex1
   W_ex2
   Wilson1
   Wink3
   Winkels
   Work1
   Work2
   Work3
   Yamamoto
   Zero

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
   Game.reveal
   Game.set_chance_probs


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
   Game.comment
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
   Node.prior_action
   Node.prior_sibling
   Node.next_sibling
   Node.infoset
   Node.player
   Node.is_successor_of

.. autosummary::

   :toctree: api/

   Infoset.label
   Infoset.game
   Infoset.is_chance
   Infoset.player
   Infoset.actions
   Infoset.members
   Infoset.precedes

.. autosummary::

   :toctree: api/

   Action.label
   Action.infoset
   Action.precedes
   Action.prob

.. autosummary::

   :toctree: api/

   Strategy.label
   Strategy.game
   Strategy.player
   Strategy.number


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
   enummixed_solve
   enumpoly_solve
   lp_solve
   lcp_solve
   liap_solve
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

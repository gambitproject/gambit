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
   ExtensiveGame
   StrategicGame
   ActionGraphGame
   BayesianActionGraphGame
   History
   TreeLayout
   TreeLayoutCoordinates


Creating, reading, and writing games
....................................

.. autosummary::
   :toctree: api/

   read_gbt
   read_efg
   read_nfg
   read_agg
   read_bagg

   StrategicGame.from_arrays
   Game.to_arrays
   StrategicGame.from_dict
   Game.to_efg
   Game.to_nfg
   Game.to_html
   Game.to_latex


Computing a tree layout for graphical display
..............................................

.. autosummary::
   :toctree: api/

   layout_tree


Transforming game trees
.......................

.. autosummary::
   :toctree: api/

   ExtensiveGame.append_move
   ExtensiveGame.append_infoset
   ExtensiveGame.append_event
   ExtensiveGame.insert_move
   ExtensiveGame.insert_infoset
   ExtensiveGame.insert_event
   ExtensiveGame.copy_tree
   ExtensiveGame.move_tree
   ExtensiveGame.delete_parent
   ExtensiveGame.delete_tree


Transforming game information structure
.......................................

.. autosummary::
   :toctree: api/

   ExtensiveGame.make_infoset
   ExtensiveGame.make_event
   ExtensiveGame.relabel_actions
   ExtensiveGame.set_move_actions
   ExtensiveGame.set_event_actions


Transforming game components
............................

.. autosummary::
   :toctree: api/

   Game.relabel_players
   ExtensiveGame.set_players
   StrategicGame.set_players
   StrategicGame.relabel_strategies
   StrategicGame.set_strategies
   ExtensiveGame.make_outcome
   StrategicGame.make_outcome
   ExtensiveGame.make_outcome_null
   StrategicGame.make_outcome_null
   ExtensiveGame.relabel_outcomes
   StrategicGame.relabel_outcomes
   Game.set_outcome_payoffs


Information about the game
..........................

.. autosummary::
   :toctree: api/

   Game.title
   Game.description
   Game.is_const_sum
   Game.is_perfect_recall
   Game.has_perfect_recall
   Game.players
   Game.min_payoff
   Game.max_payoff
   Game.get_min_payoff
   Game.get_max_payoff
   ExtensiveGame.get_infosets
   ExtensiveGame.get_events
   Game.get_strategies
   Game.contingencies
   ExtensiveGame.get_outcome
   StrategicGame.get_outcome
   Game.get_outcomes
   Game.get_outcome_payoffs
   Game.get_payoffs
   ExtensiveGame.get_subgame_roots
   ExtensiveGame.get_minimal_subgame
   ExtensiveGame.get_strategy_unreachable


Player behavior
...............

.. autosummary::
   :toctree: api/

   Game.mixed_strategy_profile
   Game.random_strategy_profile
   ExtensiveGame.get_behavior
   ExtensiveGame.mixed_behavior_profile
   ExtensiveGame.random_behavior_profile
   Game.strategy_support_profile
   ExtensiveGame.behavior_support_profile


Representation of strategic behavior
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. currentmodule:: pygambit.gambit


Computed quantities
....................

.. autosummary::
   :toctree: api/

   PlayerIndexedVector
   PlayerIndexedVector.__iter__
   PlayerIndexedVector.__getitem__

   StrategyIndexedVector
   StrategyIndexedVector.__iter__
   StrategyIndexedVector.__getitem__

   HistoryIndexedVector
   HistoryIndexedVector.__iter__
   HistoryIndexedVector.__getitem__

   InfosetIndexedVector
   InfosetIndexedVector.__iter__
   InfosetIndexedVector.__getitem__


Probability distributions over strategies
.........................................

.. autosummary::
   :toctree: api/

   MixedStrategyProfile
   MixedStrategyProfile.game
   MixedStrategyProfile.__iter__
   MixedStrategyProfile.__getitem__
   MixedStrategyProfile.__setitem__
   MixedStrategyProfile.set_mixed_strategy
   MixedStrategyProfile.payoffs
   MixedStrategyProfile.strategy_values
   MixedStrategyProfile.strategy_regrets
   MixedStrategyProfile.player_regrets
   MixedStrategyProfile.max_regret
   MixedStrategyProfile.liap_value
   MixedStrategyProfile.as_behavior
   MixedStrategyProfile.as_float
   MixedStrategyProfile.normalize
   MixedStrategyProfile.copy

   MixedStrategy
   MixedStrategy.__iter__
   MixedStrategy.__getitem__

   PayoffVector
   PlayerRegretVector
   StrategyValueVector
   StrategyRegretVector
   StrategyValuesVector
   StrategyRegretsVector


Probability distributions over behavior
.......................................

.. autosummary::
   :toctree: api/

   MixedBehaviorProfile
   MixedBehaviorProfile.game
   MixedBehaviorProfile.__iter__
   MixedBehaviorProfile.__getitem__
   MixedBehaviorProfile.__setitem__
   MixedBehaviorProfile.set_mixed_action
   MixedBehaviorProfile.payoffs
   MixedBehaviorProfile.action_values
   MixedBehaviorProfile.action_regrets
   MixedBehaviorProfile.infoset_values
   MixedBehaviorProfile.infoset_regrets
   MixedBehaviorProfile.history_values
   MixedBehaviorProfile.realiz_probs
   MixedBehaviorProfile.infoset_probs
   MixedBehaviorProfile.event_probs
   MixedBehaviorProfile.beliefs
   MixedBehaviorProfile.agent_max_regret
   MixedBehaviorProfile.agent_liap_value
   MixedBehaviorProfile.max_regret
   MixedBehaviorProfile.liap_value
   MixedBehaviorProfile.as_strategy
   MixedBehaviorProfile.as_float
   MixedBehaviorProfile.normalize
   MixedBehaviorProfile.copy

   MixedBehavior
   MixedBehavior.__iter__
   MixedBehavior.__getitem__

   MixedAction
   MixedAction.__iter__
   MixedAction.__getitem__

   InfosetValueVector
   InfosetRegretVector
   InfosetProbVector
   EventProbVector
   ActionValueVector
   ActionRegretVector
   ActionValuesVector
   ActionRegretsVector
   RealizProbVector
   BeliefVector
   HistoryValueVector
   HistoryValuesVector


Representation of supports
~~~~~~~~~~~~~~~~~~~~~~~~~~

.. currentmodule:: pygambit.gambit

Subsets of strategies
......................

.. autosummary::
   :toctree: api/

   StrategySupportProfile
   StrategySupportProfile.game
   StrategySupportProfile.__iter__
   StrategySupportProfile.__getitem__
   StrategySupportProfile.__setitem__
   StrategySupportProfile.copy
   StrategySupportProfile.restrict
   StrategySupportProfile.is_dominated

   StrategySupport
   StrategySupport.player
   StrategySupport.__iter__
   StrategySupport.__contains__

Subsets of actions
...................

.. autosummary::
   :toctree: api/

   BehaviorSupportProfile
   BehaviorSupportProfile.game
   BehaviorSupportProfile.__iter__
   BehaviorSupportProfile.__getitem__
   BehaviorSupportProfile.__setitem__
   BehaviorSupportProfile.copy
   BehaviorSupportProfile.is_infoset_reachable

   BehaviorSupport
   BehaviorSupport.player
   BehaviorSupport.__iter__
   BehaviorSupport.__getitem__

   ActionSupport
   ActionSupport.history
   ActionSupport.__iter__
   ActionSupport.__contains__


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
   hp_solve

Each of these returns its own result type, documented below:

.. autosummary::
   :toctree: api/

   NashResultBase
   EnumPureResult
   EnumMixedResult
   EnumPolyResult
   LpResult
   LcpStrategyResult
   LcpBehaviorResult
   LiapResult
   LogitResult
   SimpdivResult
   IPAResult
   GNMResult
   HPResult


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


.. _pygambit-catalog:

Catalog of games
~~~~~~~~~~~~~~~~

.. currentmodule:: pygambit.catalog

.. autosummary::
   :toctree: api/

   load
   generate_openspiel
   generate_gamut
   gamut_games
   games

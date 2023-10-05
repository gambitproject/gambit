.. _file-formats-nfg:

The strategic game (.nfg) file format, payoff version
=====================================================

This file format defines a strategic N-player game. In this version,
the payoffs are listed in a tabular format. See the next section for a
version of this format in which outcomes can be used to identify an
equivalence among multiple strategy profiles.



A sample file
-------------

This is a sample file illustrating the general format of the file.
This file is distributed in the Gambit distribution under the name
e02.nfg::

    NFG 1 R "Selten (IJGT, 75), Figure 2, normal form"
    { "Player 1" "Player 2" } { 3 2 }

    1 1 0 2 0 2 1 1 0 3 2 0


Structure of the prologue
-------------------------

The prologue is constructed as follows. The file begins with the token
NFG , identifying it as a strategic gamefile. Next is the digit 1 ;
this digit is a version number. Since only version 1 files have been
supported for more than a decade, all files have a 1 in this position.
Next comes the letter R . The letter R used to distinguish files which
had rational numbers for numerical data; this distinction is obsolete,
so all new files should have R in this position.

The prologue continues with the title of the game. Following the title
is a list of the names of the players defined in the game. This list
follows the convention found elsewhere in the file of being surrounded
by curly braces and delimited by whitespace (but not commas,
semicolons, or any other character). The order of the players is
significant; the first entry in the list will be numbered as player 1,
the second entry as player 2, and so forth.

Following the list of players is a list of positive integers. This
list specifies the number of strategies available to each player,
given in the same order as the players are listed in the list of
players.

The prologue concludes with an optional text comment field.


Structure of the body (list of payoffs)
---------------------------------------

The body of the format lists the payoffs in the game. This is a "flat"
list, not surrounded by braces or other punctuation.

The assignment of the numeric data in this list to the entries in the
strategic game table proceeds as follows. The list begins with the
strategy profile in which each player plays their first strategy. The
payoffs to all players in this contingency are listed in the same
order as the players are given in the prologue. This, in the example
file, the first two payoff entries are 1 1 , which means, when both
players play their first strategy, player 1 receives a payoff of 1,
and player 2 receives a payoff of 1.

Next, the strategy of the first player is incremented. Thus, player
1's strategy is incremented to his second strategy. In this case, when
player 1 plays his second strategy and player 2 his first strategy,
the payoffs are 0 2 : a payoff of 0 to player 1 and a payoff of 2 to
player 2.

Now the strategy of the first player is again incremented. Thus, the
first player is playing his third strategy, and the second player his
first strategy; the payoffs are again 0 2 .

Now, the strategy of the first player is incremented yet again. But,
the first player was already playing strategy number 3 of 3. Thus, his
strategy now "rolls over" to 1, and the strategy of the second player
increments to 2. Then, the next entries 1 1 correspond to the payoffs
of player 1 and player 2, respectively, in the case where player 1
plays his second strategy, and player 2 his first strategy.

In general, the ordering of contingencies is done in the same way that
we count: incrementing the least-significant digit place in the number
first, and then incrementing more significant digit places in the
number as the lower ones "roll over." The only differences are that
the counting starts with the digit 1, instead of 0, and that the
"base" used for each digit is not 10, but instead is the number of
strategies that player has in the game.


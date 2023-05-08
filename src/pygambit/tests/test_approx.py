from fractions import Fraction

import numpy as np

from pygambit import Game
import pygambit.approx


class TestKSSolver:
    """Tests of Kontogiannis-Spirakis solver."""
    def test_one(self):
        """Test that Kontogiannis-Spirakis finds exact Nash equilibrium with identity matrices."""
        for n in range(2, 20):
            A = [[1 if i == j else 0
                 for j in range(n)] for i in range(n)]
            g = Game.from_arrays(A, A)
            result = pygambit.approx.solve_wellsupported(g)
            assert result.epsilon_wsne == 0

    def test_two(self):
        """Test that Kontogiannis-Spirakis finds exact Nash equilibrium with zero-sum games."""
        for n in range(2, 20):
            A = np.array([np.random.choice(2, 10) for _ in range(10)])
            B = 1 - A
            A = [[1 if A[i, j] == 1 else 0
                  for j in range(A.shape[1])] for i in range(A.shape[0])]
            B = [[1 if B[i, j] == 1 else 0
                  for j in range(B.shape[1])] for i in range(B.shape[0])]
            g = Game.from_arrays(A, B)
            result = pygambit.approx.solve_wellsupported(g)
            assert result.epsilon_wsne == 0

    def test_three(self):
        A = [["1/3", 1], [1, "1/3"], [0, 0]]
        B = [[1, "1/3"], ["1/3", 1], [0, 0]]
        g = Game.from_arrays(A, B)
        result = pygambit.approx.solve_wellsupported(g)
        assert result.epsilon_wsne == Fraction(2, 3)

    def test_four(self):
        """Test to compute 2/3-WSNE in a 2x2 game."""
        A = [["1/3", 1], [0, 0]]
        B = [[1, "1/3"], [0, 0]]
        g = Game.from_arrays(A, B)
        result = pygambit.approx.solve_wellsupported(g)
        assert result.epsilon_wsne == Fraction(2, 3)

    def test_five(self):
        a = Fraction(1, 3)
        b = Fraction(1, 1)
        A = np.array([
            [a, b, 0, a, b, a, b, a, b, a, b, a, b],
            [0, a, b, a, b, a, b, a, b, a, b, a, b],
            [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]])
        B = np.array([
            [b, a, 0, b, a, b, a, b, a, b, a, b, a],
            [0, b, a, b, a, b, a, b, a, b, a, b, a],
            [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]])
        g = Game.from_arrays(A, B)
        result = pygambit.approx.solve_wellsupported(g)
        assert result.epsilon_wsne == Fraction(2, 3)

    def test_six(self):
        a = Fraction(1, 3)
        b = Fraction(1, 1)
        A = np.array([[a, a, b, a, b],
                      [b, a, a, a, b],
                      [a, b, a, a, b],
                      [a, a, a, b, b],
                      [0, 0, 0, 0, 0]])
        B = np.array([[b, b, a, b, a],
                      [a, b, b, b, a],
                      [b, a, b, b, a],
                      [b, b, b, a, a],
                      [0, 0, 0, 0, 0]])
        g = Game.from_arrays(A, B)
        result = pygambit.approx.solve_wellsupported(g)
        assert result.epsilon_wsne == Fraction(2, 3)

    @staticmethod
    def generate_matrix(n: int) -> Game:
        """Generate an n x n game which has a 2/3-WSNE approximation guarantee
        by the KS algorithm.
        """
        A = []
        B = []
        for i in range(n - 1):
            A.append(["1/3" if i == j else 1 for j in range(n - 1)] + ["1/3"])
            B.append([1 if i == j else "1/3" for j in range(n - 1)] + [1])
        A.append([0] * n)
        B.append([0] * n)
        return Game.from_arrays(A, B)

    def test_seven(self):
        """Test Kontogiannis-Spirakis on class of games with 2/3-WSNE."""
        for i in range(3, 20):
            g = self.generate_matrix(i)
            result = pygambit.approx.solve_wellsupported(g)
            assert result.epsilon_wsne == Fraction(2, 3)

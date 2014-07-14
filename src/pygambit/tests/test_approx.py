from fractions import Fraction
import fractions
import numpy as np
from gambit import Game
import gambit.approx

# Converts a numpy array to gambit.Game might be useful for other purposes
def numpy_to_game(A, B):
    g = Game.new_table(A.shape)
    for i in range(A.shape[0]):
        for j in range(A.shape[1]):
            g[i, j][0] = A[i, j]
            g[i, j][1] = B[i, j]

    return g

class TestKSSolver(object):
    "Tests of Kontogiannis-Spirakis solver."
    def setUp(self):
        self.solver = gambit.approx.InternalKSSolver()

    def tearDown(self):
        pass
        
    def test_one(self):
        "Test that Kontogiannis-Spirakis finds exact Nash equilibrium with identity matrices."
        for n in range(2, 20) :
            A = np.eye(n)
            A = [[Fraction(1, 1) if i == j else Fraction (0, 1) for j in range(n)] for i in range(n)]
            A = np.array(A)
            B = A
            g = numpy_to_game(A, B)
            a = self.solver.solve(g, True)
            assert a[0].epsWSNE == 0

    def test_two(self):
        "Test that Kontogiannis-Spirakis finds exact Nash equilibrium with zero-sum games."
        for n in range(2, 20) :
            A = np.array([np.random.choice(2, 10) for i in range(10)])
            B = 1 - A
            A = [[Fraction(1, 1) if A[i,j] == 1 else Fraction (0, 1) for j in range(A.shape[1])] for i in range(A.shape[0])]
            B = [[Fraction(1, 1) if B[i,j] == 1 else Fraction (0, 1) for j in range(B.shape[1])] for i in range(B.shape[0])]
            A = np.array(A)
            B = np.array(B)
            g = numpy_to_game(A, B)
            a = self.solver.solve(g, True)
            assert a[0].epsWSNE == 0

    def test_three(self):
        "Test to replicate 2/3-WSNE from Kontogiannis-Spirakis paper."
        a = Fraction(1, 3)
        b = Fraction(1, 1)
        A = np.array([[a, b], [b, a], [0, 0]])
        B = np.array([[b, a], [a, b], [0, 0]])
        g = numpy_to_game(A, B)
        a = self.solver.solve(g, True)
        assert a[0].epsWSNE == Fraction(2, 3)

    def test_four(self):
        "Test to compute 2/3-WSNE in a 2x2 game."
        a = Fraction(1, 3)
        b = Fraction(1, 1)
        A = np.array([[a, b], [0, 0]])
        B = np.array([[b, a], [0, 0]])
        g = numpy_to_game(A, B)
        a = self.solver.solve(g, True)
        assert a[0].epsWSNE == Fraction(2, 3)

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
        g = numpy_to_game(A, B)
        a = self.solver.solve(g, True)
        assert a[0].epsWSNE == Fraction(2, 3)

    def test_six(self):
        a = Fraction(1, 3)
        b = Fraction(1, 1)
        A = np.array([[a, a, b, a, b], [b, a, a, a, b], [a, b, a, a, b], [a, a, a, b, b], [0, 0, 0, 0, 0]])
        B = np.array([[b, b, a, b, a], [a, b, b, b, a], [b, a, b, b, a], [b, b, b, a, a], [0, 0, 0, 0, 0]])
        g = numpy_to_game(A, B)
        a = self.solver.solve(g, True)
        assert a[0].epsWSNE == Fraction(2, 3)

    #Generates an nxn game which has a 2/3-WSNE approximation guarantee by the KS-algorithm
    def generate_matrix(self, n):
        a = Fraction(1, 3)
        b = Fraction(1, 1)
        A = []
        B = []
        for i in range(n - 1):
            x = [a if i == j else b for j in range(n - 1)]
            x.append(a)
            A.append(x)
            y = [b if i == j else a for j in range(n - 1)]
            y.append(b)
            B.append(y)

        A.append([ 0 for i in range(n)])
        B.append([ 0 for i in range(n)])
        A = np.array(A)
        B = np.array(B)

        return (A, B)

    # Case seven: Class of games with 2/3-WSNE based on test cases 3 and 4
    def test_seven(self):
        "Test Kontogiannis-Spirakis on class of games with 2/3-WSNE."
        for i in range(3, 20):
            A, B = self.generate_matrix(i)
            g = numpy_to_game(A, B)
            a = self.solver.solve(g, True)
            assert a[0].epsWSNE == Fraction(2, 3)

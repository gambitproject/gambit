"""
Algorithm for finding the set of correlated equilibria for
two player games in normal form.

Author: Chris Ratcliff <c.j.ratcliff@gmail.com>, October 2012.

"""

import math
import numpy
import pulp

class CorrelatedEquilibriumPayoffs(object):
    def __init__(self, game, vertices, edges):
        self._game = game
        self._vertices = vertices
        self._edges = edges

    @property
    def game(self):        return self._game
    @property
    def vertices(self):    return self._vertices
    @property
    def edges(self):       return self._edges

    def visualize(self):
        print "Vertices:"
        for i in self.vertices:
            print i
       
        print "\nEdges:" 
        for i in self.edges:
            print i
        if len(self.edges) == 0:
            print "None"

        import pylab
        
        fig = pylab.figure()
        ax = fig.add_subplot(111)

        x_coords = [i[0] for i in self.vertices]
        y_coords = [i[1] for i in self.vertices]
    
        x_offset = (max(x_coords) - min(x_coords))/5
        y_offset = (max(y_coords) - min(y_coords))/5

        ax.set_xlim(min(x_coords)-x_offset-0.01, max(x_coords)+x_offset+0.01)
        ax.set_ylim(min(y_coords)-y_offset-0.01, max(y_coords)+y_offset+0.01)
    
        # Add labels
        xtext = ax.set_xlabel('Utility for player 1', fontsize=14)
        ytext = ax.set_ylabel('Utility for player 2', fontsize=14)
    
        fig.suptitle('Coordinates shown are rounded to 2dp. The console outputs the unrounded values', fontsize=12)    
        vertices = [(round(i,2), round(j,2)) for i,j in self.vertices]
    
        for x,y in vertices:
            ax.text(x+x_offset/10, y+y_offset/10, str(x)+','+str(y), fontsize=14)    

        # Ensure points are drawn in the correct order
        points = order_vertices(self.edges)
        if points is None:
            points = self.edges

        pylab.plot(*zip(*points), marker='o', markersize=5)
        pylab.show()

INF = 1e+10


def contingency_to_index((p1_strategy,p2_strategy),p2_num_strats):
    return (p1_strategy*p2_num_strats) + p2_strategy
    
    
def probs_to_utilities(optx,game):
    return (numpy.dot(optx,
                      numpy.array([float(game[c][0]) for c in game.contingencies])),
            numpy.dot(optx,
                      numpy.array([float(game[c][1]) for c in game.contingencies])))
                      


def bearing_to_probs(bearing,game):
    util_ratio = math.tan(math.radians(bearing))

    u1_weight = 1
    u2_weight = util_ratio

    if 90 < bearing < 270:
        u1_weight *= -1
        u2_weight *= -1
    elif bearing == 90:
        u1_weight = 0
        u2_weight = 1
    elif bearing == 270:
        u1_weight = 0
        u2_weight = -1
        
    u1_equation = u1_weight * numpy.array([float(game[c][0]) for c in game.contingencies])
    u2_equation = u2_weight * numpy.array([float(game[c][1]) for c in game.contingencies]) 
    
    probs = u1_equation + u2_equation
    probs = remove_infinities(probs)

    return probs


def is_between(a,b,c):
    "Returns True iff point c is between points a and b"
    EPSILON = 0.000001
    cross_product = (c[1]-a[1])*(b[0]-a[0]) - (c[0]-a[0])*(b[1]-a[1])
    
    if abs(cross_product) > EPSILON:
        return False
    
    dot_product = (c[0]-a[0])*(b[0]-a[0]) + (c[1]-a[1])*(b[1]-a[1]);
    if dot_product < 0: 
        return False
    
    squared_length_ba = (b[0]-a[0])*(b[0]-a[0]) + (b[1]-a[1])*(b[1]-a[1])
    if dot_product > squared_length_ba:
        return False

    return True

def remove_extra_edges(vertices,edges):
    vertices = set(vertices)
    edges_to_remove = set()
    
    for e in edges:
        for v in (vertices - set([e[0],e[1]])):
            if is_between(e[0],e[1],v):
                edges_to_remove.add(e)
                break
            
    return edges - edges_to_remove


def is_closed_shape(vertices, edges):
    "A shape is closed if each vertex is in at least 2 edges."
    for v in vertices:
        count = 0   
        for e in edges:
            count += (v in e)
        if count < 2:
            return False
              
    return True

def remove_infinities(a):
    for i in a:
        if abs(i) >= INF:
            a = [convert(j) for j in a]
            break  
    return a


def convert(x):
    if x >= INF:
        return 1
    elif x <= -INF:
        return -1
    else:
        return 0
    

def order_vertices(edges):
    if len(edges) <= 1:
        return None
        
    edges = list(edges)
    vertices = [edges[0][0],edges[0][1]]
    edges = edges[1:]

    i = vertices[1]

    while i != vertices[0]:
        for j in edges:
            a,b = j
            if a == i:
                vertices.append(b)
                i = b
                edges.remove(j)
                break
            elif b == i:
                vertices.append(a)
                i = a
                edges.remove(j)
                break  

    return vertices

def add_constraints(game,problem,str_vars,var_dict):
    for p in range(2): # player
        for s in range(len(game.players[p].strategies)):
            for alt_s in range(len(game.players[p].strategies)):
                if s == alt_s:
                    continue
                
                constraint = [None for i in range(len(game.contingencies))]
                
                for t in range(len(game.players[not p].strategies)):
                    if p == 0:
                        index = contingency_to_index((s,t),len(game.players[1].strategies))
                        assert constraint[index] is None
                        constraint[index] = float(game[s,t][p]) - float(game[alt_s,t][p])
                    else:
                        index = contingency_to_index((t,s),len(game.players[1].strategies))
                        assert constraint[index] is None
                        constraint[index] = float(game[t,s][p]) - float(game[t,alt_s][p])
                
                constraint = [0 if i is None else i for i in constraint]
                problem += pulp.lpSum([var_dict[str_vars[i]]*constraint[i] for i in range(len(game.contingencies))]) >= 0
                
    return problem


def reorder_probs(prob_vars,var_dict_swapped):
    optx = [None for i in prob_vars]
    for i in prob_vars:
        index = int(var_dict_swapped[i.name]) - 1
        optx[index] = i.varValue
    return optx
    

def dydx(v,v2):
    return (v[1] - v2[1],  v[0] - v2[0])
 
        
def find_polygon(game):
    RND = 10                       

    # Set up for linear programming
    problem = pulp.LpProblem("Correlated equilibria", pulp.LpMaximize)
    
    str_vars = [str(i) for i in range(1,len(game.contingencies)+1)]

    # Third and fourth arguments are the lower and upper bounds for the variable, respectively
    var_dict = pulp.LpVariable.dicts("Probability", str_vars, 0, 1,
                                     pulp.LpContinuous)
    
    var_names = map(str, var_dict.values())
    var_dict_swapped = dict(zip(var_names,var_dict.keys()))
    
    # Probabilities must sum to 1
    problem += pulp.lpSum([var_dict[i] for i in str_vars]) == 1
        
    for i in str_vars:
        problem += var_dict[i] >= 0
        problem += var_dict[i] <= 1
     
    problem = add_constraints(game,problem,str_vars,var_dict)

    # Find the vertices
    
    vertices = []
    edges = set()
    past_brgs = set()
    current_brgs = set(range(0,360,90))
    
    # The probability vector found during the maximization that derived the vertex
    vertex_probs = {}
    # The bearing whose direction was maximised to give the vertex
    vertex_brgs = {}
    edge_ortho_brgs = {}
    
    done = False
    it = 0
     
    u1 = set([float(game[c][0]) for c in game.contingencies])
    u2 = set([float(game[c][1]) for c in game.contingencies])
    
    if len(u1) == len(u2) == 1:
        return [(list(u1)[0],list(u2)[0])], set()

    while not done:
        for brg in current_brgs:
            if done:
                break

            # Skip bearings for which no additional vertices can exist
            probs = bearing_to_probs(brg,game)
            probs_dict = {str_vars[i]:probs[i] for i in range(len(probs))}
            
            # The objective function
            problem += pulp.lpSum([var_dict[i]*probs_dict[i] for i in str_vars])
            
            problem.solve(pulp.GLPK(msg = 0))
            
            optx = reorder_probs(problem.variables(),var_dict_swapped)
                
            # Calvo-Armengol (2006) - The set of CE is non-empty and compact
            assert pulp.LpStatus[problem.status] == "Optimal"

            u1,u2 = probs_to_utilities(optx,game)
            u1 = round(u1,RND)
            u2 = round(u2,RND)
            v = (u1,u2)
           
            if not v in vertices:
                vertices.append(v)
                vertex_probs[v] = optx
                vertex_brgs[v] = math.radians(brg)           
            else:
                continue
            
            ##### Find edges #####
            for v2 in vertices:              
                if v2 == v:
                    continue
                
                # Calculate the vector orthogonal to the potential edge
                (dy,dx) = dydx(v,v2)

                brg = math.atan2(dy,dx) %(2*math.pi)
                ortho_brg = (brg + (math.pi/2)) %(2*math.pi)
               
                edges_exist = [False, False]
                
                # Try both possible orthogonal bearings
                for angle in [0,math.pi]:
                    ortho_brg = (ortho_brg+angle) %(2*math.pi)
                    
                    probs = bearing_to_probs(math.degrees(ortho_brg),game)
                    probs_dict = {str_vars[i]:probs[i] for i in range(len(probs))}
                    
                    # The objective function
                    problem += pulp.lpSum([var_dict[i]*probs_dict[i] for i in str_vars])
                                   
                    problem.solve(pulp.GLPK(msg = 0))
                    optx = reorder_probs(problem.variables(),var_dict_swapped)
                    ortho_zmax = pulp.value(problem.objective)
                    
                    # Calvo-Armengol (2006) - The set of CE is non-empty and compact
                    assert pulp.LpStatus[problem.status] == "Optimal"
                    
                    v_probs = vertex_probs[v]
                    v2_probs = vertex_probs[v2]
                                
                    v_zmax = numpy.sum(numpy.multiply(probs,v_probs))
                    v2_zmax = numpy.sum(numpy.multiply(probs,v2_probs))
                                        
                    # Check all bearings are maximized to give the same number
                    # There is an edge if ortho_brg is maximised at both v and v2
                    if len(set(numpy.around([ortho_zmax,v_zmax,v2_zmax], decimals=RND))) == 1:
                        edge_ortho_brgs[(v,v2)] = ortho_brg
                        edges.add((v,v2))
                        edges_exist[int(math.sin(angle/2))] = True
                        
                    # Account for 1D sets of correlated equilibria
                    if edges_exist == [True,True]:
                        done = True
                        break
                        
            if is_closed_shape(vertices,edges):
                done = True
                break

        # Account for 0D sets of CE
        if len(current_brgs) == 4 and len(vertices) == 1:
            break

        # Double the number of linear combinations of utilities to be maximised    
        interval = 90/float(2**it)
        it += 1
        
        past_brgs = past_brgs | current_brgs
        current_brgs = set([interval*i for i in range(int(360/interval))])
        current_brgs -= past_brgs     

    return vertices, edges


def compute_correlated_payoffs(game):
    """
    Compute the set of correlated equilibrium payoffs for the game.
    """
    if len(game.players) != 2:
        raise NotImplementedError("Only implemented for two-player games.")
    
    vertices, edges = find_polygon(game)
    edges = remove_extra_edges(vertices, edges)
    return CorrelatedEquilibriumPayoffs(game, vertices, edges)


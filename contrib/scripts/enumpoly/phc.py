

###########################################################################
# A Python interface to PHCPACK
###########################################################################

# This takes the entire output of a PHCPACK run, and returns a list of
# solutions.
# Each solution is a Python dictionary with the following entries,
# each containing one of the (corresponding) data from a solution:
# * "startresidual"
# * "iterations"
# * "result"
# * "t"
# * "m"
# * "err"
# * "rco"
# * "res"
#
# There are two other dictionary entries of interest:
# * "type": the text string PHCPACK emits describing the output
#           (e.g., "no solution", "real regular", etc.)
# * "vars": a dictionary whose keys are the variable names,
#           and whose values are the solution values, represented
#           using the Python `complex` type.
def ProcessPHCOutput(output):
    # This initial phase identifies the part of the output containing
    # the solutions.  It is complicated slightly because (as of Jan 2007)
    # PHCPACK's blackbox solver uses special-case code for linear and
    # sparse systems (this per email from Jan Verschelde).  When these
    # methods are in effect, the output format is slightly different.

    startsol = output.find("THE SOLUTIONS :\n\n")

    if startsol == -1:
        startsol = output.find("THE SOLUTIONS :\n")
        
    solns = output[startsol:]

    firstequals = solns.find("solution")
    firstcut = solns[firstequals:]

    secondequals = firstcut.find("=====")
    if secondequals >= 0:
        secondcut = firstcut[:secondequals]
    else:
        secondequals = firstcut.find("TIMING")
        secondcut = firstcut[:secondequals]

    solutions = [ ]

    for line in secondcut.split("\n"):
        tokens = [ x.strip(" ")
                   for x in line.split(" ")
                   if x != "" and not x.isspace() ]

        if tokens == []: continue

        if tokens[0] == "solution":
            if len(tokens) == 3:
                # This is a solution that didn't involve iteration
                solutions.append( { "vars":   { } } )
            else:
                solutions.append({ "startresidual": float(tokens[6]),
                                   "iterations":    int(tokens[9]),
                                   "result":        tokens[10],
                                   "vars":          { } })
        elif tokens[0] == "t":
            solutions[-1]["t"] = complex(float(tokens[2]),
                                         float(tokens[3]))
        elif tokens[0] == "m":
            solutions[-1]["m"] = int(tokens[2])
        elif tokens[0] == "the":
            pass
        elif tokens[0] == "==":
            solutions[-1]["err"] = float(tokens[3])
            solutions[-1]["rco"] = float(tokens[7])
            solutions[-1]["res"] = float(tokens[11])
            try:
                solutions[-1]["type"] = " ".join([tokens[13], tokens[14]])
            except IndexError:
                # Some solutions don't have type information
                pass
        else:
            # This is a solution line
            solutions[-1]["vars"][tokens[0]] = complex(float(tokens[2]),
                                                       float(tokens[3]))
            
    return solutions


import os

# This sets up and processes a PHC run.
# 'phcpath' is the full path to PHC on the system;
# 'equations' is a text string containing the system in PHC's input format
# Returns a list of the solutions
# (see the comments on ProcessPHCOutput() above for the description of
# each of these solution entries)
def RunPHC(phcpath, filename, equations):
    infilename = filename
    outfilename = filename + ".phc"

    infile = file(infilename, "w")
    infile.write(equations)
    infile.write("\n\n")
    infile.close()

    if os.system(" ".join([phcpath, "-b", infilename, outfilename])) == 0:
        outfile = file(outfilename)
        output = outfile.read()
        outfile.close()
    else:
        # For convenience, print the equation sets that cause problems
        print equations
        os.remove(infilename)
        os.remove(outfilename)
        raise ValueError, "PHC run failed"
    
    os.remove(outfilename)
    os.remove(infilename)

    return ProcessPHCOutput(output)

if __name__ == '__main__':
    # 2x2.nfg, full support
    output = RunPHC("./phc", "foo",
                    "4\n"
                    "2*b1 - b2;\n"
                    "b1 + b2 - 1;\n"
                    "a2 - a1;\n"
                    "a1 + a2 - 1;\n")
    print output

    # 2x2x2.nfg, full support
    output = RunPHC("./phc", "foo",
                    "6\n"
                    "9*b1*c1+3*b2*c2-(3*b1*c2+9*b2*c1);\n"
                    "8*a1*c1+4*a2*c2-(4*a1*c2+8*a2*c1);\n"
                    "12*a1*b1+2*a2*b2-(6*a1*b2+6*a2*b1);\n"
                    "a1+a2-1;\n"
                    "b1+b2-1;\n"
                    "c1+c2-1;\n"
                    )
    print output

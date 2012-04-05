"""
Command-line programs for Gambit
"""

from IPython.Shell import IPShellEmbed

def gambit_shell():
    """
    Start an ipython session after initializing the environment
    """
    import gambit
    import gambit.nash
    import gambit.qre
    
    # Everything in this dictionary will be added to the top-level
    # namespace in the shell.
    ns = { 'gambit': gambit, 'nash': gambit.nash, 'qre': gambit.qre }

    s = IPShellEmbed(['-colors', 'Linux'])
    s(local_ns=ns)

    # Anything that should happen after the session terminates would go here


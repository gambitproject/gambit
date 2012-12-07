"""
Command-line programs for Gambit
"""

from IPython.frontend.terminal.embed import InteractiveShellEmbed

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

    shell = InteractiveShellEmbed()
    shell.user_ns = ns
    shell()

    # Anything that should happen after the session terminates would go here


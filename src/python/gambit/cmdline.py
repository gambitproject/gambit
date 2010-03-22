"""
Command-line programs for Gambit
"""

from IPython.Shell import IPShellEmbed

def gambit_shell():
    """
    Start an ipython session after initializing the environment
    """

    import gambit

    # Everything in this dictionary will be added to the top-level
    # namespace in the shell.
    ns = { 'gambit': gambit }

    s = IPShellEmbed(['-colors', 'Linux'])
    s(local_ns=ns)

    # Anything that should happen after the session terminates would go here


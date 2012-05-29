class MismatchError(ValueError):
    """Raised when an operation between objects in different games 
    is attempted"""
    pass

class UndefinedOperationError(ValueError):
    """Raised when an operation is undefined"""
    pass

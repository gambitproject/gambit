
%typemap(in) const gbtInteger & {
  if (!PyInt_Check($input)) {
    PyErr_SetString(PyExc_ValueError, "Expected an integer");
    return NULL;
  }
  $1 = new gbtInteger(PyInt_AsLong($input));
}

%typemap(in) const gbtArray<int> & {
  int i;
  if (!PySequence_Check($input)) {
    PyErr_SetString(PyExc_ValueError, "Expected a sequence");
    return NULL;
  }
  $1 = new gbtArray<int>(PySequence_Length($input));
  for (i = 0; i < PySequence_Length($input); i++) {
    PyObject *o = PySequence_GetItem($input, i);
    if (PyNumber_Check(o)) {
      (*$1)[i+1] = (int) PyInt_AsLong(o);
    }
    else {
      PyErr_SetString(PyExc_ValueError, "Sequence elements must be numbers");
      return NULL;
    }
  }
}

%typemap(freearg) const gbtArray<int> & {
  if ($1) free($1);
}

%typemap(out) gbtArray<double> {
  int i;
  $result = PyList_New($1.Length());
  for (i = 0; i < $1.Length(); i++) {
    PyObject *o = PyFloat_FromDouble($1[i+1]);
    PyList_SetItem($result, i, o);
  }
}

%typemap(in) const std::string & {
  if (!PyString_Check($input)) {
    PyErr_SetString(PyExc_ValueError, "Expected a string");
    return NULL;
  }
  $1 = new std::string(PyString_AsString($input));
}

%typemap(out) std::string {
  $result = PyString_FromString($1.c_str());
}

//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Python extension module for Nash equilibrium algorithms
//
// This file is part of Gambit
// Copyright (c) 2003, The Gambit Project
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//

#include <Python.h>
#include "base/gnullstatus.h"
#include "nash/efglogit.h"
#include "nash/nfglogit.h"
#include "pygambit.h"

/************************************************************************
 * MODULE METHODS
 ************************************************************************/

PyObject *
gbt_nash_logit(PyObject */*self*/, PyObject *args)
{
  PyObject *support;
  double maxLam, stepSize;
  int fullGraph;

  if (!PyArg_ParseTuple(args, "Oddi",
			&support, &maxLam, &stepSize, &fullGraph)) {
    return NULL;
  }

  if (is_nfsupportobject(support)) {
    gbtNfgNashLogit algorithm;
    algorithm.SetMaxLambda(maxLam);
    algorithm.SetStepSize(stepSize);
    algorithm.SetFullGraph(fullGraph);
    gNullStatus status;
    gList<MixedSolution> solutions = algorithm.Solve(*((nfsupportobject *) support)->m_support, status);

    PyObject *list = PyList_New(0);

    for (int i = 1; i <= solutions.Length(); i++) {
      mixedobject *p = newmixedobject();
      p->m_profile = new MixedSolution(solutions[i]);
      PyList_Append(list, (PyObject *) p);
    }  

    return list;
  }
  else if (is_efsupportobject(support)) {
    gbtEfgNashLogit algorithm;
    algorithm.SetMaxLambda(maxLam);
    algorithm.SetStepSize(stepSize);
    algorithm.SetFullGraph(fullGraph);
    gNullStatus status;
    gList<BehavSolution> solutions = algorithm.Solve(*((efsupportobject *) support)->m_support, status);

    PyObject *list = PyList_New(0);

    for (int i = 1; i <= solutions.Length(); i++) {
      behavobject *p = newbehavobject();
      p->m_profile = new BehavSolution(solutions[i]);
      PyList_Append(list, (PyObject *) p);
    }  

    return list;
  }
  else {
    return NULL;
  }
}

void
initnash(void)
{
}


//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Top-level include for Gambit Python module
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


#ifndef PYGAMBIT_H
#define PYGAMBIT_H

#include "base/base.h"
#include "game/efg.h"
#include "game/nfg.h"
#include "nash/mixedsol.h"
#include "nash/behavsol.h"

/*************************************************************************
 * EFG OBJECT
 *************************************************************************/

typedef struct {
  PyObject_HEAD
  gbtEfgGame *m_efg;
} efgobject;

extern PyTypeObject Efgtype;
#define is_efgobject(v)  ((v)->ob_type == &Efgtype)

efgobject *newefgobject(void);

/*************************************************************************
 * EFPLAYER OBJECT
 *************************************************************************/

typedef struct {
  PyObject_HEAD
  gbtEfgPlayer *m_efplayer;
} efplayerobject;

extern PyTypeObject Efplayertype;
#define is_efplayerobject(v)  ((v)->ob_type == &Efplayertype)

efplayerobject *newefplayerobject(void);

/*************************************************************************
 * INFOSET OBJECT
 *************************************************************************/

typedef struct {
  PyObject_HEAD
  gbtEfgInfoset *m_infoset;
} infosetobject;

extern PyTypeObject Infosettype;
#define is_infosetobject(v)  ((v)->ob_type == &Infosettype)

infosetobject *newinfosetobject(void);

/*************************************************************************
 * ACTION OBJECT
 *************************************************************************/

typedef struct {
  PyObject_HEAD
  gbtEfgAction *m_action;
} actionobject;

extern PyTypeObject Actiontype;
#define is_actionobject(v)  ((v)->ob_type == &Actiontype)

actionobject *newactionobject(void);

/************************************************************************
 * NODE OBJECT
 *************************************************************************/

typedef struct {
  PyObject_HEAD
  gbtEfgNode *m_node;
} nodeobject;

extern PyTypeObject Nodetype;
#define is_nodeobject(v)  ((v)->ob_type == &Nodetype)

nodeobject *newnodeobject(void);

/*************************************************************************
 * EFOUTCOME OBJECT
 *************************************************************************/

typedef struct {
  PyObject_HEAD
  gbtEfgOutcome *m_efoutcome;
} efoutcomeobject;

extern PyTypeObject Efoutcometype;
#define is_efoutcomeobject(v)  ((v)->ob_type == &Efoutcometype)

efoutcomeobject *newefoutcomeobject(void);

/*************************************************************************
 * NFG OBJECT
 *************************************************************************/

typedef struct {
  PyObject_HEAD
  gbtNfgGame *m_nfg;
} nfgobject;

extern PyTypeObject Nfgtype;
#define is_nfgobject(v)  ((v)->ob_type == &Nfgtype)

nfgobject *newnfgobject(void);

/*************************************************************************
 * NFPLAYER OBJECT
 *************************************************************************/

typedef struct {
  PyObject_HEAD
  gbtNfgPlayer *m_nfplayer;
} nfplayerobject;

extern PyTypeObject Nfplayertype;
#define is_nfplayerobject(v)  ((v)->ob_type == &Nfplayertype)

nfplayerobject *newnfplayerobject(void);

/*************************************************************************
 * STRATEGY OBJECT
 *************************************************************************/

typedef struct {
  PyObject_HEAD
  gbtNfgStrategy *m_strategy;
} strategyobject;

extern PyTypeObject Strategytype;
#define is_strategyobject(v)  ((v)->ob_type == &Strategytype)

strategyobject *newstrategyobject(void);

/*************************************************************************
 * NFOUTCOME OBJECT
 *************************************************************************/

typedef struct {
  PyObject_HEAD
  gbtNfgOutcome *m_nfoutcome;
} nfoutcomeobject;

extern PyTypeObject Nfoutcometype;
#define is_nfoutcomeobject(v)  ((v)->ob_type == &Nfoutcometype)

nfoutcomeobject *newnfoutcomeobject(void);

/*************************************************************************
 * MIXED PROFILE OBJECT
 *************************************************************************/

//
// Following the GCL implementation, mixed profiles are actually implemented
// as MixedSolution.  This probably should be streamlined in a future
// version.
//

typedef struct {
  PyObject_HEAD
  MixedSolution *m_profile;
} mixedobject;

extern PyTypeObject Mixedtype;
#define is_mixedobject(v)  ((v)->ob_type == &Mixedtype)

mixedobject *newmixedobject(void);

/*************************************************************************
 * BEHAV PROFILE OBJECT
 *************************************************************************/

//
// Following the GCL implementation, behav profiles are actually implemented
// as BehavSolution.  This probably should be streamlined in a future
// version.
//

typedef struct {
  PyObject_HEAD
  BehavSolution *m_profile;
} behavobject;

extern PyTypeObject Behavtype;
#define is_behavobject(v)  ((v)->ob_type == &Behavtype)

behavobject *newbehavobject(void);

#endif  // PYGAMBIT_H

//
// FILE: accels.h -- Definition of accelerator classes
//
// $Id$
//

#ifndef ACCELS_H
#define ACCELS_H

#include "garray.h"
#include "glist.h"
#include "gtext.h"

// This structure describes an accelerator--key combination 
// with event id to trigger.

typedef enum { accelOff = 0, accelOn, accelEither } AccelState;

class Accel {
private:
  AccelState m_control, m_shift;
  long m_key, m_id;

public:
  Accel(void);
  Accel(AccelState p_control, AccelState p_shift, long p_key, long p_id);

  bool operator==(const Accel &) const;
  bool operator!=(const Accel &p_accel) const
    { return !this->operator==(p_accel); }
  bool operator==(wxKeyEvent &) const; 

  AccelState Control(void) const { return m_control; }
  AccelState Shift(void) const { return m_shift; }
  long Key(void) const { return m_key; }
  long Id(void) const { return m_id; }
};

gOutput &operator<<(gOutput &, const Accel &);

// This structure defines a possible accelerator event.
typedef struct accelevent_struct
{
    gText name;
    long id;

    // Constructor
    accelevent_struct(void) : name(""), id(0) { }
    accelevent_struct(const gText &n, long i) : name(n), id(i) { }

    // Operators
    accelevent_struct &operator=(const accelevent_struct &a)
    {
        name = a.name;
        id   = a.id;
        return *this;
    }

    int operator==(const accelevent_struct &a)
    { return (name == a.name && id == a.id); }

    int operator!=(const accelevent_struct &a)
    { return !(name == a.name && id == a.id); }

    int operator==(long new_id)          { return (id == new_id);     }
    int operator==(const gText &new_name) { return (name == new_name); }
} AccelEvent;

gOutput &operator<<(gOutput &o, const AccelEvent &p);

// Functions for working with accelerators
long    CheckAccelerators(gList<Accel> &list, wxKeyEvent &ev);

int     ReadAccelerators(gList<Accel> &list, 
                         const char *section,
                         const char *file);

int     WriteAccelerators(const gList<Accel> &list, 
                          const char *section,
                          const char *file);

void EditAccelerators(gList<Accel> &list, const gArray<AccelEvent> &events);

#endif // ACCELS_H 



#ifndef ACCELS_H
#define ACCELS_H

#include "garray.h"
#include "glist.h"
#include "gstring.h"

// This structure describes an accelerator--key combination w/ event id to trigger
typedef enum {aOff=0,aOn,aEither} AccelState;
typedef struct accel_struct
{
// Data
	AccelState ctrl;
	AccelState shift;
	long key;
	long id;
// Constructor
	accel_struct(void):ctrl(aOff),shift(aOff),key(0),id(0) { }
	accel_struct(AccelState c,AccelState s,long k,long i):ctrl(c),shift(s),key(k),id(i) { }
// Operators
	accel_struct &operator=(const accel_struct &a) {ctrl=a.ctrl;shift=a.shift;key=a.key;id=a.id;return *this;}
	int operator==(const accel_struct &a) const {return (ctrl==a.ctrl && shift==a.shift && key==a.key);}
	int operator!=(const accel_struct &a) const {return !(ctrl==a.ctrl && shift==a.shift && key==a.key);}
	int operator==(wxKeyEvent &a) const 
	{
	return ((ctrl==a.ControlDown() || ctrl==aEither) && (shift==a.ShiftDown() || shift==aEither)
				 && key==a.KeyCode());
	}
} Accel;
gOutput &operator<<(gOutput &o,const Accel &p);
// This structure defines a possible accelerator event.
typedef struct accelevent_struct
{
	gString name;
	long id;
	// Constructor
	accelevent_struct(void) : name(""),id(0) { }
	accelevent_struct(const gString &n,long i) :name(n),id(i) { }
	// operators
	accelevent_struct &operator=(const accelevent_struct &a) {name=a.name;id=a.id;return *this;}
	int operator==(const accelevent_struct &a) {return (name==a.name && id==a.id);}
	int operator!=(const accelevent_struct &a) {return !(name==a.name && id==a.id);}
	int operator==(long new_id) {return (id==new_id);}
	int operator==(const gString new_name) {return (name==new_name);}
} AccelEvent;
gOutput &operator<<(gOutput &o,const AccelEvent &p);
// Functions for working with accelerators
long 	CheckAccelerators(gList<Accel> &list,wxKeyEvent &ev);
int 	ReadAccelerators(gList<Accel> &list,const char *section="Accelerators",const char *file="gambit.ini");
int 	WriteAccelerators(const gList<Accel> &list,const char *section="Accelerators",const char *file="gambit.ini");
void	EditAccelerators(gList<Accel> &list,const gArray<AccelEvent> &events);

#define ACCELERATORS_HELP	"Accelerator Keys"

#endif /* mutual exclusion */

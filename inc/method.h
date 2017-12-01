#ifndef _METHOD_H
#define _METHOD_H

#include "type.h"
//int sysload(filedate*,CYCMK);
int sysinit(cycdata *);
int syslogin(cycdata *);
int syshowdo(cycdata *);
int sysdeldata(cycdata *);//传入的是该sysdata *
int sysseach(cycdata *);
int syssend(cycdata *);

//int uploadDo(cycdata *);
//int downDo(cycdata *);

#endif

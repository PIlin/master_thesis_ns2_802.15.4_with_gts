/*
 * Copyright (c) 1997 by the University of Southern California
 * All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation in source and binary forms for non-commercial purposes
 * and without fee is hereby granted, provided that the above copyright
 * notice appear in all copies and that both the copyright notice and
 * this permission notice appear in supporting documentation. and that
 * any documentation, advertising materials, and other materials related
 * to such distribution and use acknowledge that the software was
 * developed by the University of Southern California, Information
 * Sciences Institute.  The name of the University may not be used to
 * endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THE UNIVERSITY OF SOUTHERN CALIFORNIA makes no representations about
 * the suitability of this software for any purpose.  THIS SOFTWARE IS
 * PROVIDED "AS IS" AND WITHOUT ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Other copyrights might apply to parts of this software and are so
 * noted when applicable.
 *
 * $Header: /nfs/jade/vint/CVSROOT/nam-1/group.h,v 1.3 2001/04/18 00:14:15 mehringe Exp $
 */

// Group membership management for session-level animation

#ifndef nam_group_h
#define nam_group_h

#include <tcl.h>
#include "animation.h"

class Group : public Animation {
public:
	Group(const char *name, unsigned int addr);
	virtual ~Group();
	virtual void draw(View*, double now);

	inline int size() const { return size_; }
	inline unsigned int addr() const { return addr_; }
	int join(int id);
	void leave(int id);
	void get_members(int *mbrs);
	virtual void update_bb() {} // Do nothing

protected:
	Tcl_HashTable *nodeHash_;
	int size_;
	unsigned int addr_;
	char *name_;
};

#endif // nam_group_h

/*
 * opencog/atomspace/AtomSpace.cc
 *
 * Copyright (C) 2008-2011 OpenCog Foundation
 * Copyright (C) 2002-2007 Novamente LLC
 * All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License v3 as
 * published by the Free Software Foundation and including the exceptions
 * at http://opencog.org/wiki/Licenses
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program; if not, write to:
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <stdlib.h>
#include <string>

#include <boost/bind.hpp>

#include <opencog/util/Logger.h>
#include "AtomSpace.h"

//#define DPRINTF printf
#define DPRINTF(...)

using namespace opencog;

// ====================================================================
//

AtomSpace::AtomSpace(void)
{
    _atomSpaceImpl = new AtomSpaceImpl();
    _ownsAtomSpaceImpl = true;

    c_add = addAtomSignal(
        boost::bind(&AtomSpace::handleAddSignal, this, _1));
}

AtomSpace::AtomSpace(const AtomSpace& other)
{
    _atomSpaceImpl = other._atomSpaceImpl;
    _ownsAtomSpaceImpl = false;

    c_add = addAtomSignal(
        boost::bind(&AtomSpace::handleAddSignal, this, _1));
}

AtomSpace::AtomSpace(AtomSpaceImpl* a)
{
    _atomSpaceImpl = a;
    _ownsAtomSpaceImpl = false;

    c_add = addAtomSignal(
        boost::bind(&AtomSpace::handleAddSignal, this, _1));
}

AtomSpace::~AtomSpace()
{
    c_add.disconnect();
    // Will be unnecessary once GC is implemented
    if (_ownsAtomSpaceImpl)
        delete _atomSpaceImpl;
}

void AtomSpace::handleAddSignal(Handle h)
{
    // XXX TODO FIXME The design here is fundamentally broken!
    // The queue below is used by python to get atom add signals.
    // However, if there's no agent running to drain the queue,
    // it will grow without bound, viz its a memory leak.
    // Add insult to injury: chances are very high that the added
    // atoms are removed a short time later, so this ends up
    // holding a huge list of dead atoms. Add to that, the fact
    // this contraption isn't even thread-safe.  Yuck.
    // Some of this could be partly aleviated by using weak pointers
    // instead of handles, so that dead atoms can die gracefully.
    // But still, this design is basically just plain broken.

    // Avoid bufferbloat. If no one is bothering to unqueue these
    // atoms, don't keep adding more to the list.
    if (1000 < addAtomSignalQueue.size()) return;

    // XXX TODO FIXME  this must be locked to avoid corruption!!!
    addAtomSignalQueue.push_back(h);
}

AtomSpace& AtomSpace::operator=(const AtomSpace& other)
{
    throw opencog::RuntimeException(TRACE_INFO,
            "AtomSpace - Cannot copy an object of this class");
}

Handle AtomSpace::addPrefixedNode(Type t, const std::string& prefix, TruthValuePtr tvn)
{
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    static unsigned long int cnt = 0;
    srand(++cnt);
    static const int len = 16;
    std::string name;
    Handle result;
    //Keep trying new random suffixes until you generate a new name
    do {
        name = prefix;
        for (int i = 0; i < len; ++i) {
            name += alphanum[rand() % (sizeof(alphanum) - 1)];
        }
        result = getHandle(t, name);
    } while (isValidHandle(result)); // If the name already exists, try again
    return addNode(t, name, tvn);
}


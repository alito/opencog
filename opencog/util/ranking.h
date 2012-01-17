/** ranking.h --- 
 *
 * Copyright (C) 2012 Nil Geisweiller
 *
 * Author: Nil Geisweiller <nilg@desktop>
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


#ifndef _OPENCOG_RANKING_H
#define _OPENCOG_RANKING_H

#include "Counter.h"

namespace opencog {

/**
 * Modify a counter to be a rank. Ties ranks are averaged.
 */
template<typename Key, typename FloatT>
Counter<Key, FloatT> ranking(Counter<Key, FloatT>& counter) {
    Counter<Key, FloatT> res;
    FloatT lrank = 1;
    foreach(const auto& v, counter) {
        res.insert(res.end(), {v.first, (2*lrank + v.second - 1) / 2});
        lrank += v.second;
    }
    return res;
}

} // ~namespace opencog

#endif // _OPENCOG_RANKING_H
/*
 * Id: $Id: stdrandomnumbergenerator.h,v 1.1 2004/01/06 23:32:17 bwalle Exp $
 * -------------------------------------------------------------------------------------------------
 * 
 * This program is free software; you can redistribute it and/or modify it under the terms of the 
 * GNU General Public License as published by the Free Software Foundation; You may only use 
 * version 2 of the License, you have no option to use any other version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; 
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See 
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program; if 
 * not, write to the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * ------------------------------------------------------------------------------------------------- 
 */
#ifndef STDRANDOMNUMBERGENERATOR_H
#define STDRANDOMNUMBERGENERATOR_H
 
#include <functional>

template <class type>
class StdRandomNumberGenerator : public std::unary_function<type, type> 
{
    public:
        StdRandomNumberGenerator();
    
    public:
        type operator() (type max);
        type operator() ();
};

#include "stdrandomnumbergenerator.ipp"
        
#endif // STDRANDOMNUMBERGENERATOR_H

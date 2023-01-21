//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#include <string>

#include "PsetTable.h"

namespace omnetvlr {

// When compiler encounters declaration of PsetTable object of some specific type, 
// e.g. PsetTable<int> in VlrRing.h, it must have access to the template implementation source,
// otherwise it won't know how to construct the PsetTable member functions.
// If you have put the implementation in a separate (PsetTable.cc) file,
// compiler won't find it when it tries to compile the source file VlrRing.h and will give "unresolved references" error
//
// Therefore, let's just define the PsetTable class member functions in "PsetTable.h"

//
// overload << operator to print PNeiState
//
std::ostream& operator<<(std::ostream& o, const PNeiState ps){
    static std::map<PNeiState, std::string> namemap_PNeiState;
    if (namemap_PNeiState.size() == 0){
#define INSERT_TO_NAMEMAP(p) namemap_PNeiState[p] = #p
        INSERT_TO_NAMEMAP(PNEI_PENDING);
        INSERT_TO_NAMEMAP(PNEI_LINKED);
#undef INSERT_TO_NAMEMAP
    }
    return o << namemap_PNeiState[ps];
}



} /* namespace omnetvlr */
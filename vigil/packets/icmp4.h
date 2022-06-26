/*     
    This file is part of Vigil.

    Vigil is free software: you can redistribute it and/or modify it under the terms of the 
    GNU General Public License as published by the Free Software Foundation, either version 3 
    of the License, or (at your option) any later version.

    Vigil is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; 
    without  even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
    See the GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along with Vigil. 
    If not, see <https://www.gnu.org/licenses/>. 

*/
#ifndef ICMP4_H
#define ICMP4_H
#include <stdint.h>
struct __icmp4{
  uint8_t type;
  uint8_t code;
  uint16_t checksum;
  uint16_t id;
  uint16_t seq;
  uint8_t timestamp[8];
  uint8_t data[128];
};

#endif /* ICMP4_H */
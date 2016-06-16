/*
 * Misc functions used by the attack wrapper
 *
 * Copyright (C) 2010 Guillaume Duc <guillaume.duc@telecom-paristech.fr>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */


#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>

#include <dpa_contest.h>


/*
 * Convert a 32 hexadecimal digits string into a 16 bytes array of
 * type uint8_t
 */
void
hex2uint8_16bytes (const char *hex, uint8_t *bytes);


#endif /* UTILS_H */

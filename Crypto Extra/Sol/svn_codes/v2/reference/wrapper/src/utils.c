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


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#include "utils.h"


/*
 * Convert a 32 hexadecimal digits string into a 16 bytes array of
 * type uint8_t
 */
void
hex2uint8_16bytes (const char *hex, uint8_t *bytes)
{
  sscanf (hex,
	  "%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx",
	  &bytes[0],
	  &bytes[1],
	  &bytes[2],
	  &bytes[3],
	  &bytes[4],
	  &bytes[5],
	  &bytes[6],
	  &bytes[7],
	  &bytes[8],
	  &bytes[9],
	  &bytes[10],
	  &bytes[11],
	  &bytes[12],
	  &bytes[13],
	  &bytes[14],
	  &bytes[15]);
}

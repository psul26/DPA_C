/*
 * AES related functions used by the attack wrapper
 *
 * Copyright (C) 2009, 2010 Guillaume Duc <guillaume.duc@telecom-paristech.fr>
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


#ifndef AES_H
#define AES_H

#include <stdint.h>

uint32_t
AES_rot_word (uint32_t value);

uint32_t
AES_sub_word (uint32_t value);

void
AES_key_schedule (uint8_t *correct_key, uint8_t *correct_subkey,
		  uint8_t subkey_num);


#endif /* AES_H */

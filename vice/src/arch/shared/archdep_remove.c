/** \file   archdep_remove.c
 * \brief   Remove a file
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 *
 * OS support:
 *  - Linux
 *  - Windows
 *  - BSD
 *  - MacOS
 *  - Haiku
 */

/*
 * This file is part of VICE, the Versatile Commodore Emulator.
 * See README for copyright notice.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 *  02111-1307  USA.
 *
 */

#include "vice.h"
#include "archdep_defs.h"

#if defined(ARCHDEP_OS_UNIX) || defined(ARCHDEP_OS_HAIKU)
# include <unistd.h>
#elif defined(ARCHDEP_OS_WINDOWS)
# include <stdio.h>
#else
# error "Unsupported OS!"
#endif

#include "archdep_remove.h"


/** \brief  Remove a file
 *
 * \param[in]   path    file to remove
 *
 * \return  0 on success, -1 on failure
 */
int archdep_remove(const char *path)
{
#if defined(ARCHDEP_OS_UNIX) || defined(ARCHDEP_OS_HAIKU)
    return unlink(path);
#elif defined(ARCHDEP_OS_WINDOWS)
    return remove(path);
#else
    return -1;
#endif
}

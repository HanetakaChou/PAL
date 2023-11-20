//
// Copyright (C) YuqiaoZhang(HanetakaChou)
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#include "brx_malloc.h"
#include <stdlib.h>
#include <memory.h>

extern void *brx_malloc(size_t size, size_t alignment)
{
#if defined(__GNUC__)
	return aligned_alloc(alignment, size);
#elif defined(_MSC_VER)
	return _aligned_malloc(size, alignment);
#else
#error Unknown Compiler
#endif
}

extern void brx_free(void *ptr)
{
#if defined(__GNUC__)
	free(ptr);
#elif defined(_MSC_VER)
	_aligned_free(ptr);
#else
#error Unknown Compiler
#endif
}

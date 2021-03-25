// Copyright (c) Acconeer AB, 2020
// All rights reserved
// This file is subject to the terms and conditions defined in the file
// 'LICENSES/license_acconeer.txt', (BSD 3-Clause License) which is part
// of this source code package.
#include <stdio.h>
#include <stdlib.h>


void __wrap___assert(const char *file, int line, const char *failedexpr)
{
	printf("Assertion \"%s\" failed at %s:%d\n", failedexpr, file, line);
	abort();
}


void __wrap___assert_func(const char *file,
                          int        line,
                          const char *func,
                          const char *failedexpr)
{
	printf("Assertion \"%s\" failed in %s at %s:%d\n", failedexpr, func, file, line);
	abort();
}

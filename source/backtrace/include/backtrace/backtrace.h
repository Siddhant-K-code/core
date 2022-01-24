/*
 *	Backtrace Library by Parra Studios
 *	A cross-platform library for supporting SEGV catching and backtracing.
 *
 *	Copyright (C) 2016 - 2022 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	Licensed under the Apache License, Version 2.0 (the "License");
 *	you may not use this file except in compliance with the License.
 *	You may obtain a copy of the License at
 *
 *		http://www.apache.org/licenses/LICENSE-2.0
 *
 *	Unless required by applicable law or agreed to in writing, software
 *	distributed under the License is distributed on an "AS IS" BASIS,
 *	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *	See the License for the specific language governing permissions and
 *	limitations under the License.
 *
 */

#ifndef BACKTRACE_H
#define BACKTRACE_H 1

/* -- Headers -- */

#include <backtrace/backtrace_api.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Methods -- */

BACKTRACE_API int backtrace_initialize(void);

BACKTRACE_API int backtrace_destroy(void);

BACKTRACE_API const char *backtrace_print_info(void);

#ifdef __cplusplus
}
#endif

#endif /* BACKTRACE_H */

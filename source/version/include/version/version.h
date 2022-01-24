/*
 *	CMake Versioning Utility by Parra Studios
 *	A template for generating versioning utilities.
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

#ifndef VERSION_H
#define VERSION_H 1

#include <version/version_api.h>

#ifdef __cplusplus
extern "C" {
#endif

VERSION_API const char *version_print_info(void);

#ifdef __cplusplus
}
#endif

#endif /* VERSION_H */

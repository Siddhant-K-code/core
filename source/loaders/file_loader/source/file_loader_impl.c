/*
 *	Loader Library by Parra Studios
 *	A plugin for loading file code at run-time into a process.
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

#include <file_loader/file_loader_impl.h>

#include <loader/loader.h>
#include <loader/loader_impl.h>
#include <loader/loader_path.h>

#include <reflect/reflect_context.h>
#include <reflect/reflect_function.h>
#include <reflect/reflect_scope.h>
#include <reflect/reflect_type.h>

#include <adt/adt_vector.h>

#include <log/log.h>

#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#if defined(WIN32) || defined(_WIN32)
	#ifndef NOMINMAX
		#define NOMINMAX
	#endif

	#ifndef WIN32_LEAN_AND_MEAN
		#define WIN32_LEAN_AND_MEAN
	#endif

	#include <windows.h>

	#define LOADER_IMPL_FILE_SIZE MAX_PATH

typedef struct _stat file_stat_type;

	#define file_stat _stat

#elif defined(unix) || defined(__unix__) || defined(__unix) ||                          \
	defined(linux) || defined(__linux__) || defined(__linux) || defined(__gnu_linux) || \
	defined(__CYGWIN__) || defined(__CYGWIN32__) ||                                     \
	defined(__MINGW32__) || defined(__MINGW64__) ||                                     \
	(defined(__APPLE__) && defined(__MACH__)) || defined(__MACOSX__)

	#include <limits.h>
	#include <unistd.h>

	#define LOADER_IMPL_FILE_SIZE PATH_MAX

typedef struct stat file_stat_type;

	#define file_stat stat

#endif

typedef char loader_impl_file_path[LOADER_IMPL_FILE_SIZE];

typedef struct loader_impl_file_descriptor_type
{
	loader_impl_file_path path;
	size_t length;

} * loader_impl_file_descriptor;

typedef struct loader_impl_file_type
{
	vector execution_paths;

} * loader_impl_file;

typedef struct loader_impl_file_handle_type
{
	vector paths;

} * loader_impl_file_handle;

typedef struct loader_impl_file_function_type
{
	loader_impl_file_descriptor descriptor;

} * loader_impl_file_function;

static int file_loader_impl_load_path(loader_impl_file_handle handle, const loader_naming_name path);
static void file_loader_impl_load_execution_path(loader_impl_file file_impl, loader_impl_file_handle handle, const loader_naming_name path);

int function_file_interface_create(function func, function_impl impl)
{
	(void)func;
	(void)impl;

	return 0;
}

function_return function_file_interface_invoke(function func, function_impl impl, function_args args, size_t size)
{
	loader_impl_file_function file_function = (loader_impl_file_function)impl;

	(void)func;
	(void)args;
	(void)size;

	return value_create_string(file_function->descriptor->path, file_function->descriptor->length);
}

function_return function_file_interface_await(function func, function_impl impl, function_args args, size_t size, function_resolve_callback resolve_callback, function_reject_callback reject_callback, void *context)
{
	/* TODO */

	(void)func;
	(void)impl;
	(void)args;
	(void)size;
	(void)resolve_callback;
	(void)reject_callback;
	(void)context;

	return NULL;
}

void function_file_interface_destroy(function func, function_impl impl)
{
	loader_impl_file_function file_function = (loader_impl_file_function)impl;

	(void)func;

	if (file_function != NULL)
	{
		free(file_function);
	}
}

function_interface function_file_singleton(void)
{
	static struct function_interface_type file_interface = {
		&function_file_interface_create,
		&function_file_interface_invoke,
		&function_file_interface_await,
		&function_file_interface_destroy
	};

	return &file_interface;
}

int file_loader_impl_initialize_types(loader_impl impl)
{
	static struct
	{
		type_id id;
		const char *name;
	} type_id_name_pair[] = {
		{ TYPE_STRING, "File" },
	};

	size_t index, size = sizeof(type_id_name_pair) / sizeof(type_id_name_pair[0]);

	for (index = 0; index < size; ++index)
	{
		type t = type_create(type_id_name_pair[index].id, type_id_name_pair[index].name, NULL, NULL);

		if (t != NULL)
		{
			if (loader_impl_type_define(impl, type_name(t), t) != 0)
			{
				type_destroy(t);

				return 1;
			}
		}
	}

	return 0;
}

loader_impl_data file_loader_impl_initialize(loader_impl impl, configuration config)
{
	loader_impl_file file_impl;

	(void)impl;
	(void)config;

	file_impl = malloc(sizeof(struct loader_impl_file_type));

	if (file_impl == NULL)
	{
		return NULL;
	}

	if (file_loader_impl_initialize_types(impl) != 0)
	{
		free(file_impl);

		return NULL;
	}

	file_impl->execution_paths = vector_create(sizeof(loader_naming_path));

	if (file_impl->execution_paths == NULL)
	{
		free(file_impl);

		return NULL;
	}

	/* Register initialization */
	loader_initialization_register(impl);

	return (loader_impl_data)file_impl;
}

int file_loader_impl_execution_path(loader_impl impl, const loader_naming_path path)
{
	loader_impl_file file_impl = loader_impl_get(impl);
	loader_naming_name *execution_path;

	vector_push_back_empty(file_impl->execution_paths);

	execution_path = vector_back(file_impl->execution_paths);

	strncpy(*execution_path, path, LOADER_NAMING_PATH_SIZE);

	return 0;
}

int file_loader_impl_load_path(loader_impl_file_handle handle, const loader_naming_name path)
{
	file_stat_type fs;

	if (file_stat(path, &fs) == 0)
	{
		loader_impl_file_descriptor descriptor = NULL;

		vector_insert_empty(handle->paths, vector_size(handle->paths));

		descriptor = vector_back(handle->paths);

		strncpy(descriptor->path, path, LOADER_IMPL_FILE_SIZE);

		descriptor->length = strnlen(descriptor->path, LOADER_IMPL_FILE_SIZE);

		log_write("metacall", LOG_LEVEL_DEBUG, "File %s loaded from file", path);

		return 0;
	}

	return 1;
}

void file_loader_impl_load_execution_path(loader_impl_file file_impl, loader_impl_file_handle handle, const loader_naming_name path)
{
	size_t size = vector_size(file_impl->execution_paths);

	if (file_loader_impl_load_path(handle, path) != 0 && size > 0)
	{
		size_t iterator;

		for (iterator = 0; iterator < size; ++iterator)
		{
			loader_naming_name *execution_path = vector_at(file_impl->execution_paths, iterator);
			loader_naming_name absolute_path;

			(void)loader_path_join(*execution_path, strlen(*execution_path) + 1, path, strlen(path) + 1, absolute_path);

			if (file_loader_impl_load_path(handle, absolute_path) == 0)
			{
				log_write("metacall", LOG_LEVEL_ERROR, "File %s not found", absolute_path);
				return;
			}
		}
	}
}

loader_handle file_loader_impl_load_from_file(loader_impl impl, const loader_naming_path paths[], size_t size)
{
	loader_impl_file file_impl = loader_impl_get(impl);

	if (file_impl == NULL)
	{
		return NULL;
	}

	loader_impl_file_handle handle = malloc(sizeof(struct loader_impl_file_handle_type));

	if (handle != NULL)
	{
		size_t iterator;

		handle->paths = vector_create(sizeof(struct loader_impl_file_descriptor_type));

		if (handle->paths == NULL)
		{
			free(handle);

			return NULL;
		}

		for (iterator = 0; iterator < size; ++iterator)
		{
			file_loader_impl_load_execution_path(file_impl, handle, paths[iterator]);
		}

		if (vector_size(handle->paths) == 0)
		{
			vector_destroy(handle->paths);

			free(handle);

			return NULL;
		}

		return (loader_handle)handle;
	}

	return NULL;
}

loader_handle file_loader_impl_load_from_memory(loader_impl impl, const loader_naming_name name, const char *buffer, size_t size)
{
	(void)impl;
	(void)name;
	(void)buffer;
	(void)size;

	// TODO: In theory this should load the buffer, not the name... no?
	// Although this loader does not take care of the file itself, only the paths;
	// so probably this make or neither makes sense to be used or it should handle more than the paths

	/*
	loader_impl_file_handle handle = malloc(sizeof(struct loader_impl_file_handle_type));

	if (handle != NULL)
	{
		loader_impl_file_descriptor descriptor = NULL;

		handle->paths = vector_create(sizeof(struct loader_impl_file_descriptor_type));

		if (handle->paths == NULL)
		{
			free(handle);

			return NULL;
		}

		vector_insert_empty(handle->paths, 0);

		descriptor = vector_back(handle->paths);

		strncpy(descriptor->path, name, LOADER_IMPL_FILE_SIZE);

		descriptor->length = strnlen(descriptor->path, LOADER_IMPL_FILE_SIZE);

		log_write("metacall", LOG_LEVEL_DEBUG, "File module %s loaded from memory", name);

		return (loader_handle)handle;
	}
	*/

	return NULL;
}

loader_handle file_loader_impl_load_from_package(loader_impl impl, const loader_naming_path path)
{
	loader_impl_file file_impl = loader_impl_get(impl);

	if (file_impl == NULL)
	{
		return NULL;
	}

	loader_impl_file_handle handle = malloc(sizeof(struct loader_impl_file_handle_type));

	if (handle != NULL)
	{
		handle->paths = vector_create(sizeof(struct loader_impl_file_descriptor_type));

		if (handle->paths == NULL)
		{
			free(handle);

			return NULL;
		}

		file_loader_impl_load_execution_path(file_impl, handle, path);

		if (vector_size(handle->paths) == 0)
		{
			vector_destroy(handle->paths);

			free(handle);

			return NULL;
		}

		return (loader_handle)handle;
	}

	return NULL;
}

int file_loader_impl_clear(loader_impl impl, loader_handle handle)
{
	loader_impl_file_handle file_handle = (loader_impl_file_handle)handle;

	(void)impl;

	if (file_handle != NULL)
	{
		vector_destroy(file_handle->paths);

		free(file_handle);

		return 0;
	}

	return 1;
}

int file_loader_impl_discover(loader_impl impl, loader_handle handle, context ctx)
{
	loader_impl_file file_impl = loader_impl_get(impl);

	loader_impl_file_handle file_handle = (loader_impl_file_handle)handle;

	scope sp = context_scope(ctx);

	size_t iterator, size = vector_size(file_handle->paths);

	(void)file_impl;

	log_write("metacall", LOG_LEVEL_DEBUG, "File module %p discovering", handle);

	for (iterator = 0; iterator < size; ++iterator)
	{
		loader_impl_file_descriptor descriptor = vector_at(file_handle->paths, iterator);

		loader_impl_file_function file_function = malloc(sizeof(struct loader_impl_file_function_type));

		if (file_function != NULL)
		{
			const char *script_path = getenv("LOADER_SCRIPT_PATH");

			function f;

			signature s;

			file_function->descriptor = descriptor;

			if (script_path != NULL)
			{
				loader_naming_name name;

				(void)loader_path_get_relative(script_path, descriptor->path, name);

				f = function_create(name, 0, file_function, &function_file_singleton);
			}
			else
			{
				f = function_create(descriptor->path, 0, file_function, &function_file_singleton);
			}

			s = function_signature(f);

			signature_set_return(s, loader_impl_type(impl, "Path"));

			scope_define(sp, function_name(f), value_create_function(f));
		}
	}

	return 0;
}

int file_loader_impl_destroy(loader_impl impl)
{
	loader_impl_file file_impl = loader_impl_get(impl);

	if (file_impl != NULL)
	{
		/* Destroy children loaders */
		loader_unload_children(impl, 0);

		if (file_impl->execution_paths != NULL)
		{
			vector_destroy(file_impl->execution_paths);
		}

		free(file_impl);

		return 0;
	}

	return 1;
}

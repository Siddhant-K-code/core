/*
 *	Loader Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for loading executable code at run-time into a process.
 *
 */

#ifndef LOADER_IMPL_H
#define LOADER_IMPL_H 1

#include <loader/loader_api.h>

#include <loader/loader_path.h>
#include <loader/loader_impl_interface.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Definitions -- */

#define LOADER_HOST_PROXY_NAME		"__metacall_host__"

/* -- Methods -- */

LOADER_API loader_impl loader_impl_create(const char * path, const loader_naming_tag tag, loader_host host);

LOADER_API loader_impl_data loader_impl_get(loader_impl impl);

LOADER_API loader_impl_interface loader_impl_symbol(loader_impl impl);

LOADER_API loader_naming_tag * loader_impl_tag(loader_impl impl);

LOADER_API context loader_impl_context(loader_impl impl);

LOADER_API type loader_impl_type(loader_impl impl, const char * name);

LOADER_API int loader_impl_type_define(loader_impl impl, const char * name, type t);

LOADER_API int loader_impl_execution_path(loader_impl impl, const loader_naming_path path);

LOADER_API int loader_impl_load_from_file(loader_impl impl, const loader_naming_path paths[], size_t size, void ** handle_ptr);

LOADER_API int loader_impl_load_from_memory(loader_impl impl, const char * buffer, size_t size, void ** handle_ptr);

LOADER_API int loader_impl_load_from_package(loader_impl impl, const loader_naming_path path, void ** handle_ptr);

LOADER_API void * loader_impl_get_handle(loader_impl impl, const char * name);

LOADER_API const char * loader_impl_handle_id(void * handle);

LOADER_API int loader_impl_clear(void * handle);

LOADER_API value loader_impl_metadata(loader_impl impl);

LOADER_API void loader_impl_destroy(loader_impl impl);

LOADER_API loader_impl loader_impl_create_proxy();

#ifdef __cplusplus
}
#endif

#endif /* LOADER_IMPL_H */

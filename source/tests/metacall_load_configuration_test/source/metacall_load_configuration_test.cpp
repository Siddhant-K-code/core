/*
 *	MetaCall Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for providing a foreign function interface calls.
 *
 */

#include <gmock/gmock.h>

#include <metacall/metacall.h>
#include <metacall/metacall-plugins.h>

#include <reflect/reflect_value_type.h>

#include <log/log.h>

class metacall_load_configuration_test : public testing::Test
{
public:
};

TEST_F(metacall_load_configuration_test, DefaultConstructor)
{
	EXPECT_EQ((int) 0, (int) log_configure("metacall",
		log_policy_format_text(),
		log_policy_schedule_sync(),
		log_policy_storage_sequential(),
		log_policy_stream_stdio(stdout)));

	metacall_print_info();

	ASSERT_EQ((int) 0, (int) metacall_initialize());

	/* Python */
	#if defined(OPTION_BUILD_PLUGINS_PY)
	{
		const long seven_multiples_limit = 10;

		long iterator;

		void * ret = NULL;

		ASSERT_EQ((int) 0, (int) metacall_load_from_configuration("metacall_load_from_configuration_test.json"));

		ret = metacall("multiply", 5, 15);

		EXPECT_NE((void *) NULL, (void *) ret);

		EXPECT_EQ((long) metacall_value_to_long(ret), (long) 75);

		metacall_value_destroy(ret);

		for (iterator = 0; iterator <= seven_multiples_limit; ++iterator)
		{
			ret = metacall("multiply", 7, iterator);

			EXPECT_NE((void *) NULL, (void *) ret);

			EXPECT_EQ((long) metacall_value_to_long(ret), (long) (7 * iterator));

			metacall_value_destroy(ret);
		}

		ret = metacall("divide", 64.0, 2.0);

		EXPECT_NE((void *) NULL, (void *) ret);

		EXPECT_EQ((double) metacall_value_to_double(ret), (double) 32.0);

		metacall_value_destroy(ret);

		ret = metacall("sum", 1000, 3500);

		EXPECT_NE((void *) NULL, (void *) ret);

		EXPECT_EQ((long) metacall_value_to_long(ret), (long) 4500);

		metacall_value_destroy(ret);

		ret = metacall("sum", 3, 4);

		EXPECT_NE((void *) NULL, (void *) ret);

		EXPECT_EQ((long) metacall_value_to_long(ret), (long) 7);

		metacall_value_destroy(ret);

		EXPECT_EQ((void *) NULL, (void *)metacall("hello"));

		ret = metacall("strcat", "Hello ", "Universe");

		EXPECT_NE((void *) NULL, (void *) ret);

		EXPECT_EQ((int) 0, (int) strcmp(metacall_value_to_string(ret), "Hello Universe"));

		metacall_value_destroy(ret);
	}
	#endif /* OPTION_BUILD_PLUGINS_PY */

	EXPECT_EQ((int) 0, (int) metacall_destroy());
}
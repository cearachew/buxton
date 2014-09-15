/*
 * This file is part of buxton.
 *
 * Copyright (C) 2014 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "buxtonsimple.h"

/* DEMONSTRATION */
void tk_s5_notify_cb(void *key_data, char *key_name)
{
	//tk_s5 is a STRING
	char *data = (char *)key_data;
	printf("key %s was changed to value %s\n", key_name, data);
}

void tk_i32_notify_cb(void *key_data, char *key_name)
{
	//tk_s6 is a INT32
	int32_t *data = (int32_t *)key_data;
	printf("key %s was changed to value %d\n", key_name, *data);
	if (*data == 125) {
		ecore_shutdown();
	}
}

int main(void)
{
	//first test if stuff works with sbuxton_open and sbuxton_close
	sbuxton_open();
	
	/* Create group */
	errno = 0;
	sbuxton_set_group("tg_s5", "user");
	printf("set_group: 'tg_s5', 'user', Error number: %s.\n", strerror(errno));

	/* Test string setting */
	srand((unsigned)time(NULL));
	char * s1="Watermelon";
	printf("value should be set to %s.\n", s1);
	errno = 0;
	sbuxton_set_string("tk_s5", s1);
	printf("set_string: 'tg_s5', 'tk_s5', Error number: %s.\n", strerror(errno));

	/* Test string getting */
	char * sv1 = sbuxton_get_string("tk_s5");
	printf("Got value: %s(string).\n", sv1);		
	printf("get_string: 'tk_s5', Error number: %s.\n", strerror(errno));

	/* Test Int setting */
	int32_t i32 = (int32_t) rand() % 50 + 1;
	printf("value should be set to %d.\n", i32);
	errno = 0;
	sbuxton_set_int32("tk_i32", i32);
	printf("set_int32: 'tg_s5', 'tk_i32', Error number: %s.\n", strerror(errno));
	/* Test uint32 getting */
	errno = 0;
	int32_t i32v = sbuxton_get_int32("tk_i32");
	printf("Got value: %d(int32_t).\n", i32v);
	printf("get_int32: 'tg_s3', 'tk_i32', Error number: %s.\n", strerror(errno));

	if (!ecore_init()) {
		printf("Could not initialize ecore");
		return -1;
	}

	// Get notifications
	printf("Register for string tk_s5\n");
	sbuxton_register_notify("tk_s5", tk_s5_notify_cb);
	printf("Register for int32_t tk_i32\n");
	sbuxton_register_notify("tk_i32", tk_i32_notify_cb);

	//start_main_loop
	printf("Start mainloop\n");
	ecore_main_loop_begin();

	//change some keys....
	//printf("Change keys\n");
	//sbuxton_set_string("tk_s5", "changed1");
	//sbuxton_set_int32("tk_i32", 5);
	//sbuxton_set_int32("tk_i32", 25);
	//sbuxton_set_string("tk_s5", "changed again");
	//sbuxton_set_int32("tk_i32", 125);

	//shutdown main loop
	ecore_shutdown();

	//close connection
	sbuxton_close();

	//end testing with open and close
	return 0;
}

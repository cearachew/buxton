/*
 * This file is part of buxton.
 *
 * Copyright (C) 2014 Intel Corporation
 *
 * buxton is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1
 * of the license, or (at your option) any later version.
 */
 
 /**
 * \file lbuxtonsimple.c Buxton library implementation
 */
 
#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "buxtonclient.h"
#include "buxtonsimple.h"
#include "buxtonsimple-internals.h"
#include "log.h"
/* Max length of layer and group names  */
#define MAX_LG_LEN 256

extern BuxtonClient client;
static char _layer[MAX_LG_LEN];
static char _group[MAX_LG_LEN];
static int saved_errno;
static bool client_locked = false;

/* Open a client with a flag that does not let other functions close it */
void sbuxton_open(void)
{
	if (client_locked) {
		errno = EACCES;
		return;
	}
	if (!_client_connection()) {
		errno = ENOTCONN;
		return;
	}
	client_locked = true;
}

/* Close a client that was opened with sbuxton_open, clear the flag */
void sbuxton_close(void)
{
	client_locked = false;
	if (client != NULL) {
		_client_disconnect();
	}
}

/* Register a key for notification */
void sbuxton_register_notify(char *key, NotifyCallback callback)
{
	if (!client_locked || !client) {
		buxton_debug("Connection must first be opened via sbuxton_open");
		errno = ENOTCONN;
		return;
	}

	nstatus cb;
	saved_errno = errno;
	BuxtonKey *_key = NULL;

	_key = _buxton_notify_create(_layer, _group, key);

	if (!_key) {
		errno = EACCES;
		return;
	}

	cb.callback = callback;
	cb.status = 0;

	if(buxton_register_notification(client, *_key, _rn_cb, &cb, false)) {
		buxton_debug("register notification call failed\n");
		errno = EACCES;
		return;
	} else {
		errno = saved_errno;
	}

	buxton_debug("registration successful\n");
}

/* add the client's fd to the ecore main loop */
void sbuxton_register_ecore(void)
{
	_BuxtonClient *c = NULL;

	c = (_BuxtonClient *)client;

	Ecore_Fd_Handler *e_handler = ecore_main_fd_handler_add(c->fd,
					ECORE_FD_READ | ECORE_FD_ERROR,
					_buxton_update_cb, NULL, NULL, NULL);
	if (!e_handler) {
		buxton_debug("Call to ecore_main_fd_handler_add failed\n");
	}
}

/* Initialization of group */
void sbuxton_set_group(char *group, char *layer)
{
	if (!client_locked) {
		if (!_client_connection()) {
			errno = ENOTCONN;
			return;
		}
	}
	saved_errno = errno;
	int status = 0;
	/* strcpy the name of the layer and group*/
	strncpy(_layer, layer, MAX_LG_LEN-1);
	strncpy(_group, group, MAX_LG_LEN-1);
	/* In case a string is longer than MAX_LG_LEN, set the last byte to null */
	_layer[MAX_LG_LEN -1] = '\0';
	_group[MAX_LG_LEN -1] = '\0';
	BuxtonKey g = buxton_key_create(_group, NULL, _layer, STRING);
	buxton_debug("buxton key group = %s\n", buxton_key_get_group(g));
	if (buxton_create_group(client, g, _cg_cb, &status, true)
		|| !status) {
		buxton_debug("Create group call failed.\n");
		errno = EBADMSG;
	} else {
		buxton_debug("Switched to group: %s, layer: %s.\n", buxton_key_get_group(g),
 	buxton_key_get_layer(g));
		errno = saved_errno;
	}
	if (!client_locked) {
		_client_disconnect();
	}
}

/* Set and get int32_t value for buxton key with type INT32 */
void sbuxton_set_int32(char *key, int32_t value)
{
	/* make sure client connection is open */
	if (!client_locked) {
		if (!_client_connection()) {
			errno = ENOTCONN;
			return;
		}
	}
	/* create key  */
	BuxtonKey _key = buxton_key_create(_group, key, _layer, INT32);
	/* return value and status */
	vstatus ret;
	ret.type = INT32;
	ret.val.i32val = value;
	saved_errno = errno;
	/* call buxton_set_value for type INT32 */
	if (buxton_set_value(client, _key, &value, _bs_cb, &ret, true)) {
		buxton_debug("Set int32_t call failed.\n");
		return;
	}
	if (!ret.status) {
		errno = EACCES;
	} else {
		errno = saved_errno;
	}
	if (!client_locked) {
		_client_disconnect();
	}
}

int32_t sbuxton_get_int32(char *key)
{
	/* make sure client connection is open */
	if (!client_locked) {
		if (!_client_connection()) {
			errno = ENOTCONN;
			return 0;
		}
	}
	/* create key */
	BuxtonKey _key = buxton_key_create(_group, key, _layer, INT32);
	/* return value */
	vstatus ret;
	ret.type = INT32;
	saved_errno = errno;
	/* get value */
	if (buxton_get_value(client, _key, _bg_cb, &ret, true)) {
		buxton_debug("Get int32_t call failed.\n");
	}
	if (!ret.status) {
		errno = EACCES;
	} else {
		errno = saved_errno;
	}
	if (!client_locked) {
		_client_disconnect();
	}
	return ret.val.i32val;
}

/* Set and get char * value for buxton key with type STRING */
void sbuxton_set_string(char *key, char *value )
{
	/* make sure client connection is open */
	if (!client_locked) {
		if (!_client_connection()) {
			errno = ENOTCONN;
			return;
		}
	}
	/* create key */
	BuxtonKey _key = buxton_key_create(_group, key, _layer, STRING);
	/* return value and status */
	vstatus ret;
	ret.type = STRING;
	ret.val.sval = value;
	saved_errno = errno;
	/* set value */
	if (buxton_set_value(client, _key, &value, _bs_cb, &ret, true)) {
		buxton_debug("Set string call failed.\n");
	}
	if (!ret.status) {
		errno = EACCES;
	} else {
		errno = saved_errno;
	}
	if (!client_locked) {
		_client_disconnect();
	}
}

char* sbuxton_get_string(char *key)
{
	/* make sure client connection is open */
	if (!client_locked) {
		if (!_client_connection()) {
			errno = ENOTCONN;
			return "";
		}
	}
	/* create key */
	BuxtonKey _key = buxton_key_create(_group, key, _layer, STRING);
	/* return value */
	vstatus ret;
	ret.type = STRING;
	saved_errno = errno;
	/* get value */
	if (buxton_get_value(client, _key, _bg_cb, &ret, true)) {
		buxton_debug("Get string call failed.\n");
	}
	if (!ret.status) {
		errno = EACCES;
	} else {
		errno = saved_errno;
	}
	if (!client_locked) {
		_client_disconnect();
	}
	return ret.val.sval;
}

/* Set and get uint32_t value for buxton key with type UINT32 */
void sbuxton_set_uint32(char *key, uint32_t value)
{
	/* make sure client connection is open */
	if (!client_locked) {
		if (!_client_connection()) {
			errno = ENOTCONN;
			return;
		}
	}
	/* create key */
	BuxtonKey _key = buxton_key_create(_group, key, _layer, UINT32);
	/* return value and status */
	vstatus ret;
	ret.type = UINT32;
	ret.val.ui32val = value;
	saved_errno = errno;
	if (buxton_set_value(client,_key, &value, _bs_cb, &ret, true)) {
		buxton_debug("Set uint32_t call failed.\n");
	}
	if (!ret.status) {
		errno = EACCES;
	} else {
		errno = saved_errno;
	}
	if (!client_locked) {
		_client_disconnect();
	}
}

uint32_t sbuxton_get_uint32(char *key)
{
	/* make sure client connection is open */
	if (!client_locked) {
		if (!_client_connection()) {
			errno = ENOTCONN;
			return 0;
		}
	}
	/* create key */
	BuxtonKey _key = buxton_key_create(_group, key, _layer, UINT32);
	/* return value */
	vstatus ret;
	ret.type = UINT32;
	saved_errno = errno;
	/* get value */
	if (buxton_get_value(client, _key, _bg_cb, &ret, true)) {
		buxton_debug("Get uint32_t call failed.\n");
	}
	if (!ret.status) {
		errno = EACCES;
	} else {
		errno = saved_errno;
	}
	if (!client_locked) {
		_client_disconnect();
	}
	return ret.val.ui32val;
}

/* Set and get int64_t value for buxton key with type INT64 */
void sbuxton_set_int64(char *key, int64_t value)
{
	/* make sure client connection is open */
	if (!client_locked) {
		if (!_client_connection()) {
			errno = ENOTCONN;
			return;
		}
	}
	/* create key */
	BuxtonKey _key = buxton_key_create(_group, key, _layer, INT64);
	/* return value and status */
	vstatus ret;
	ret.type = INT64;
	ret.val.i64val = value;
	saved_errno = errno;
	if (buxton_set_value(client, _key, &value, _bs_cb, &ret, true)) {
		buxton_debug("Set int64_t call failed.\n");
	}
	if (!ret.status) {
		errno = EACCES;
	} else {
		errno = saved_errno;
	}
	if (!client_locked) {
		_client_disconnect();
	}
}

int64_t sbuxton_get_int64(char *key)
{
	/* make sure client connection is open */
	if (!client_locked) {
		if (!_client_connection()) {
			errno = ENOTCONN;
			return 0;
		}
	}
	/* create key */
	BuxtonKey _key = buxton_key_create(_group, key, _layer, INT64);
	/* return value */
	vstatus ret;
	ret.type = INT64;
	saved_errno = errno;
	/* get value */
	if (buxton_get_value(client, _key, _bg_cb, &ret, true)) {
		buxton_debug("Get int64_t call failed.\n");
	}
	if (!ret.status) {
		errno = EACCES;
	} else {
		errno = saved_errno;
	}
	if (!client_locked) {
		_client_disconnect();
	}
	return ret.val.i64val;
}

/* Set and get uint64_t value for buxton key with type UINT64 */
void sbuxton_set_uint64(char *key, uint64_t value)
{
	/* make sure client connection is open */
	if (!client_locked) {
		if (!_client_connection()) {
			errno = ENOTCONN;
			return;
		}
	}
	/* create key */
	BuxtonKey _key = buxton_key_create(_group, key, _layer, UINT64);
	/* return value and status */
	vstatus ret;
	ret.type = UINT64;
	ret.val.ui64val = value;
	saved_errno = errno;
	if (buxton_set_value(client, _key, &value, _bs_cb, &ret, true)) {
		buxton_debug("Set uint64_t call failed.\n");
	}
	if (!ret.status) {
		errno = EACCES;
	} else {
		errno = saved_errno;
	}
	if (!client_locked) {
		_client_disconnect();
	}
}

uint64_t sbuxton_get_uint64(char *key)
{
	/* make sure client connection is open */
	if (!client_locked) {
		if (!_client_connection()) {
			errno = ENOTCONN;
			return 0;
		}
	}
	/* create key */
	BuxtonKey _key = buxton_key_create(_group, key, _layer, UINT64);
	/* return value */
	vstatus ret;
	ret.type = UINT64;
	saved_errno = errno;
	/* get value */
	if (buxton_get_value(client, _key, _bg_cb, &ret, true)) {
		buxton_debug("Get uint64_t call failed.\n");
	}
	if (!ret.status) {
		errno = EACCES;
	} else {
		errno = saved_errno;
	}
	if (!client_locked) {
		_client_disconnect();
	}
	return ret.val.ui64val;
}

/* Set and get float value for buxton key with type FLOAT */
void sbuxton_set_float(char *key, float value)
{
	/* make sure client connection is open */
	if (!client_locked) {
		if (!_client_connection()) {
			errno = ENOTCONN;
			return;
		}
	}
	/* create key */
	BuxtonKey _key = buxton_key_create(_group, key, _layer, FLOAT);
	/* return value and status */
	vstatus ret;
	ret.type = FLOAT;
	ret.val.fval = value;
	saved_errno = errno;
	if (buxton_set_value(client, _key, &value, _bs_cb, &ret, true)) {
		buxton_debug("Set float call failed.\n");
	}
	if (!ret.status) {
		errno = EACCES;
	} else {
		errno = saved_errno;
	}
	if (!client_locked) {
		_client_disconnect();
	}
}

float sbuxton_get_float(char *key)
{
	/* make sure client connection is open */
	if (!client_locked) {
		if (!_client_connection()) {
			errno = ENOTCONN;
			return 0;
		}
	}
	/* create key */
	BuxtonKey _key = buxton_key_create(_group, key, _layer, FLOAT);
	/* return value */
	vstatus ret;
	ret.type = FLOAT;
	saved_errno = errno;
	/* get value */
	if (buxton_get_value(client, _key, _bg_cb, &ret, true)) {
		buxton_debug("Get float call failed.\n");
	}
	if (!ret.status) {
		errno = EACCES;
	} else {
		errno = saved_errno;
	}
	if (!client_locked) {
		_client_disconnect();
	}
	return ret.val.fval;
}

/* Set and get double value for buxton key with type DOUBLE */
void sbuxton_set_double(char *key, double value)
{
	/* make sure client connection is open */
	if (!client_locked) {
		if (!_client_connection()) {
			errno = ENOTCONN;
			return;
		}
	}
	/* create key */
	BuxtonKey _key = buxton_key_create(_group, key, _layer, DOUBLE);
	/* return value and status */
	vstatus ret;
	ret.type = DOUBLE;
	ret.val.dval = value;
	saved_errno = errno;
	if (buxton_set_value(client, _key, &value, _bs_cb, &ret, true)) {
		buxton_debug("Set double call failed.\n");
	}
	if (!ret.status) {
		errno = EACCES;
	} else {
		errno = saved_errno;
	}
	if (!client_locked) {
		_client_disconnect();
	}
}

double sbuxton_get_double(char *key)
{
	/* make sure client connection is open */
	if (!client_locked) {
		if (!_client_connection()) {
			errno = ENOTCONN;
			return 0;
		}
	}
	/* create key */
	BuxtonKey _key = buxton_key_create(_group, key, _layer, DOUBLE);
	/* return value */
	vstatus ret;
	ret.type = DOUBLE;
	saved_errno = errno;
	/* get value */
	if (buxton_get_value(client, _key, _bg_cb, &ret, true)) {
		buxton_debug("Get double call failed.\n");
	}
	if (!ret.status) {
		errno = EACCES;
	} else {
		errno = saved_errno;
	}
	if (!client_locked) {
		_client_disconnect();
	}
	return ret.val.dval;
}

/* Set and get bool value for buxton key with type BOOLEAN */
void sbuxton_set_bool(char *key, bool value)
{
	/* make sure client connection is open */
	if (!client_locked) {
		if (!_client_connection()) {
			errno = ENOTCONN;
			return;
		}
	}
	/* create key */
	BuxtonKey _key = buxton_key_create(_group, key, _layer, BOOLEAN);
	/* return value and status */
	vstatus ret;
	ret.type = BOOLEAN;
	ret.val.bval = value;
	saved_errno = errno;
	if (buxton_set_value(client, _key, &value, _bs_cb, &ret, true)) {
		buxton_debug("Set bool call failed.\n");
	}
	if (!ret.status) {
		errno = EACCES;
	} else {
		errno = saved_errno;
	}
	if (!client_locked) {
		_client_disconnect();
	}
}

bool sbuxton_get_bool(char *key)
{
	/* make sure client connection is open */
	if (!client_locked) {
		if (!_client_connection()) {
			errno = ENOTCONN;
			return false;
		}
	}
	/* create key */
	BuxtonKey _key = buxton_key_create(_group, key, _layer, BOOLEAN);
	/* return value */
	vstatus ret;
	ret.type = BOOLEAN;
	saved_errno = errno;
	/* get value */
	if (buxton_get_value(client, _key, _bg_cb, &ret, true)) {
		buxton_debug("Get bool call failed.\n");
	}
	if (!ret.status) {
		errno = EACCES;
	} else {
		errno = saved_errno;
	}
	if (!client_locked) {
		_client_disconnect();
	}
	return ret.val.bval;
}

/* Remove group given its name and layer */
void sbuxton_remove_group(char *group_name, char *layer)
{
	/* make sure client connection is open */
	if (!client_locked) {
		if (!_client_connection()) {
			errno = ENOTCONN;
			return;
		}
	}
	saved_errno = errno;
	BuxtonKey group = _buxton_group_create(group_name, layer);
	int status;
	if (buxton_remove_group(client, group, _rg_cb, &status, true)) {
		buxton_debug("Remove group call failed.\n");
	}
	if (!status) {
		errno = EACCES;
	} else {
		errno = saved_errno;
	}
	if (!client_locked) {
		_client_disconnect();
	}
}

/*
 * Editor modelines  -	http://www.wireshark.org/tools/modelines.html
 *
 * Local variables:
 * c-basic-offset: 8
 * tab-width: 8
 * indent-tabs-mode: t
 * End:
 *
 * vi: set shiftwidth=8 tabstop=8 noexpandtab:
 * :indentSize=8:tabSize=8:noTabs=false:
 */

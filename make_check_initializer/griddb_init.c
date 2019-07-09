#include <gridstore.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

typedef struct
{
	GSContainer *container;
	GSContainerInfo info;
}			table_info;

GSResult set_tableInfo(GSGridStore * store,
			  const GSChar * tbl_name,
			  table_info * tbl_info,
			  size_t column_count,...);

GSResult griddb_init(const char *addr,
			const char *port,
			const char *cluster_name,
			const char *user,
			const char *passwd);

/**
 * Create table info
 * Arguments: GridStore instance, table name, table info, number of column, [ column1_name, column1_type, column1_options, column2_name, column2_type, column2_options,...
 */
GSResult
set_tableInfo(GSGridStore * store,
			  const GSChar * tbl_name,
			  table_info * tbl_info,
			  size_t column_count,...)
{
	GSResult	ret = GS_RESULT_OK;
	int			i;
	va_list		valist;

	/* Set column info */
	GSColumnInfo column_info = GS_COLUMN_INFO_INITIALIZER;
	GSColumnInfo *column_info_list = calloc(column_count, sizeof(GSColumnInfo));

	tbl_info->info = (GSContainerInfo) GS_CONTAINER_INFO_INITIALIZER;
	tbl_info->info.type = GS_CONTAINER_COLLECTION;
	tbl_info->info.name = tbl_name;
	tbl_info->info.columnCount = column_count;
	va_start(valist, column_count);
	for (i = 0; i < column_count; i++)
	{
		column_info.name = va_arg(valist, GSChar *);
		column_info.type = va_arg(valist, GSType);
		column_info.options = va_arg(valist, GSTypeOption);
		column_info_list[i] = column_info;
	}
	va_end(valist);
	tbl_info->info.columnInfoList = column_info_list;
	tbl_info->info.rowKeyAssigned = GS_TRUE;
	/* Drop the old container if it existed */
	ret = gsDropContainer(store, tbl_info->info.name);
	if (!GS_SUCCEEDED(ret))
	{
		printf("Can not drop container \"%s\"\n", tbl_name);
		return ret;
	}
	/* Create a Collection (Delete if schema setting is NULL) */
	ret = gsPutContainerGeneral(store, NULL, &(tbl_info->info), GS_FALSE, &(tbl_info->container));
	if (!GS_SUCCEEDED(ret))
	{
		printf("Create container \"%s\" failed\n", tbl_name);
		return ret;
	}
	/* Set the autocommit mode to OFF */
	ret = gsSetAutoCommit(tbl_info->container, GS_FALSE);
	if (!GS_SUCCEEDED(ret))
	{
		printf("Set autocommit for container %s failed\n", tbl_name);
		return ret;
	}

	return GS_RESULT_OK;
}

/**
 * Connect to GridDB cluster and insert data to the database
 * Arguments: IP address, port, cluster name, username, password
 */
GSResult
griddb_init(const char *addr,
			const char *port,
			const char *cluster_name,
			const char *user,
			const char *passwd)
{
	GSGridStore *store;
	GSResult	ret = GS_RESULT_OK;

	/* For griddb_fdw */
	table_info	department,
				employee,
				empdata,
				numbers,
				evennumbers,
				shorty;

	/* For griddb_fdw_data_type */
	table_info	type_string,
				type_boolean,
				type_byte,
				type_short,
				type_integer,
				type_long,
				type_float,
				type_double,
				type_timestamp,
				type_blob,
				type_string_array,
				type_bool_array,
				type_byte_array,
				type_short_array,
				type_integer_array,
				type_long_array,
				type_float_array,
				type_double_array,
				type_timestamp_array;

	/* For griddb_fdw_post */
	table_info	T0,
				T1,
				T2,
				T3,
				T4,
				ft1,
				ft2,
				ft4,
				ft5,
				base_tbl,
				loc1,
				loc2,
				loct,
				loct1,
				loct2,
				loct3,
				loct4,
				locp1,
				locp2,
				fprt1_p1,
				fprt1_p2,
				fprt2_p1,
				fprt2_p2,
				pagg_tab_p1,
				pagg_tab_p2,
				pagg_tab_p3;
	const		GSPropertyEntry props[] = {
		{"notificationAddress", addr},
		{"notificationPort", port},
		{"clusterName", cluster_name},
		{"user", user},
		{"password", passwd}
	};
	const size_t prop_count = sizeof(props) / sizeof(*props);

	/* Create a GridStore instance */
	ret = gsGetGridStore(gsGetDefaultFactory(), props, prop_count, &store);
	if (!GS_SUCCEEDED(ret))
	{
		printf("Get GridDB instance failed\n");
		goto EXIT;
	}

	/*
	 * CREATE TABLE department (department_id integer primary key,
	 * department_name text)
	 */
	ret = set_tableInfo(store, "department", &department,
						2,
						"department_id", GS_TYPE_INTEGER, GS_TYPE_OPTION_NOT_NULL,
						"department_name", GS_TYPE_STRING, GS_TYPE_OPTION_NULLABLE);
	if (!GS_SUCCEEDED(ret))
		goto EXIT;

	/*
	 * CREATE TABLE employee (emp_id integer primary key, emp_name text,
	 * emp_dept_id integer)
	 */
	ret = set_tableInfo(store, "employee", &employee,
						3,
						"emp_id", GS_TYPE_INTEGER, GS_TYPE_OPTION_NOT_NULL,
						"emp_name", GS_TYPE_STRING, GS_TYPE_OPTION_NULLABLE,
						"emp_dept_id", GS_TYPE_INTEGER, GS_TYPE_OPTION_NULLABLE);
	if (!GS_SUCCEEDED(ret))
		goto EXIT;
	/* CREATE TABLE empdata (emp_id integer primary key, emp_dat blob) */
	ret = set_tableInfo(store, "empdata", &empdata,
						2,
						"emp_id", GS_TYPE_INTEGER, GS_TYPE_OPTION_NOT_NULL,
						"emp_dat", GS_TYPE_BLOB, GS_TYPE_OPTION_NULLABLE);
	if (!GS_SUCCEEDED(ret))
		goto EXIT;
	/* CREATE TABLE numbers (a integer primary key, b text) */
	ret = set_tableInfo(store, "numbers", &numbers,
						2,
						"a", GS_TYPE_INTEGER, GS_TYPE_OPTION_NOT_NULL,
						"b", GS_TYPE_STRING, GS_TYPE_OPTION_NULLABLE);
	if (!GS_SUCCEEDED(ret))
		goto EXIT;
	/* CREATE TABLE evennumbers (a integer primary key, b text) */
	ret = set_tableInfo(store, "evennumbers", &evennumbers,
						2,
						"a", GS_TYPE_INTEGER, GS_TYPE_OPTION_NOT_NULL,
						"b", GS_TYPE_STRING, GS_TYPE_OPTION_NULLABLE);
	if (!GS_SUCCEEDED(ret))
		goto EXIT;
	/* CREATE TABLE shorty (id integer primary key, c text) */
	ret = set_tableInfo(store, "shorty", &shorty,
						2,
						"id", GS_TYPE_INTEGER, GS_TYPE_OPTION_NOT_NULL,
						"c", GS_TYPE_STRING, GS_TYPE_OPTION_NULLABLE);
	if (!GS_SUCCEEDED(ret))
		goto EXIT;
	/* CREATE TABLE type_string (col1 text primary key, col2 text) */
	ret = set_tableInfo(store, "type_string", &type_string,
						2,
						"col1", GS_TYPE_STRING, GS_TYPE_OPTION_NOT_NULL,
						"col2", GS_TYPE_STRING, GS_TYPE_OPTION_NULLABLE);
	if (!GS_SUCCEEDED(ret))
		goto EXIT;
	/* CREATE TABLE type_boolean (col1 integer primary key, col2 boolean) */
	ret = set_tableInfo(store, "type_boolean", &type_boolean,
						2,
						"col1", GS_TYPE_INTEGER, GS_TYPE_OPTION_NOT_NULL,
						"col2", GS_TYPE_BOOL, GS_TYPE_OPTION_NULLABLE);
	if (!GS_SUCCEEDED(ret))
		goto EXIT;
	/* CREATE TABLE type_byte (col1 integer primary key, col2 char) */
	ret = set_tableInfo(store, "type_byte", &type_byte,
						2,
						"col1", GS_TYPE_INTEGER, GS_TYPE_OPTION_NOT_NULL,
						"col2", GS_TYPE_BYTE, GS_TYPE_OPTION_NULLABLE);
	if (!GS_SUCCEEDED(ret))
		goto EXIT;
	/* CREATE TABLE type_short (col1 integer primary key, col2 short) */
	ret = set_tableInfo(store, "type_short", &type_short,
						2,
						"col1", GS_TYPE_INTEGER, GS_TYPE_OPTION_NOT_NULL,
						"col2", GS_TYPE_SHORT, GS_TYPE_OPTION_NULLABLE);
	if (!GS_SUCCEEDED(ret))
		goto EXIT;
	/* CREATE TABLE type_integer (col1 integer primary key, col2 integer) */
	ret = set_tableInfo(store, "type_integer", &type_integer,
						2,
						"col1", GS_TYPE_INTEGER, GS_TYPE_OPTION_NOT_NULL,
						"col2", GS_TYPE_INTEGER, GS_TYPE_OPTION_NULLABLE);
	if (!GS_SUCCEEDED(ret))
		goto EXIT;
	/* CREATE TABLE type_long (col1 long primary key, col2 long) */
	ret = set_tableInfo(store, "type_long", &type_long,
						2,
						"col1", GS_TYPE_LONG, GS_TYPE_OPTION_NOT_NULL,
						"col2", GS_TYPE_LONG, GS_TYPE_OPTION_NULLABLE);
	if (!GS_SUCCEEDED(ret))
		goto EXIT;
	/* CREATE TABLE type_float (col1 integer primary key, col2 float) */
	ret = set_tableInfo(store, "type_float", &type_float,
						2,
						"col1", GS_TYPE_INTEGER, GS_TYPE_OPTION_NOT_NULL,
						"col2", GS_TYPE_FLOAT, GS_TYPE_OPTION_NULLABLE);
	if (!GS_SUCCEEDED(ret))
		goto EXIT;
	/* CREATE TABLE type_double (col1 integer primary key, col2 double) */
	ret = set_tableInfo(store, "type_double", &type_double,
						2,
						"col1", GS_TYPE_INTEGER, GS_TYPE_OPTION_NOT_NULL,
						"col2", GS_TYPE_DOUBLE, GS_TYPE_OPTION_NULLABLE);
	if (!GS_SUCCEEDED(ret))
		goto EXIT;

	/*
	 * CREATE TABLE type_timestamp (col1 timestamp primary key, col2
	 * timestamp)
	 */
	ret = set_tableInfo(store, "type_timestamp", &type_timestamp,
						2,
						"col1", GS_TYPE_TIMESTAMP, GS_TYPE_OPTION_NOT_NULL,
						"col2", GS_TYPE_TIMESTAMP, GS_TYPE_OPTION_NULLABLE);
	if (!GS_SUCCEEDED(ret))
		goto EXIT;
	/* CREATE TABLE type_blob (col1 integer primary key, col2 blob) */
	ret = set_tableInfo(store, "type_blob", &type_blob,
						2,
						"col1", GS_TYPE_INTEGER, GS_TYPE_OPTION_NOT_NULL,
						"col2", GS_TYPE_BLOB, GS_TYPE_OPTION_NULLABLE);
	if (!GS_SUCCEEDED(ret))
		goto EXIT;
	/* CREATE TABLE type_string_array (col1 integer primary key, col2 text[]) */
	ret = set_tableInfo(store, "type_string_array", &type_string_array,
						2,
						"col1", GS_TYPE_INTEGER, GS_TYPE_OPTION_NOT_NULL,
						"col2", GS_TYPE_STRING_ARRAY, GS_TYPE_OPTION_NULLABLE);
	if (!GS_SUCCEEDED(ret))
		goto EXIT;
	/* CREATE TABLE type_bool_array (col1 integer primary key, col2 boolean[]) */
	ret = set_tableInfo(store, "type_bool_array", &type_bool_array,
						2,
						"col1", GS_TYPE_INTEGER, GS_TYPE_OPTION_NOT_NULL,
						"col2", GS_TYPE_BOOL_ARRAY, GS_TYPE_OPTION_NULLABLE);
	if (!GS_SUCCEEDED(ret))
		goto EXIT;
	/* CREATE TABLE type_byte_array (col1 integer primary key, col2 char[]) */
	ret = set_tableInfo(store, "type_byte_array", &type_byte_array,
						2,
						"col1", GS_TYPE_INTEGER, GS_TYPE_OPTION_NOT_NULL,
						"col2", GS_TYPE_BYTE_ARRAY, GS_TYPE_OPTION_NULLABLE);
	if (!GS_SUCCEEDED(ret))
		goto EXIT;
	/* CREATE TABLE type_short_array (col1 integer primary key, col2 short[]) */
	ret = set_tableInfo(store, "type_short_array", &type_short_array,
						2,
						"col1", GS_TYPE_INTEGER, GS_TYPE_OPTION_NOT_NULL,
						"col2", GS_TYPE_SHORT_ARRAY, GS_TYPE_OPTION_NULLABLE);
	if (!GS_SUCCEEDED(ret))
		goto EXIT;

	/*
	 * CREATE TABLE type_integer_array (col1 integer primary key, col2
	 * integer[])
	 */
	ret = set_tableInfo(store, "type_integer_array", &type_integer_array,
						2,
						"col1", GS_TYPE_INTEGER, GS_TYPE_OPTION_NOT_NULL,
						"col2", GS_TYPE_INTEGER_ARRAY, GS_TYPE_OPTION_NULLABLE);
	if (!GS_SUCCEEDED(ret))
		goto EXIT;
	/* CREATE TABLE type_long_array (col1 integer primary key, col2 long[]) */
	ret = set_tableInfo(store, "type_long_array", &type_long_array,
						2,
						"col1", GS_TYPE_INTEGER, GS_TYPE_OPTION_NOT_NULL,
						"col2", GS_TYPE_LONG_ARRAY, GS_TYPE_OPTION_NULLABLE);
	if (!GS_SUCCEEDED(ret))
		goto EXIT;
	/* CREATE TABLE type_float_array (col1 integer primary key, col2 float[]) */
	ret = set_tableInfo(store, "type_float_array", &type_float_array,
						2,
						"col1", GS_TYPE_INTEGER, GS_TYPE_OPTION_NOT_NULL,
						"col2", GS_TYPE_FLOAT_ARRAY, GS_TYPE_OPTION_NULLABLE);
	if (!GS_SUCCEEDED(ret))
		goto EXIT;

	/*
	 * CREATE TABLE type_double_array (col1 integer primary key, col2
	 * double[])
	 */
	ret = set_tableInfo(store, "type_double_array", &type_double_array,
						2,
						"col1", GS_TYPE_INTEGER, GS_TYPE_OPTION_NOT_NULL,
						"col2", GS_TYPE_DOUBLE_ARRAY, GS_TYPE_OPTION_NULLABLE);
	if (!GS_SUCCEEDED(ret))
		goto EXIT;

	/*
	 * CREATE TABLE type_timestamp_array (col1 integer primary key, col2
	 * timestamp[])
	 */
	ret = set_tableInfo(store, "type_timestamp_array", &type_timestamp_array,
						2,
						"col1", GS_TYPE_INTEGER, GS_TYPE_OPTION_NOT_NULL,
						"col2", GS_TYPE_TIMESTAMP_ARRAY, GS_TYPE_OPTION_NULLABLE);
	if (!GS_SUCCEEDED(ret))
		goto EXIT;

	ret = set_tableInfo(store, "T0", &T0,
						8,
						"c1", GS_TYPE_INTEGER, GS_TYPE_OPTION_NOT_NULL,
						"c2", GS_TYPE_INTEGER, GS_TYPE_OPTION_NOT_NULL,
						"c3", GS_TYPE_STRING, GS_TYPE_OPTION_NULLABLE,
						"c4", GS_TYPE_TIMESTAMP, GS_TYPE_OPTION_NULLABLE,
						"c5", GS_TYPE_TIMESTAMP, GS_TYPE_OPTION_NULLABLE,
						"c6", GS_TYPE_STRING, GS_TYPE_OPTION_NULLABLE,
						"c7", GS_TYPE_STRING, GS_TYPE_OPTION_NULLABLE,
						"c8", GS_TYPE_STRING, GS_TYPE_OPTION_NULLABLE);
	if (!GS_SUCCEEDED(ret))
		goto EXIT;

	ret = set_tableInfo(store, "T1", &T1,
						8,
						"c1", GS_TYPE_INTEGER, GS_TYPE_OPTION_NOT_NULL,
						"c2", GS_TYPE_INTEGER, GS_TYPE_OPTION_NOT_NULL,
						"c3", GS_TYPE_STRING, GS_TYPE_OPTION_NULLABLE,
						"c4", GS_TYPE_TIMESTAMP, GS_TYPE_OPTION_NULLABLE,
						"c5", GS_TYPE_TIMESTAMP, GS_TYPE_OPTION_NULLABLE,
						"c6", GS_TYPE_STRING, GS_TYPE_OPTION_NULLABLE,
						"c7", GS_TYPE_STRING, GS_TYPE_OPTION_NULLABLE,
						"c8", GS_TYPE_STRING, GS_TYPE_OPTION_NULLABLE);
	if (!GS_SUCCEEDED(ret))
		goto EXIT;

	ret = set_tableInfo(store, "T2", &T2,
						2,
						"c1", GS_TYPE_INTEGER, GS_TYPE_OPTION_NOT_NULL,
						"c2", GS_TYPE_STRING, GS_TYPE_OPTION_NULLABLE);
	if (!GS_SUCCEEDED(ret))
		goto EXIT;

	ret = set_tableInfo(store, "T3", &T3,
						3,
						"c1", GS_TYPE_INTEGER, GS_TYPE_OPTION_NOT_NULL,
						"c2", GS_TYPE_INTEGER, GS_TYPE_OPTION_NOT_NULL,
						"c3", GS_TYPE_STRING, GS_TYPE_OPTION_NULLABLE);
	if (!GS_SUCCEEDED(ret))
		goto EXIT;

	ret = set_tableInfo(store, "T4", &T4,
						3,
						"c1", GS_TYPE_INTEGER, GS_TYPE_OPTION_NOT_NULL,
						"c2", GS_TYPE_INTEGER, GS_TYPE_OPTION_NOT_NULL,
						"c3", GS_TYPE_STRING, GS_TYPE_OPTION_NULLABLE);
	if (!GS_SUCCEEDED(ret))
		goto EXIT;

	ret = set_tableInfo(store, "ft1", &ft1,
						8,
						"c1", GS_TYPE_INTEGER, GS_TYPE_OPTION_NOT_NULL,
						"c2", GS_TYPE_INTEGER, GS_TYPE_OPTION_NOT_NULL,
						"c3", GS_TYPE_STRING, GS_TYPE_OPTION_NULLABLE,
						"c4", GS_TYPE_TIMESTAMP, GS_TYPE_OPTION_NULLABLE,
						"c5", GS_TYPE_TIMESTAMP, GS_TYPE_OPTION_NULLABLE,
						"c6", GS_TYPE_STRING, GS_TYPE_OPTION_NULLABLE,
						"c7", GS_TYPE_STRING, GS_TYPE_OPTION_NULLABLE,
						"c8", GS_TYPE_STRING, GS_TYPE_OPTION_NULLABLE);
	if (!GS_SUCCEEDED(ret))
		goto EXIT;

	ret = set_tableInfo(store, "ft2", &ft2,
						2,
						"c1", GS_TYPE_INTEGER, GS_TYPE_OPTION_NOT_NULL,
						"c2", GS_TYPE_STRING, GS_TYPE_OPTION_NULLABLE);
	if (!GS_SUCCEEDED(ret))
		goto EXIT;

	ret = set_tableInfo(store, "ft4", &ft4,
						3,
						"c1", GS_TYPE_INTEGER, GS_TYPE_OPTION_NOT_NULL,
						"c2", GS_TYPE_INTEGER, GS_TYPE_OPTION_NOT_NULL,
						"c3", GS_TYPE_STRING, GS_TYPE_OPTION_NULLABLE);
	if (!GS_SUCCEEDED(ret))
		goto EXIT;

	ret = set_tableInfo(store, "ft5", &ft5,
						3,
						"c1", GS_TYPE_INTEGER, GS_TYPE_OPTION_NOT_NULL,
						"c2", GS_TYPE_INTEGER, GS_TYPE_OPTION_NOT_NULL,
						"c3", GS_TYPE_STRING, GS_TYPE_OPTION_NULLABLE);
	if (!GS_SUCCEEDED(ret))
		goto EXIT;

	ret = set_tableInfo(store, "base_tbl", &base_tbl,
						2,
						"a", GS_TYPE_INTEGER, GS_TYPE_OPTION_NOT_NULL,
						"b", GS_TYPE_INTEGER, GS_TYPE_OPTION_NULLABLE);
	if (!GS_SUCCEEDED(ret))
		goto EXIT;

	ret = set_tableInfo(store, "loc1", &loc1,
						2,
						"f1", GS_TYPE_INTEGER, GS_TYPE_OPTION_NOT_NULL,
						"f2", GS_TYPE_STRING, GS_TYPE_OPTION_NOT_NULL);
	if (!GS_SUCCEEDED(ret))
		goto EXIT;

	ret = set_tableInfo(store, "loc2", &loc2,
						2,
						"f1", GS_TYPE_INTEGER, GS_TYPE_OPTION_NOT_NULL,
						"f2", GS_TYPE_STRING, GS_TYPE_OPTION_NULLABLE);
	if (!GS_SUCCEEDED(ret))
		goto EXIT;

	ret = set_tableInfo(store, "loct", &loct,
						2,
						"aa", GS_TYPE_STRING, GS_TYPE_OPTION_NOT_NULL,
						"bb", GS_TYPE_STRING, GS_TYPE_OPTION_NULLABLE);
	if (!GS_SUCCEEDED(ret))
		goto EXIT;

	ret = set_tableInfo(store, "loct1", &loct1,
						3,
						"f1", GS_TYPE_INTEGER, GS_TYPE_OPTION_NOT_NULL,
						"f2", GS_TYPE_INTEGER, GS_TYPE_OPTION_NULLABLE,
						"f3", GS_TYPE_INTEGER, GS_TYPE_OPTION_NULLABLE);
	if (!GS_SUCCEEDED(ret))
		goto EXIT;

	ret = set_tableInfo(store, "loct2", &loct2,
						3,
						"f1", GS_TYPE_INTEGER, GS_TYPE_OPTION_NOT_NULL,
						"f2", GS_TYPE_INTEGER, GS_TYPE_OPTION_NULLABLE,
						"f3", GS_TYPE_INTEGER, GS_TYPE_OPTION_NULLABLE);
	if (!GS_SUCCEEDED(ret))
		goto EXIT;

	ret = set_tableInfo(store, "loct3", &loct3,
						2,
						"a", GS_TYPE_INTEGER, GS_TYPE_OPTION_NOT_NULL,
						"b", GS_TYPE_STRING, GS_TYPE_OPTION_NULLABLE);
	if (!GS_SUCCEEDED(ret))
		goto EXIT;

	ret = set_tableInfo(store, "loct4", &loct4,
						2,
						"a", GS_TYPE_INTEGER, GS_TYPE_OPTION_NOT_NULL,
						"b", GS_TYPE_STRING, GS_TYPE_OPTION_NULLABLE);
	if (!GS_SUCCEEDED(ret))
		goto EXIT;

	ret = set_tableInfo(store, "locp1", &locp1,
						3,
						"a", GS_TYPE_INTEGER, GS_TYPE_OPTION_NOT_NULL,
						"b", GS_TYPE_INTEGER, GS_TYPE_OPTION_NULLABLE,
						"c", GS_TYPE_STRING, GS_TYPE_OPTION_NULLABLE);
	if (!GS_SUCCEEDED(ret))
		goto EXIT;

	ret = set_tableInfo(store, "locp2", &locp2,
						3,
						"a", GS_TYPE_INTEGER, GS_TYPE_OPTION_NOT_NULL,
						"b", GS_TYPE_INTEGER, GS_TYPE_OPTION_NULLABLE,
						"c", GS_TYPE_STRING, GS_TYPE_OPTION_NULLABLE);
	if (!GS_SUCCEEDED(ret))
		goto EXIT;

	ret = set_tableInfo(store, "fprt1_p1", &fprt1_p1,
						3,
						"a", GS_TYPE_INTEGER, GS_TYPE_OPTION_NOT_NULL,
						"b", GS_TYPE_INTEGER, GS_TYPE_OPTION_NULLABLE,
						"c", GS_TYPE_STRING, GS_TYPE_OPTION_NULLABLE);
	if (!GS_SUCCEEDED(ret))
		goto EXIT;

	ret = set_tableInfo(store, "fprt1_p2", &fprt1_p2,
						3,
						"a", GS_TYPE_INTEGER, GS_TYPE_OPTION_NOT_NULL,
						"b", GS_TYPE_INTEGER, GS_TYPE_OPTION_NULLABLE,
						"c", GS_TYPE_STRING, GS_TYPE_OPTION_NULLABLE);
	if (!GS_SUCCEEDED(ret))
		goto EXIT;

	ret = set_tableInfo(store, "fprt2_p1", &fprt2_p1,
						3,
						"a", GS_TYPE_INTEGER, GS_TYPE_OPTION_NOT_NULL,
						"b", GS_TYPE_INTEGER, GS_TYPE_OPTION_NULLABLE,
						"c", GS_TYPE_STRING, GS_TYPE_OPTION_NULLABLE);
	if (!GS_SUCCEEDED(ret))
		goto EXIT;

	ret = set_tableInfo(store, "fprt2_p2", &fprt2_p2,
						3,
						"a", GS_TYPE_INTEGER, GS_TYPE_OPTION_NOT_NULL,
						"b", GS_TYPE_INTEGER, GS_TYPE_OPTION_NULLABLE,
						"c", GS_TYPE_STRING, GS_TYPE_OPTION_NULLABLE);
	if (!GS_SUCCEEDED(ret))
		goto EXIT;

	ret = set_tableInfo(store, "pagg_tab_p1", &pagg_tab_p1,
						4,
						"t", GS_TYPE_INTEGER, GS_TYPE_OPTION_NOT_NULL,
						"a", GS_TYPE_INTEGER, GS_TYPE_OPTION_NOT_NULL,
						"b", GS_TYPE_INTEGER, GS_TYPE_OPTION_NULLABLE,
						"c", GS_TYPE_STRING, GS_TYPE_OPTION_NULLABLE);
	if (!GS_SUCCEEDED(ret))
		goto EXIT;

	ret = set_tableInfo(store, "pagg_tab_p2", &pagg_tab_p2,
						4,
						"t", GS_TYPE_INTEGER, GS_TYPE_OPTION_NOT_NULL,
						"a", GS_TYPE_INTEGER, GS_TYPE_OPTION_NOT_NULL,
						"b", GS_TYPE_INTEGER, GS_TYPE_OPTION_NULLABLE,
						"c", GS_TYPE_STRING, GS_TYPE_OPTION_NULLABLE);
	if (!GS_SUCCEEDED(ret))
		goto EXIT;

	ret = set_tableInfo(store, "pagg_tab_p3", &pagg_tab_p3,
						4,
						"t", GS_TYPE_INTEGER, GS_TYPE_OPTION_NOT_NULL,
						"a", GS_TYPE_INTEGER, GS_TYPE_OPTION_NOT_NULL,
						"b", GS_TYPE_INTEGER, GS_TYPE_OPTION_NULLABLE,
						"c", GS_TYPE_STRING, GS_TYPE_OPTION_NULLABLE);
	if (!GS_SUCCEEDED(ret))
		goto EXIT;

EXIT:
	/* Release the resource */
	gsCloseGridStore(&store, GS_TRUE);
	return ret;
}

/* Main funtion */
GSResult
main(int argc, char *argv[])
{
	GSResult	ret = !GS_RESULT_OK;
	int			i = 0;
	char	   *key,
			   *value;
	char	   *host,
			   *port,
			   *cluster,
			   *user,
			   *passwd;

	if (argc < 6)
	{
		printf("Missing arguments\n");
		goto TERMINATE;
	}
	else
	{
		for (i = 1; i < argc; i++)
		{
			/* Argument format: key=value */
			key = strtok(argv[i], "=");
			value = strtok(NULL, " ");
			if (value == NULL)
			{
				printf("Invalid options\n"
					   "Usage:\n    ./griddb_init host=a port=b cluster=c user=d passwd=e\n");
				goto TERMINATE;
			}
			else
			{
				if (strcmp(key, "host") == 0)
				{
					host = value;
				}
				else if (strcmp(key, "port") == 0)
				{
					/* Validate port number limitation */
					if (atoi(value) > 65535 || atoi(value) < 0)
					{
						printf("Invalid port number\n");
						goto TERMINATE;
					}
					port = value;
				}
				else if (strcmp(key, "cluster") == 0)
				{
					cluster = value;
				}
				else if (strcmp(key, "user") == 0)
				{
					user = value;
				}
				else if (strcmp(key, "passwd") == 0)
				{
					passwd = value;
				}
				else
				{
					printf("Invalid options\n"
						   "Usage:\n    ./griddb_init host=a port=b cluster=c user=d passwd=e\n");
					goto TERMINATE;
				}
			}
		}
	}
	ret = griddb_init(host, port, cluster, user, passwd);
	if (GS_SUCCEEDED(ret))
	{
		printf("Initialize all containers sucessfully.\n");
	}
	else
	{
		printf("Initializer has some problems!\n");
	}
TERMINATE:
	return ret;
}
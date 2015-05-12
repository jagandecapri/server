/*
   Copyright (c) 2014, SkySQL Ab & MariaDB Foundation

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */

/*
  This file implements the group_by_handler interface. This interface
  can be used by storage handlers that can intercept summary or GROUP
  BY queries from MariaDB and itself return the result to the user or
  upper level.

  Both main and sub queries are supported. Here are some examples of what the
  storage engine could intersept:

  SELECT count(*) FROM t1;
  SELECT a,count(*) FROM t1 group by a;
  SELECT a,count(*) as sum FROM t1 where b > 10 group by a, order by sum;
  SELECT a,count(*) FROM t1,t2;
  SELECT a, (select sum(*) from t2 where t1.a=t2.a) from t2;

  See https://mariadb.atlassian.net/browse/MDEV-6080 for more information.
*/

class JOIN;

class group_by_handler
{
public:
  /* Arguments for group_by_handler, for usage later */
  THD *thd;
  SELECT_LEX *select_lex;
  List<Item> *fields;
  TABLE_LIST *table_list;
  ORDER *group_by, *order_by;
  Item *where, *having;
  handlerton *ht;                 /* storage engine of this handler */

  /*
    Bit's of things the storage engine can do for this query.
    Should be initialized on object creation.
  */
  /* Temporary table where all results should be stored in record[0] */
  TABLE *table;

  bool store_data_in_temp_table;  /* Set by mariadb */

  group_by_handler(THD *thd_arg, SELECT_LEX *select_lex_arg,
                   List<Item> *fields_arg,
                   TABLE_LIST *table_list_arg, ORDER *group_by_arg,
                   ORDER *order_by_arg, Item *where_arg,
                   Item *having_arg, handlerton *ht_arg)
    : thd(thd_arg), select_lex(select_lex_arg), fields(fields_arg),
    table_list(table_list_arg), group_by(group_by_arg),
    order_by(order_by_arg), where(where_arg), having(having_arg),
    ht(ht_arg), table(0), store_data_in_temp_table(0)
  {}
  virtual ~group_by_handler() {}

  /*
    Store pointer to temporary table and objects modified to point to
    the temporary table.  This will happen during the optimize phase.

    We provide new 'having' and 'order_by' elements here. The differ from the
    original ones in that these are modified to point to fields in the
    temporary table 'table'.

    Return 1 if the storage handler cannot handle the GROUP BY after all,
    in which case we have to give an error to the end user for the query.
    This is becasue we can't revert back the old having and order_by elements.
  */

  virtual bool init(TABLE *temporary_table, Item *having_arg,
                    ORDER *order_by_arg)
  {
    table= temporary_table;
    having= having_arg;
    order_by= order_by_arg;
    return 0;
  }

  /*
    Result data is sorted by the storage engine according to order_by (if it
    exists) else according to the group_by.  If this is not specified,
    MariaDB will store the result set into the temporary table and sort the
    result.
  */
  #define GROUP_BY_ORDER_BY 1
  /* The storage engine can handle DISTINCT */
  #define GROUP_BY_DISTINCT 2
  virtual uint flags() { return 0; }

  /*
    Functions to scan data. All these returns 0 if ok, error code in case
    of error
  */

  /*
    Initialize group_by scan, prepare for next_row().
    If this is a sub query with group by, this can be called many times for
    a query.
  */
  virtual int init_scan()= 0;

  /*
    Return next group by result in table->record[0].
    Return 0 if row found, HA_ERR_END_OF_FILE if last row and other error
    number in case of fatal error.
   */
  virtual int next_row()= 0;

  /* End scanning */
  virtual int end_scan()=0;

  /* Information for optimizer (used by EXPLAIN) */
  virtual int info(uint flag, ha_statistics *stats)= 0;

  /* Function that calls the above scan functions */
  int execute(JOIN *join);

  /* Report errors */
  virtual void print_error(int error, myf errflag);
};

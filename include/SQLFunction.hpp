#ifndef SQLFunction_H
#define SQLFunction_H

extern MYSQL *mysql_con;
extern MYSQL_RES *mysql_result;
extern MYSQL_ROW mysql_row;
extern char sql_buffer[2000];

MYSQL_ROW fetch_row_value();
void sent_query();
int turn_int(int col_num);
float turn_float(int col_num);
int turn_value_to_int(int col_num);
float turn_value_to_float(int col_num);

#endif 
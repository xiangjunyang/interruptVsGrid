#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <glpk.h>
#include <math.h>
#include <mysql/mysql.h>
#include <iostream>
#include <mysql/mysql.h>
// #include "HEMS.h" 
#include "SQLFunction.hpp" 

#define NEW2D(H, W, TYPE) (TYPE **)new2d(H, W, sizeof(TYPE))
void *new2d(int, int, int);
void GLPK(int *, int *, int *, int *, float *, int, float *, int *);

int interrupt_num = 0, app_count = 0, sample_time = 0, variable = 0, divide = 4, time_block = 96;
int h, i, j, k, m, n = 0;
double z = 0;
float Pgrid_max = 0.0, delta_T = 0.25;
char sql_buffer[2000] = { '\0' };

time_t t = time(NULL);
struct tm now_time = *localtime(&t);
using namespace std;

char column[400] = "A0,A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13,A14,A15,A16,A17,A18,A19,A20,A21,A22,A23,A24,A25,A26,A27,A28,A29,A30,A31,A32,A33,A34,A35,A36,A37,A38,A39,A40,A41,A42,A43,A44,A45,A46,A47,A48,A49,A50,A51,A52,A53,A54,A55,A56,A57,A58,A59,A60,A61,A62,A63,A64,A65,A66,A67,A68,A69,A70,A71,A72,A73,A74,A75,A76,A77,A78,A79,A80,A81,A82,A83,A84,A85,A86,A87,A88,A89,A90,A91,A92,A93,A94,A95";


int main(void)
{

	if ((mysql_real_connect(mysql_con, "140.124.42.70", "root", "fuzzy314", "wang", 6666, NULL, 0)) == NULL) {

		printf("Failed to connect to Mysql!\n");
		system("pause");
		return 0;

	}
	printf("Connect to Mysql sucess!!\n");
	mysql_set_character_set(mysql_con, "utf8");

    // get count = 3 of interrupt group 
    snprintf(sql_buffer, sizeof(sql_buffer), "SELECT count(*) AS numcols FROM load_list WHERE group_id=1 && number>=3 && number<6 ");
	interrupt_num = turn_value_to_int(0);
	printf("interruptable app num:%d\n", interrupt_num);

	snprintf(sql_buffer, sizeof(sql_buffer), "SELECT value FROM `LP_BASE_PARM` WHERE parameter_id = %d", 13);
	Pgrid_max = turn_value_to_float(0);
	printf("Pgrid_max:%.2f\n", Pgrid_max);

    app_count = interrupt_num;  // 3
	variable = app_count + 1;  // 買電狀態
	int *position = new int[app_count];
	float **INT_power = NEW2D(interrupt_num, 4, float);

    for (i = 1; i < interrupt_num + 1; i++) {

		snprintf(sql_buffer, sizeof(sql_buffer), "SELECT start_time, end_time, operation_time, power1 FROM load_list WHERE group_id = 1 ORDER BY number ASC LIMIT %d,1", i + 1);
		fetch_row_value();
		for (j = 0; j < 4; j++)
		{INT_power[i - 1][j] = turn_float(j);}

	}
	
	float *price = new float[24];
    int *interrupt_start = new int[interrupt_num];
	int *interrupt_end = new int[interrupt_num];
	int *interrupt_ot = new int[interrupt_num];
	int *interrupt_reot = new int[interrupt_num];
	float *interrupt_p = new float[interrupt_num];

    // initialize INT_power[interrupt num][4] = 0
    for (i = 0; i < interrupt_num; i++) {

		interrupt_start[i] = 0;
		interrupt_end[i] = 0;
		interrupt_ot[i] = 0;
		interrupt_reot[i] = 0;
		interrupt_p[i] = 0.0;

	}
    // interrupt load array: INT_power[interrupt num][4] 
	printf("interrupt multi array: \n");
    for (i = 0; i < interrupt_num; i++)	{

		interrupt_start[i] = ((int)(INT_power[i][0] * divide));
		interrupt_end[i] = ((int)(INT_power[i][1] * divide)) - 1;
		interrupt_ot[i] = ((int)(INT_power[i][2] * divide));
		interrupt_p[i] = INT_power[i][3];
		cout<<interrupt_start[i]<<"\t"<<interrupt_end[i]<<"\t"<<interrupt_ot[i]<<"\t"<<interrupt_p[i]<<"\n";

	
    }
    // price
    for (i = 1; i < 25; i++) {

		snprintf(sql_buffer, sizeof(sql_buffer), "SELECT price_value FROM price WHERE price_period = %d", i - 1);
		// fetch_row_value();
		// price[i - 1] = atof(mysql_row[0]);		
		price[i - 1] = turn_value_to_float(0);			
		memset(sql_buffer, 0, sizeof(sql_buffer));


	}

	for (i = 0; i < app_count; i++) {
		snprintf(sql_buffer, sizeof(sql_buffer), "select number from load_list WHERE group_id<>0 ORDER BY group_id ASC,number ASC LIMIT %d,1", i);
		// fetch_row_value();
		// position[i] = atoi(mysql_row[0]);
		position[i] = turn_value_to_int(0);
		cout<<position[i]<<" ";
	}

	cout << "\npostion finish " << endl;

	snprintf(sql_buffer, sizeof(sql_buffer), "SELECT value FROM LP_BASE_PARM WHERE parameter_id = 15 ");
	int real_time = turn_value_to_int(0);

	snprintf(sql_buffer, sizeof(sql_buffer), "SELECT value FROM LP_BASE_PARM WHERE parameter_id = 28 ");
	sample_time = turn_value_to_int(0);
	
	cout<< "real time "<< real_time << "\t sample time "<< sample_time << endl;

	if (real_time == 0)
	{
		snprintf(sql_buffer, sizeof(sql_buffer), "TRUNCATE TABLE control_status"); //clean control_status;
		sent_query();
		snprintf(sql_buffer, sizeof(sql_buffer), "TRUNCATE TABLE real_status"); //clean control_status;
		sent_query();
		snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE LP_BASE_PARM set value = 0 where parameter_id=28 ");
		sent_query();
		real_time = 1;
		snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE LP_BASE_PARM SET value = %d WHERE parameter_id = 15 ", real_time);
		sent_query();
	}
	else
	{
		snprintf(sql_buffer, sizeof(sql_buffer), "TRUNCATE TABLE real_status"); //clean control_status;
		sent_query();
	}

	snprintf(sql_buffer, sizeof(sql_buffer), "SELECT value FROM LP_BASE_PARM WHERE parameter_id = 28 ");
	sample_time = turn_value_to_int(0);

    GLPK(interrupt_start, interrupt_end, interrupt_ot, interrupt_reot, interrupt_p, app_count, price, position);
	
	sample_time++;
	printf("\nupdate time block to %d\n",sample_time);
	snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE LP_BASE_PARM set value = %d where parameter_id= 28", sample_time);
	sent_query();
}

void GLPK(int *interrupt_start, int *interrupt_end, int *interrupt_ot, int *interrupt_reot, float *interrupt_p, int app_count, float *price, int *position)
{
	int *buff = new int[app_count];	//存放剩餘執行次數(The number of remaining executions)
	for (i = 0; i < app_count; i++)
	{
		buff[i] = 0;
	}
	int noo;
	//get now time that can used in the real experiment
	if (((now_time.tm_min) % (60 / divide)) != 0)
	{
		noo = (now_time.tm_hour) * divide + (int)((now_time.tm_min) / (60 / divide)) + 1;
	}
	else
	{
		noo = (now_time.tm_hour) * divide + (int)((now_time.tm_min) / (60 / divide));
	}
	printf("sampleNoo:%d\n", noo);

	float *price2 = new float[time_block];
	for (int x = 0; x < 24; x++)	
	{
		for (int y = x*divide; y < (x*divide)+divide; y++)
		{
			price2[y] = price[x];
		}
	}

	if (sample_time != 0)
	{
		for (i = 1; i <= app_count; i++)
		{
			int coun = 0;

			snprintf(sql_buffer, sizeof(sql_buffer), "SELECT %s FROM control_status WHERE (control_id = '%d')", column, i);
			mysql_query(mysql_con, sql_buffer);
			mysql_result = mysql_store_result(mysql_con);
			mysql_row = mysql_fetch_row(mysql_result);
			for (j = 0; j < sample_time; j++)
			{
				coun += atoi(mysql_row[j]);
			}
			buff[i - 1] = coun;
			printf("buff[%d]: %d ",i ,coun);
			memset(sql_buffer, 0, sizeof(sql_buffer));
		}
		mysql_free_result(mysql_result);
	}
	printf("\n");

	for (i = 0; i < interrupt_num; i++)	//可中斷負載 (Interrupt load)
	{
		if ((interrupt_ot[i] - buff[i]) == interrupt_ot[i])
		{
			interrupt_reot[i] = interrupt_ot[i];
		}
		else if (((interrupt_ot[i] - buff[i]) < interrupt_ot[i]) && ((interrupt_ot[i] - buff[i]) > 0))
		{
			interrupt_reot[i] = interrupt_ot[i] - buff[i];
		}
		else if ((interrupt_ot[i] - buff[i]) <= 0)
		{
			interrupt_reot[i] = 0;
		}
	}
 
	float *s = new float[time_block];
	/*============================ 總規劃功率矩陣(Total planning power matrix) ====================================*/
	float **power1 = NEW2D((((time_block - sample_time) * 1) + app_count), (variable * (time_block - sample_time)), float);

	/*============================ GLPK參數矩陣定義(GLPK parameter matrix definition) ==================================*/
	glp_prob *mip;
	int *ia = new int[((((time_block - sample_time) * 1) + app_count) * (variable * (time_block - sample_time))) + 1]; 			// Row
	int *ja = new int[((((time_block - sample_time) * 1) + app_count) * (variable * (time_block - sample_time))) + 1];			// Column
	double *ar = new double[((((time_block - sample_time) * 1) + app_count) * (variable * (time_block - sample_time))) + 1];		// structural variable
	/*============================== GLPK變數宣告(GLPK variable definition) =====================================*/
	mip = glp_create_prob();
	glp_set_prob_name(mip, "hardware_algorithm_case");
	glp_set_obj_dir(mip, GLP_MIN);
	glp_add_rows(mip, (((time_block - sample_time) * 1) + app_count));
	glp_add_cols(mip, (variable * (time_block - sample_time)));	

	/*=============================== 初始化矩陣(initial the matrix) ======================================*/
	for (m = 0; m < ((time_block - sample_time) * 1) + app_count; m++)
	{
		for (n = 0; n < (variable * (time_block - sample_time)); n++)
		{
			power1[m][n] = 0.0;
		}
	}

	for (h = 0; h < interrupt_num; h++)		// 可中斷負載(Interrupt load)
	{
		if ((interrupt_end[h] - sample_time) >= 0)
		{
			if ((interrupt_start[h] - sample_time) >= 0)
			{
				for (i = (interrupt_start[h] - sample_time); i <= (interrupt_end[h] - sample_time); i++)
				{
					power1[h][i*variable + h] = 1.0;
				}
			}
			else if ((interrupt_start[h] - sample_time) < 0)
			{
				for (i = 0; i <= (interrupt_end[h] - sample_time); i++)
				{
					power1[h][i*variable + h] = 1.0;
				}
			}
		}
	}

	// 決定是否輸出市電(Decide whether to buy electricity from utility)
	for (i = 0; i < (time_block - sample_time); i++)
	{
		power1[app_count + i][i*variable + app_count] = -1.0; // Pgrid
	}

	// ========================== 平衡式(Balanced function) ==========================
	// example >>
	//[3][0] [4][4] [5][8] .... [8][20] ... [13][40] i=0~10, h=0
	//       [4][5] [5][9] .... [8][21] 			 i=1~5, h=1
	//[3][2] [4][6] [5][10] 						 i=0~2, h=2
	//    0   1   2   3  |  4   5   6   7  |  8   9 ...
	// 0  .              |                 |
	// 1  .              |                 |
	// 2  .              |                 |
	// 3  P1  0  P3   0  |  0   0   0   0  |  0   0
	// 4  0   0  0   P1  | P2  P3   0   0  |  0   0
	// 5  0   0  0    0  |  0   0   0  P1  | P2  P3
	for (h = 0; h < interrupt_num; h++)	// 可中斷負載(Interrupt load) 
	{
		if ((interrupt_end[h] - sample_time) >= 0)
		{
			if ((interrupt_start[h] - sample_time) >= 0)
			{
				for (i = (interrupt_start[h] - sample_time); i <= (interrupt_end[h] - sample_time); i++)
				{
					power1[app_count + i][i*variable + h] = interrupt_p[h];
				}
			}
			else if ((interrupt_start[h] - sample_time) < 0)
			{
				for (i = 0; i <= (interrupt_end[h] - sample_time); i++)
				{
					power1[app_count + i][i*variable + h] = interrupt_p[h];
				}
			}
		}
	}
	
	/*============================== 宣告限制式條件範圍(row) ===============================*/
	// GLPK讀列從1開始
	// 限制式-家庭負載最低耗能
	for (i = 1; i <= interrupt_num; i++)	// 可中斷負載(Interrupt load)
	{
		glp_set_row_name(mip, i, "");
		glp_set_row_bnds(mip, i, GLP_LO, ((float)interrupt_reot[i - 1]), 0.0);	// ok
	}

	// 決定是否輸出市電
	for (i = 1; i <= (time_block - sample_time); i++)
	{
		glp_set_row_name(mip, (app_count + i), "");
		glp_set_row_bnds(mip, (app_count + i), GLP_UP, 0.0, 0.0);
	}
	
	/*============================== 宣告決策變數(column) ================================*/
	for (i = 0; i < (time_block - sample_time); i++)
	{
		for (j = 1; j <= app_count; j++)
		{
			glp_set_col_bnds(mip, (j + i*variable), GLP_DB, 0.0, 1.0);	// 負載決策變數
			glp_set_col_kind(mip, (j + i*variable), GLP_BV);
		}
		glp_set_col_bnds(mip, ((app_count + 1) + i*variable), GLP_DB, 0.0, Pgrid_max);	// 決定市電輸出功率  一定要大於總負載功率才不會有太大問題
		glp_set_col_kind(mip, ((app_count + 1) + i*variable), GLP_CV);
	}
	

	/*============================== 宣告目標式參數(column) ===============================*/
	for (j = 0; j < (time_block - sample_time); j++)
	{
		glp_set_obj_coef(mip, (app_count + 1 + j*variable), price2[j + sample_time] * delta_T);		// 單目標cost(步驟一)
	}

	/*============================== GLPK寫入矩陣(ia,ja,ar) ===============================*/
	for (i = 0; i < (((time_block - sample_time) * 1) + app_count); i++)
	{
		for (j = 0; j < (variable * (time_block - sample_time)); j++)
		{
			ia[i*((time_block - sample_time)*variable) + j + 1] = i + 1;
			ja[i*((time_block - sample_time)*variable) + j + 1] = j + 1;
			ar[i*((time_block - sample_time)*variable) + j + 1] = power1[i][j];
		}
	}
	printf("array finish\n");	
	/*============================== GLPK讀取資料矩陣 ====================================*/
	glp_load_matrix(mip, (((time_block - sample_time) * 1) + app_count)*(variable * (time_block - sample_time)), ia, ja, ar);

	glp_iocp parm;
	glp_init_iocp(&parm);
	parm.tm_lim = 100000;
        
	parm.presolve = GLP_ON;
	parm.gmi_cuts = GLP_ON;
	parm.fp_heur = GLP_ON;
	parm.bt_tech = GLP_BT_BFS;
	parm.br_tech = GLP_BR_PCH;

	int err = glp_intopt(mip, &parm);
	z = glp_mip_obj_val(mip);
	printf("%f\n",glp_mip_col_val(mip,1));
	printf("%f\n",glp_mip_col_val(mip,2));
	printf("%f\n",glp_mip_col_val(mip,3));
	printf("%.2f\n", glp_mip_col_val(mip,4));

	printf("\n");
	printf("sol = %f; \n", z);

	// if (z == 0.0 && glp_mip_col_val(mip, (app_count + 7)) == 0.0)
	// {
	// 	printf("No Solotion,give up the solution\n");
	// 	system("pause");
	// 	exit(1);
	// }

	/*============================== 將決策變數結果輸出 ==================================*/
	for (i = 1; i <= variable; i++)
	{
		h = i;

		if (sample_time == 0)
		{
			for (j = 0; j < time_block; j++)
			{
				s[j] = glp_mip_col_val(mip, h);

				if (i <= app_count && j== noo)
				{
					snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE now_status set status = %d where id=%d ", (int)s[j], position[i-1]);
					mysql_query(mysql_con, sql_buffer);
					snprintf(sql_buffer, sizeof(sql_buffer), "INSERT INTO control_history (id,status,schedule) VALUES(%d,%d,%d)", position[i - 1], (int)s[j], 1);
					mysql_query(mysql_con, sql_buffer);
				}
				h = (h + variable);
			}

			snprintf(sql_buffer, sizeof(sql_buffer), "INSERT INTO control_status (%s, equip_id) VALUES('%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%d');"
				, column, s[0], s[1], s[2], s[3], s[4], s[5], s[6], s[7], s[8], s[9], s[10], s[11], s[12], s[13], s[14], s[15], s[16], s[17], s[18], s[19], s[20], s[21], s[22], s[23], s[24], s[25], s[26], s[27], s[28], s[29], s[30], s[31], s[32], s[33], s[34], s[35], s[36], s[37], s[38], s[39], s[40], s[41], s[42], s[43], s[44], s[45], s[46], s[47], s[48], s[49], s[50], s[51], s[52], s[53], s[54], s[55], s[56], s[57], s[58], s[59], s[60], s[61], s[62], s[63], s[64], s[65], s[66], s[67], s[68], s[69], s[70], s[71], s[72], s[73], s[74], s[75], s[76], s[77], s[78], s[79], s[80], s[81], s[82], s[83], s[84], s[85], s[86], s[87], s[88], s[89], s[90], s[91], s[92], s[93], s[94], s[95], i);
			mysql_query(mysql_con, sql_buffer);
			memset(sql_buffer, 0, sizeof(sql_buffer));
			printf("%d,", i);
		}
		if (sample_time != 0)
		{
			snprintf(sql_buffer, sizeof(sql_buffer), "SELECT %s FROM control_status WHERE (control_id = '%d')", column, i);
			mysql_query(mysql_con, sql_buffer);
			mysql_result = mysql_store_result(mysql_con);
			mysql_row = mysql_fetch_row(mysql_result);
			for (k = 0; k < sample_time; k++)
			{
				s[k] = atof(mysql_row[k]);
				printf("%.2f  ",s[k]);
			}
			memset(sql_buffer, 0, sizeof(sql_buffer));
			printf("\n");
			for (j = 0; j < (time_block - sample_time); j++)
			{
				s[j + sample_time] = glp_mip_col_val(mip, h);

				if (i <= interrupt_num && j== 0)
				{
					snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE now_status set status = %d where id=%d ", (int)s[j+sample_time], position[i-1]);
					// printf("%s\n",sql_buffer);
					mysql_query(mysql_con, sql_buffer);
				}
				// l = (l + variable);
				h = (h + variable);
			}
			
			snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE control_status set A0 = '%.3f', A1 = '%.3f', A2 = '%.3f', A3 = '%.3f', A4 = '%.3f', A5 = '%.3f', A6 = '%.3f', A7 = '%.3f', A8 = '%.3f', A9 = '%.3f', A10 = '%.3f', A11 = '%.3f', A12 = '%.3f', A13 = '%.3f', A14 = '%.3f', A15 = '%.3f', A16 = '%.3f', A17 = '%.3f', A18 = '%.3f', A19 = '%.3f', A20 = '%.3f', A21 = '%.3f', A22 = '%.3f', A23 = '%.3f', A24 = '%.3f', A25 = '%.3f', A26 = '%.3f', A27 = '%.3f', A28 = '%.3f', A29 = '%.3f', A30 = '%.3f', A31 = '%.3f', A32 = '%.3f', A33 = '%.3f', A34 = '%.3f', A35 = '%.3f', A36 = '%.3f', A37 = '%.3f', A38 = '%.3f', A39 = '%.3f', A40 = '%.3f', A41 = '%.3f', A42 = '%.3f', A43 = '%.3f', A44 = '%.3f', A45 = '%.3f', A46 = '%.3f', A47 = '%.3f', A48 = '%.3f', A49 = '%.3f', A50 = '%.3f', A51 = '%.3f', A52 = '%.3f', A53 = '%.3f', A54 = '%.3f', A55 = '%.3f', A56 = '%.3f', A57 = '%.3f', A58 = '%.3f', A59 = '%.3f', A60 = '%.3f', A61 = '%.3f', A62 = '%.3f', A63 = '%.3f', A64 = '%.3f', A65 = '%.3f', A66 = '%.3f', A67 = '%.3f', A68 = '%.3f', A69 = '%.3f', A70 = '%.3f', A71 = '%.3f', A72 = '%.3f', A73 = '%.3f', A74 = '%.3f', A75 = '%.3f', A76 = '%.3f', A77 = '%.3f', A78 = '%.3f', A79 = '%.3f', A80 = '%.3f', A81 = '%.3f', A82 = '%.3f', A83 = '%.3f', A84 = '%.3f', A85 = '%.3f', A86 = '%.3f', A87 = '%.3f', A88 = '%.3f', A89 = '%.3f', A90 = '%.3f', A91 = '%.3f', A92 = '%.3f', A93 = '%.3f', A94 = '%.3f', A95 = '%.3f' WHERE equip_id = '%d';", s[0], s[1], s[2], s[3], s[4], s[5], s[6], s[7], s[8], s[9], s[10], s[11], s[12], s[13], s[14], s[15], s[16], s[17], s[18], s[19], s[20], s[21], s[22], s[23], s[24], s[25], s[26], s[27], s[28], s[29], s[30], s[31], s[32], s[33], s[34], s[35], s[36], s[37], s[38], s[39], s[40], s[41], s[42], s[43], s[44], s[45], s[46], s[47], s[48], s[49], s[50], s[51], s[52], s[53], s[54], s[55], s[56], s[57], s[58], s[59], s[60], s[61], s[62], s[63], s[64], s[65], s[66], s[67], s[68], s[69], s[70], s[71], s[72], s[73], s[74], s[75], s[76], s[77], s[78], s[79], s[80], s[81], s[82], s[83], s[84], s[85], s[86], s[87], s[88], s[89], s[90], s[91], s[92], s[93], s[94], s[95], i);
			mysql_query(mysql_con, sql_buffer);
			memset(sql_buffer, 0, sizeof(sql_buffer));
			
			for (j = 0; j < sample_time; j++) { s[j] = 0; }

			snprintf(sql_buffer, sizeof(sql_buffer), "INSERT INTO real_status (%s, equip_id) VALUES('%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%d');"
				, column, s[0], s[1], s[2], s[3], s[4], s[5], s[6], s[7], s[8], s[9], s[10], s[11], s[12], s[13], s[14], s[15], s[16], s[17], s[18], s[19], s[20], s[21], s[22], s[23], s[24], s[25], s[26], s[27], s[28], s[29], s[30], s[31], s[32], s[33], s[34], s[35], s[36], s[37], s[38], s[39], s[40], s[41], s[42], s[43], s[44], s[45], s[46], s[47], s[48], s[49], s[50], s[51], s[52], s[53], s[54], s[55], s[56], s[57], s[58], s[59], s[60], s[61], s[62], s[63], s[64], s[65], s[66], s[67], s[68], s[69], s[70], s[71], s[72], s[73], s[74], s[75], s[76], s[77], s[78], s[79], s[80], s[81], s[82], s[83], s[84], s[85], s[86], s[87], s[88], s[89], s[90], s[91], s[92], s[93], s[94], s[95], i);
			mysql_query(mysql_con, sql_buffer);
			memset(sql_buffer, 0, sizeof(sql_buffer));

		}
	}
	glp_delete_prob(mip);

	delete[] ia, ja, ar, s;
	delete[] power1;

	return;
	//end
}

void *new2d(int h, int w, int size)
{
	register int i;
	void **p;

	p = (void**)new char[h * sizeof(void*) + h*w*size];

	for (i = 0; i < h; i++)
	{
		p[i] = ((char *)(p + h)) + i*w*size;
	}
	return p;
}

// MYSQL_ROW fetch_row_value() {

// 	mysql_query(mysql_con, sql_buffer);
// 	mysql_result = mysql_store_result(mysql_con);
// 	mysql_row = mysql_fetch_row(mysql_result);
// 	mysql_free_result(mysql_result);
// 	return mysql_row;
// }

// float turn_value_to_float(int col_num) {
	
// 	mysql_row = fetch_row_value();
// 	float result = atof(mysql_row[col_num]);
// 	return result;
// }

// int turn_value_to_int(int col_num) {

// 	mysql_row = fetch_row_value();
// 	float result = atoi(mysql_row[col_num]);
// 	return result;
// }
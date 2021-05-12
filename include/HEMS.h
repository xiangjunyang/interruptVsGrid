#ifndef HEMS_H
#define HEMS_H
// Comment by ColinWang on 2020/3/15 
// According to KU's data
//typedef struct
//{
//	int *tse;
//	float *Pa;
//}changeable;

extern float step1_bill;        //用於步驟一計算電費
extern float step1_sell;        //用於步驟一計算電費
extern float step1_PESS;	      //用於步驟一計算PESS-值


extern int sample_time;         //第___個取樣時間
extern int time_block;          //總取樣時間 = 96
extern int app_count;           //總負載數量(可中斷負載 + 不可中斷負載+變動型負載) = 15
extern int variable;            //變數數量 = 43
extern int divide;              //每一小時有___個取樣時間 = 4
extern int sa_counter;          //表示sa中未啟動不可中斷負載+變動負載個數
extern int rasa_counter;        //表示ra()>=sa()中未啟動不可中斷負載+變動負載個數


extern float delta_T;

extern int i, j, m, n, h, k;
extern int RT_enable;           //是否啟動及時排程(如果一次排程未做則不啟動)
extern double z;

extern int interrupt_num;       //可中斷負載數量
extern int *interrupt_start;    //可中斷負載排程啟始時刻
extern int *interrupt_end;      //可中斷負載排程中止時刻
extern int *interrupt_ot;	      //可中斷負載預計執行時間
extern int *interrupt_reot;	    //可中斷負載剩餘執行時間
extern float *interrupt_p;	    //可中斷負載固定耗能


extern int uninterrupt_num;     //不可中斷負載數量
extern int *uninterrupt_start;  //不可中斷負載排程啟始時刻
extern int *uninterrupt_end;	  //不可中斷負載排程中止時刻
extern int *uninterrupt_ot;	    //不可中斷負載預計執行時間
extern int *uninterrupt_reot;	  //不可中斷負載剩餘執行時間
extern float *uninterrupt_p;    //不可中斷負載固定耗能
extern int *uninterrupt_flag;   //不可中斷負載是否已開啟旗標

extern int varying_num;         //變動負載數量
extern int *varying_start;      //變動負載排程啟始時刻
extern int *varying_end;        //變動負載排程中止時刻
extern int *varying_ot;         //變動負載預計執行時間
extern int *varying_reot;       //變動負載剩餘執行時間

extern int ponit_num;


extern int *total_block;
extern int **block;
extern float **INT_power;
extern float **UNINT_power;
extern float **power;

extern int *varying_flag;       //變動負載狀態旗標(是否已開啟)

extern float *price;
extern float solar[24];

extern float Cbat;
extern float Vsys;
extern float SOC_ini;
extern float SOC_min;
extern float SOC_max;
extern float SOC_thres;
extern float Pbat_min;
extern float Pbat_max;
extern float Pgrid_max;
extern float Psell_max;
extern float Pfc_max;
extern float Delta_battery;

extern char column[400] ;



void GLPK(int *, int *, int *, int *, float *, int *, int *, int *, int *, float *, int *, int *, int *, int *, int *, int *, int **, float **, int, float *, int *);
//void GLPK_1(int *, int *, int *, int *, float *, int *, int *, int *, int *, float *, int *, int *, int *, int *, int *, int *, int **, float **, int, float *);
void *new2d(int, int, int);
#define NEW2D(H, W, TYPE) (TYPE **)new2d(H, W, sizeof(TYPE))

#endif 

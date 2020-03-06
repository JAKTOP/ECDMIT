#define byte unsigned char
#include <fstream>
#include <iostream>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



#define SMP_FREQ_250 250
using namespace std;

int *buildECD(char *ecddata, int length);

int *v1 = NULL, *v2 = NULL, *v3 = NULL;

/*
    0   设备名 采集时间 性别 inECD outEDF
 */

int main(int argc, char **argv)
{

    string inFilePath = "D:\\ECG_DAT\\265-11-HC+190500HK000008_20200225080925.ECD";

    ifstream infile;
    infile.open(inFilePath, ios::binary | ios::in);
    if (!infile)
    {
        printf("open file ecd fail[%s]\n", inFilePath);
        return EXIT_FAILURE;
    }

    infile.seekg(0, infile.end);
    int length = infile.tellg();

    infile.seekg(0, infile.beg);

    char *buffer = new char[length];

    infile.read(buffer, length);
    infile.close();
       
    int *val = buildECD(buffer, length);
    int slength = 75000;
    int n = 0;
    FILE *fp=NULL;
    char _sfname[100]="",sfname[]="D:\\ECG_DAT\\";
    strcpy(_sfname,sfname);
    sprintf(_sfname,"%secg-%d.txt",_sfname,n++);       
    fp = fopen(_sfname,"wb+");//打开文件
   
    for(int i=0,j=1;i<length;i++,j++){
        // if(i%slength==0){
        //     char sfname[]="";
        //     sprintf(sfname,"F:\\ECG_DAT\\ecg-%d.txt",n++);       
        //     fp = fopen(sfname,"wb+");//打开文件
        // }

        fprintf(fp,"%d,",val[i]);
        if(i!=0&&i%slength==0){
            if(fp!=NULL){
                fprintf(fp,"\r\n");
                fclose(fp);
            }
            fp=NULL;
            char _sfname[100]="",sfname[]="d:\\ECG_DAT\\";
            strcpy(_sfname,sfname);
            sprintf(_sfname,"%secg-%d.txt",_sfname,n++);       
            fp = fopen(_sfname,"wb+");//打开文件
           
        }
       
    }
    if(fp!=NULL){
        fprintf(fp,"\r\n");
        fclose(fp);
    }
    


    cout << n << endl;

    //for(int i=0;i<128;i++)
    //  cout<<buffer[i]<<" ";

    //v1 = (int *)malloc(length * sizeof(int));
    //v2 = (int *)malloc(length * sizeof(int));
    //v3 = (int *)malloc(length * sizeof(int));

    // int *val = buildECD(buffer, length);

    // for(int i=0;i<length;i++){
    //     int v=val[i];
    //     cout<<v<<endl;
    //  }

    system("pause");
    return EXIT_SUCCESS;
}

int *buildECD(char *ecddata, int length)
{
    int *v2 = new int[length];
    int Ch1b[4] = {0, 0, 0, 0};
    int Ch2b[4] = {0, 0, 0, 0};

    int Ch1d[4] = {0, 0, 0, 0};

    int Ch2d[4] = {0, 0, 0, 0};

    int ecg_wk1, abs_wk16, abs_wk161, abs_wk162, fg0, mIndex = 0;

    int **values = new int *[length];

    for (int j = 0; j < length; ++j)
    {
        int *E316 = new int[4];
        //************第一通道变换开始**********************************
        int b2c4 = (int)(ecddata[j] & 0xff);
        Ch1b[3] = (int)(b2c4 / 16);
        Ch2b[3] = (int)(b2c4 & 0x0F);
        ecg_wk1 = Ch1b[1] & 15;
        fg0 = 0;
        if (Ch1b[1] < 16)
        { //需要计算
            if (ecg_wk1 < 8)
            { //f4
                if (ecg_wk1 < 4)
                { // f4+
                    Ch1d[1] = Ch1d[0] + ecg_wk1;
                }
                else
                { //f4-
                    Ch1d[1] = Ch1d[0] - 8 + ecg_wk1;
                }
            }
            else
            {
                if (ecg_wk1 < 0x0C)
                { // f8
                    abs_wk16 = Ch1b[1] & 1;
                    abs_wk16 <<= 4;
                    abs_wk161 = Ch1b[2] & 0x0F;
                    abs_wk16 += abs_wk161; // 得到增量值
                    if (ecg_wk1 > 0x09)
                    {
                        abs_wk16 = abs_wk16 - 32; //f8-
                    }
                    abs_wk161 = abs_wk16 / 2; // 得到1/2增量值
                    Ch1d[2] = Ch1d[0] + abs_wk16;
                    Ch1d[1] = Ch1d[0] + abs_wk161;
                    Ch1b[2] += 16;
                }
                else
                {                           //f12
                    abs_wk16 = Ch1b[1] & 3; //Ch1b[1]高9,10位
                    abs_wk16 <<= 8;
                    abs_wk161 = Ch1b[2] & 0x0F;
                    abs_wk161 <<= 4;
                    abs_wk162 = abs_wk16 + abs_wk161 + Ch1b[3];
                    if (abs_wk162 < 1001)
                    { // 正常ECG 数据
                        abs_wk16 = abs_wk162 - Ch1d[0];
                        abs_wk161 = abs_wk16 / 3;
                        Ch1d[1] = Ch1d[0] + abs_wk161;
                        Ch1d[2] = Ch1d[1] + abs_wk161;
                        Ch1d[3] = abs_wk162;

                        Ch1b[2] += 16;
                        Ch1b[3] += 16;
                    }
                    else
                    {
                        fg0 = abs_wk162;
                        if ((abs_wk162 == 1001) || (abs_wk162 == 1002))
                        {
                            if ((Ch2b[1] == 0) && (Ch2b[2] == 0) && (Ch2b[3] == 0))
                            {
                                Ch1d[1] = abs_wk162;
                            }
                            else
                            {
                                Ch1d[1] = Ch1d[0];
                            }
                        }
                        else if (abs_wk162 == 1010)
                        {
                            Ch1d[1] = abs_wk162;
                        }
                        else
                        {
                            Ch1d[1] = Ch1d[0];
                        }
                        Ch1d[2] = Ch1d[0];
                        Ch1d[3] = Ch1d[0];
                    }
                    Ch1b[2] += 16;
                    Ch1b[3] += 16;
                }
            }
        }
        //************第一通道变换结束***********************************

        //************第二通道变换开始***********************************
        ecg_wk1 = Ch2b[1] & 0x0F;
        if (fg0 == 0)
        { // 正常数据
            if (Ch2b[1] < 16)
            { //需要计算
                if (ecg_wk1 < 8)
                { //f4
                    if (ecg_wk1 < 4)
                    { // f4+
                        Ch2d[1] = Ch2d[0] + ecg_wk1;
                    }
                    else
                    { //f4-
                        Ch2d[1] = Ch2d[0] - 8 + ecg_wk1;
                    }
                }
                else
                {
                    if (ecg_wk1 < 0x0C)
                    { // f8
                        abs_wk16 = Ch2b[1] & 1;
                        abs_wk16 <<= 4;
                        abs_wk161 = Ch2b[2] & 0x0F;
                        abs_wk16 += abs_wk161; // 得到增量值
                        if (ecg_wk1 > 0x09)
                        {
                            abs_wk16 = abs_wk16 - 32; //f8-
                        }
                        abs_wk161 = abs_wk16 / 2; // 得到1/2增量值
                        Ch2d[2] = Ch2d[0] + abs_wk16;
                        Ch2d[1] = Ch2d[0] + abs_wk161;
                        Ch2b[2] += 16;
                    }
                    else
                    { //f12
                        abs_wk16 = Ch2b[1] & 3;
                        abs_wk16 <<= 8;
                        abs_wk161 = Ch2b[2] & 0x0F;
                        abs_wk161 <<= 4;
                        abs_wk162 = abs_wk16 + abs_wk161 + Ch2b[3];
                        Ch2d[3] = abs_wk162;
                        abs_wk16 = abs_wk162 - Ch2d[0];
                        abs_wk161 = abs_wk16 / 3;
                        Ch2d[1] = Ch2d[0] + abs_wk161;
                        Ch2d[2] = Ch2d[1] + abs_wk161;
                        Ch2b[2] += 16;
                        Ch2b[3] += 16;
                    }
                }
            }
        }
        else
        { // 特殊数据
            if (fg0 != 1010)
            {
                Ch2d[1] = Ch2d[0];
            }
            else if ((Ch2b[1] & 3) == 0)
            {
                abs_wk161 = Ch2b[2] & 0x0F;
                abs_wk161 <<= 4;
                abs_wk162 = abs_wk161 + Ch2b[3];
                Ch2d[1] = abs_wk162;
            }
            else
                Ch2d[1] = Ch2d[0];
            Ch2d[2] = Ch2d[0];
            Ch2d[3] = Ch2d[0];
            Ch2b[2] += 16;
            Ch2b[3] += 16;
        }
        Ch1b[0] = Ch1b[1];
        Ch1b[1] = Ch1b[2];
        Ch1b[2] = Ch1b[3];

        Ch1d[0] = Ch1d[1];
        Ch1d[1] = Ch1d[2];
        Ch1d[2] = Ch1d[3];

        Ch2b[0] = Ch2b[1];
        Ch2b[1] = Ch2b[2];
        Ch2b[2] = Ch2b[3];

        Ch2d[0] = Ch2d[1];
        Ch2d[1] = Ch2d[2];
        Ch2d[2] = Ch2d[3];

        if (Ch1d[0] > 1000)
        {
            E316[0] = 0;
            E316[1] = Ch1d[1];
            E316[2] = Ch2d[1];
            E316[3] = E316[2] - E316[1] + 500;
            if (Ch1d[0] < 1003)
            {
                E316[0] = Ch1d[0];
            }
            else
            {
                if (Ch1d[0] == 1010)
                {
                    E316[4] = ((Ch2d[0] - 139) * 100 / 31);
                }
            }
        }
        else
        {

            E316[0] = 0;
            E316[1] = Ch1d[0];
            E316[2] = Ch2d[0];
            E316[3] = E316[2] - E316[1] + 500;
        }

        v2[j] = E316[2] > 1000 ? 1000 : E316[2];
    }
    // out.close();
    return v2;
}
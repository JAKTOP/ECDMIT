#include "ecg.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

// Serial input buffer
byte buffer[64];

byte next_in = 0;
byte next_out = 0;

#define bkbhit (next_in != next_out)

// Time interval constants.

// #define MS80 20
// #define MS95 24
// #define MS150 38
// #define MS200 50
// #define MS360 90
// #define MS450 112
// #define MS1000 250
// #define MS1500 375

#define MS80 16
#define MS95 19
#define MS150 30
#define MS200 40
#define MS360 72
#define MS450 90
#define MS1000 200
#define MS1500 300

#define WINDOW_WIDTH MS80
#define FILTER_DELAY 21 + MS200

/***********************************************
 Main function.
***********************************************/

// Global values for QRS detector.

int16_t Q0 = 0, Q1 = 0, Q2 = 0, Q3 = 0, Q4 = 0, Q5 = 0, Q6 = 0, Q7 = 0;
int16_t N0 = 0, N1 = 0, N2 = 0, N3 = 0, N4 = 0, N5 = 0, N6 = 0, N7 = 0;
int16_t RR0 = 0, RR1 = 0, RR2 = 0, RR3 = 0, RR4 = 0, RR5 = 0, RR6 = 0, RR7 = 0;
int16_t QSum = 0, NSum = 0, RRSum = 0;
int16_t det_thresh, sbcount;

int16_t tempQSum, tempNSum, tempRRSum;

int16_t QN0 = 0, QN1 = 0;
int Reg0 = 0;
#define THREAD_NUMBER 2
FILE *fp = NULL;

static int ii = 1;

int main(void)
{

    PICQRSDet(0, 1);

    char c;
    int16_t x;
    char filename[] = "F:\\MATLAB\\TEMP-MIT-BIH\\100_200.txt"; //每行最大读取的字符数
    if ((fp = fopen(filename, "r")) == NULL)                   //判断文件是否存在及可读
    {
        printf("error!");
        return -1;
    }
    char StrLine[16];
    int16_t lTemp = 0, delay = 0;
    int16_t index = 0;

    int16_t SampleCount = 0;
    int16_t mSampleCount = 0;
    long DetectionTime;

    char atrName[20];
    sprintf(atrName, "%d_t.txt", "T100");

    FILE *tAtrFile = fopen(atrName, "a");
    fprintf(tAtrFile, "%s\t%s\t%s\n", "Position", "Time", "Type");

    while (!feof(fp))
    {
        ++SampleCount;

        int16_t c;
        if (index % 10 == 0)
        {
            c = 0x55;
        }
        else
        {
            fgets(StrLine, 1024, fp); //读取一行
            c = atoi(StrLine);
        }

        if (SyncRx(c, &x) != 0)
        {

            delay = PICQRSDet(x, 0);

            if (delay != 0)
            {

                DetectionTime = SampleCount - delay;

                // Convert sample count to input file sample
                // rate.

                DetectionTime *= 200;
                DetectionTime /= 200;

                double rr = (SampleCount - mSampleCount) / 200.0;
                mSampleCount = SampleCount;

                fprintf(tAtrFile, "%ld\t%d\t%.3f\n", SampleCount, DetectionTime, rr);
            }
        }
        index++;
    }
    fclose(tAtrFile);
    fclose(fp); //关闭文件
    system("pause");
    return 0;
}

// int NextSample(int *vout, int nosig, int ifreq,
// 			   int ofreq, int init)
// {
// 	int i;
// 	static int m, n, mn, ot, it, vv[32], v[32], rval;

// 	if (init)
// 	{
// 		i = gcd(ifreq, ofreq);
// 		m = ifreq / i;
// 		n = ofreq / i;
// 		mn = m * n;
// 		ot = it = 0;
// 		getvec(vv);
// 		rval = getvec(v);

// 	}

// 	else
// 	{
// 		while (ot > it)
// 		{
// 			for (i = 0; i < nosig; ++i)
// 				vv[i] = v[i];
// 			rval = getvec(v);
// 			if (it > mn)
// 			{
// 				it -= mn;
// 				ot -= mn;
// 			}
// 			it += n;
// 		}
// 		for (i = 0; i < nosig; ++i)
// 			vout[i] = vv[i] + (ot % n) * (v[i] - vv[i]) / n;
// 		ot += m;
// 	}

// 	return (rval);
// }

// Greatest common divisor of x and y (Euclid's algorithm)

int gcd(int x, int y)
{
    while (x != y)
    {
        if (x > y)
            x -= y;
        else
            y -= x;
    }
    return (x);
}

/***************************************************************************
* SyncRx handles communication synchronization of input data recieved
* over the RS232 connection.  SyncRx expects data to be transmitted in
* 21 byte frames composed of a synchronization character followed by 10
* 16-bit data values.  Data values are transmitted MSB first followed by
* the LSB.
*
* SyncRx returns a 1 to indicate that a datum has been received. The
* data value is returned in *out.
****************************************************************************/

#define SYNC_CHAR 0x55
#define RESET_CHAR 0xAA
#define FRAME_LGTH 10

int SyncRx(int16_t in, int16_t *out)
{
    static int SyncState = 0, frameCount = 0;
    static int16_t Datum1;
    int16_t temp;

    // Wait for a sync character.

    if (SyncState == 0)
    {

        // If a sync character is detected,
        // start receiving a frame.

        if (in == SYNC_CHAR)
        {
            SyncState = 1;
            frameCount = 0;
        }

        // If a reset character is detected,
        // reset the QRS detector, and start
        // receiving a frame.

        else if (in == RESET_CHAR)
        {
            SyncState = 1;
            frameCount = 0;
            PICQRSDet(0, 1);
        }

        return (0);
    }
    else
    {
        if (++frameCount == FRAME_LGTH)
            SyncState = 0;
        else
            SyncState = 1;
        *out = in;
        return (1);
    }

    return (0);
}

/**********************************************************************
* SendInt transmitts a 16-bit integer, MSB first followed by the LSB.
***********************************************************************/

void SendInt(int16_t x)
{
    int8_t c;
    c = (x >> 8) & 0xFF;
    putc(c, fp);
    c = x & 0xFF;
    putc(c, fp);
}

/************************************************************************
* bgetc() (buffered getc()) returns the next value in the serial input FIFO.
*************************************************************************/

char bgetc()
{
    char c;

    c = buffer[next_out];
    next_out = (next_out + 1) & 0x3F;
    return (c);
}

/******************************************************************************
*  PICQRSDet takes 16-bit ECG samples (5 uV/LSB) as input and returns the
*  detection delay when a QRS is detected.  Passing a nonzero value for init
*  resets the QRS detector.
******************************************************************************/

int16_t PICQRSDet(int16_t x, int init)
{
    static int16_t tempPeak, initMax;
    static int16_t preBlankCnt = 0, qpkcnt = 0, initBlank = 0;
    static int16_t count, sbpeak, sbloc;
    int16_t QrsDelay = 0;
    int16_t temp0, temp1;

    if (init)
    {
        hpfilt(0, 1);
        lpfilt(0, 1);
        deriv1(0, 1);
        mvwint(0, 1);
        Peak(0, 1);
        qpkcnt = count = sbpeak = 0;
        QSum = NSum = 0;

        RRSum = MS1000 << 3;
        RR0 = RR1 = RR2 = RR3 = RR4 = RR5 = RR6 = RR7 = MS1000;

        Q0 = Q1 = Q2 = Q3 = Q4 = Q5 = Q6 = Q7 = 0;
        N0 = N1 = N2 = N3 = N4 = N5 = N6 = N7 = 0;
        NSum = 0;

        return (0);
    }

    x = lpfilt(x, 0);
    x = hpfilt(x, 0);
    x = deriv1(x, 0);
    if (x < 0)
        x = -x;
    x = mvwint(x, 0);
    x = Peak(x, 0);

    // Hold any peak that is detected for 200 ms
    // in case a bigger one comes along.  There
    // can only be one QRS complex in any 200 ms window.

    if (!x && !preBlankCnt)
        x = 0;

    else if (!x && preBlankCnt) // If we have held onto a peak for
    {                           // 200 ms pass it on for evaluation.
        if (--preBlankCnt == 0)
            x = tempPeak;
        else
            x = 0;
    }

    else if (x && !preBlankCnt) // If there has been no peak for 200 ms
    {                           // save this one and start counting.
        tempPeak = x;
        preBlankCnt = MS200;
        x = 0;
    }

    else if (x)           // If we were holding a peak, but
    {                     // this ones bigger, save it and
        if (x > tempPeak) // start counting to 200 ms again.
        {
            tempPeak = x;
            preBlankCnt = MS200;
            x = 0;
        }
        else if (--preBlankCnt == 0)
            x = tempPeak;
        else
            x = 0;
    }

    // Initialize the qrs peak buffer with the first eight
    // local maximum peaks detected.

    if (qpkcnt < 8)
    {
        ++count;
        if (x > 0)
            count = WINDOW_WIDTH;
        if (++initBlank == MS1000)
        {
            initBlank = 0;
            UpdateQ(initMax);
            initMax = 0;
            ++qpkcnt;
            if (qpkcnt == 8)
            {

                RRSum = MS1000 << 3;
                RR0 = RR1 = RR2 = RR3 = RR4 = RR5 = RR6 = RR7 = MS1000;

                sbcount = MS1500 + MS150;
            }
        }
        if (x > initMax)
            initMax = x;
    }

    else
    {
        ++count;

        // Check if peak is above detection threshold.

        if (x > det_thresh)
        {
            UpdateQ(x);

            // Update RR Interval estimate and search back limit

            UpdateRR(count - WINDOW_WIDTH);
            count = WINDOW_WIDTH;
            sbpeak = 0;
            QrsDelay = WINDOW_WIDTH + FILTER_DELAY;
        }

        // If a peak is below the detection threshold.

        else if (x != 0)
        {
            UpdateN(x);

            QN1 = QN0;
            QN0 = count;

            if ((x > sbpeak) && ((count - WINDOW_WIDTH) >= MS360))
            {
                sbpeak = x;
                sbloc = count - WINDOW_WIDTH;
            }
        }

        // Test for search back condition.  If a QRS is found in
        // search back update the QRS buffer and det_thresh.

        if ((count > sbcount) && (sbpeak > (det_thresh >> 1)))
        {
            UpdateQ(sbpeak);

            // Update RR Interval estimate and search back limit

            UpdateRR(sbloc);

            QrsDelay = count = count - sbloc;
            QrsDelay += FILTER_DELAY;
            sbpeak = 0;
        }
    }
    return QrsDelay;
}

/**************************************************************************
*  UpdateQ takes a new QRS peak value and updates the QRS mean estimate
*  and detection threshold.
**************************************************************************/

void UpdateQ(int16_t newQ)
{

    QSum -= Q7;
    Q7 = Q6;
    Q6 = Q5;
    Q5 = Q4;
    Q4 = Q3;
    Q3 = Q2;
    Q2 = Q1;
    Q1 = Q0;
    Q0 = newQ;
    QSum += Q0;

    det_thresh = QSum - NSum;
    det_thresh = NSum + (det_thresh >> 1) - (det_thresh >> 3);

    det_thresh >>= 3;
}

/**************************************************************************
*  UpdateN takes a new noise peak value and updates the noise mean estimate
*  and detection threshold.
**************************************************************************/

void UpdateN(int16_t newN)
{
    NSum -= N7;
    N7 = N6;
    N6 = N5;
    N5 = N4;
    N4 = N3;
    N3 = N2;
    N2 = N1;
    N1 = N0;
    N0 = newN;
    NSum += N0;

    det_thresh = QSum - NSum;
    det_thresh = NSum + (det_thresh >> 1) - (det_thresh >> 3);

    det_thresh >>= 3;
}

/**************************************************************************
*  UpdateRR takes a new RR value and updates the RR mean estimate
**************************************************************************/

void UpdateRR(int16_t newRR)
{
    RRSum -= RR7;
    RR7 = RR6;
    RR6 = RR5;
    RR5 = RR4;
    RR4 = RR3;
    RR3 = RR2;
    RR2 = RR1;
    RR1 = RR0;
    RR0 = newRR;
    RRSum += RR0;

    sbcount = RRSum + (RRSum >> 1);
    sbcount >>= 3;
    sbcount += WINDOW_WIDTH;
}

/*************************************************************************
*  lpfilt() implements the digital filter represented by the difference
*  equation:
*
* 	y[n] = 2*y[n-1] - y[n-2] + x[n] - 2*x[n-5] + x[n-10]
*
*	Note that the filter delay is five samples.
*
**************************************************************************/

int16_t lpfilt(int16_t datum, int init)
{
    static int16_t y1 = 0, y2 = 0;
    static int16_t d0, d1, d2, d3, d4, d5, d6, d7, d8, d9;
    int16_t y0;
    int16_t output;

    if (init)
    {
        d0 = d1 = d2 = d3 = d4 = d5 = d6 = d7 = d8 = d9 = 0;
        y1 = y2 = 0;
    }

    y0 = (y1 << 1) - y2 + datum - (d4 << 1) + d9;
    y2 = y1;
    y1 = y0;
    if (y0 >= 0)
        output = y0 >> 5;
    else
        output = (y0 >> 5) | 0xF800;

    d9 = d8;
    d8 = d7;
    d7 = d6;
    d6 = d5;
    d5 = d4;
    d4 = d3;
    d3 = d2;
    d2 = d1;
    d1 = d0;
    d0 = datum;

    return (output);
}

/******************************************************************************
*  hpfilt() implements the high pass filter represented by the following
*  difference equation:
*
*	y[n] = y[n-1] + x[n] - x[n-32]
*	z[n] = x[n-16] - y[n] ;
*
*  Note that the filter delay is 15.5 samples
******************************************************************************/

#define HPBUFFER_LGTH 32

int16_t hpfilt(int16_t datum, int init)
{
    static int16_t y = 0;
    static int16_t data[HPBUFFER_LGTH];
    static int ptr = 0;
    int16_t z;
    int halfPtr;

    if (init)
    {
        for (ptr = 0; ptr < HPBUFFER_LGTH; ++ptr)
            data[ptr] = 0;
        ptr = 0;
        y = 0;
        return (0);
    }

    y += datum - data[ptr];

    halfPtr = ptr - (HPBUFFER_LGTH / 2);
    halfPtr &= 0x1F;

    z = data[halfPtr]; // Compensate for CCS shift bug.
    if (y >= 0)
        z -= (y >> 5);
    else
        z -= (y >> 5) | 0xF800;

    data[ptr] = datum;
    ptr = (ptr + 1) & 0x1F;

    return (z);
}

/*****************************************************************************
*  deriv1 and deriv2 implement derivative approximations represented by
*  the difference equation:
*
*	y[n] = 2*x[n] + x[n-1] - x[n-3] - 2*x[n-4]
*
*  The filter has a delay of 2.
*****************************************************************************/

int16_t deriv1(int16_t x0, int init)
{
    static int16_t x1, x2, x3, x4;
    int16_t output;
    if (init)
        x1 = x2 = x3 = x4 = 0;

    output = x1 - x3;
    if (output < 0)
        output = (output >> 1) | 0x8000; // Compensate for shift bug.
    else
        output >>= 1;

    output += (x0 - x4);
    if (output < 0)
        output = (output >> 1) | 0x8000;
    else
        output >>= 1;

    x4 = x3;
    x3 = x2;
    x2 = x1;
    x1 = x0;
    return (output);
}

/*****************************************************************************
* mvwint() implements a moving window integrator, averaging
* the signal values over the last 16
******************************************************************************/

int16_t mvwint(int16_t datum, int init)
{
    static uint16_t sum = 0;
    static unsigned int d0, d1, d2, d3, d4, d5, d6, d7, d8, d9, d10, d11, d12, d13, d14, d15;

    if (init)
    {
        d0 = d1 = d2 = d3 = d4 = d5 = d6 = d7 = d8 = d9 = d10 = d11 = d12 = d13 = d14 = d15 = 0;
        sum = 0;
    }
    sum -= d15;

    d15 = d14;
    d14 = d13;
    d13 = d12;
    d12 = d11;
    d11 = d10;
    d10 = d9;
    d9 = d8;
    d8 = d7;
    d7 = d6;
    d6 = d5;
    d5 = d4;
    d4 = d3;
    d3 = d2;
    d2 = d1;
    d1 = d0;
    if (datum >= 0x0400)
        d0 = 0x03ff;
    else
        d0 = (datum >> 2);
    sum += d0;

    return (sum >> 2);
}

/**************************************************************
* peak() takes a datum as input and returns a peak height
* when the signal returns to half its peak height, or it has been
* 95 ms since the peak height was detected. 
**************************************************************/

int16_t Peak(int16_t datum, int init)
{
    static int16_t max = 0, lastDatum;
    static int timeSinceMax = 0;
    int16_t pk = 0;

    if (init)
    {
        max = 0;
        timeSinceMax = 0;
        return (0);
    }

    if (timeSinceMax > 0)
        ++timeSinceMax;

    if ((datum > lastDatum) && (datum > max))
    {
        max = datum;
        if (max > 2)
            timeSinceMax = 1;
    }

    else if (datum < (max >> 1))
    {
        pk = max;
        max = 0;
        timeSinceMax = 0;
    }

    else if (timeSinceMax > MS95)
    {
        pk = max;
        max = 0;
        timeSinceMax = 0;
    }
    lastDatum = datum;
    return pk;
}

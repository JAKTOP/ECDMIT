#include <stdint.h>
#include <pthread.h>


char bgetc(void);
int SyncRx(int16_t in,int16_t *out);
void SendInt(int16_t x);
int16_t hpfilt(int16_t datum, int init);
int16_t lpfilt(int16_t datum ,int init);
int16_t deriv1(int16_t x0, int init);
int16_t mvwint(int16_t datum, int init);
int16_t PICQRSDet(int16_t, int init);
int16_t Peak(int16_t datum, int init);
void UpdateQ(int16_t newQ);
void UpdateRR(int16_t newRR);
void UpdateN(int16_t newN);
void buildECD(char *ecddata, int length);

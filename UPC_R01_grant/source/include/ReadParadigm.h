#ifndef __ReadParadigm_Header__
#define __ReadParadigm_Header__
int 
ReadParadigm(ParaType& pendingPara);

int 
ReadDispPara(void);

extern int cursorExpContext[PRACTICE_NUM][EVENT_NUM];
extern int targetExpContext[PRACTICE_NUM][EVENT_NUM];
extern int iniposExpContext[PRACTICE_NUM][EVENT_NUM];


extern int cursorPtbContext[PRACTICE_NUM][EVENT_NUM];
extern int targetPtbContext[PRACTICE_NUM][EVENT_NUM];
extern int iniposPtbContext[PRACTICE_NUM][EVENT_NUM];

#endif
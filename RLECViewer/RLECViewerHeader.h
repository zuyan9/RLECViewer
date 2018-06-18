#ifndef _RLECVIEWERHEADER_H
#define _RLECVIEWERHEADER_H

#pragma warning(disable: 4786)
#pragma warning(disable: 4503)
#include <map>
#include <list>
#include <string>
#include <vector>
#include <deque>
#include <algorithm>

using namespace std;

struct ECData
{
	byte Remote;
	byte Local;
	byte FanDuty;
};

typedef  void( SetFanDuty )(int p1, int p2);		
typedef  int( SetFANDutyAuto )(int p1);				
typedef  ECData( GetTempFanDuty )(int p1);			
typedef  int( GetFANCounter )(void);					
typedef  string( GetECVersion )(void);					
typedef  int( GetCPUFANRPM )(void);				
typedef  int( GetGPUFANRPM )(void);		
typedef  int( GetGPU1FANRPM )(void);			
typedef  int( GetX72FANRPM)(void);				

struct ECData2
{
	byte Remote;
	byte Local;
	byte FanDuty;
	byte Reserve;
};

typedef  BOOL( InitIo )(void);		
typedef  ECData2( GetTempFanDuty2 )(int p1);			
typedef  int( SetFANDutyAuto2 )(int p1);	
typedef  void( SetFanDuty2 )(int p1, int p2);		
			

#endif //
#include <time.h>
#include <stdio.h>
//************************* wx STOP WATCH *********************************
class StopWatch
{
private:
	time_t	start,stop;
  char		*time_str;
public:
	StopWatch(void)	{start=0;stop=0;time_str=new char[30];}
  ~StopWatch(void) {if (time_str) delete [] time_str;}
	void Start(void)	{start=time(NULL);}
	void Stop(void)	{stop=time(NULL);}
	time_t	Elapsed(void) {return (time(NULL)-start);}
	char *ElapsedStr(void)
	{
  	time_t total=Elapsed();
		int secs=total%60;
		int	temp=(total-secs)/60;
		int mins=temp%60;
		int hours=(temp-mins)/60;
		sprintf(time_str,"%02d:%02d:%02d",hours,mins,secs);
		return time_str;
	}
};


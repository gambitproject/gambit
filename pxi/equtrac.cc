#include "equtrac.h"

/**************************** DISTANCE ************************************/
double EquTracker::Distance(const PointNd &first,const PointNd &second)
  /*
   * Note, this calculates the distance between two n-dimentional points
   * X=(x1,x2,...xn) and Y=(y1,y2,...yn) using the formula:
   * sqrt((x1-y2)^+(x2-y2)^2+...(xn-yn)^2)
   */
{
  double	quotent=0.0;
  if (first.Length()!=second.Length()) return LARGE_NUMBER;
  for (int i=1;i<=first.Length();i++)
    quotent+=(first[i]-second[i])*(first[i]-second[i]);
  return (sqrt(quotent));
}

/*************************** CLOSEST **************************************/

int EquTracker::Closest(const DataLine &this_line)
{
  int i,j;
  double temp;
  double least_so_far=LARGE_NUMBER,least_pos=1;
  
  // Find which curve this point is closest to: find the distance to each curve
  for (i=1;i<=equs.Length();i++) {
	// if not, there is no way i is the right equ #
    if (equs[i].NumInfosets()==this_line.NumInfosets()) {
      temp=0.0;
      for (j=1;j<=this_line.NumInfosets();j++)
	temp+=Distance(equs[i][j],this_line[j]);
      if (temp<least_so_far) {least_so_far=temp;least_pos=i;}
    }
  }
  if (least_so_far<merror)
    return (int)least_pos;
  else
    return 0;
}

/**************************** CHECK EQU *********************************/
int EquTracker::Check_Equ(const DataLine &dl,int *new_equ,DataLine *prev_point)
{
  /*----------------- ROW: check for known equilibria and color ----------*/
  /* Find the closest curve to the point, if distance is less than merror
   * assign the point to that curve, update the curve's position
   */
  /*-------------------find closest curve---------------*/
  int equ_num=0;
  int found_new=0;
  if (new_equ)	(*new_equ)=FALSE;
  if ((equ_num=Closest(dl))==0) {
    equs.Append(dl);
    equ_num=equs.Length();
    if (new_equ) (*new_equ)=TRUE;
    found_new=1;
  }
  // If we just found a new equilibrium, return this point as the previous point 
  if (prev_point) (*prev_point)=(found_new) ? dl : equs[equ_num];
  equs[equ_num]=dl;
  return equ_num;
}

EquTracker::EquTracker(void)
{merror=RECT_SIZE;}

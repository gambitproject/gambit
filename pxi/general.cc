#include "general.h"

// Find In Array function--finds an integer in an array of integers, returns index
int FindIntArray(int *array,int num,int what)
{
for (int i=0;i<num;i++) if (array[i]==what) return i;
return -1;
}

// Find String In File -- uses a file pointer to find a string in the file.
// This is useful for quickly getting to the section of the file you need
void FindStringInFile(FILE *fp,const char *s)
{
char fsif_str[200];
int result;
fseek(fp,0L,0);
do                    
{
	result=fscanf(fp,"%s",fsif_str);
} while (strcmp(fsif_str,s)!=0 && result!=0 && result!=EOF);
}
// Find Char In File -- uses a file pointer to find a char in the file.
// This is useful for quickly getting to the section of the file you need
void FindCharInFile(FILE *fp,char c,int rewind)
{
int result;
char fcif_chr;
if (rewind) fseek(fp,0L,0);
do
{
	result=fscanf(fp,"%c",&fcif_chr);
} while (fcif_chr!=c && result!=0 && result!=EOF);
}



// Virtual keycodes
#define WXK_START           300

#define WXK_LBUTTON	    (WXK_START + 1)
#define WXK_RBUTTON	    (WXK_START + 2)
#define WXK_CANCEL	    (WXK_START + 3)
#define WXK_MBUTTON          (WXK_START + 4)
#define WXK_BACK 	    104
#define WXK_TAB		    9
#define WXK_CLEAR	    (WXK_START + 5)
#define WXK_RETURN	    13
#define WXK_SHIFT	    (WXK_START + 6)
#define WXK_CONTROL	    (WXK_START + 7)
#define WXK_MENU 	    (WXK_START + 8)
#define WXK_PAUSE	    (WXK_START + 9)
#define WXK_CAPITAL	    (WXK_START + 10)
#define WXK_ESCAPE	    27
#define WXK_SPACE	    32
#define WXK_PRIOR	    (WXK_START + 11)
#define WXK_NEXT 	    (WXK_START + 12)
#define WXK_END		    (WXK_START + 13)
#define WXK_HOME 	    (WXK_START + 14)
#define WXK_LEFT 	    (WXK_START + 15)
#define WXK_UP		    (WXK_START + 16)
#define WXK_RIGHT	    (WXK_START + 17)
#define WXK_DOWN 	    (WXK_START + 18)
#define WXK_SELECT	    (WXK_START + 20)
#define WXK_PRINT	    (WXK_START + 21)
#define WXK_EXECUTE	    (WXK_START + 22)
#define WXK_SNAPSHOT	    (WXK_START + 23)
#define WXK_INSERT	    (WXK_START + 24)
#define WXK_DELETE	    127
#define WXK_HELP 	    (WXK_START + 25)
#define WXK_NUMPAD0	    (WXK_START + 26)
#define WXK_NUMPAD1	    (WXK_START + 27)
#define WXK_NUMPAD2	    (WXK_START + 28)
#define WXK_NUMPAD3	    (WXK_START + 29)
#define WXK_NUMPAD4	    (WXK_START + 30)
#define WXK_NUMPAD5	    (WXK_START + 31)
#define WXK_NUMPAD6	    (WXK_START + 32)
#define WXK_NUMPAD7	    (WXK_START + 33)
#define WXK_NUMPAD8	    (WXK_START + 34)
#define WXK_NUMPAD9	    (WXK_START + 35)
#define WXK_MULTIPLY	    (WXK_START + 36)
#define WXK_ADD		    (WXK_START + 37)
#define WXK_SEPARATOR	    (WXK_START + 38)
#define WXK_SUBTRACT	    (WXK_START + 39)
#define WXK_DECIMAL	    (WXK_START + 40)
#define WXK_DIVIDE	    (WXK_START + 41)
#define WXK_F1		    (WXK_START + 42)
#define WXK_F2		    (WXK_START + 43)
#define WXK_F3		    (WXK_START + 44)
#define WXK_F4		    (WXK_START + 45)
#define WXK_F5		    (WXK_START + 46)
#define WXK_F6		    (WXK_START + 47)
#define WXK_F7		    (WXK_START + 48)
#define WXK_F8		    (WXK_START + 49)
#define WXK_F9		    (WXK_START + 50)
#define WXK_F10		    (WXK_START + 51)
#define WXK_F11		    (WXK_START + 52)
#define WXK_F12		    (WXK_START + 53)
#define WXK_F13		    (WXK_START + 54)
#define WXK_F14		    (WXK_START + 55)
#define WXK_F15		    (WXK_START + 56)
#define WXK_F16		    (WXK_START + 57)
#define WXK_F17		    (WXK_START + 58)
#define WXK_F18		    (WXK_START + 59)
#define WXK_F19		    (WXK_START + 60)
#define WXK_F20		    (WXK_START + 61)
#define WXK_F21		    (WXK_START + 62)
#define WXK_F22		    (WXK_START + 63)
#define WXK_F23		    (WXK_START + 64)
#define WXK_F24		    (WXK_START + 65)
#define WXK_NUMLOCK	    (WXK_START + 66)
#define WXK_SCROLL           (WXK_START + 67)

Bool	IsNumeric(int ch)
{
return (ch=='0' || ch=='1' || ch=='2' || ch=='3' || ch=='4' || ch=='5' ||
				ch=='6' || ch=='7' || ch=='8' || ch=='9' || ch=='+' || ch=='-' ||
				ch=='.');
}

Bool	IsAlphaNum(int ch)
{
return !(IsCursor(ch) || IsDelete(ch));
}

Bool	IsCursor(int ch)
{
return	 (ch==WXK_UP || ch==WXK_DOWN || ch==WXK_LEFT || ch==WXK_RIGHT ||
					ch==WXK_TAB || ch==WXK_RETURN);
}

Bool	IsDelete(int ch)
{
return (ch==WXK_DELETE || ch==WXK_BACK);
} 

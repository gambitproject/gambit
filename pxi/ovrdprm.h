#include "wxmisc.h"

void ExpDataDialog:public MyDialogBox
{
private:
	char *likename;
	int save_like;
public:
	ExpDataDialog(void);
	char *LikeName(void);
};

ExpDataDialog::ExpDataDialog(void):MyDialogBox(0,"Likelihood")
{
likename=new char[250];strcpy(likename,"like.out");
save_like=FALSE;
Form()->Add(wxMakeFormBool("Save Like",&save_like));
Form()->Add(wxMakeFormString("LikeFile",*likename));
Go();
}

char *ExpDataDialog::LikeName(void)
{
return (save_like) ? likename : 0;
}

ExpDataDialog::~ExpDataDialog(void)
{
delete [] likename;
}


#ifndef NCALCSET_H
#define NCALCSET_H

class NormalCalcSettings;
typedef struct	{
  NormalCalcSettings *dl;
  wxTextCtrl *dim_item;
  wxTextCtrl *l_start_item;
  wxTextCtrl *l_stop_item;
  wxTextCtrl *l_step_item;
  wxTextCtrl *p_step_item;
  wxTextCtrl *m_error_item;
  wxRadioBox *data_type_item;
} enter_game_struct;

typedef struct {float row;float col;} egf_payoff_struct;
class NormalCalcSettings
{
private:
  float l_start,l_stop,dl,dp,m_error;
  int dim;
  int data_type;
  NormalMatrix *matrix;
  char *file_name;
  int completed;
  Bool one_dot;
public:
  NormalCalcSettings(void);
  ~NormalCalcSettings(void);
  // Set* functions
  void SetMatrix(egf_payoff_struct **array,int dim);
  void SetMatrix(char *file_name);
  void SetMatrix(FILE *fp);
  // Get* functions
  double GetLStart(void) {return l_start;}
  double GetLStop(void) {return l_stop;}
  double GetLStep(void) {return dl;}
  double GetPStep(void) {return dp;}
  double GetMError(void) {return m_error;}
  int GetDim(void) {return dim;}
  int GetDataType(void) {return data_type;}
  char *GetFileName(void) {return file_name;}
  Bool GotMatrix(void) {return !(matrix==NULL);}
  Bool OneDot(void) {return one_dot;}
  int Completed(void) {return completed;}
  NormalMatrix *GetMatrix(void) {return matrix;}
};

void enter_game_func(wxButton &ob,wxCommandEvent &ev);
void load_game_func(wxButton &ob,wxCommandEvent &ev);
void save_game_func(wxButton &ob,wxCommandEvent &ev);

#endif // NCALCSET_H

//
// FILE: command.h -- Declaration of abstract GCL command-line class
//
// $Id$
//

#ifndef COMMAND_H
#define COMMAND_H

namespace GCL {

class CommandLine : public gInput {
private:
  int m_historyDepth;

protected:
  virtual char GetNextChar(void) = 0;

public:
  CommandLine(int p_historyDepth) : m_historyDepth(p_historyDepth) { }
  virtual ~CommandLine() { }

  int HistoryDepth(void) const { return m_historyDepth; }

  virtual void SetPrompt(bool) = 0;
};

}  // namespace GCL

#endif  // COMMAND_H


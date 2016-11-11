
#include <WString.h>

enum fanEnum { fanOff, fanLow, fanHigh };

class Relays {
public:
  Relays();
  bool getDehumidistat();
  String getDehumidistatStr();
  void setDehumidistat(bool value);
  fanEnum getFan();
  String getFanStr();
  void setFan(fanEnum value);
private:
  fanEnum mFan;
  bool mDehumidistat;
  void update();
};

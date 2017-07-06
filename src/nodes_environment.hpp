
#include <WString.h>
#include <Homie.h>

// How long data is considered "not too stale". If last data was
// received over 2 hours ago, we don't have data anymore.
static const unsigned long DATA_LIFETIME_MS  = 2 * 60 * 60 * 1000;

// EMA smoothing. 0.7 provides reasonable smoothing over several hours.
static const float EMA_ALPHA = 0.7;

class Environment {
public:
  Environment(char* envType) :
   mSampleTemp(NAN),
   mEmaTemp(NAN),
   mSampleHumi(NAN),
   mEmaHumi(NAN),
   mTimestampTemp(0),
   mTimestampHumi(0),
   mType(envType),
   mNode(envType, "hvac")
   {};
  void setup();
  void publishAll();
  bool isDataAvailable();
  // Get the current data
  float temperature();
  float absHumidity();
  float relHumidity();
private:
  float mSampleTemp, mEmaTemp, mSampleHumi, mEmaHumi;
  unsigned long mTimestampTemp, mTimestampHumi;
  char* mType;
  HomieNode mNode;
  float calcAbsoluteHumidity(float temperature, float relative_humidity);
  float exponentialMovingAverageIrregular(float alpha, float sample,
    float prevSample, float deltaTime, float emaPrev);
  void applyLowPassFilter(unsigned long& timestamp,
    float& oldSample, float& ema, float newSample);

};

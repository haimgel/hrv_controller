
extern float env_ext_temp;
extern float env_ext_humi;
extern float env_int_temp;
extern float env_int_humi;

static const unsigned long DATA_LIFETIME_MS  = 2 * 60 * 60 * 1000;

void envNodesSetup();
void envNodesPublishAll();
void envNodesExpire();
bool isEnvironmentalDataCurrent();


class Environment {
public:
  Environment(String envType);
  void setup();
  void publishAll();
  void expire();
  bool isDataAvailable();
  // Get the current data
  float temperature();
  float absHumidity();
  float relHumidity();
private:
  float absoluteHumidity(float temperature, float relative_humidity);
}

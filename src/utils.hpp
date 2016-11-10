
bool str2bool(String value);
String bool2str(bool value);
bool debugLevelAbove(int lvl);
void debugPrint(int lvl, String str);

template <typename T> int sgn(T val) {
    return (T(0) < val) - (val < T(0));
}

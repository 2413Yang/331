
#ifndef PROGRESSBAR__H__
#define PROGRESSBAR__H__
#ifndef WIN32

#include <vector>
#include <utility>
#include <string>

class ProgressBar
{
private:
    /* data */
public:
    ProgressBar(float sca, float step);
    ~ProgressBar();
    bool IsEqual();
    std::pair<float, std::string> Value(float, int strLen = 3);
    float ProgressValue(void);
    void operator()(float);
    void cfgBarTxt(std::pair<float, float> bar, std::pair<float, float> txt);

protected:
    unsigned int getOffset(const float &targ);

private:
    float currentValue, targetValue;
    std::vector<std::pair<float, float>> mBar;
    std::vector<std::pair<float, float>> mTxt;
    const float fScale, fStepValue;
};
#endif
#endif /*PROGRESSBAR__H__*/

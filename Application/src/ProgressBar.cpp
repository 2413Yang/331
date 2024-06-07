
#ifndef WIN32

#include "ProgressBar.h"
#include <math.h>

ProgressBar::ProgressBar(float sca, float step)
    : fScale(sca), fStepValue(step)
{
    currentValue = 0.f, targetValue = 0.f;
}

ProgressBar::~ProgressBar()
{
}

bool ProgressBar::IsEqual()
{
    if (fabsf(targetValue - currentValue) < fStepValue)
        return true;
    else
        return false;
}

void ProgressBar::cfgBarTxt(std::pair<float, float> bar, std::pair<float, float> txt)
{
    mBar.push_back(bar);
    mTxt.push_back(txt);
}

float ProgressBar::ProgressValue(void)
{
    if (targetValue > currentValue)
        currentValue += fStepValue;
    else if (targetValue < currentValue)
        currentValue -= fStepValue;

    if (IsEqual())
        currentValue = targetValue;

    return currentValue;
}

std::pair<float, std::string> ProgressBar::Value(float targ, int prec)
{

    std::string Str;
    std::pair<float, std::string> ret;
    switch (prec)
    {
    case 0:
        Str = std::to_string((int)targ);
        break;
    case 1:
        Str = std::to_string((int)targ) + "." + std::to_string((int)(targ * 10) % 10);
        break;
    }
    ret.second = Str;

    unsigned int tmpOffset = getOffset(targ);
    float tmp = mBar[tmpOffset].second - mBar[tmpOffset].first;
    ret.first = ((tmp / fScale) * (targ - mTxt[tmpOffset].first)) + mBar[tmpOffset].first;

    if (ret.first > mBar[mBar.size() - 1].second)
        ret.first = mBar[mBar.size() - 1].second;

    if (ret.first < mBar[0].first)
        ret.first = mBar[0].first;
    // printf("offset: %d \n", offset);

    return ret;
}

unsigned int ProgressBar::getOffset(const float &targ)
{
    unsigned int tmpOffset = 0;

    for (unsigned i = 0; i < mTxt.size(); i++)
    {
        if (targ < mTxt[i].second && targ >= mTxt[i].first)
            tmpOffset = i;
    }

    if (targ >= mTxt[mTxt.size() - 1].second)
        tmpOffset = mTxt.size() - 1;
    else if (targ <= mTxt[0].first)
        tmpOffset = 0;

    return tmpOffset;
}

void ProgressBar::operator()(float value)
{
    targetValue = value;
}

#endif
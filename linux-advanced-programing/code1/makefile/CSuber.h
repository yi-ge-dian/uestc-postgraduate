#pragma once

class CSuber{
public:
    CSuber();
    CSuber(int LeftPod, int RightPod);
    virtual ~CSuber();
public:
    virtual int Sub(int LeftPod, int RightPod);
    virtual int Sub();
private:
    int m_LeftPod;
    int m_RightPod;
};
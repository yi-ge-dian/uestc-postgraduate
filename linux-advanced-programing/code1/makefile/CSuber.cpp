#include "CSuber.h"

CSuber::CSuber(){
    m_LeftPod = 0;
    m_RightPod = 0;
}
CSuber::CSuber(int LeftPod, int RightPod){
    m_LeftPod = LeftPod;
    m_RightPod = RightPod;
}
CSuber::~CSuber(){
}
int CSuber::Sub(int LeftPod, int RightPod){
    return LeftPod - RightPod;
}
int CSuber::Sub(){
    return m_LeftPod - m_RightPod;
}

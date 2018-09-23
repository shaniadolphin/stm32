

#include "Msg.h"


unsigned int Msg[(MSG_ID_MAX / 32)+ 1] = {0};

void SetMsg(MSG_ID MsgId)
{
    Msg[MsgId / 32] |= 1 << (MsgId % 32);
}

unsigned char GetMsg(MSG_ID MsgId)
{
    if (Msg[MsgId / 32] & (1 << (MsgId % 32))) 
    {     
        Msg[MsgId / 32] &= ~(1 << (MsgId % 32));
        
        return(1);
    }
    return(0);
}

void ClearMsg(MSG_ID MsgId)
{
    Msg[MsgId / 32] &= ~(1 << (MsgId % 32));
}

unsigned char CheckMsg(MSG_ID MsgId)
{
    if (Msg[MsgId / 32] & (1 << (MsgId % 32))) 
    {     
        return(1);
    }
    return(0);
}


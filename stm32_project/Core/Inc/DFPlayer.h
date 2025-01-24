#ifndef __DFPlayer
#define __DFPlayer


#include "main.h"

#define     DF_UART                &huart3
#define     Source                 0x02  // TF CARD


# define Start_Byte                  0x7E
# define End_Byte                    0xEF
# define Version                     0xFF
# define Cmd_Len                     0x06
# define Feedback                    0x00        //If need for Feedback: 0x01,  No Feedback: 0


extern int audio_num;
extern int audio_state;
extern int audio_flag;

void Send_cmd (uint8_t cmd, uint8_t Parameter1, uint8_t Parameter2);
void DF_Init (uint8_t volume);
void Query_Status(void);
void Sound_Track(uint8_t nums);
void checkState_DF();
#endif

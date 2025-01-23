#include  "DFPlayer.h"
#include <stdio.h>

#define SOURCE_TF_CARD         0x02

int flag1,flag2,i;
int audio_num=0;
int audio_flag=0;
int audio_state=0;

extern UART_HandleTypeDef huart3;



void Send_cmd (uint8_t cmd, uint8_t Parameter1, uint8_t Parameter2)
{
	uint16_t Checksum = Version + Cmd_Len + cmd + Feedback + Parameter1 + Parameter2;
	Checksum = 0-Checksum;

	uint8_t CmdSequence[10] = { Start_Byte, Version, Cmd_Len, cmd, Feedback, Parameter1, Parameter2, (Checksum>>8)&0x00ff, (Checksum&0x00ff), End_Byte};

	HAL_UART_Transmit(DF_UART, CmdSequence, 10, HAL_MAX_DELAY);
}

void Query_Status(void) {
    // 0x42 명령어 전송
    Send_cmd(0x42, 0x00, 0x00);

    // 응답 수신 버퍼
    uint8_t response[10] = {0};
    if (HAL_UART_Receive(&huart3, response, 10, 100) == HAL_OK) {
        // 응답 처리
        if (response[3] == 0x42) { // 응답이 상태 정보일 때
            uint8_t current_track = response[6]; // 현재 트랙 번호 확인
            if (current_track == 0) {
                audio_state = 0; // 재생 완료 상태
            } else {
                audio_state = 1; // 재생 중 상태
            }
        }
    }
}

void Sound_Track(uint8_t nums){
    switch (nums) {
        case 19:
            Send_cmd(0x03, 0x00, 1); // 001.mp3 실행
            break;
        case 20:
            Send_cmd(0x03, 0x00, 2); // 002.mp3 실행
            break;
        case 21:
            Send_cmd(0x03, 0x00, 3); // 003.mp3 실행
            break;
        case 22:
            Send_cmd(0x03, 0x00, 4); // 004.mp3 실행
            break;
        case 23:
            Send_cmd(0x03, 0x00, 5); // 005.mp3 실행
            break;
        case 24:
            Send_cmd(0x03, 0x00, 6); // 006.mp3 실행
            break;
        case 25:
            Send_cmd(0x03, 0x00, 7); // 007.mp3 실행
            break;
        case 1:
            Send_cmd(0x03, 0x00, 8); // 008.mp3 실행
            break;
        case 2:
            Send_cmd(0x03, 0x00, 9); // 009.mp3 실행
            break;
        case 3:
            Send_cmd(0x03, 0x00, 10); // 017.mp3 실행
            break;
        case 5:
            Send_cmd(0x03, 0x00, 11); // 018.mp3 실행
            break;
        case 6:
            Send_cmd(0x03, 0x00, 12); // 019.mp3 실행
            break;
        case 7:
            Send_cmd(0x03, 0x00, 13); // 020.mp3 실행
            break;
        case 8:
            Send_cmd(0x03, 0x00, 14); // 021.mp3 실행
            break;
        case 9:
            Send_cmd(0x03, 0x00, 15); // 022.mp3 실행
            break;
        case 10:
            Send_cmd(0x03, 0x00, 16); // 023.mp3 실행
            break;
        case 11:
            Send_cmd(0x03, 0x00, 17); // 024.mp3 실행
            break;
        case 43:
            Send_cmd(0x03, 0x00, 18); // 025.mp3 실행
            break;
        case 26:
            Send_cmd(0x03, 0x00, 19); // 026.mp3 실행
            break;
        case 27:
            Send_cmd(0x03, 0x00, 20); // 027.mp3 실행
            break;
        case 28:
            Send_cmd(0x03, 0x00, 21); // 028.mp3 실행
            break;
        case 29:
            Send_cmd(0x03, 0x00, 22); // 029.mp3 실행
            break;
        case 30:
            Send_cmd(0x03, 0x00, 23); // 030.mp3 실행
            break;
        case 31:
            Send_cmd(0x03, 0x00, 24); // 031.mp3 실행
            break;
        case 32:
            Send_cmd(0x03, 0x00, 25); // 032.mp3 실행
            break;
        case 33:
            Send_cmd(0x03, 0x00, 26); // 033.mp3 실행
            break;
        case 34:
            Send_cmd(0x03, 0x00, 27); // 034.mp3 실행
            break;
        case 35:
            Send_cmd(0x03, 0x00, 28); // 035.mp3 실행
            break;
        case 36:
            Send_cmd(0x03, 0x00, 29); // 036.mp3 실행
            break;
        case 37:
            Send_cmd(0x03, 0x00, 30); // 037.mp3 실행
            break;
        case 38:
            Send_cmd(0x03, 0x00, 31); // 038.mp3 실행
            break;
        case 39:
            Send_cmd(0x03, 0x00, 32); // 039.mp3 실행
            break;
        case 40:
            Send_cmd(0x03, 0x00, 33); // 040.mp3 실행
            break;
        case 41:
            Send_cmd(0x03, 0x00, 34); // 041.mp3 실행
            break;
        case 42:
            Send_cmd(0x03, 0x00, 35); // 042.mp3 실행
            break;
        case 44:
            Send_cmd(0x03, 0x00, 36); // 043.mp3 실행
            break;
        case 45:
            Send_cmd(0x03, 0x00, 37); // 044.mp3 실행
            break;
        case 46:
            Send_cmd(0x03, 0x00, 38); // 045.mp3 실행
            break;
        case 47:
            Send_cmd(0x03, 0x00, 39); // 046.mp3 실행
            break;
        case 48:
            Send_cmd(0x03, 0x00, 40); // 047.mp3 실행
            break;
        case 49:
            Send_cmd(0x03, 0x00, 41); // 048.mp3 실행
            break;
        case 50:
            Send_cmd(0x03, 0x00, 42); // 049.mp3 실행
            break;
        default:
            // 기본 동작이 필요한 경우 처리
            break;
    }

	audio_flag=0;
}

//****************************Send_Command************************************//
void DF_Init (uint8_t volume)
{
	Send_cmd(0x3F, 0x00, Source);
	HAL_Delay(200);
	Send_cmd(0x06, 0x00, volume);
	HAL_Delay(500);
}
	//****************************Check_Key Code***************************************//



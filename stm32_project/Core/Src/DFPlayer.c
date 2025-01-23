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
	if (nums==19){
	Send_cmd(0x03, 0x00, 1);}
	if (nums==20){
		Send_cmd(0x03, 0x00, 1);}
	if (nums==21){
		Send_cmd(0x03, 0x00, 3);}
	if (nums==22){
		Send_cmd(0x03, 0x00, 4);}
	if (nums==23){
		Send_cmd(0x03, 0x00, 5);}
	if (nums==24){
		Send_cmd(0x03, 0x00, 6);}
	if (nums==25){
		Send_cmd(0x03, 0x00, 7);}

	audio_flag=0;
}

//****************************Send_Command************************************//
void DF_Init (uint8_t volume)
{
	Send_cmd(0x3F, 0x00, Source);
	//HAL_Delay(200);
	Send_cmd(0x06, 0x00, volume);
	//HAL_Delay(500);
}
	//****************************Check_Key Code***************************************//



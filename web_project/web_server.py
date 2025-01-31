import asyncio
import websockets
import json

g_data = {}
in_data = {}
async def send_periodic_signal(websocket, path):

    try:
        while True:
            
            g_data = get_data() #json파일에 있는거 받아서서
            message = json.dumps(g_data)  # JSON 형식으로 변환
            await websocket.send(message)  # WebSocket을 통해 전송
            print(f"보낸 메시지: {message}")  
            message = await websocket.recv()  # 클라이언트로부터 메시지 받기
            received_data = json.loads(message)  # JSON 형식으로 변환
            in_data.update(received_data)  # 받은 데이터를 갱신
            save_data(in_data)
            await asyncio.sleep(0.5)  # 3초마다 메시지 전송
    except Exception as e:
        print(f"에러 발생: {e}")
        await websocket.send(f"서버 오류: {str(e)}")

async def start_server():
    print("서버 시작 전")
    server = await websockets.serve(send_periodic_signal, "0.0.0.0", 54322)
    print("WebSocket 서버 시작")
    await server.wait_closed()

def get_data():
    with open("datatoout.json", "r") as file:
        data = json.load(file)
        print(data)
    return data

def save_data(data):
    with open("datatoin.json", "w") as file:
        json.dump(data, file, indent=4)  

asyncio.get_event_loop().run_until_complete(start_server())
